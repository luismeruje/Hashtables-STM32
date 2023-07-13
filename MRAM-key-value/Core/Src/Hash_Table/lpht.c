/*
 * hashmap_string.c
 *
 *  Created on: 08/03/2023
 *      Author: luisferreira
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>

#include "lpht.h"
#include "crc32.h"
#include "settings.h"

#if STM32
#include "stm32h7xx_hal.h"
#else
#include <stdint.h>
#endif

#if MRAM
#include "mram_commons.h"
#include "mram_driver.h"
#endif

#if MMAP
#include <sys/mman.h>
#endif

#define KEY_VALUE_PAIR_SIZE  (2 * FIELD_SIZE)

//NUM_BYTES_CAPACITY minus space reserved for metadata (i.e., HashMapString struct) rounded to the nearest multiple of 32
//For some reason it is not being correctly calculated in C code, so I am calculating it by hand. NUM_BYTES_CAPACITY is in the settings header file
#define USABLE_CAPACITY 299392 // Calculated as: ((double) NUM_BYTES_CAPACITY - 17)/(1 + (1/((double)KEY_VALUE_PAIR_SIZE*8))) | round(nearest 32 aligned number lower than the calculated value) . Equation from three variable system.
#define NUM_BLOCKS (USABLE_CAPACITY / KEY_VALUE_PAIR_SIZE)
#define RESERVED_CAPACITY_METADATA (NUM_BYTES_CAPACITY - USABLE_CAPACITY)

typedef struct lpht {
	char * startAddress; // 4 bytes on stm32, 8 bytes on 64-bit machines
	uint32_t collisions; //4 bytes on stm32
	uint32_t replacements; //4 bytes in stm32
	uint8_t block_availability[(NUM_BLOCKS / 8) + 1];
	//PLus 4 bytes extra, just in case (in the case of MRAM)

} LPHT;

void print_block_availability(LPHT * hashMap){
   printf("[");
   for(int i = 0; i < (NUM_BLOCKS / 8) + 1; i++){
        printf("%d,",hashMap->block_availability[i]);
   }
   printf("]");
}

LPHT * create_lpht(){

	LPHT * hashMap;
	printf("Creating Hash Table with total bytes: %d\nUsable capacity: %d\nMax pairs available: %d\nSize reserved for block occupation and other: %d\n", NUM_BYTES_CAPACITY, (int) USABLE_CAPACITY, NUM_BLOCKS, (int) RESERVED_CAPACITY_METADATA);
	#if MRAM
		mram_wipe();
		hashMap = (LPHT *) MRAM_BANK_ADDR;
	//At the beginning all blocks are available, set to 0
		uint32_t numBlocksToWipe = (NUM_BLOCKS / 8) + 1;
		if(numBlocksToWipe % 2){
			numBlocksToWipe++;
		}
		mram_partial_wipe((uint32_t) hashMap->block_availability, numBlocksToWipe);
		uint32_t address =  MRAM_BANK_ADDR + RESERVED_CAPACITY_METADATA;
		mram_write_16bit_blocks((uint32_t) &(hashMap->startAddress), &address, 2);
		address = 0;
		mram_write_16bit_blocks((uint32_t) &(hashMap->collisions), &address, 2);
		mram_write_16bit_blocks((uint32_t) &(hashMap->replacements), &address, 2);
	#elif MMAP
		int fd = -1;
		if ((fd = open("mmap_file.data", O_RDWR | O_SYNC | O_CREAT)) == -1){
				puts("Not able to open mmap file.");
		}
		ftruncate(fd, sizeof(char) * NUM_BYTES_CAPACITY);
		hashMap = malloc(sizeof(LPHT));
		memset(hashMap->block_availability, 0, (NUM_BLOCKS / 8) + 1);
		hashMap->startaddress = mmap(NULL, sizeof(char) * NUM_BYTES_CAPACITY, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		hashMap->collisions = 0;
		hashMap->replacements = 0;

	#else
		hashMap = malloc(sizeof(LPHT));
		memset(hashMap->block_availability, 0, (NUM_BLOCKS / 8) + 1);
		hashMap->startAddress = (char *) malloc(sizeof(char) * NUM_BYTES_CAPACITY);
		hashMap->collisions = 0;
		hashMap->replacements = 0;
	#endif



	return hashMap;
}

// 1 BLOCK <=> 1 record <=> 1 key-value pair
char * _lpht_get_block_address(char * startAddr, uint16_t block_number){
	//8-bit per address memory
	char * address = startAddr + (KEY_VALUE_PAIR_SIZE * block_number);
	return address;
}

#if STM32
extern CRC_HandleTypeDef hcrc;
#endif

uint8_t _lpht_is_block_occupied(uint16_t block_number, uint8_t * block_availability){
	uint16_t block_byte_index = block_number / 8;
	uint8_t byte_bit_index = block_number % 8;

	
	/*if(block_byte_index==686){
		HAL_Delay(100);
	}*/



	uint8_t occupied = 0;
	uint16_t block;

	#if MRAM
		uint32_t address = (uint32_t) &block_availability[block_byte_index];
		if(address % 2){
			mram_read_16bit_blocks(address-1,&block,1);
			occupied = (uint8_t)(block >> (byte_bit_index+8)) & 0x01;
		} else{
			mram_read_16bit_blocks(address,&block,1);
			occupied = (uint8_t)(block >> byte_bit_index) & 0x01;
		}
	#else
		occupied = (block_availability[block_byte_index] >> byte_bit_index) & 0x01;
	#endif

	return occupied;
}

