/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */

#include "main.h"
#include "math.h"
#include "stdio.h"
#include "stdbool.h"
#include "stdlib.h"
#include "string.h"
#include "kalman.h"
#include "bitmaps.h"
#include "config.h"
#include "wifi.h"
#include "passwords.h"	//contains the wifi credentials (for gitignore purposes)

#include "../../Drivers/BSP/B-L475E-IOT01/stm32l475e_iot01.h"
#include "../../Drivers/BSP/B-L475E-IOT01/stm32l475e_iot01_accelero.h"
#include "../../Drivers/BSP/B-L475E-IOT01/stm32l475e_iot01_tsensor.h"
#include "../../Drivers/BSP/B-L475E-IOT01/stm32l475e_iot01_hsensor.h"
#include "../../Drivers/BSP/B-L475E-IOT01/stm32l475e_iot01_magneto.h"
#include "../../Drivers/BSP/B-L475E-IOT01/stm32l475e_iot01_psensor.h"
#include "../../Drivers/BSP/B-L475E-IOT01/stm32l475e_iot01_gyro.h"

#include "../../Drivers/OLED/ssd1306.h"
#include "../../Drivers/OLED/ssd1306_fonts.h"
#include "../../Drivers/STM32L4xx_HAL_Driver/Inc/stm32l4xx_hal_cortex.h"
#include "../../Drivers/BSP/Components/spirit1/SPIRIT1_Library/Inc/SPIRIT_Types.h"
#include "../../Drivers/BSP/Components/spirit1/SPIRIT1_Library/Inc/SPIRIT_PktBasic.h"
#include "../../Drivers/BSP/Components/spirit1/SPIRIT1_Library/Inc/SPIRIT_Irq.h"

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM3_Init(void);
static void UART1_Init(void);
void IMU_calib(void);
static void MX_I2C1_Init(void);
//static void MX_DMA_Init(void);
void Error_Handler(void);
void ACC_EXTI11_Init(void);

float mod(float x, float y);
void get_imu_readings(void);
void get_pht_readings(void);
void print_readings(int);
void print_text(char msg[]);
void draw_wireframe(void);
int presForFrequency (int frequency);
static void RF_GPIO_Init();
static void RF_SPI3_Init();

UART_HandleTypeDef huart1;
I2C_HandleTypeDef hi2c1;
//DMA_HandleTypeDef hdma_i2c1_tx;
SPI_HandleTypeDef hspi3;
TIM_HandleTypeDef htim3;

Kalman kalman_roll = {0.0f,0.0f,0.0f,{{0.0f,0.0f},{0.0f,0.0f}}};
Kalman kalman_pitch = {0.0f,0.0f,0.0f,{{0.0f,0.0f},{0.0f,0.0f}}};
Kalman kalman_yaw = {0.0f,0.0f,0.0f,{{0.0f,0.0f},{0.0f,0.0f}}};


uint8_t MODE = 0; // 0 = standard, 1 = emergency, 2 = cube
volatile uint32_t first_tick, second_tick;
volatile uint8_t button_counter = 0;
uint8_t input_buf[1];

float temp, humidity, pressure;
float gyro_data[3], accel_data[3], mag_data[3];
float gyro_offset[3] = {0,0,0}, accel_offset[3] = {0,0,0};
const float mag_offset[3]={22.625,-167.95,8.56667};
float yaw_offset = 0;
float rpy[3], kalman_rpy[3];	//roll pitch yaw
float dt = 0.05;

volatile bool uart_recv = false;
volatile bool fall_detect=false;
bool sudden_move_detect=false, posture_detect_first=false, posture_detect_second=false, fever_detect=false;
uint8_t flags_detect = 0, last_flags_detect = 0;
bool start_beep = false, first_beep = true;
uint32_t beep_start_tick, beep_end_tick;
uint32_t sudden_move_tick, fall_tick, posture_first_tick, posture_second_tick, fever_tick, cube_tick;
int step = 0;

//rf
volatile SpiritFlagStatus xTxDoneFlag = S_RESET;
volatile SpiritFlagStatus xRxDoneFlag = S_RESET;
volatile SpiritFlagStatus xRxTimeOutFlag = S_SET;
volatile bool waitflag = 0;
uint8_t spsgrf_D2S_buffer[64];
uint8_t spsgrf_S2D_buffer[64];

//wifi
#define MAX_LENGTH 100
#define WIFI_READ_TIMEOUT 100
#define WIFI_WRITE_TIMEOUT 100

