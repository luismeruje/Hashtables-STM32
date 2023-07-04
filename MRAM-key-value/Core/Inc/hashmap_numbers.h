/*
 * hashmap.h
 *
 *  Created on: 03/03/2023
 *      Author: luisferreira
 */

#ifndef SRC_HASHMAP_H_
#define SRC_HASHMAP_H_

#include <stdint.h>

typedef struct hashMapNumbers HashMapNumbers;

HashMapNumbers * create_hash_map_numbers();
uint32_t hash_map_numbers_get(HashMapNumbers * hash, uint32_t key);
int8_t hash_map_numbers_put(HashMapNumbers * hash, uint32_t key, uint32_t value);


#endif /* SRC_HASHMAP_H_ */
