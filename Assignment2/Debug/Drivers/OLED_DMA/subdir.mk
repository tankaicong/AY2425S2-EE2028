################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/OLED_DMA/fonts.c \
../Drivers/OLED_DMA/ssd1306.c 

OBJS += \
./Drivers/OLED_DMA/fonts.o \
./Drivers/OLED_DMA/ssd1306.o 

C_DEPS += \
./Drivers/OLED_DMA/fonts.d \
./Drivers/OLED_DMA/ssd1306.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/OLED_DMA/%.o Drivers/OLED_DMA/%.su Drivers/OLED_DMA/%.cyclo: ../Drivers/OLED_DMA/%.c Drivers/OLED_DMA/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DDEBUG -DSTM32L475xx -c -I../Drivers/CMSIS/Include -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-OLED_DMA

clean-Drivers-2f-OLED_DMA:
	-$(RM) ./Drivers/OLED_DMA/fonts.cyclo ./Drivers/OLED_DMA/fonts.d ./Drivers/OLED_DMA/fonts.o ./Drivers/OLED_DMA/fonts.su ./Drivers/OLED_DMA/ssd1306.cyclo ./Drivers/OLED_DMA/ssd1306.d ./Drivers/OLED_DMA/ssd1306.o ./Drivers/OLED_DMA/ssd1306.su

.PHONY: clean-Drivers-2f-OLED_DMA