const char* WiFi_SSID = ANDROID_SSID;
const char* WiFi_password = ANDROID_PW;
const WIFI_Ecn_t WiFi_security = WIFI_ECN_WPA2_PSK;
const uint16_t SOURCE_PORT = 1234;
uint8_t ipaddr[4] = ANDROID_IP_ADDR;


const uint16_t DEST_PORT = 2028; // 'server' port number - this is the port Packet Sender listens to

uint8_t req[MAX_LENGTH]; // request packet
uint8_t resp[MAX_LENGTH]; // response packet
uint16_t Datalen;
WIFI_Status_t WiFi_Stat; // WiFi status. Should remain WIFI_STATUS_OK if everything goes well


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == BUTTON_EXTI13_Pin) {
		if (button_counter == 0) {
			first_tick = HAL_GetTick();
			button_counter = 1;
		} else {
			second_tick = HAL_GetTick();
			button_counter = 2;
		}
	}
	// if (GPIO_Pin == LSM6DSL_INT1_EXTI11_Pin) {
	// 	uint8_t reg_step = SENSOR_IO_Read(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_FUNC_SRC);

	// 	if (reg_step & (1 << 4)) {
	// 		if (MODE == 0) {
	// 			step += 1;
	// 			print_text("Step is detected!");
	// 		}
	// 	}
	// 	else {
	// 		if (!fall_detect && MODE == 1) {
	// 			print_text("Falling detected!\r\n");
	// 			fall_detect = true;
	// 		}
	// 	}
	// }
	switch(GPIO_Pin){
	case GPIO_PIN_1:
		SPI_WIFI_ISR();
		break;
	}
}

void SPI3_IRQHandler(void) {
	HAL_SPI_IRQHandler(&hspi3);
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if(huart->Instance == huart1.Instance) {
		uart_recv = true;	//can do this since expect only 1 char to be sent
		HAL_UART_Receive_IT(&huart1, input_buf, 1);
	}
}

uint8_t LAST_MODE = 0;
uint32_t frame_timer1,frame_timer2;

