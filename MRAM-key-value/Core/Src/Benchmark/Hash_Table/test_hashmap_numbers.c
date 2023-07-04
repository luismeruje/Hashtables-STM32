/*
 * test_hashmap_numbers.c
 *
 *  Created on: Apr 3, 2023
 *      Author: luisferreira
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hashmap_numbers.h"
#include "stm32h7xx_hal.h"

void test_hashmap_numbers_write_throughput(){

	uint32_t MRAM_startTime = HAL_GetTick();
	uint32_t MRAM_finalTime = 0;
	int32_t result = 0;
	uint32_t value = 0;
	HashMapNumbers * hash = create_hash_map_numbers();

	for (int i=0; i < 4; i++){
		result += hash_map_numbers_put(hash,i,i);
	}

	MRAM_finalTime = HAL_GetTick();
	printf("Done 60 000 puts in: %ld ms\n", MRAM_finalTime - MRAM_startTime);
	//printf("Occupied blocks: %ld\n", _count_occupied_blocks(block_availability));
	printf("Sum of put returned values: %ld\n", result);
	printf("Checking values\n");
	uint32_t good_values = 0;
	for (int i=0; i < 4; i++){
		value = hash_map_numbers_get(hash, i);
		if(value == i){
			good_values++;
		} else{
			printf("Differing value from value stored in hashmap: %lu  (hashmap), %lu (value)\n", value, (uint32_t) i);
			HAL_Delay(10*1000);
		}
	}
	printf("Good values: %ld\n", good_values);



}
