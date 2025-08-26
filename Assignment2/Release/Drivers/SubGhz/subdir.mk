################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/SubGhz/clocks_and_power.c \
../Drivers/SubGhz/p2p_app.c \
../Drivers/SubGhz/spsgrf.c \
../Drivers/SubGhz/spsgrf_app.c \
../Drivers/SubGhz/spsgrf_io.c 

OBJS += \
./Drivers/SubGhz/clocks_and_power.o \
./Drivers/SubGhz/p2p_app.o \
./Drivers/SubGhz/spsgrf.o \
./Drivers/SubGhz/spsgrf_app.o \
./Drivers/SubGhz/spsgrf_io.o 

C_DEPS += \
./Drivers/SubGhz/clocks_and_power.d \
./Drivers/SubGhz/p2p_app.d \
./Drivers/SubGhz/spsgrf.d \
./Drivers/SubGhz/spsgrf_app.d \
./Drivers/SubGhz/spsgrf_io.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/SubGhz/%.o Drivers/SubGhz/%.su Drivers/SubGhz/%.cyclo: ../Drivers/SubGhz/%.c Drivers/SubGhz/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32L475xx -c -I../Drivers/CMSIS/Include -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-SubGhz

clean-Drivers-2f-SubGhz:
	-$(RM) ./Drivers/SubGhz/clocks_and_power.cyclo ./Drivers/SubGhz/clocks_and_power.d ./Drivers/SubGhz/clocks_and_power.o ./Drivers/SubGhz/clocks_and_power.su ./Drivers/SubGhz/p2p_app.cyclo ./Drivers/SubGhz/p2p_app.d ./Drivers/SubGhz/p2p_app.o ./Drivers/SubGhz/p2p_app.su ./Drivers/SubGhz/spsgrf.cyclo ./Drivers/SubGhz/spsgrf.d ./Drivers/SubGhz/spsgrf.o ./Drivers/SubGhz/spsgrf.su ./Drivers/SubGhz/spsgrf_app.cyclo ./Drivers/SubGhz/spsgrf_app.d ./Drivers/SubGhz/spsgrf_app.o ./Drivers/SubGhz/spsgrf_app.su ./Drivers/SubGhz/spsgrf_io.cyclo ./Drivers/SubGhz/spsgrf_io.d ./Drivers/SubGhz/spsgrf_io.o ./Drivers/SubGhz/spsgrf_io.su

.PHONY: clean-Drivers-2f-SubGhz

