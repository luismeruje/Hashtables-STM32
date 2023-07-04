/*
 * hashmap_string.c
 *
 *  Created on: 08/03/2023
 *      Author: luisferreira
 */

#include "hashmap_string.h"

#if MRAMEN
#include "stm32h7xx_hal.h"
#include "mram_commons.h"
#include "mram_driver.h"
#endif

#include "settings.h"


#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>
#include "crc32.h"

#define BILLION  1000000000.0


/*
 * hashmap_string.c
 *
 *  Created on: 08/03/2023
 *      Author: luisferreira
 */



#if MMAP
#include <sys/mman.h>
#endif

//String based implementation

#define KEY_VALUE_PAIR_SIZE  (2 * FIELD_SIZE)


//WARNING: Only tested with this specific NUM_BYTES_CAPACITY
//Solved a three variable system

#define NUM_BYTES_CAPACITY 524288
#define USABLE_CAPACITY 522208// Calculated as: ((double) NUM_BYTES_CAPACITY - 17)/(1 + (1/((double)KEY_VALUE_PAIR_SIZE*8))) //NUM_BYTES_CAPACITY minus space reserved for metadata, i.e., HashMapString, rounded to the nearest multiple of 32
#define NUM_BLOCKS (USABLE_CAPACITY / KEY_VALUE_PAIR_SIZE) //For FIELD_SIZE 2 == 131072
#define RESERVED_CAPACITY_METADATA (NUM_BYTES_CAPACITY - USABLE_CAPACITY)

//#define NUM_BYTES_MEMORY_SIZE 524288
typedef struct hashmapString {
	char * startAddress; // 4 bytes on stm32
	//TODO: block_availability should also be stored on MRAM
	int collisions; //4 bytes on stm32
	int replacements; //4 bytes in stm32
	uint8_t block_availability[(NUM_BLOCKS / 8) + 1];
	//PLus 4 bytes extra, just in case (in the case of MRAM)

} HashMapString;

void print_block_availability(HashMapString * hashMap){
   printf("[");
   for(int i = 0; i < (NUM_BLOCKS / 8) + 1; i++){
        printf("%d,",hashMap->block_availability[i]);
   }
   printf("]");
}

