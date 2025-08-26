/**
 ******************************************************************************
 * @project        : CG/[T]EE2028 Assignment 1 Program Template
 * @file           : main.c
 * @author         : Hou Linxin, ECE, NUS
 * @brief          : Main program body
 ******************************************************************************
 *
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
#include "stdio.h"
#include "inttypes.h"

#define F 3 //original
#define S 2 //original
#define E 5
#define M 12

extern void asm_func(int* arg1, int* arg2, int* arg3, int* arg4);
extern int asm_func_ultra(int* arg5, int* arg6, int* arg7, int* arg8);
extern void initialise_monitor_handles(void);

//for measuring clock cycles
volatile unsigned int *DWT_CYCCNT   = (volatile unsigned int *)0xE0001004; //address of the register
volatile unsigned int *DWT_CONTROL  = (volatile unsigned int *)0xE0001000; //address of the register
volatile unsigned int *DWT_LAR      = (volatile unsigned int *)0xE0001FB0; //address of the register
volatile unsigned int *SCB_DEMCR    = (volatile unsigned int *)0xE000EDFC; //address of the register

//for tracking error flags in asm_func_ultra
int code = 0;

int main(void)
{
 	initialise_monitor_handles();
	int i,j;

	int building[F][S] = {{9,10},{7,8},{4,4}};
	int entry[E] = {2,4,6,8,10};
	int exit[F][S] = {{1,1},{1,1},{1,1}};
//	 should return: {{11,11},{11,11},{11,11}}

	// ensure that FSEM are contiguous in memory
	int result[F][S] = {{F,S},{E,M},{0,0}};

	*SCB_DEMCR |= 0x01000000;
	*DWT_LAR = 0xC5ACCE55; // enable access
	*DWT_CYCCNT = 0; // reset the counter
	*DWT_CONTROL |= 1 ; // enable the counter
	uint32_t x = *DWT_CYCCNT;

	asm_func((int*)building, (int*)entry, (int*)exit, (int*)result);
//	code = asm_func_ultra((int*)building, (int*)entry, (int*)exit, (int*)result);

	uint32_t y = *DWT_CYCCNT;
	printf("Clock cycle: %lu\n", y-x);
	printf("Error flags: ");
	for (int8_t i=3; i>=0; i--) {	//print in binary
		printf("%d", (code>>i)&0x1);
	}
	printf("\n");

	// print result[][]
	printf("RESULT\n");
	for (i=0; i<F; i++)	{
		for (j=0; j<S; j++) {
			building[i][j] = result[i][j];
			printf("%d\t", building[i][j]);
		}
		printf("\n");
	}

}
