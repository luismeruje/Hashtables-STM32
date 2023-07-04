/*
 * hashmap_string.h
 *
 *  Created on: 08/03/2023
 *      Author: luisferreira
 */

#ifndef INC_HASHMAP_STRING_H_
#define INC_HASHMAP_STRING_H_
#include <stdio.h>

//SETTINGS
//===========
#define MMAP 0
#define MRAMEN 1
//Different for 32-bit and 64-bit systems
#define MEMORY_ADDRESS_SIZE uint32_t

typedef struct hashmapString HashMapString;
//===========

HashMapString * create_hash_map_string();
int8_t ht_string_put(HashMapString * hashMap, char *key, char *value);
char *ht_string_get(HashMapString * hashMap, char *key);
int get_collisions(HashMapString * hashMap);
int get_replacements(HashMapString * hashMap);
void destroy_hash_map_string(HashMapString * hashMap);
#endif /* INC_HASHMAP_STRING_H_ */