int main(void) {
	HAL_Init();
	SystemClock_Config();

	MX_GPIO_Init();
	MX_TIM3_Init();
	UART1_Init();
	BSP_LED_Init(LED2);
	BSP_TSENSOR_Init();
	BSP_HSENSOR_Init();
	BSP_PSENSOR_Init();
	BSP_ACCELERO_Init();
	BSP_MAGNETO_Init();
	BSP_GYRO_Init();
	IMU_calib();
	MX_I2C1_Init();
	ssd1306_Init();
	ACC_EXTI11_Init();

	//wifi init
	WiFi_Stat = WIFI_Init(); // if it gets stuck here, you likely did not include EXTI1_IRQHandler() in stm32l4xx_it.c as mentioned above
	WiFi_Stat &= WIFI_Connect(WiFi_SSID, WiFi_password, WiFi_security); // joining a WiFi network takes several seconds. Don't be too quick to judge that your program has 'hung' :)
	if(WiFi_Stat!=WIFI_STATUS_OK) while(1); // halt computations if a WiFi connection could not be established.

//	WiFi_Stat = WIFI_Ping(ipaddr, 3, 200); // Optional ping 3 times in 200 ms intervals
	WiFi_Stat = WIFI_OpenClientConnection(1, WIFI_TCP_PROTOCOL, "conn", ipaddr, DEST_PORT, SOURCE_PORT); // Make a TCP connection.
	// "conn" is just a name and serves no functional purpose
	if(WiFi_Stat!=WIFI_STATUS_OK) while(1); // halt computations if a connection could not be established with the server

	__HAL_TIM_SET_PRESCALER(&htim3, presForFrequency(1000));

	HAL_UART_Receive_IT(&huart1, input_buf, 1);

	uint32_t start_tick = HAL_GetTick();
	uint32_t start_tick_emergency = HAL_GetTick();
	uint32_t readings_tick = HAL_GetTick();
	uint32_t display_tick = HAL_GetTick();
	uint32_t wifi_tick = HAL_GetTick();
	struct Warnings warning_ticks={0,0,0,0};
	char message_print[32];
	sprintf(message_print, "Entering Standard Mode.\r\n");
	print_text(message_print);
	uint16_t seconds_count = 0;
	uint16_t blink_time = 1000;

	while (1) {
		//wifi
		if(HAL_GetTick()-wifi_tick >= 250) {
			wifi_tick = HAL_GetTick();
			sprintf((char*)req, "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f",
					gyro_data[0], gyro_data[1], gyro_data[2],
					accel_data[0], accel_data[1], accel_data[2],
					mag_data[0], mag_data[1], mag_data[2],
					pressure, humidity, temp,
					kalman_rpy[0], kalman_rpy[1], kalman_rpy[2]);
			WiFi_Stat = WIFI_SendData(1, req, (uint16_t)strlen((char*)req), &Datalen,WIFI_WRITE_TIMEOUT);
			WiFi_Stat = WIFI_ReceiveData(1, resp, MAX_LENGTH, &Datalen, WIFI_READ_TIMEOUT); // Get response from the server.
			// char txt[10];
			// sprintf(txt, "%d\n", atoi(resp));
			// print_text(txt);
			MODE = atoi(resp);
		}

		if (LAST_MODE != MODE) {
			ssd1306_Fill(Black);
		}

		//asynchronous non-blocking buzzer handler
		if (start_beep) {	//will get triggered once only
			HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
			beep_start_tick = HAL_GetTick();
		} else {
			if (HAL_GetTick() - beep_start_tick > 20) {
				HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
			}
		}

		//asynchronous uart receive
		if (uart_recv) {
			MODE = atoi(input_buf);
			char msg[20];
			sprintf(msg,"Received: %s\r\n\r\n", input_buf);
			print_text(msg);
			ssd1306_Fill(Black);
			uart_recv = false;
		}

		//asynchronous reading of sensors
		if (HAL_GetTick() - readings_tick >= (dt*1000)) {
			readings_tick = HAL_GetTick();
			get_imu_readings();
			get_pht_readings();

			kalman_update_angle(&kalman_roll, rpy[0], gyro_data[0], dt);
			kalman_update_angle(&kalman_pitch, rpy[1], gyro_data[1], dt);
			rpy[2] = atan2(mag_data[1],mag_data[0])*RAD_TO_DEG;
			kalman_update_angle(&kalman_yaw, rpy[2], gyro_data[2], dt);
			kalman_rpy[0] = kalman_roll.angle;
			kalman_rpy[1] = kalman_pitch.angle;
			kalman_rpy[2] = kalman_yaw.angle;
		}

		/* STANDARD MODE */
		if (MODE == 0) {
			ssd1306_SetCursor(5, 5);
			char message_print[32];
			sprintf(message_print, "Steps: %d", step);
			ssd1306_WriteString(message_print, Font_7x10, White);
			ssd1306_UpdateScreen();
			dt = 0.05;	//sample at 1 hz
			if (HAL_GetTick() - start_tick >= 1000) { // print out readings every 1 second
				start_tick = HAL_GetTick();
				seconds_count++;
				print_readings(seconds_count);
			}
			if (button_counter == 2) {
				if (second_tick - first_tick <= 1000) { // button pressed twice within 1s, change to MODE=1
					char message_print[32];
					sprintf(message_print, "Entering Emergency Mode.\r\n");
					print_text(message_print);
					ssd1306_Fill(Black);
					MODE = 1;
				}
				button_counter = 0;
			} else if (button_counter == 1 && HAL_GetTick() - first_tick > 1000) {
				button_counter= 0;	//reset counter if not pressed again after 1 sec
			}
		}
		/* EMERGENCY MODE */
		else if (MODE == 1){
			dt = 0.05; //sample at 20hz

			/* LED2 blinks at 1hz, 2hz or 3hz */
			if (HAL_GetTick() - start_tick >= blink_time) {
				start_tick = HAL_GetTick();
				BSP_LED_Toggle(LED2);
			}

			// check abnormal movement
			float gyro_magnitude = gyro_data[0]*gyro_data[0] + gyro_data[1]*gyro_data[1]+ gyro_data[2]*gyro_data[2];
			if(gyro_magnitude>=GYRO_THRESHOLD_SQUARED) {
				if (!sudden_move_detect) {
					sudden_move_detect = true;
					sudden_move_tick = HAL_GetTick();
				}
			}

			// check improper posture (mag can only give yaw, so use yaw)
			float yaw = atan2(mag_data[1],mag_data[0])*RAD_TO_DEG;
			float error = mod(yaw-yaw_offset, 360);
			if (error > 180) error -= 360;
			if(fabs(error) > MAG_THRESHOLD) {
				if (!posture_detect_first) {
					posture_detect_first = true;
					posture_first_tick = HAL_GetTick();
				} else {	//initial improper posture detected
					if (HAL_GetTick() - posture_first_tick > 3000) {	//only consider improper posture after 5 secs continuous
						posture_detect_second = true;
						posture_second_tick = HAL_GetTick();
					}
				}
			}

			//check fever
			if(temp >= TEMP_THRESHOLD) {
				fever_detect = true;
				fever_tick = HAL_GetTick();
			}

			flags_detect = (fall_detect<<3) | (sudden_move_detect<<2) | (fever_detect << 1) | posture_detect_second;

			// warnings handler to OLED & UART
			// OLED only updates for lower priority if its the only one activated
			/* Fall display takes priority */
			if (fall_detect) {
				blink_time = 500;
				if (HAL_GetTick()-warning_ticks.fall > warning_dt.fall) {	//print at same rate as blink
					warning_ticks.fall = HAL_GetTick();
					print_text("Falling detected!\r\n");
				}
				if (HAL_GetTick() - display_tick >= DISPLAY_DT) {
					ssd1306_Fill(Black);
					ssd1306_DrawBitmap(10, 10, warning_sign, 50, 50, White);
					ssd1306_SetCursor(70, 15);
					ssd1306_WriteString("WATCH", Font_7x10, White);
					ssd1306_SetCursor(70, 28);
					ssd1306_WriteString("YOUR", Font_7x10, White);
					ssd1306_SetCursor(70, 42);
					ssd1306_WriteString("STEP", Font_11x18, White);
					ssd1306_UpdateScreen();
				}
				if (HAL_GetTick() - display_tick >= 5000) { // stop display after 5s
					fall_tick = HAL_GetTick();
					ssd1306_Fill(Black);
					ssd1306_UpdateScreen();
					fall_detect = false;
				}
			}
			/* Sudden Movement takes 2nd priority */
			if (sudden_move_detect) {
				blink_time = 500;
				if (HAL_GetTick()-warning_ticks.sudden_move > warning_dt.sudden_move) {	//print at same rate as blink
					warning_ticks.sudden_move = HAL_GetTick();
					print_text("Abnormal movement detected!\r\n");
				}
				if (flags_detect < 0b1000) {
					if (HAL_GetTick() - display_tick >= DISPLAY_DT) { // refresh display every 2s
						display_tick = HAL_GetTick();
						ssd1306_Fill(Black);
						ssd1306_DrawBitmap(10, 10, abnormal_movement, 44, 60, White);
						ssd1306_SetCursor(70, 15);
						ssd1306_WriteString("SUDDEN", Font_7x10, White);
						ssd1306_SetCursor(70, 28);
						ssd1306_WriteString("MOVEMENT", Font_7x10, White);
						ssd1306_SetCursor(70, 38);
						ssd1306_WriteString("DETECTED!", Font_7x10, White);
						ssd1306_UpdateScreen();
					}
					if (HAL_GetTick() - sudden_move_tick >= 5000) { // clear after 5s
						sudden_move_tick = HAL_GetTick();
						ssd1306_Fill(Black);
						ssd1306_UpdateScreen();
						sudden_move_detect = false;
					}
				}

			}
			/* Fever is 3rd priority */
			if (fever_detect){
				blink_time = 333;
				if (HAL_GetTick()-warning_ticks.fever > warning_dt.fever) {	//print at same rate as blink
					warning_ticks.fever = HAL_GetTick();
					print_text("Fever is detected!\r\n\r\n");
				}
				if (flags_detect < 0b0100) {
					if (HAL_GetTick() - display_tick >= DISPLAY_DT) {
						display_tick = HAL_GetTick();
						ssd1306_Fill(Black);
						ssd1306_DrawBitmap(5, 5, fever, 60, 56, White);
						ssd1306_SetCursor(70, 15);
						ssd1306_WriteString("FEVER", Font_11x18, White);
						ssd1306_SetCursor(70, 36);
						ssd1306_WriteString("Detected", Font_7x10, White);
						ssd1306_UpdateScreen();
					}
					if (HAL_GetTick() - fever_tick >= 5000) {
						fever_tick = HAL_GetTick();
						ssd1306_Fill(Black);
						ssd1306_UpdateScreen();
						fever_detect = false;
					}
				}
			}
			// Improper posture is 4th priority
			if (posture_detect_second) {
				blink_time = 500;
				if (HAL_GetTick()-warning_ticks.posture > warning_dt.posture) {	//print at same rate as blink
					warning_ticks.posture = HAL_GetTick();
					print_text("Improper posture detected!\r\n\r\n");
				}
//				posture_detect_second = false;
				if (flags_detect < 0b0010) {
					if (HAL_GetTick() - display_tick >= DISPLAY_DT) {
						display_tick = HAL_GetTick();
						ssd1306_Fill(Black);
						ssd1306_DrawBitmap(15, 5, posture, 40, 55, White);
						ssd1306_SetCursor(70, 15);
						ssd1306_WriteString("IMPROPER", Font_7x10, White);
						ssd1306_SetCursor(70, 28);
						ssd1306_WriteString("POSTURE", Font_7x10, White);
						ssd1306_SetCursor(70, 38);
						ssd1306_WriteString("DETECTED!", Font_7x10, White);
						ssd1306_UpdateScreen();
					}
					if (HAL_GetTick() - posture_second_tick >= 5000) {
						posture_second_tick = HAL_GetTick();
						ssd1306_Fill(Black);
						ssd1306_UpdateScreen();
						posture_detect_first = false;
						posture_detect_second = false;
					}
				}

			}
			//no warning detected
			if (!flags_detect){
				blink_time = 1000;
				if (HAL_GetTick() - display_tick >= DISPLAY_DT) {
					display_tick = HAL_GetTick();
					ssd1306_Reset();
					ssd1306_Fill(Black);
					ssd1306_DrawBitmap(0, 0, smiley_face, 64, 64, White);
					ssd1306_SetCursor(70, 15);
					ssd1306_WriteString("Normal", Font_7x10, White);
					ssd1306_SetCursor(70, 28);
					ssd1306_WriteString("Temp:", Font_7x10, White);
					ssd1306_SetCursor(70, 38);
					char buffer[30];
					sprintf(buffer, "%0.2f", temp);
					ssd1306_WriteString(buffer, Font_7x10, White);
					ssd1306_UpdateScreen();
				}
			}

			//buzzer trigger
			if (flags_detect > last_flags_detect) {	//new warning added
				start_beep = true;
			} else {
				start_beep = false;
			}
			last_flags_detect = flags_detect;

			/* Readings Reporting */
			if (HAL_GetTick() - start_tick_emergency >= 5000) {
				start_tick_emergency = HAL_GetTick();
				char msg[200];
				sprintf(msg, "TEMP: %0.2f\r\nGYRO: X:%0.2f, Y:%0.2f, Z:%0.2f\r\nACCEL: X:%0.2f, Y:%0.2f, Z:%0.2f\r\nMAG: X:%0.2f, Y:%0.2f, Z:%0.2f\r\n\r\n",
						temp,
						gyro_data[0],gyro_data[1],gyro_data[2],
						accel_data[0],accel_data[1],accel_data[2],
						mag_data[0],mag_data[1],mag_data[2]);
				HAL_UART_Transmit(&huart1, (uint8_t*) msg,strlen(msg), 0xFFFF);
			}

			if (button_counter == 1) {
				char message_print[32];
				sprintf(message_print, "Entering Emergency Mode.\r\n");
				print_text(message_print);
				button_counter = 0;
				ssd1306_Reset();
				ssd1306_Fill(Black);
				MODE = 0;
			}
		} else {	//MODE == 2
			dt=0.025;	//sample at 40 hz
			frame_timer1 = HAL_GetTick();
			ssd1306_Fill(Black);
			float rpy_rad[3];
			rpy_rad[0] = -kalman_rpy[0] * DEG_TO_RAD;
			rpy_rad[1] = -kalman_rpy[1] * DEG_TO_RAD;
			rpy_rad[2] = -kalman_rpy[2] * DEG_TO_RAD;	//yaw is opposite

			for (int i=0; i<8; i++) {
			    // rotate 3d points around the Y axis (changing X and Z positions)
			    float rot_x = orig_points [i][0] * cos(rpy_rad[2]) - orig_points [i][2] * sin(rpy_rad[2]);
			    float rot_y = orig_points [i][1];
			    float rot_z = orig_points [i][0] * sin(rpy_rad[2]) + orig_points [i][2] * cos(rpy_rad[2]);

			    // rotate 3d points around the X axis (changing Y and Z positions)
			    float rot_xx = rot_x;
				float rot_yy = rot_y * cos(rpy_rad[0]) - rot_z * sin(rpy_rad[0]);
				float rot_zz = rot_y * sin(rpy_rad[0]) + rot_z * cos(rpy_rad[0]);

				// rotate 3d points around the Z axis (changing X and Y positions)
				float rot_xxx = rot_xx * cos(rpy_rad[1]) - rot_yy * sin(rpy_rad[1]);
				float rot_yyy = rot_xx * sin(rpy_rad[1]) + rot_yy * cos(rpy_rad[1]);
				float rot_zzz = rot_zz + z_offset;

			    // project 3d points into 2d space with perspective divide -- 2D x = x/z,   2D y = y/z
			    points[i][0] = round(64 + rot_xxx / rot_zzz * cube_size);
			    points[i][1] = round(32 + rot_yyy / rot_zzz * cube_size);
			    ssd1306_DrawPixel(points[i][0], points[i][1], White);
			}
			draw_wireframe();
			ssd1306_UpdateScreen();
			frame_timer2 = HAL_GetTick();
			if (HAL_GetTick() - cube_tick > 1000) {
				cube_tick = HAL_GetTick();
				char msg[100];
				sprintf(msg,"Kalman Roll, Pitch, Yaw: %.2f, %.2f, %.2f\r\nFrame rate: %.2f\r\n\r\n",kalman_rpy[0],kalman_rpy[1],kalman_rpy[2], 1000.0/(float)(frame_timer2-frame_timer1));
				print_text(msg);
			}
		}
		LAST_MODE = MODE;
	}

}

