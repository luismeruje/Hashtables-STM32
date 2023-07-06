/*
 * test_hashmap_string.c
 *
 *  Created on: Apr 4, 2023
 *      Author: luisferreira
 */
#include "benchmark_lpht.h"
#include "lpht.h"
#include "settings.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "settings.h"

#if STM32
#include "stm32h7xx_hal.h"
extern char keys[NUM_RECORDS_TO_INSERT][FIELD_SIZE];
#else
#include <time.h>
#include KEYS_FILE
#endif




//Time count from: https://stackoverflow.com/questions/66241806/calculate-the-execution-time-of-program-in-c
void benchmark_lpht_throughput(){
	int keysNotFound = 0, correctValues = 0;
	char * value;
#if STM32
	int startTime = 0, elapsedTime = 0;
#else
	struct timespec start, end;
	double time_spent;
#endif


//char keys [NUM_RECORDS_TO_INSERT][FIELD_SIZE];

//Generate random values
//    for (int i = 0; i < NUM_RECORDS_TO_INSERT ; i++){
//        for (int j = 0; j < FIELD_SIZE-1; j++){
//            //Random char between 48 and 122 ASCII decimal
//            keys[i][j] = (char) (rand() % 93) + 33;
//        }
//        keys[i][FIELD_SIZE-1] = '\0';
//        //ht_string_put((uint32_t) hashTable, key, key, block_availability);
//    }


//====== Write speed===========
printf("Keys pointer: %p\n", keys);
printf("First string pointer: %p\n", keys[0]);
printf("First string: %s\n", keys[0]);
LPHT * hashMap = create_lpht();
#if STM32
    startTime = HAL_GetTick();
#else
    clock_gettime(CLOCK_REALTIME, &start);
#endif


    //Insert values
    for(int i = 0; i < NUM_RECORDS_TO_INSERT ; i++){
        lpht_put(hashMap, keys[i], keys[i]);
        //printf("Num_collisions: %lu\n", lpht_get_collisions(hashMap));
    }
#if STM32
    elapsedTime = HAL_GetTick() - startTime;
    printf("Took %d ms to insert %d records \n", elapsedTime, NUM_RECORDS_TO_INSERT);
	printf("Number of collisions %lu\n Number of replacements %lu \n", lpht_get_collisions(hashMap), lpht_get_replacements(hashMap));
#else
    clock_gettime(CLOCK_REALTIME, &end);
	time_spent = (end.tv_sec - start.tv_sec) +
							(end.tv_nsec - start.tv_nsec) / BILLION;
	printf("Took %lf to insert %d records\n", time_spent, NUM_RECORDS_TO_INSERT);
	printf("Number of collisions %u\n Number of replacements %u \n", lpht_get_collisions(hashMap), lpht_get_replacements(hashMap));
#endif


//====== Read speed===========
#if STM32
    startTime = HAL_GetTick();
#else
    clock_gettime(CLOCK_REALTIME, &start);
#endif


	for(int i = 0; i < NUM_RECORDS_TO_INSERT ; i++){
		value = lpht_get(hashMap, keys[i]);
		if(value != NULL){
			free(value);
		}
	}

#if STM32
    elapsedTime = HAL_GetTick() - startTime;
    printf("Took %d ms to read %d records \n", elapsedTime, NUM_RECORDS_TO_INSERT);

#else
    clock_gettime(CLOCK_REALTIME, &end);
	time_spent = (end.tv_sec - start.tv_sec) +
							(end.tv_nsec - start.tv_nsec) / BILLION;
	printf("Took %lf to read %d records\n", time_spent, NUM_RECORDS_TO_INSERT);
#endif

//========= Consistency check ========

	for(int i = 0; i < NUM_RECORDS_TO_INSERT ; i++){
		value = lpht_get(hashMap, keys[i]);
		if (value == NULL){
			keysNotFound++;
			printf("Key %s not found \n", keys[i]);
		} else if(!strcmp(value, keys[i])){
			correctValues++;
		} else{
			printf("Error in retrieved string hashtable value: %s (value)\n%s (expected). Index: %d\n", value, keys[i], i);
			//HAL_Delay(10*1000);
		}
		if(value != NULL){
			free(value);
		}
	}

	destroy_lpht(hashMap);

	printf("Consistency check.\n \tKeys not found: %d\n \tNon-matching values: %d\n", keysNotFound, (NUM_RECORDS_TO_INSERT - correctValues - keysNotFound));

}


#if STM32 == 0
int main (int argc, char ** argv){
	benchmark_lpht_throughput();
}
#endif
