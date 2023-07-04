/*
 * settings.h
 *
 *  Created on: Apr 6, 2023
 *      Author: luisferreira
 */

#ifndef INC_SETTINGS_H_
#define INC_SETTINGS_H_

#define MRAMEN 1
//For clht, select only 4 byte-sized keys/values.
#define KEYS_FILE "keys-64bytes.h"
#define NUM_RECORDS_TO_INSERT 2048 //25000
#define FIELD_SIZE 64 //Number of bytes for each key/value. Don't forget 1 byte is for \0

//WARNING: For hashmap_string, must set USABLE_CAPACITY to correct value according to PAIR_SIZE ,i.e. (2 * FIELD_SIZE)

#endif /* INC_SETTINGS_H_ */