//------------------------SYSTEM INIT FUNCTIONS------------------------

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

static void MX_TIM3_Init(void) {
	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_OC_InitTypeDef sConfigOC = {0};

	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 0;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 1000;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim3) != HAL_OK) {
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_Init(&htim3) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK) {
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 500;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) {
		Error_Handler();
	}

	HAL_TIM_MspPostInit(&htim3);

}

static void MX_GPIO_Init(void) {
	__HAL_RCC_GPIOC_CLK_ENABLE();  // Enable AHB2 Bus for GPIOC
	__HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	// Configuration of BUTTON_EXTI13_Pin (GPIO-C Pin-13) as AF,
	GPIO_InitStruct.Pin = BUTTON_EXTI13_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	// Enable NVIC EXTI line 13
	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0x00, 0x00);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

static void UART1_Init(void) {
	/* Pin configuration for UART. BSP_COM_Init() can do this automatically */
	__HAL_RCC_GPIOB_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
	GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_6;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* Configuring UART1, can be used in project (copy and paste) since we only can use UART1*/
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart1) != HAL_OK) {
		while (1);
	}
	HAL_NVIC_SetPriority(USART1_IRQn, 0x02, 0x00);
	HAL_NVIC_EnableIRQ(USART1_IRQn);
}

static void MX_I2C1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOB_CLK_ENABLE();

	// Configure GPIO PB8 and PB9
	GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
	GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	// Configure I2C1
	hi2c1.Instance = I2C1;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.Timing = 0x00C0216C;	//400khz
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

	__HAL_RCC_I2C1_CLK_ENABLE();

	if (HAL_I2C_Init(&hi2c1) != HAL_OK)
	{
		Error_Handler();
	}
}