HashMapString * create_hash_map_string(){

	HashMapString * hashMap;
	printf("Creating Hash Table with total bytes: %d\nUsable capacity: %d\nMax pairs available: %d\nSize reserved for block occupation and other: %d\n", NUM_BYTES_CAPACITY, (int) USABLE_CAPACITY, NUM_BLOCKS, (int) RESERVED_CAPACITY_METADATA);
	#if MRAMEN
		hashMap = (HashMapString *) MRAM_BANK_ADDR;
		hashMap->startAddress = (char *) MRAM_BANK_ADDR + RESERVED_CAPACITY_METADATA;


	#elif MMAP
		int fd = -1;
		if ((fd = open("mmap_file.data", O_RDWR | O_SYNC | O_CREAT)) == -1){
				puts("Not able to open mmap file.");
		}
		ftruncate(fd, sizeof(char) * NUM_BYTES_CAPACITY);
		hashMap = malloc(sizeof(HashMapString));
		hashMap->startaddress = mmap(NULL, sizeof(char) * NUM_BYTES_CAPACITY, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	#else
		hashMap = malloc(sizeof(HashMapString));
		hashMap->startAddress = (char *) malloc(sizeof(char) * NUM_BYTES_CAPACITY);
	#endif

	//At the beginning all blocks are available, set to 0
	memset(hashMap->block_availability, 0, (NUM_BLOCKS / 8) + 1);
	hashMap->collisions = 0;
	hashMap->replacements = 0;
	return hashMap;
}

// 1 BLOCK <=> 1 record <=> 1 key-value pair
char * _ht_string_get_block_address(char * startAddr, uint16_t block_number){
        //8-bit per address memory
        char * address = startAddr + (KEY_VALUE_PAIR_SIZE * block_number);
        return address;
}

/*uint16_t get_block_number(uint32_t key){
        //Hash function from key to block number. Hash simply uses keys last 16 bits
        uint16_t block_number = 0;
        block_number = (uint16_t) (key & 0xFFFF) ;
        return block_number;
}*/

#if MRAMEN
extern CRC_HandleTypeDef hcrc;
#endif

uint8_t _ht_string_is_block_occupied(uint16_t block_number, uint8_t * block_availability){
	uint16_t block_byte_index = block_number / 8;
	uint8_t byte_bit_index = block_number % 8;

	//MRAM does not allow byte reads from uneven addresses.
	/*if(block_byte_index==686){
		HAL_Delay(100);
	}*/

      uint32_t address = (uint32_t) &block_availability[block_byte_index];
	//Uneven number
	uint8_t occupied = 0;
	uint16_t block;
       // occupied = (block_availability[block_byte_index] >> byte_bit_index) & 0x01;
       if(address % 2){
    	mram_read_16bit_blocks(address-1,&block,1);
        occupied = (uint8_t)(block >> (byte_bit_index+8)) & 0x01;
       } else{
    	   mram_read_16bit_blocks(address,&block,1);
        occupied = (uint8_t)(block >> byte_bit_index) & 0x01;
       }
        return occupied;


}

void _ht_string_occupy_block(uint16_t block_number, uint8_t * block_availability){
	uint16_t block_byte_index = block_number / 8;
	uint8_t byte_bit_index = block_number % 8;

	uint32_t address = (uint32_t) &block_availability[block_byte_index];
	uint16_t block;
	/*if(block_byte_index==686){
			HAL_Delay(100);
	}*/
	if(address % 2){
	   mram_read_16bit_blocks(address-1,&block,1);
	   block = block | (0x01 << (byte_bit_index + 8)) ;
	   mram_write_16bit_blocks(address-1, &block,1);
	} else{
	   mram_read_16bit_blocks(address,&block,1);
	   block = block | (0x01 << byte_bit_index);
	   mram_write_16bit_blocks(address, &block,1);
	}
	//uint8_t byte = block_availability[block_byte_index] | (0x01 << byte_bit_index);

	//block_availability[block_byte_index] = byte;



}

// crc32 hash function from gcc github repository
uint32_t _ht_string_get_block_number(char *str) {

   uint32_t hash = xcrc32 ((unsigned char *)str, strlen(str), 0xffffffff);
   //uint32_t hash = HAL_CRC_Calculate(&hcrc, (uint32_t *) str, strlen(str));

   return hash % NUM_BLOCKS;
}


//TODO: check key/value size constraints before attempting write
//NOTE: if full rotation of block_number is done, memory is full.
int8_t ht_string_put(HashMapString * hashMap, char *key, char *value){
        char * address;
        uint16_t block_number = _ht_string_get_block_number(key);
        uint8_t done = 0;
        uint8_t block_occupied;
        int8_t result = -1;
        uint32_t counter = 0;
        static int passed = 0;


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
        	address = _ht_string_get_block_address(hashMap->startAddress, block_number);
                block_occupied = _ht_string_is_block_occupied(block_number, hashMap->block_availability);
                if (!block_occupied){

            //Could be done with memcpy for generic solution, but doing strcpy is more efficient for strings.
					#if MRAMEN
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
					_ht_string_occupy_block(block_number, hashMap->block_availability);
					done = 1;
					result = 0;
                } else if ( !strncmp((char *) address, key, FIELD_SIZE)){
					hashMap->replacements++;
					//Replace existing value
					#if MRAMEN
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
                } else{
					hashMap->collisions++;
					block_number = (block_number + 1) % NUM_BLOCKS;
					counter += 1;
                }
        }

        return result;
}

//@ret String equal to the one in the hashtable. Returned string must be freed
char *ht_string_get(HashMapString * hashMap, char *key){
	 	 /*if(!strcmp(key, "?:`T=k2)@7d1!kR")){
	        HAL_Delay(100);
	     }*/
        uint32_t block_number = _ht_string_get_block_number(key);
        char *address;
        uint8_t done = 0;
        char *value = NULL;
        uint32_t counter = 0;
        uint8_t block_occupied = 0;

        while(!done && (counter < NUM_BLOCKS)){
        	 	address = _ht_string_get_block_address(hashMap->startAddress, block_number);
                block_occupied = _ht_string_is_block_occupied(block_number, hashMap->block_availability);
                if( block_occupied && (!strncmp((char *) address, key, FIELD_SIZE))){
#if MRAMEN
                		value = malloc(sizeof(char) * FIELD_SIZE);

                		//HAL_Delay(10*1000);
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


uint32_t ht_string_count_occupied_blocks(uint8_t * block_availability){
        uint32_t counter = 0;
        for(int i = 0; i < 8192; i++){
                uint8_t byte = block_availability[i];
                while(byte){
                        counter += byte & 0x1;
                        byte = byte >> 1;
                }
        }
        return counter;
}

int get_collisions(HashMapString * hashMap){
	return hashMap->collisions;
}

int get_replacements(HashMapString * hashMap){
	return hashMap->replacements;
}

void destroy_hash_map_string(HashMapString * hashMap){
	//free(hashMap);
}

/*void test_put(char *hashTable, uint8_t *block_availability){
    char key[] = "7";
	char value[] = "10";

	//test_hash_function();
	ht_string_put((MEMORY_ADDRESS_SIZE) hashTable, key,value,block_availability);
	printf("Is block ocuppied %hhu\n", ht_string_is_block_occupied(ht_string_get_block_number(key),block_availability));
    printf("%.*s\n", 10, value);
}*/


/*int main (int argc, char *argv[]){
    srand(time(0));
    key_value_store_test_string();
    return 0 ;
}*/

