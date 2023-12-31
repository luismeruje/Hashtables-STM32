#include "stm32h7xx_hal.h"
#include "mram_commons.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stm32h7xx.h"

//Note: Accessing memory without bit shift. Not needed!

__RAM_FUNC void mram_write_8bit_blocks(uint32_t address, void *data, uint16_t nrWords){
	uint8_t *mramDest = (uint8_t *) address;
	uint8_t *dataSrc = (uint8_t *) data;
	for(int i = 0; i < nrWords; i++){
		*(__IO uint8_t *) mramDest = *dataSrc;
		mramDest++;
		dataSrc++;
	}
	__DSB();
}


__RAM_FUNC void mram_write_16bit_blocks(uint32_t address, void *data, uint16_t nrWords){
	uint16_t *mramDest = (uint16_t *) address;
	uint16_t *dataSrc = (uint16_t *) data;

	for(int i = 0; i < nrWords; i++){
		*(__IO uint16_t *) mramDest = *dataSrc;
		mramDest++;
		dataSrc++;
	}
	__DSB();
}

__RAM_FUNC void mram_write_32bit_blocks(uint32_t address, void *data, uint16_t nrWords){
	uint32_t *mramDest = (uint32_t *) address;
	uint32_t *dataSrc = (uint32_t *) data;
	for(int i = 0; i < nrWords; i++){
		*(__IO uint32_t *) mramDest = *dataSrc;
		mramDest++;
		dataSrc++;
	}
	__DSB();
}

__RAM_FUNC void mram_write_64bit_blocks(uint32_t address, void *data, uint16_t nrWords){
	uint64_t *mramDest = (uint64_t *) address;
	uint64_t *dataSrc = (uint64_t *) data;
	for(int i = 0; i < nrWords; i++){
		*(__IO uint64_t *) mramDest = *dataSrc;
		mramDest++;
		dataSrc ++;
	}
	__DSB();
}

__RAM_FUNC void mram_read_8bit_blocks(uint32_t address, void *data, uint16_t nrWords){
	__IO uint8_t *mramSrc = (__IO uint8_t *) address;
	uint8_t *dataDest = (uint8_t *) data;
	for(int i = 0; i < nrWords; i++){
		*(__IO uint8_t *) dataDest = *mramSrc;
		mramSrc++;
		dataDest++;
	}
	__DSB();
}


__RAM_FUNC void mram_read_16bit_blocks(uint32_t address, void *data, uint16_t nrWords){
//	if(address % 2){
//		HAL_Delay(100);
//	}
	__IO uint16_t *mramSrc = (__IO uint16_t *) address;
	uint16_t *dataDest = (uint16_t *) data;

	for(int i = 0; i < nrWords; i++){
		*(__IO uint16_t *) dataDest = *mramSrc;
		mramSrc++;
		dataDest++;
	}
	__DSB();
}

__RAM_FUNC void mram_read_32bit_blocks(uint32_t address, void *data, uint16_t nrWords){
	__IO uint32_t *mramSrc = (__IO uint32_t *) address;
	uint32_t *dataDest = (uint32_t *) data;
	for(int i = 0; i < nrWords; i++){
		*(__IO uint32_t *) dataDest = *mramSrc;
		mramSrc++;
		dataDest++;
	}
	__DSB();
}

__RAM_FUNC void mram_read_64bit_blocks(uint32_t address, void *data, uint16_t nrWords){
	__IO uint64_t *mramSrc = (__IO uint64_t *) address;
	uint64_t *dataDest = (uint64_t *) data;
	for(int i = 0; i < nrWords; i++){
		*(__IO uint64_t *) dataDest = *mramSrc;
		mramSrc++;
		dataDest ++;
	}
	__DSB();
}


uint16_t compare_array_with_MRAM(uint32_t mramAddress, const void *data, uint32_t numBytes){
	uint32_t nrMismatchedBlocks = 0;

	//if(memcmp((void *) mramAddress, (void *) data, (size_t) numBytes)){
		//nrMismatchedBlocks = 1;
	for(int i = 0; i < numBytes / 2; i++){
		if(* (uint16_t *) mramAddress != *(uint16_t *) data){
			nrMismatchedBlocks++;

			printf("Data: %s\n\n MRAM: %s\n", (char *) data, (char *) mramAddress);
			printf("Data address: %p\nMRAM address: %p\n",(void *) data, (void *) mramAddress);
			if(* (uint16_t *) mramAddress != *(uint16_t *) data){
				printf("No error on second comparison\n");
			}
		}
	}

	return nrMismatchedBlocks;
}

void mram_read(void * dest, uint32_t mramAddr, uint16_t numBytes){
	//TODO: handle misaligned single byte reads with bit shift
	if(numBytes % 2){
		//Single byte reads are not supported at misaligned addresses, so I just force numBytes to be even.
		printf("ERROR: mram memory copy only supports copies of an even number of bytes\n");
		return;
	}

	mram_read_16bit_blocks(mramAddr, dest, numBytes/2);

}

void mram_write (uint32_t mramAddr, void * src, uint16_t numBytes){
	//TODO: change to uint8_t if there is one byte left
	if(numBytes % 2){
		printf("ERROR: mram single byte write not implemented for this method\n");
		return;
	}


	/*if(mramAddr >= MRAM_BANK_ADDR_LIMIT_4Mb - 1){
		printf("Trying to write to address %p which is outside limits.\n", (void *) mramAddr);
		HAL_Delay(10*1000);
	}*/
	//printf("Trying to write to address %p \n", (void *) mramAddr);

	mram_write_16bit_blocks(mramAddr, src, numBytes/2);

}

__RAM_FUNC void mram_wipe(){
	__IO uint32_t addr = MRAM_BANK_ADDR;
	for(; addr < MRAM_BANK_ADDR_LIMIT_4Mb; addr+=2){
		* (uint16_t *) addr = 0;
	}
}

__RAM_FUNC void mram_partial_wipe(uint32_t mramAddr, uint16_t numBytes){
	if((numBytes%2)){
		exit(-1);
	}
	__IO uint16_t * addr = (uint16_t *) mramAddr;
	uint16_t zero = 0;
	for(int i = 0; i < numBytes/2; i++){
		*addr = zero;
		addr++;
	}
}

__RAM_FUNC void mram_increment_uint32_t(uint32_t address){
	__IO uint32_t * add = (__IO uint32_t*) address;
	__IO uint32_t val = *add;
	__DSB();
	val++;
	*add = val;
	__DSB();
}

