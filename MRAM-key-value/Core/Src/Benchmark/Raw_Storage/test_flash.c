#include <flash_driver.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32h7xx_hal.h"
#include "FLASH_SECTOR_H7.h"


//WARNING: flash has limited write capacity.
//WARNING: not 100% legit, is writing over the same words without erasing, but shouldn't affect the results, since we are not considering erase operations.
//TODO: ignoring writeChunkSize, assuming 32bit
__RAM_FUNC long double test_flash_write_throughput(uint16_t writeChunkSize, uint32_t numBitsToWrite, int numIterations)
{
	uint16_t writeChunkSizeBytes = writeChunkSize/8;
	uint32_t numBytesToWrite = numBitsToWrite/8;
	uint32_t numWords = numBytesToWrite / writeChunkSizeBytes;


	//Randomize sector to write to, to improve wear-leveling
	uint8_t sector = rand() % 8;
	printf("Using sector %d for flash test\n", sector);

	uint32_t startPageAddress = 0x08100000 | (0x00020000 * sector) ; //BANK 2 sector random

	int end_time;
	int start_time;
	int elapsed_time;

	char * srcArray = malloc(sizeof(char) * (numBytesToWrite));
	strcpy(srcArray,"aabbsdfciioo"); //Let it be filled with garbage
	uint32_t readArray [numBytesToWrite / 4];

	//Erase sector
	uint32_t erase_result = erase_flash_sector(sector, 2);
	printf("Erase sector result: %ld\n",erase_result);

	start_time = HAL_GetTick();
	for(int i = 0; i < numIterations; i++){
		//Flash_Write_Data (startPageAddress, &data, 0x01U);
		//Writing without erasing. Always writing the same thing so should be fine.
		flash_write_32bit_hal_copy(FLASH_TYPEPROGRAM_FLASHWORD, startPageAddress,  (uint32_t) srcArray, numWords);
	}
	end_time = HAL_GetTick();

	elapsed_time = end_time - start_time;

	Flash_Read_Data(startPageAddress, readArray, numBytesToWrite/4);
	if(memcmp((void *) srcArray, (void *)readArray, numBytesToWrite/4)){
		puts("Flash write sanity verification failed during throughput test.");
	}
	printf("Wrote %ld bytes * %d in %d ms.\n", numBytesToWrite, numIterations, elapsed_time);
	long double avg = ((long double)(numBytesToWrite * numIterations)) / elapsed_time;

	printf("Avg: %.3Lf KB/s\n", avg);


	return avg;

}
