/*
 * settings.h
 *
 *  Created on: Apr 6, 2023
 *      Author: luisferreira
 */

#ifndef INC_SETTINGS_H_
#define INC_SETTINGS_H_

//LPHT settings
//=============
#define STM32 1
#define MRAM 1
#define MMAP 0
#define BILLION  1000000000.0

//Different for 32-bit and 64-bit systems
#define MEMORY_ADDRESS_SIZE uint32_t
//==============

//BENCHMARK Settings
//==================
//For clht, select only 4 byte-sized keys/values.
#define KEYS_FILE "keys-32bytes.h"
#define NUM_RECORDS_TO_INSERT 4096 //25000
#define FIELD_SIZE 32 //Number of bytes for each key/value. Don't forget 1 byte is for \0
//==================

//WARNING: For hashmap_string, must set USABLE_CAPACITY to correct value according to PAIR_SIZE ,i.e. (2 * FIELD_SIZE)

#endif /* INC_SETTINGS_H_ */
