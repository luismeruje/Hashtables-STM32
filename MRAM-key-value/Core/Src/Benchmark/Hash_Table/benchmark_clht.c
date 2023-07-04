/*
 * benchmark_clht.c
 *
 *  Created on: Apr 6, 2023
 *      Author: luisferreira
 */



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hashmap_numbers.h"
#include "stm32h7xx_hal.h"
#include "clht_lb.h"
#include "settings.h"
#include "mram_commons.h"

extern char keys[NUM_RECORDS_TO_INSERT][FIELD_SIZE];
#define INITIAL_SIZE_PERCENTAGE 2

/*TODO:
 * 	Test : Amount of data? (50% of memory size, as before, is not possible, due to malloc issue)
 * 		   10%, 20%, 30% maybe?
 * 		   Average over the 5 keys sets.
 *
 * 	 Compare: Nr of collisions, write throughput, read throughput, effective storage space
 *
 * 	 Other: Test writing, reflashing just with reads and then turning off/on to see if data is lost.
 *
 * */


//Time count from: https://stackoverflow.com/questions/66241806/calculate-the-execution-time-of-program-in-c
void benchmark_clht_write_throughput(){
	uint32_t failedPutsError = 0;
	uint32_t failedPutsValueAlreadyExists = 0;
	int keysNotFound = 0, correctValues = 0;
	char * value;
	uint64_t storedValue;
#if MRAMEN
	int startTime = 0, elapsedTime = 0;
	//TODO: For the case of clht, uint64_t would make more sense as the value type

#else
	struct timespec start, end;
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

    printf("Example value: %s\n", keys[0]);
    clht_t *hash = clht_create(INITIAL_SIZE_PERCENTAGE * (NUM_RECORDS_TO_INSERT/3));



//====================== WRITE THROUGHPUT ======================
#if MRAMEN
    startTime = HAL_GetTick();
#else
    clock_gettime(CLOCK_REALTIME, &start);
#endif

    //uint64_t a  = ((clht_addr_t) 8) & (hash->ht->num_buckets - 1);
    //uint32_t key;
    int result = 0;
    for(int i = 0; i < NUM_RECORDS_TO_INSERT ; i++){
    	//key = rand();
    	result = clht_put(hash, *(clht_addr_t *) keys[i], *(clht_val_t *) keys[i]);
		#ifdef DEBUG
            //print_debug();
        #endif
    	if (result == false){
    		failedPutsValueAlreadyExists++;
    	} else if(result == MRAM_ISSUE_ERROR){
    		failedPutsError++;
    	}
    }
    printf("Put operation duplicates: %lu\nMram errors: %lu\n", failedPutsValueAlreadyExists, failedPutsError);
#if MRAMEN
    elapsedTime = HAL_GetTick() - startTime;
    printf("Took %d ms to insert %d records \n", elapsedTime, NUM_RECORDS_TO_INSERT);
    //printf("Number of collisions %d\n Number of replacements %d \n", get_collisions(hashMap), get_replacements(hashMap));
#else
    clock_gettime(CLOCK_REALTIME, &end);
        double time_spent = (end.tv_sec - start.tv_sec) +
                                (end.tv_nsec - start.tv_nsec) / BILLION;
        printf("Took %lf to insert %d records\n", time_spent, NUM_RECORDS_TO_INSERT);
#endif

//====================== READ THROUGHPUT ======================
#if MRAMEN
    startTime = HAL_GetTick();
#else
    clock_gettime(CLOCK_REALTIME, &start);
#endif

    for(int i = 0; i < NUM_RECORDS_TO_INSERT ; i++){
		storedValue = clht_get(hash->ht, * (clht_addr_t *) keys[i]);
	}

#if MRAMEN
    elapsedTime = HAL_GetTick() - startTime;
    printf("Took %d ms to read %d records \n", elapsedTime, NUM_RECORDS_TO_INSERT);
    //printf("Number of collisions %d\n Number of replacements %d \n", get_collisions(hashMap), get_replacements(hashMap));
#else
    clock_gettime(CLOCK_REALTIME, &end);
        double time_spent = (end.tv_sec - start.tv_sec) +
                                (end.tv_nsec - start.tv_nsec) / BILLION;
        printf("Took %lf to read %d records\n", time_spent, NUM_RECORDS_TO_INSERT);
#endif


//====================== CONSISTENCY CHECK ======================
	for(int i = 0; i < NUM_RECORDS_TO_INSERT ; i++){
		storedValue = clht_get(hash->ht, * (clht_addr_t *) keys[i]);
		value = (char *) &storedValue;
		if (storedValue == 0){
			keysNotFound++;
		} else if(!strcmp(value, keys[i])){
			correctValues++;
		} else{
			printf("Error in retrieved string hashtable value: %s (value)\n%s (expected)\n", value, keys[i]);
			//HAL_Delay(10*1000);
		}
	}
	//print_mem_map(hash->ht);

	print_stats();
	//clht_destroy(hash->ht);

	printf("Consistency check.\n \tKeys not found: %d\n \tNon-matching values: %d\n", keysNotFound, (NUM_RECORDS_TO_INSERT - correctValues - keysNotFound));

}

