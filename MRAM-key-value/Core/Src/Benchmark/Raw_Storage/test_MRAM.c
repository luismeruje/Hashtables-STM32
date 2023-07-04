#include "mram_commons.h"
#include "test_MRAM.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stm32h7xx_hal.h"
#include "MRAM_driver.h"
#include <assert.h>


__RAM_FUNC long double test_write_throughput_MRAM_memcpy_string(uint16_t blockSize, uint32_t numBytesToWrite, int numIterations){
	long double avg = -1;
	int startTime;
	int elapsedTime = 0;
	uint32_t mismatchedWords = 0;
	uint32_t address = MRAM_BANK_ADDR;
	int numWrites = numBytesToWrite/blockSize;


	if(blockSize > numBytesToWrite){
		printf("The number of bytes to write should be less or equal to the block size.\n");
	} else if(numBytesToWrite > MRAM_NR_BYTES){
		printf("There is not enough available space in the MRAM device to write %ld bytes.\n", numBytesToWrite);
	}

	printf("Num bytes to write: %ld, block size: %d\n", numBytesToWrite, blockSize);

	char array[blockSize];

	for(int j = 0; j < numIterations; j++){
		address = MRAM_BANK_ADDR;
		for(int i = 0; i < blockSize; i++){
			array[i] = (char) ((rand() % 93) + 33);
		}
		array[blockSize - 1] = '\0';

		//Note: If read is not divisible by block size, the leftover bytes will not be written

		startTime = HAL_GetTick();
		for(int i = 0; i < numWrites; i++){
			//mram_write_16bit_blocks(address, array, blockSize/2);
			memcpy((void *) address, (void *) array, blockSize);
			address += blockSize;
			//srcArray = srcArray + blockSize;
		}


		elapsedTime += HAL_GetTick() - startTime;
	}
	avg = ((((long double) numWrites * blockSize) * numIterations) / elapsedTime); //B/ms or KB/s
	printf("Avg: %Lf KB/s\n", avg);

	address = MRAM_BANK_ADDR;
	//Note: 8 bit read at misaligned address is not supported

	for(int i = 0; i < numWrites; i++){
		mismatchedWords += compare_array_with_MRAM(address, array, blockSize);
		address += blockSize;
	}

	if(mismatchedWords > 0){
		printf("MRAM memory test failed for write test with %d block size. %ld mismatched chunks\n", blockSize, mismatchedWords);
	}


	return avg;
}

/**
 * @brief Fill array of a given size with MRAM addresses
 *
 * Fill array of MRAM addresses with randomly or sequentially generated MRAM addresses for throughput testing purposes.
 * Addresses generated are aligned to "alignment"
 * Addresses must be at least "spacing" bytes from the MRAM limit in the case of random addresses.
 *
 *
 * @param addresses Array of addresses to fill.
 * @param numAddresses Size of address array.
 * @param alignment Addresses are a multiple of this parameter.
 * @param spacing Space between addresses. Minimum space between address and MRAM memory limit.
 * @param random If true, addresses are random. Otherwise, will be sequential
 *
 * @return void
 */
void generate_MRAM_addresses(uint32_t * addresses, uint32_t numAddresses, uint32_t alignment, uint32_t spacing, uint32_t random){
	uint32_t mram_address = MRAM_BANK_ADDR;
	for(int i = 0; i < numAddresses; i++){
		if(random == 0){
			addresses[i] = mram_address;
			mram_address += spacing;
			if((mram_address + spacing) > MRAM_BANK_ADDR_LIMIT_4Mb){
				mram_address = MRAM_BANK_ADDR;
			}
		} else{
			addresses[i] = MRAM_BANK_ADDR + (rand() % MRAM_NR_BYTES);
			//Make it a multiplier of writeChunkSize in bytes
			addresses[i] = round((addresses[i] / (float) alignment)) * alignment;
			if((addresses[i] + spacing) >= MRAM_BANK_ADDR_LIMIT_4Mb){
				addresses[i] -= spacing;
				if(addresses[i] < MRAM_BANK_ADDR){
					addresses[i] = MRAM_BANK_ADDR;
				}
			}
		}
	}

}

/** @brief Return mram driver write function that corresponds to a given write size
 *
 *
 *
 * @param writeChunkSize Unit of write size to MRAM device
 * @return Mram driver write function
 *
 */

void  (* get_write_func(uint16_t writeChunkSize))(uint32_t, void *, uint16_t){
	switch(writeChunkSize){
		case 8:
			return mram_write_8bit_blocks;
			break;
		case 16:
			return mram_write_16bit_blocks;
			break;
		case 32:
			return mram_write_32bit_blocks;
			break;
		case 64:
			return mram_write_64bit_blocks;
			break;
		default:
			puts("Warning, operation_size for MRAM write not supported, defaulting to 8bit-sized blocks write");
			return mram_write_8bit_blocks;
			writeChunkSize = 8;
	}
}



/** @brief Tests write throughput of MRAM device given a size for elementary operations, number of bytes to write on each operation, number of write operations to perform, and type of access.
 *
 *  Performs "numIterations" write operations, each with size numBytesToWrite, using the underlying MRAM driver write granularity of "writeChunkSize".
 *  Type describes either sequential (0) or random (1) access.
 *  A consistency check is performed at the end, that confirms the last iteration was correctly written.
 *
 *  @param writeChunkSize Number of bits set at a time. Should be one of 16, 32, or 64 bits.
 *  @param numBytesToWrite Number of bytes per write operation. Should be a multiple of writeChunkSize.
 *  @param numIterations Number of write operation.
 *  @param random Type of access. 0 - Sequential; 1 - Random.
 *  @return Average write throughput in KB/s
 */