void ACC_EXTI11_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_GPIOD_CLK_ENABLE();

	// Configuration of LSM6DSL_INT1_EXTI11_Pin (GPIO-D Pin-11) as AF,
	GPIO_InitStruct.Pin = LSM6DSL_INT1_EXTI11_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0x01, 0x00);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

	SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL1_XL, 0x60); // initialise and set accelerometer
	SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_TAP_CFG1, 0x80); // enable freefall
	SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_WAKE_UP_DUR, 0xF8); // set ff duration
	SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_FREE_FALL, 0x02); // set ff threshold
	SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_MD1_CFG, 0x10); // route free fall event on INT1

	// Pedo interrupt
	SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL10_C, 0X16); // enable pedo and func
	SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_FIFO_CTRL2, 0xC0); // Enable pedometer step counter and write mode at every step
	SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_INT1_CTRL, 0x80); // route step detector interrupt on INT1
}

static void RF_GPIO_Init() {
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(SPSGRF_915_SDN_GPIO_Port, SPSGRF_915_SDN_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(SPSGRF_915_SPI3_CSN_GPIO_Port, SPSGRF_915_SPI3_CSN_Pin, GPIO_PIN_SET);

    /*Configure GPIO pins : Shutdown Pin on SPSGRF SDN */
    GPIO_InitStruct.Pin = SPSGRF_915_SDN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(SPSGRF_915_SDN_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pin : SPSGRF CS */
    GPIO_InitStruct.Pin = SPSGRF_915_SPI3_CSN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(SPSGRF_915_SPI3_CSN_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pin : SPSGRF GPIO3 for EXTI */
    GPIO_InitStruct.Pin = SPSGRF_915_GPIO3_EXTI5_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(SPSGRF_915_GPIO3_EXTI5_GPIO_Port, &GPIO_InitStruct);

    /* EXTI interrupt init*/
    HAL_NVIC_SetPriority(SPSGRF_915_GPIO3_EXTI5_EXTI_IRQn, SPSGRF_915_GPIO3_EXTI5_EXTI_IRQn_PREEMPT_PRIO, SPSGRF_915_GPIO3_EXTI5_EXTI_IRQn_SUB_PRIO);
    HAL_NVIC_EnableIRQ(SPSGRF_915_GPIO3_EXTI5_EXTI_IRQn);
}

static void RF_SPI3_Init() {
    hspi3.Instance = SPI3;
    hspi3.Init.Mode = SPI_MODE_MASTER;
    hspi3.Init.Direction = SPI_DIRECTION_2LINES;
    hspi3.Init.DataSize = SPI_DATASIZE_4BIT;
    hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi3.Init.NSS = SPI_NSS_SOFT;
    hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi3.Init.CRCPolynomial = 7;
    hspi3.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
	hspi3.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
    if (HAL_SPI_Init(&hspi3) != HAL_OK) {
    	Error_Handler();
	}
}

void Error_Handler(void)
{
	printf("ERROR");
}

//------------------------CUSTOM FUNCTIONS------------------------

void IMU_calib(void) {
	//calibrate initial offsets, assumes board pointing north for the raw mag threshold
	char msg[] = "Calibrating IMU, do not move!\r\n";
	HAL_UART_Transmit(&huart1, (uint8_t*) msg, strlen(msg),0xFFFF);
	HAL_Delay(1000);
	float gyro_raw[3], accel_raw[3], mag_raw[3];
	int num = 50;
	for (int i=0; i<num; ++i) {
		BSP_GYRO_GetXYZ(gyro_raw);
		BSP_ACCELERO_AccGetXYZ(accel_raw);
		BSP_MAGNETO_GetXYZ(mag_raw);
		for (int j=0; j<3; ++j) {
			gyro_offset[j] += gyro_raw[j];	//sum up gyro errors
			accel_offset[j] += accel_raw[j];	//sum up accel errors
		}
		yaw_offset += atan2(mag_raw[1]-mag_offset[1],mag_raw[0]-mag_offset[0]);
		HAL_Delay(5);
	}
	for (int i=0; i<3; ++i) {
		gyro_offset[i] /= (float)num;
		if (i==2) accel_offset[i] = accel_offset[i]/(float)num - 1.0;	//z will be reading g normally (1.0)
		else accel_offset[i] /= (float)num;
	}
	yaw_offset = yaw_offset/(float)num *RAD_TO_DEG;
	char offsets_msg[150];
	sprintf(offsets_msg,"Offsets:\r\nGyro -> X:%f, Y:%f, Z:%f\r\nAcc -> X:%f, Y:%f, Z:%f\r\nYaw -> %f\r\n", gyro_offset[0],gyro_offset[1],gyro_offset[2],accel_offset[0],accel_offset[1],accel_offset[2],yaw_offset);
	HAL_UART_Transmit(&huart1, (uint8_t*) offsets_msg, strlen(offsets_msg),0xFFFF);
}

void get_pht_readings(void) {
	//pht readings
	pressure = BSP_PSENSOR_ReadPressure();
	humidity = BSP_HSENSOR_ReadHumidity();
	temp = BSP_TSENSOR_ReadTemp();
}

void get_imu_readings(void) {
	// imu readings
	BSP_GYRO_GetXYZ(gyro_data);	// units of deg/sec
	BSP_ACCELERO_AccGetXYZ(accel_data);	//units of g
	BSP_MAGNETO_GetXYZ(mag_data);	//units of uT (micro Tesla)
	for (int i=0; i<3; ++i) {
		gyro_data[i] -= gyro_offset[i];
		accel_data[i] -= accel_offset[i];
		mag_data[i] -= mag_offset[i];
	}

	rpy[0] = atan2(accel_data[1],sqrt(accel_data[0]*accel_data[0] + accel_data[2]*accel_data[2])) * RAD_TO_DEG;
	rpy[1] = atan2(-accel_data[0],accel_data[2]) * RAD_TO_DEG;
}

void print_text(char msg[]) {
	HAL_UART_Transmit(&huart1, (uint8_t*) msg,strlen(msg), 0xFFFF);
}

void print_readings(int seconds_count) {
	char message_print1[500];
	sprintf(message_print1,
			"%03d: GYRO -> %0.4f, %0.4f, %0.4f\r\n%03d: ACCEL -> %0.4f, %0.4f, %0.4f\r\n%03d: MAG -> %0.4f, %0.4f, %0.4f\r\n%03d: BARO -> %0.4f; HUMIDITY -> %0.4f; TEMP -> %0.4f;\r\n\r\n",
			seconds_count,  gyro_data[0], gyro_data[1], gyro_data[2],
			seconds_count,  accel_data[0], accel_data[1], accel_data[2],
			seconds_count,  mag_data[0], mag_data[1], mag_data[2],
			seconds_count, pressure, humidity, temp);
	HAL_UART_Transmit(&huart1, (uint8_t*) message_print1,strlen(message_print1), 0xFFFF);

	//OLED readings
	char oled_message_readings1[15];
	char oled_message_readings2[15];
	char oled_message_readings3[15];

	sprintf(oled_message_readings1, "%.2fC", temp);
	sprintf(oled_message_readings2, "%.1f%%", humidity);
	sprintf(oled_message_readings3, "%.2fhPa", pressure);
	ssd1306_Reset();
	ssd1306_SetCursor(79, 24);
	ssd1306_WriteString(oled_message_readings1, Font_7x10, White);
	ssd1306_SetCursor(79, 34);
	ssd1306_WriteString(oled_message_readings2, Font_7x10, White);
	ssd1306_SetCursor(51, 44);
	ssd1306_WriteString(oled_message_readings3, Font_7x10, White);

	ssd1306_UpdateScreen();
}

float mod(float x, float y) { //get python style modulo
  x = fmod(x,y);
  return x < 0 ? x+y : x;
}

void draw_wireframe(void) {
	ssd1306_Line(points[ 0 ][ 0 ], points[ 0 ][ 1 ] , points[ 1 ][ 0 ] , points[ 1 ][ 1 ], White );  // connect points 0-1
	ssd1306_Line(points[ 1 ][ 0 ], points[ 1 ][ 1 ] , points[ 2 ][ 0 ] , points[ 2 ][ 1 ], White );  // connect points 1-2
	ssd1306_Line(points[ 2 ][ 0 ], points[ 2 ][ 1 ] , points[ 3 ][ 0 ] , points[ 3 ][ 1 ], White );  // connect points 2-3
	ssd1306_Line(points[ 3 ][ 0 ], points[ 3 ][ 1 ] , points[ 0 ][ 0 ] , points[ 0 ][ 1 ], White );  // connect points 3-0

	ssd1306_Line(points[ 4 ][ 0 ], points[ 4 ][ 1 ] , points[ 5 ][ 0 ] , points[ 5 ][ 1 ], White );  // connect points 4-5
	ssd1306_Line(points[ 5 ][ 0 ], points[ 5 ][ 1 ] , points[ 6 ][ 0 ] , points[ 6 ][ 1 ], White );  // connect points 5-6
	ssd1306_Line(points[ 6 ][ 0 ], points[ 6 ][ 1 ] , points[ 7 ][ 0 ] , points[ 7 ][ 1 ], White );  // connect points 6-7
	ssd1306_Line(points[ 7 ][ 0 ], points[ 7 ][ 1 ] , points[ 4 ][ 0 ] , points[ 4 ][ 1 ], White );  // connect points 7-4

	ssd1306_Line(points[ 0 ][ 0 ], points[ 0 ][ 1 ] , points[ 4 ][ 0 ] , points[ 4 ][ 1 ], White );  // connect points 0-4
	ssd1306_Line(points[ 1 ][ 0 ], points[ 1 ][ 1 ] , points[ 5 ][ 0 ] , points[ 5 ][ 1 ], White );  // connect points 1-5
	ssd1306_Line(points[ 2 ][ 0 ], points[ 2 ][ 1 ] , points[ 6 ][ 0 ] , points[ 6 ][ 1 ], White );  // connect points 2-6
	ssd1306_Line(points[ 3 ][ 0 ], points[ 3 ][ 1 ] , points[ 7 ][ 0 ] , points[ 7 ][ 1 ], White );  // connect points 3-7

	ssd1306_Line(points[ 0 ][ 0 ], points[ 0 ][ 1 ] , points[ 2 ][ 0 ] , points[ 2 ][ 1 ], White );  // cross
	ssd1306_Line(points[ 1 ][ 0 ], points[ 1 ][ 1 ] , points[ 3 ][ 0 ] , points[ 3 ][ 1 ], White );  // cross
}

int presForFrequency (int frequency) {	//convert frequency to prescaler
	if (frequency == 0) return 0;
	return ((TIM_FREQ/(1000*frequency))-1);  // 1 is added in the register
}
