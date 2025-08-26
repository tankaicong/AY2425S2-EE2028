################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/SubGhz/Common/spsgrf.c \
../Drivers/SubGhz/Common/spsgrf_app.c \
../Drivers/SubGhz/Common/spsgrf_io.c 

OBJS += \
./Drivers/SubGhz/Common/spsgrf.o \
./Drivers/SubGhz/Common/spsgrf_app.o \
./Drivers/SubGhz/Common/spsgrf_io.o 

C_DEPS += \
./Drivers/SubGhz/Common/spsgrf.d \
./Drivers/SubGhz/Common/spsgrf_app.d \
./Drivers/SubGhz/Common/spsgrf_io.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/SubGhz/Common/%.o Drivers/SubGhz/Common/%.su Drivers/SubGhz/Common/%.cyclo: ../Drivers/SubGhz/Common/%.c Drivers/SubGhz/Common/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DDEBUG -DSTM32L475xx -c -I../Drivers/CMSIS/Include -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-SubGhz-2f-Common

clean-Drivers-2f-SubGhz-2f-Common:
	-$(RM) ./Drivers/SubGhz/Common/spsgrf.cyclo ./Drivers/SubGhz/Common/spsgrf.d ./Drivers/SubGhz/Common/spsgrf.o ./Drivers/SubGhz/Common/spsgrf.su ./Drivers/SubGhz/Common/spsgrf_app.cyclo ./Drivers/SubGhz/Common/spsgrf_app.d ./Drivers/SubGhz/Common/spsgrf_app.o ./Drivers/SubGhz/Common/spsgrf_app.su ./Drivers/SubGhz/Common/spsgrf_io.cyclo ./Drivers/SubGhz/Common/spsgrf_io.d ./Drivers/SubGhz/Common/spsgrf_io.o ./Drivers/SubGhz/Common/spsgrf_io.su

.PHONY: clean-Drivers-2f-SubGhz-2f-Common

