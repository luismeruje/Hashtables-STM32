#include "test_embbeded_SRAM.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stm32h7xx_hal.h"
#include "embbeded_SRAM_driver.h"

long double test_SRAM_write_throughput(uint16_t writeChunkSize, uint32_t numBitsToWrite, int numIterations){
	void (*write_func) (void* destArray, void *srcArray, uint32_t nrWords) = NULL;

		switch(writeChunkSize){
			case 8:
				write_func = embbeded_sram_write_8bit_blocks;
				break;
			case 16:
				write_func = embbeded_sram_write_16bit_blocks;
				break;
			case 32:
				write_func = embbeded_sram_write_32bit_blocks;
				break;
			case 64:
				write_func = embbeded_sram_write_64bit_blocks;
				break;
			default:
				puts("Warning, operation_size for MRAM write not supported, defaulting to 8bit-sized blocks write");
				write_func = embbeded_sram_write_8bit_blocks;
				writeChunkSize = 8;
		}

		long double avg = -1;
		int startTime;
		int elapsedTime = 0;
		uint32_t mismatchedWords = 0;
		uint16_t writeChunkSizeBytes = writeChunkSize / 8;
		uint32_t numBytesToWrite = numBitsToWrite / 8;
		uint32_t numWords = (numBytesToWrite / writeChunkSizeBytes);
		char srcArray[numBytesToWrite];
		char destArray[numBytesToWrite];

		printf("Writing numBytesToWrite %lu, in %u writeChunkSizeBytes, equaling %lu writes.\n", numBytesToWrite, writeChunkSizeBytes, numWords);

		for(int i = 0; i < numBytesToWrite; i++){
			srcArray[i] = (char) (rand() % 255);
		}


		startTime = HAL_GetTick();
		for(int i = 0; i < numIterations; i++){
			write_func((void *)destArray, (void *)srcArray, numWords);
		}


		elapsedTime = HAL_GetTick() - startTime;

		avg = (((long double) numIterations * numBytesToWrite) / elapsedTime); //B/ms or KB/s
	    printf("Avg: %Lf KB/s\n", avg);

	    //Note: 8 bit read at misaligned address is not supported
		mismatchedWords = compare_arrays(destArray, srcArray, numBytesToWrite);

		if(mismatchedWords > 0){
			printf("MRAM memory test failed for write test with %d-bit sized write chunks. %ld mismatched chunks\n", writeChunkSize, mismatchedWords);
		}


		return avg;
}
