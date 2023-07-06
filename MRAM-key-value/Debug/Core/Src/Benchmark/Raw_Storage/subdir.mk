################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Benchmark/Raw_Storage/test_MRAM.c \
../Core/Src/Benchmark/Raw_Storage/test_embbeded_SRAM.c \
../Core/Src/Benchmark/Raw_Storage/test_flash.c 

OBJS += \
./Core/Src/Benchmark/Raw_Storage/test_MRAM.o \
./Core/Src/Benchmark/Raw_Storage/test_embbeded_SRAM.o \
./Core/Src/Benchmark/Raw_Storage/test_flash.o 

C_DEPS += \
./Core/Src/Benchmark/Raw_Storage/test_MRAM.d \
./Core/Src/Benchmark/Raw_Storage/test_embbeded_SRAM.d \
./Core/Src/Benchmark/Raw_Storage/test_flash.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Benchmark/Raw_Storage/%.o Core/Src/Benchmark/Raw_Storage/%.su: ../Core/Src/Benchmark/Raw_Storage/%.c Core/Src/Benchmark/Raw_Storage/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g -DDEBUG -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Core/Inc/settings -I../Core/Inc/clht -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@"  -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Benchmark-2f-Raw_Storage

clean-Core-2f-Src-2f-Benchmark-2f-Raw_Storage:
	-$(RM) ./Core/Src/Benchmark/Raw_Storage/test_MRAM.d ./Core/Src/Benchmark/Raw_Storage/test_MRAM.o ./Core/Src/Benchmark/Raw_Storage/test_MRAM.su ./Core/Src/Benchmark/Raw_Storage/test_embbeded_SRAM.d ./Core/Src/Benchmark/Raw_Storage/test_embbeded_SRAM.o ./Core/Src/Benchmark/Raw_Storage/test_embbeded_SRAM.su ./Core/Src/Benchmark/Raw_Storage/test_flash.d ./Core/Src/Benchmark/Raw_Storage/test_flash.o ./Core/Src/Benchmark/Raw_Storage/test_flash.su

.PHONY: clean-Core-2f-Src-2f-Benchmark-2f-Raw_Storage

