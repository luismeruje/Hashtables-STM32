#ifndef EMBBEDED_SRAM_DRIVER_H
#define EMBBEDED_SRAM_DRIVER_H


#include "stm32h7xx_hal.h"

__RAM_FUNC void embbeded_sram_write_64bit_blocks(void *destArray, void *srcArray, uint32_t nrWords);

__RAM_FUNC void embbeded_sram_write_32bit_blocks(void *destArray, void *srcArray, uint32_t nrWords);

__RAM_FUNC void embbeded_sram_write_16bit_blocks(void *destArray, void *srcArray, uint32_t nrWords);

__RAM_FUNC void embbeded_sram_write_8bit_blocks(void *destArray, void *srcArray, uint32_t nrWords);

__RAM_FUNC uint32_t compare_arrays(void *destArray, void *srcArray, uint32_t numBytes);


#endif
