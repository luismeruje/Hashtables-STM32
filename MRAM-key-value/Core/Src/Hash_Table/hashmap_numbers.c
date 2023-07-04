/*
 * hashmap.c
 *
 *  Created on: 03/03/2023
 *      Author: luisferreira
 */

#include "hashmap_numbers.h"
#include "mram_commons.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//TODO: Has an error in implementation. Get key 0 is returning value of key 1.
//SETTINGS
//===========
#define FIELD_SIZE 2 //Number of bytes for each key/value
//===========

#define KEY_VALUE_PAIR_SIZE  (2 * FIELD_SIZE)
// ############## 32byte (i.e. 4 key-value pairs) block functions ##############

typedef struct hashMapNumbers{
	uint8_t block_availability[8192];
} HashMapNumbers;


HashMapNumbers * create_hash_map_numbers(){
    //At the beginning all blocks are available, set to 0
	//TODO: block_availability should also be stored on MRAM
	HashMapNumbers * hashMap = (HashMapNumbers *) malloc(sizeof(HashMapNumbers));
	memset(hashMap->block_availability, 0, sizeof(hashMap->block_availability));
	return hashMap;
}

// 1 BLOCK <=> 1 record <=> 1 key-value pair. I.e., buckets only hold one key-value pair
uint32_t _get_block_address(uint16_t block_number){
	//8-bit per address memory
	uint32_t address = MRAM_BANK_ADDR + (KEY_VALUE_PAIR_SIZE * block_number);
	return address;
}

uint16_t _get_block_number(uint32_t key){
	//Hash function from key to block number. Hash simply uses keys last 16 bits
	uint16_t block_number = 0;
	block_number = (uint16_t) (key & 0xFFFF) ;
	return block_number;
}

uint32_t _is_block_occupied(uint16_t block_number, uint8_t * block_availability){
	uint16_t block_byte_index = block_number / 8;
	uint8_t byte_bit_index = block_number % 8;
	uint8_t occupied = (block_availability[block_byte_index] >> byte_bit_index) & 0x01;
	return occupied;

}

void _occupy_block(uint16_t block_number, uint8_t * block_availability){
	uint16_t block_byte_index = block_number / 8;
	uint8_t byte_bit_index = block_number % 8;
	block_availability[block_byte_index] = block_availability[block_byte_index] | (0x01 << byte_bit_index);
}

//TODO: if full rotation of block_number is done, memory is full.
int8_t hash_map_numbers_put(HashMapNumbers * hash, uint32_t key, uint32_t value){
	uint32_t address;
	uint16_t block_number = _get_block_number(key);
	uint8_t done = 0;
	uint8_t block_occupied;
	int8_t result = -1;
	uint32_t counter = 0;

	while(!done && counter < 65536){
		address = _get_block_address(block_number);
		block_occupied = _is_block_occupied(block_number, hash->block_availability);
		if (!block_occupied){
			*(uint32_t *) address = key;
			*(uint32_t *) (address + 4) = value;
			_occupy_block(block_number, hash->block_availability);
			done = 1;
			result = 0;
		} else if ( *(uint32_t *) address == key){
			//Replace existing value
			*(uint32_t *) (address + 4) = value;
			done = 1;
			result = 1;
		} else{
			block_number = (block_number + 1) % 0xFFFF;
			counter += 1;
		}
	}
	printf("Put value %lu on address %lu with key %lu\n", value, address, key);

	return result;
}

uint32_t hash_map_numbers_get(HashMapNumbers * hash, uint32_t key){
	uint16_t block_number = _get_block_number(key);
	uint32_t address = _get_block_address(block_number);
	uint8_t done = 0;
	uint32_t value;
	uint32_t counter = 0;
	uint8_t block_occupied = 0;

	value = 123456789;

	while(!done && (counter < 65536)){
		block_occupied = _is_block_occupied(block_number, hash->block_availability);
		if( block_occupied && ( *(uint32_t *) (address) == key )){
			value = *(uint32_t *) (address+4);
			done = 1;
		} else if(block_occupied){
			address += 8;
			block_number += 1;
			counter += 1;
		} else {
			counter = 65536;
		}
	}

	printf("Got value %lu from address %lu from key %lu, key at address: %lu\n", value, address, key, *(uint32_t *) (address));

	return value;
}

uint32_t _count_occupied_blocks(uint8_t * block_availability){
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

void test_hash_function(){
	//Test hash function:

	uint32_t key = rand() & 0xFF;
	key |= (rand() & 0xFF) << 8 ;
	key |= (rand() & 0xFF) << 18;
	key |= (rand() & 0xFF) << 24;

	uint16_t block_number = _get_block_number(key);
	printf("Block number: %d\n", block_number);
}
