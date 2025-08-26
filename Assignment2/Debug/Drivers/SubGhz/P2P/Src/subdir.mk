################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/SubGhz/P2P/Src/stm32l4xx_it.c \
../Drivers/SubGhz/P2P/Src/system_stm32l4xx.c 

OBJS += \
./Drivers/SubGhz/P2P/Src/stm32l4xx_it.o \
./Drivers/SubGhz/P2P/Src/system_stm32l4xx.o 

C_DEPS += \
./Drivers/SubGhz/P2P/Src/stm32l4xx_it.d \
./Drivers/SubGhz/P2P/Src/system_stm32l4xx.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/SubGhz/P2P/Src/%.o Drivers/SubGhz/P2P/Src/%.su Drivers/SubGhz/P2P/Src/%.cyclo: ../Drivers/SubGhz/P2P/Src/%.c Drivers/SubGhz/P2P/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DDEBUG -DSTM32L475xx -c -I../Drivers/CMSIS/Include -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-SubGhz-2f-P2P-2f-Src

clean-Drivers-2f-SubGhz-2f-P2P-2f-Src:
	-$(RM) ./Drivers/SubGhz/P2P/Src/stm32l4xx_it.cyclo ./Drivers/SubGhz/P2P/Src/stm32l4xx_it.d ./Drivers/SubGhz/P2P/Src/stm32l4xx_it.o ./Drivers/SubGhz/P2P/Src/stm32l4xx_it.su ./Drivers/SubGhz/P2P/Src/system_stm32l4xx.cyclo ./Drivers/SubGhz/P2P/Src/system_stm32l4xx.d ./Drivers/SubGhz/P2P/Src/system_stm32l4xx.o ./Drivers/SubGhz/P2P/Src/system_stm32l4xx.su

.PHONY: clean-Drivers-2f-SubGhz-2f-P2P-2f-Src

