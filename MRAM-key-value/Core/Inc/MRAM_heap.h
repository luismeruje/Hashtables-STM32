/*
 * MRAM_heap.h
 *
 *  Created on: Jun 6, 2023
 *      Author: luisferreira
 */


#ifndef INC_MRAM_HEAP_H_
#define INC_MRAM_HEAP_H_
#include <stdio.h>

void * memalign_mram(uint16_t alignment, uint32_t nBytes);

#endif /* INC_MRAM_HEAP_H_ */
