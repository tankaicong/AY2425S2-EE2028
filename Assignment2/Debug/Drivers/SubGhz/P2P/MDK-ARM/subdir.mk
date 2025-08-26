################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Drivers/SubGhz/P2P/MDK-ARM/startup_stm32l475xx.s 

OBJS += \
./Drivers/SubGhz/P2P/MDK-ARM/startup_stm32l475xx.o 

S_DEPS += \
./Drivers/SubGhz/P2P/MDK-ARM/startup_stm32l475xx.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/SubGhz/P2P/MDK-ARM/%.o: ../Drivers/SubGhz/P2P/MDK-ARM/%.s Drivers/SubGhz/P2P/MDK-ARM/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m4 -g3 -c -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-Drivers-2f-SubGhz-2f-P2P-2f-MDK-2d-ARM

clean-Drivers-2f-SubGhz-2f-P2P-2f-MDK-2d-ARM:
	-$(RM) ./Drivers/SubGhz/P2P/MDK-ARM/startup_stm32l475xx.d ./Drivers/SubGhz/P2P/MDK-ARM/startup_stm32l475xx.o

.PHONY: clean-Drivers-2f-SubGhz-2f-P2P-2f-MDK-2d-ARM

