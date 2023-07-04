#ifndef TEST_EMBBEDED_SRAM_H
#define TEST_EMBBEDED_SRAM_H

#include "stm32h7xx_hal.h"

long double test_SRAM_write_throughput(uint16_t writeChunkSize, uint32_t numBitsToWrite, int numIterations);

#endif
