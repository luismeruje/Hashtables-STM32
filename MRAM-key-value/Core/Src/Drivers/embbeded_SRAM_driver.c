#include "embbeded_SRAM_driver.h"

//TODO: same as MRAM driver. Maybe join both.
__RAM_FUNC void embbeded_sram_write_8bit_blocks(void *destArray, void *srcArray, uint32_t nrWords){
	uint8_t *dest = (uint8_t *) destArray;
	uint8_t *src = (uint8_t *) srcArray;

	for(int i = 0; i < nrWords; i++){
		*dest = *src;
		dest++;
		src++;
	}
}

__RAM_FUNC void embbeded_sram_write_16bit_blocks(void *destArray, void *srcArray, uint32_t nrWords){
	uint16_t *dest = (uint16_t *) destArray;
	uint16_t *src = (uint16_t *) srcArray;

	for(int i = 0; i < nrWords; i++){
		*dest = *src;
		dest++;
		src++;
	}
}

__RAM_FUNC void embbeded_sram_write_32bit_blocks(void *destArray, void *srcArray, uint32_t nrWords){
	uint32_t *dest = (uint32_t *) destArray;
	uint32_t *src = (uint32_t *) srcArray;

	for(int i = 0; i < nrWords; i++){
		*dest = *src;
		dest++;
		src++;
	}
}

__RAM_FUNC void embbeded_sram_write_64bit_blocks(void *destArray, void *srcArray, uint32_t nrWords){
	uint64_t *dest = (uint64_t *) destArray;
	uint64_t *src = (uint64_t *) srcArray;

	for(int i = 0; i < nrWords; i++){
		*dest = *src;
		dest++;
		src++;
	}
}

__RAM_FUNC uint32_t compare_arrays(void *destArray, void *srcArray, uint32_t numBytes){
	char *src = (char *)srcArray;
	char *dest = (char *)destArray;
	uint32_t mismatchedBytes = 0;

	for(int i = 0; i < numBytes; i++){
		if(src[i] != dest[i]){
			mismatchedBytes++;
		}
	}

	return mismatchedBytes;
}