//Writes nrWords in operationSize bits chunks (e.g. 16 8bit words). Averages experiment results over "numIterations" trials.
__RAM_FUNC long double test_write_throughput_MRAM(uint16_t writeChunkSize, uint32_t numBytesToWrite, int numIterations, uint8_t random){
	assert(numBytesToWrite <= MRAM_NR_BYTES);
	assert( (numBytesToWrite % (writeChunkSize/8) ) == 0);

	void (*write_func) (uint32_t address, void *data, uint16_t nrWords) = get_write_func(writeChunkSize);

	long double avg = -1;
	int startTime;
	int elapsedTime = 0;
	uint32_t mismatchedWords = 0;
	uint16_t writeChunkSizeBytes = writeChunkSize / 8;
	unsigned int numWords = (numBytesToWrite / writeChunkSizeBytes);
	char srcArray[numBytesToWrite];
	uint32_t mramAddress = MRAM_BANK_ADDR;
	uint32_t numGeneratedAddresses = 32768; //2 to the power of 15. To ease go-back-around iteration. See below.
	uint32_t addressMask = numGeneratedAddresses - 1;
	uint32_t addresses[numGeneratedAddresses];

	generate_MRAM_addresses(addresses, numGeneratedAddresses, writeChunkSizeBytes,numBytesToWrite, random);

	//Generate random string to fill memory
	for(int i = 0; i < numBytesToWrite; i++){
		srcArray[i] = (char) ((rand() % 93) + 33);
	}
	srcArray[numBytesToWrite - 1] = '\0';

	startTime = HAL_GetTick();

	for(int i = 0; i < numIterations; i++){
		mramAddress = addresses[i  & addressMask]; //Rotates back to index 0
		write_func(mramAddress, srcArray, numWords);
	}

	elapsedTime = HAL_GetTick() - startTime;

	avg = (((long double) numIterations * numBytesToWrite) / elapsedTime); //B/ms or KB/s

    //Note: 8 bit read at misaligned address is not supported


	mismatchedWords = compare_array_with_MRAM(mramAddress, srcArray, numBytesToWrite);

	if(mismatchedWords > 0){
		printf("MRAM memory test failed for write test with %d-bit sized write chunks. %ld mismatched chunks\n", writeChunkSize, mismatchedWords);
	}


	return avg;
}


/** @brief Return mram driver write function that corresponds to a given write size
 *
 *
 *
 * @param writeChunkSize Unit of write size to MRAM device
 * @return Mram driver write function
 *
 */

void  (* get_read_func(uint16_t readChunkSize))(uint32_t, void *, uint16_t){
	switch(readChunkSize){
		case 8:
			return mram_read_8bit_blocks;
			break;
		case 16:
			return mram_read_16bit_blocks;
			break;
		case 32:
			return mram_read_32bit_blocks;
			break;
		case 64:
			return mram_read_64bit_blocks;
			break;
		default:
			puts("Warning, operation_size for MRAM write not supported, defaulting to 8bit-sized blocks write");
			return mram_read_8bit_blocks;
	}
}


/** @brief Tests read throughput of MRAM device given a size for elementary operations, number of bytes to read on each operation, number of read operations to perform, and type of access.
 *
 *  Performs "numIterations" write operations, each with size numBytesToWrite, using the underlying MRAM driver write granularity of "writeChunkSize".
 *  Type describes either sequential (0) or random (1) access.
 *  A consistency check is performed at the end, that confirms the last iteration was correctly written.
 *
 *  @param readChunkSize Number of bits to read at a time on MRAM. Should be one of 16, 32, or 64 bits.
 *  @param numBytesToRead Number of bytes per read operation. Should be a multiple of readChunkSize.
 *  @param numIterations Number of read operations.
 *  @param random Type of access. 0 - Sequential; 1 - Random.
 *  @return Average read throughput in KB/s
 */
__RAM_FUNC long double test_read_throughput_MRAM(uint16_t readChunkSize, uint32_t numBytesToRead, int numIterations, uint8_t random){

	assert(numBytesToRead <= MRAM_NR_BYTES);
	assert((numBytesToRead % (readChunkSize/8) ) == 0);

	uint32_t mram_address = MRAM_BANK_ADDR;
	uint32_t numGeneratedAddresses = 32768; //2 to the power of 15. To ease go-back-around iteration. See below.
	uint32_t addressMask = numGeneratedAddresses - 1;
	uint32_t addresses[numGeneratedAddresses];


	long double avg = -1;
	int startTime;
	int elapsedTime = 0;
	uint32_t mismatchedWords = 0;
	uint16_t readChunkSizeBytes = readChunkSize / 8;
	unsigned int numWords = (numBytesToRead / readChunkSizeBytes);
	char srcArray[numBytesToRead];
	void (*read_func) (uint32_t address, void *data, uint16_t nrWords) = get_read_func(readChunkSize);

	generate_MRAM_addresses(addresses, numGeneratedAddresses, readChunkSizeBytes, numBytesToRead, random);

	mram_address = addresses[0];
	startTime = HAL_GetTick();
	for(int i = 0; i < numIterations; i++){
		mram_address = addresses[i  & addressMask];
		read_func(mram_address, srcArray, numWords);

	}
	elapsedTime += HAL_GetTick() - startTime;


	avg = (((long double) numIterations * numBytesToRead) / elapsedTime);

	if(mismatchedWords > 0){
		printf("MRAM memory test failed for read test with %d-bit sized read chunks. %ld mismatched chunks\n", readChunkSize, mismatchedWords);
	}


	return avg;


}

