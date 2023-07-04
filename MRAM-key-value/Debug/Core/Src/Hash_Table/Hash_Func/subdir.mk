################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Hash_Table/Hash_Func/crc32.c 

OBJS += \
./Core/Src/Hash_Table/Hash_Func/crc32.o 

C_DEPS += \
./Core/Src/Hash_Table/Hash_Func/crc32.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Hash_Table/Hash_Func/%.o Core/Src/Hash_Table/Hash_Func/%.su: ../Core/Src/Hash_Table/Hash_Func/%.c Core/Src/Hash_Table/Hash_Func/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Core/Inc/settings -I../Core/Inc/clht -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O3 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@"  -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Hash_Table-2f-Hash_Func

clean-Core-2f-Src-2f-Hash_Table-2f-Hash_Func:
	-$(RM) ./Core/Src/Hash_Table/Hash_Func/crc32.d ./Core/Src/Hash_Table/Hash_Func/crc32.o ./Core/Src/Hash_Table/Hash_Func/crc32.su

.PHONY: clean-Core-2f-Src-2f-Hash_Table-2f-Hash_Func

