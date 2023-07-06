################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Benchmark/Hash_Table/benchmark_clht.c \
../Core/Src/Benchmark/Hash_Table/benchmark_lpht.c \
../Core/Src/Benchmark/Hash_Table/test_hashmap_numbers.c 

OBJS += \
./Core/Src/Benchmark/Hash_Table/benchmark_clht.o \
./Core/Src/Benchmark/Hash_Table/benchmark_lpht.o \
./Core/Src/Benchmark/Hash_Table/test_hashmap_numbers.o 

C_DEPS += \
./Core/Src/Benchmark/Hash_Table/benchmark_clht.d \
./Core/Src/Benchmark/Hash_Table/benchmark_lpht.d \
./Core/Src/Benchmark/Hash_Table/test_hashmap_numbers.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Benchmark/Hash_Table/%.o Core/Src/Benchmark/Hash_Table/%.su: ../Core/Src/Benchmark/Hash_Table/%.c Core/Src/Benchmark/Hash_Table/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g -DDEBUG -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Core/Inc/settings -I../Core/Inc/clht -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@"  -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Benchmark-2f-Hash_Table

clean-Core-2f-Src-2f-Benchmark-2f-Hash_Table:
	-$(RM) ./Core/Src/Benchmark/Hash_Table/benchmark_clht.d ./Core/Src/Benchmark/Hash_Table/benchmark_clht.o ./Core/Src/Benchmark/Hash_Table/benchmark_clht.su ./Core/Src/Benchmark/Hash_Table/benchmark_lpht.d ./Core/Src/Benchmark/Hash_Table/benchmark_lpht.o ./Core/Src/Benchmark/Hash_Table/benchmark_lpht.su ./Core/Src/Benchmark/Hash_Table/test_hashmap_numbers.d ./Core/Src/Benchmark/Hash_Table/test_hashmap_numbers.o ./Core/Src/Benchmark/Hash_Table/test_hashmap_numbers.su

.PHONY: clean-Core-2f-Src-2f-Benchmark-2f-Hash_Table

