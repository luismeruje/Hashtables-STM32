/*
 * mram_heap.c
 *
 *  Created on: Jun 6, 2023
 *      Author: luisferreira
 */

#include "MRAM_heap.h"
#include "mram_commons.h"
#include "MRAM_driver.h"

static uint32_t current_position = MRAM_BANK_ADDR;

void * memalign_mram(uint16_t alignment, uint32_t nBytes){
	uint32_t address = current_position;
	if(( address + nBytes ) < MRAM_BANK_ADDR_LIMIT_4Mb){
		current_position = (address + nBytes);
		if(current_position % 32){
			current_position += 32 - (current_position % 32);
			//Should also work
			//current_position = (current_position & 0xffffffe0) + 32;
		}
		return (void *) address;
	} else {
		return NULL;
	}
}
