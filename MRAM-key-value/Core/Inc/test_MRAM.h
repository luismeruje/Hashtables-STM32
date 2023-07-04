#ifndef TEST_MRAM_H
#define TEST_MRAM_H
#include "stm32h7xx_hal.h"

__RAM_FUNC long double test_write_throughput_MRAM(uint16_t writeChunkSize, uint32_t numBitsToWrite, int numIterations, uint8_t type);
__RAM_FUNC long double test_write_throughput_MRAM_memcpy_string(uint16_t blockSize, uint32_t numBytesToWrite, int numIterations);
__RAM_FUNC long double test_read_throughput_MRAM(uint16_t readChunkSize, uint32_t numBitsToRead, int numIterations, uint8_t type);

#endif //TEST_MRAM_H
