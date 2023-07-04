################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/hashtables/clht.c 

OBJS += \
./Core/Src/hashtables/clht.o 

C_DEPS += \
./Core/Src/hashtables/clht.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/hashtables/%.o Core/Src/hashtables/%.su: ../Core/Src/hashtables/%.c Core/Src/hashtables/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Core/Inc/clht -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O3 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@"  -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-hashtables

clean-Core-2f-Src-2f-hashtables:
	-$(RM) ./Core/Src/hashtables/clht.d ./Core/Src/hashtables/clht.o ./Core/Src/hashtables/clht.su

.PHONY: clean-Core-2f-Src-2f-hashtables