void _lpht_occupy_block(uint16_t block_number, uint8_t * block_availability){
	uint16_t block_byte_index = block_number / 8;
	uint8_t byte_bit_index = block_number % 8;


	uint16_t block;
	/*if(block_byte_index==686){
			HAL_Delay(100);
	}*/
	#if MRAM
	uint32_t address = (uint32_t) &block_availability[block_byte_index];
	if(address % 2){
	   mram_read_16bit_blocks(address-1,&block,1);
	   block = block | (0x01 << (byte_bit_index + 8)) ;
	   mram_write_16bit_blocks(address-1, &block,1);
	} else{
	   mram_read_16bit_blocks(address,&block,1);
	   block = block | (0x01 << byte_bit_index);
	   mram_write_16bit_blocks(address, &block,1);
	}
	#else
	uint8_t byte = block_availability[block_byte_index] | (0x01 << byte_bit_index);
    block_availability[block_byte_index] = byte;
	#endif



}

// crc32 hash function from gcc github repository
uint32_t _lpht_get_block_number(char *str) {

   uint32_t hash = xcrc32 ((unsigned char *)str, strlen(str), 0xffffffff);
   //uint32_t hash = HAL_CRC_Calculate(&hcrc, (uint32_t *) str, strlen(str));

   return hash % NUM_BLOCKS;
}


//TODO: check key/value size constraints before attempting write
//NOTE: if full rotation of block_number is done, memory is full.
int8_t lpht_put(LPHT * hashMap, char *key, char *value){
        char * address;
		#if MRAM
        	uint32_t mramAddress;
		#endif
        uint16_t block_number = _lpht_get_block_number(key);
        uint8_t done = 0;
        uint8_t block_occupied;
        int8_t result = -1;
        uint32_t counter = 0;
        //static int passed = 0;


        /*if(!strcmp(key, "?:`T=k2)@7d1!kR")){
        	passed = 1;
        	HAL_Delay(100);
        }*/
        //printf("Block number: %d\n", block_number);
        //WARNING: Cannot read 1 byte from uneven addresses!!

        while(!done && counter < NUM_BLOCKS){
            /*if(address >  MRAM_BANK_ADDR_LIMIT_4Mb){
                printf("Address %ld out of range\n", address);
            }*/
		#if MRAM
        	mram_read_16bit_blocks((uint32_t) &(hashMap->startAddress), &mramAddress, 2);
        	address = _lpht_get_block_address((char *) mramAddress, block_number);
        	block_occupied = _lpht_is_block_occupied(block_number, (uint8_t *) &(hashMap->block_availability));
		#else
        	address = _lpht_get_block_address(hashMap->startAddress, block_number);
        	block_occupied = _lpht_is_block_occupied(block_number, hashMap->block_availability);
		#endif

            if (!block_occupied){

            //Could be done with memcpy for generic solution, but doing strcpy is more efficient for strings.
				#if MRAM
					mram_write((uint32_t) address, key, FIELD_SIZE);
					mram_write((uint32_t) address + FIELD_SIZE, value, FIELD_SIZE);
					//printf("Wrote in address: %p, value %s\n", (void *) address, value);
				#else
					memcpy((void *) address, key, FIELD_SIZE);
					memcpy((void *) (address + FIELD_SIZE), value, FIELD_SIZE);
				#endif
				#if MMAP
					msync((void *) address, KEY_VALUE_PAIR_SIZE, MS_SYNC);
				#endif
				_lpht_occupy_block(block_number, (uint8_t *) &(hashMap->block_availability));
				done = 1;
				result = 0;
			} else if ( !strncmp((char *) address, key, FIELD_SIZE)){
				#if MRAM
					mram_increment_uint32_t((uint32_t)&(hashMap->replacements));
				#else
					hashMap->replacements++;
				#endif

				//Replace existing value
				#if MRAM
					mram_write((uint32_t) address + FIELD_SIZE, value, FIELD_SIZE);
					//printf("Replaced value in address: %p, with value %s\n", (void *) address, value);
				#else
					memcpy((void *) (address + FIELD_SIZE), value, FIELD_SIZE);
				#endif
				#if MMAP
						msync((void *) (address + 4), FIELD_SIZE, MS_SYNC);
				#endif
				done = 1;
				result = 1;
			} else {
#if MRAM
				mram_increment_uint32_t((uint32_t)&(hashMap->collisions));
#else
				hashMap->collisions++;
#endif
				block_number = (block_number + 1) % NUM_BLOCKS;
				counter += 1;
			}
        }

        return result;
}

