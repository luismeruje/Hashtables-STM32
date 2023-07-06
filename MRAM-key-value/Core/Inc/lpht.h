/*
 * hashmap_string.h
 *
 *  Created on: 08/03/2023
 *      Author: luisferreira
 */

#ifndef INC_HASHMAP_STRING_H_
#define INC_HASHMAP_STRING_H_
#include <stdio.h>
#include "settings.h"
#if !MRAM
#include <stdint.h>
#endif
typedef struct lpht LPHT;


LPHT * create_lpht();
int8_t lpht_put(LPHT * hashMap, char *key, char *value);
char *lpht_get(LPHT * hashMap, char *key);
uint32_t lpht_get_collisions(LPHT * hashMap);
uint32_t lpht_get_replacements(LPHT * hashMap);
void destroy_lpht(LPHT * hashMap);
#endif /* INC_HASHMAP_STRING_H_ */