//@ret String equal to the one in the hashtable. Returned string must be freed
char *lpht_get(LPHT * hashMap, char *key){
	 	 /*if(!strcmp(key, "?:`T=k2)@7d1!kR")){
	        HAL_Delay(100);
	     }*/
        uint32_t block_number = _lpht_get_block_number(key);
        char *address;
        uint8_t done = 0;
        char *value = NULL;
        uint32_t counter = 0;
        uint8_t block_occupied = 0;

        while(!done && (counter < NUM_BLOCKS)){
		#if MRAM
        	uint32_t mramAddress;
        	mram_read_16bit_blocks((uint32_t) &(hashMap->startAddress), &mramAddress, 2);
        	address = _lpht_get_block_address((char *) mramAddress, block_number);
        	block_occupied = _lpht_is_block_occupied(block_number, (uint8_t *) &(hashMap->block_availability));
		#else
        	address = _lpht_get_block_address(hashMap->startAddress, block_number);
        	block_occupied = _lpht_is_block_occupied(block_number, hashMap->block_availability);
		#endif
			if( block_occupied && (!strncmp((char *) address, key, FIELD_SIZE))){
				#if MRAM
					value = malloc(sizeof(char) * FIELD_SIZE);
					mram_read(value,(uint32_t) address + FIELD_SIZE, FIELD_SIZE);
					//printf("Reading from address: %p, value %s\n", (void *) address, value);
				#else
					value = strdup(address + FIELD_SIZE);
				#endif
					done = 1;
			} else if(block_occupied){
					block_number = (block_number + 1) % NUM_BLOCKS;
					counter += 1;
			} else {
					break;
			}
        }

        return value;
}


//TODO: Not MRAM safe
uint32_t ht_string_count_occupied_blocks(uint8_t * block_availability){
        uint32_t counter = 0;
        for(int i = 0; i < NUM_BLOCKS; i++){
			uint8_t byte = block_availability[i];
			while(byte){
				counter += byte & 0x1;
				byte = byte >> 1;
			}
        }
        return counter;
}

//TODO: Not MRAM safe
uint32_t lpht_get_collisions(LPHT * hashMap){
	#if MRAM
		uint32_t collisions;
		mram_read(&collisions, (uint32_t)&(hashMap->collisions), 4);
		return collisions;
	#else
		return hashMap->collisions;
	#endif
}

uint32_t lpht_get_replacements(LPHT * hashMap){
#if MRAM
		uint32_t replacements;
		mram_read(&replacements, (uint32_t)&(hashMap->replacements), 4);
		return replacements;
	#else
		return hashMap->replacements;
	#endif
}

void destroy_lpht(LPHT * hashMap){
	#if !MRAM
		free(hashMap);
	#endif
}



