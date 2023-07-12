/*
 * clht_lb_mram.c
 *
 *  Created on: Apr 17, 2023
 *      Author: luisferreira
 */


/*
 *   File: clht_lb.c
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: lock-based cache-line hash table with no resizing
 *   clht_lb.c is part of ASCYLIB
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *	      	      Distributed Programming Lab (LPD), EPFL
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

//WARNING: only one of clht_lb.c and clht_lb_mram.c can be used to compile at each time

#include "clht_lb.h"


#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "settings.h"

#if STM32
#define PRINT_UNSIGNED_FORMAT "%lu"
#include "stm32h7xx_hal.h"
extern CRC_HandleTypeDef hcrc;
#include <malloc.h>
#else
#define PRINT_UNSIGNED_FORMAT "%u"
#include <unistd.h>
#include <stdlib.h>
#endif

#if MRAM
#include "mram_commons.h"
#include "MRAM_driver.h"
#include "MRAM_heap.h"
#include "MRAM_driver.h"
uint32_t zero = 0;
#endif





int clhtBusy = 0;
int clhtHardFault = 0;



#define MRAM_CHECKS

#ifdef DEBUG
 uint32_t put_num_restarts = 0;
 uint32_t put_num_failed_expand = 0;
 uint32_t put_num_failed_on_new = 0;
#endif

#ifdef STATS
 uint32_t numBuckets = 0;
 uint32_t numCollisions = 0;
#endif

#include <assert.h>


const char*
clht_type_desc()
{
  return "CLHT-LB-NO-RESIZE";
}

inline int
is_power_of_two (unsigned int x)
{
return ((x != 0) && !(x & (x - 1)));
}

static inline
int is_odd (int x)
{
    return x & 1;
}

/** Jenkins' hash function for 64-bit integers. */
uint64_t
__ac_Jenkins_hash_64(uint64_t key)
{
    key += ~(key << 32);
    key ^= (key >> 22);
    key += ~(key << 13);
    key ^= (key >> 8);
    key += (key << 3);
    key ^= (key >> 15);
    key += ~(key << 27);
    key ^= (key >> 31);
    return key;
}

/* Create a new bucket. */
bucket_t*
clht_bucket_create()
{

  bucket_t* bucket = NULL;
  #if MRAM
  bucket = memalign_mram(CACHE_LINE_SIZE, sizeof(bucket_t));
  #else
  posix_memalign((void **) &bucket, CACHE_LINE_SIZE, sizeof(bucket_t));
  #endif

#if MRAM
#ifdef MRAM_CHECKS
  if(bucket == NULL){
	  printf("Malloc failed. Probably out of memory.\n");
  } else if((uint32_t) bucket < MRAM_BANK_ADDR || (uint32_t) bucket > MRAM_BANK_ADDR_LIMIT_4Mb){
	  printf("WARNING: Malloc sits outside of MRAM boundaries!! \n");
  }
#endif
#endif
  if (bucket == NULL)
    {
      return NULL;
    }

  bucket->lock = 0;


  uint32_t j;
  for (j = 0; j < ENTRIES_PER_BUCKET; j++)
    {
      #if MRAM
      mram_write_32bit_blocks((uint32_t) &(bucket->key[j]), &zero, 1);
      #else
      bucket->key[j] = 0;
      #endif
    }
    #if MRAM
    mram_write_32bit_blocks((uint32_t)&(bucket->next), &zero, 1);
    #else
    bucket->next = NULL;
    #endif
   
  
  numBuckets++;
#if MRAM
#ifdef MRAM_CHECKS
  uint32_t nextAddress = (uint32_t) bucket->next;
  if((nextAddress != 0) || ((uint32_t) bucket < MRAM_BANK_ADDR) || ( (uint32_t)bucket > MRAM_BANK_ADDR_LIMIT_4Mb)){
	  printf("WARNING: Created bucket where next is %lx and bucket has address %lx\n", nextAddress, (uint32_t) bucket);
  }
#endif
#endif

  return bucket;
}

clht_t*
clht_create(uint64_t num_buckets)
{
  #if MRAM
  clht_t* w = (clht_t*) memalign_mram(CACHE_LINE_SIZE, sizeof(clht_t));
  #else
  clht_t* w;
  posix_memalign((void **) &w, CACHE_LINE_SIZE, sizeof(clht_t));
  #endif
  if (w == NULL)
    {
      printf("** malloc @ hatshtalbe\n");
      return NULL;
    }
  clht_hashtable_t * table = clht_hashtable_create(num_buckets);
  #if MRAM
  mram_write_32bit_blocks((uint32_t) &(w->ht), (void *) &table, 1);
  #else
  w->ht = table;
  #endif
  if (w->ht == NULL)
    {
      free(w);
      return NULL;
    }

  return w;
}

clht_hashtable_t*
clht_hashtable_create(uint64_t num_buckets)
{
 //Global variables for stats
#ifdef STATS
  numBuckets = 0;
  numCollisions = 0;
#endif
  clht_hashtable_t* hashtable = NULL;

  if (num_buckets == 0)
    {
      return NULL;
    }

  /* Allocate the table itself. */
  #if MRAM
  hashtable = (clht_hashtable_t*) memalign_mram(CACHE_LINE_SIZE, sizeof(clht_hashtable_t));
  #else
  posix_memalign((void **) &hashtable, CACHE_LINE_SIZE, sizeof(clht_hashtable_t));
  #endif
  printf("Hashtable base address: %p\n", hashtable);
  if (hashtable == NULL)
    {
      printf("** malloc @ hatshtalbe\n");
      return NULL;
    }

  /* hashtable->table = calloc(num_buckets, (sizeof(bucket_t))); */
  #if MRAM
  void * pointer = memalign_mram(CACHE_LINE_SIZE, num_buckets * (sizeof(bucket_t)));
  printf("Table base address: %p\n", pointer);
  mram_write_32bit_blocks((uint32_t)  &hashtable->table, &pointer, 1);
  printf("Stored table base address: %p\n", hashtable->table);
  #else
  posix_memalign( (void **) &hashtable->table, CACHE_LINE_SIZE, num_buckets * (sizeof(bucket_t)));
  #endif

  if (hashtable->table == NULL)
    {
      printf("** alloc: hashtable->table\n"); fflush(stdout);
#if !MRAM
      free(hashtable);
#endif
      return NULL;
    }

#if MRAM
  for(int i = 0; i < ((num_buckets * (sizeof(bucket_t))) / 32); i++){
	  mram_write_32bit_blocks((uint32_t)(hashtable->table) + (i*32), &zero, 1);
  }

  uint64_t i;
  for (i = 0; i < num_buckets; i++)
    {
	  mram_write_32bit_blocks((uint32_t) &(hashtable->table[i].lock), &zero,1);
	  mram_write_32bit_blocks((uint32_t) &(hashtable->table[i].next), &zero,1);
#ifdef MRAM_CHECKS
      if(hashtable->table[i].next != NULL){
    	  printf("On table creation, one bucket's next pointer is not NULL\n");
      }
#endif
      uint32_t j;
      for (j = 0; j < ENTRIES_PER_BUCKET; j++)
	{
      mram_write_32bit_blocks((uint32_t) &hashtable->table[i].key[j], &zero, 1);
	}
   }
#else
  memset(hashtable->table, 0, num_buckets * (sizeof(bucket_t)));

  uint64_t i;
  for (i = 0; i < num_buckets; i++)
    {
      hashtable->table[i].lock = 0;
      hashtable->table[i].next = NULL;
      uint32_t j;
      for (j = 0; j < ENTRIES_PER_BUCKET; j++)
	{
	  hashtable->table[i].key[j] = 0;
	}
    }
  #endif
  hashtable->num_buckets = num_buckets;


#ifdef STATS
  //Global variable
  numBuckets += num_buckets;
#endif

  return hashtable;
}

/* Hash a key for a particular hash table. */
//TODO: try arm-v7 SHA3 or similar native hashing instructions(?)
uint32_t
clht_hash(clht_hashtable_t* hashtable, clht_addr_t key)
{
	uint32_t hashval;
	hashval = (uint32_t)__ac_Jenkins_hash_64(key);
	return hashval % (hashtable->num_buckets - 1);

	//Castagnoli CRC32, as per: https://community.st.com/s/question/0D50X00009Xkf9BSAR/bytewise-only-crc-for-stm32l486
	/*uint32_t hash = (HAL_CRC_Calculate(&hcrc, (uint32_t *) &key, 1));// ^ 0xFFFFFFFF;  //Final XOR does not seem to make much of a difference, but true Castagnoli requires this, reverse output data, and reverse input data by byte, which I am not using.
    return hash % hashtable->num_buckets;*/
  /* return key % hashtable->num_buckets; */
  /* return key & (hashtable->num_buckets - 1); */

}


  /* Retrieve a key-value entry from a hash table. */
clht_val_t
clht_get(clht_hashtable_t* hashtable, clht_addr_t key)
{
  size_t bin = clht_hash(hashtable, key);
  volatile bucket_t* bucket = hashtable->table + bin;


  uint32_t j;
  do
    {
      for (j = 0; j < ENTRIES_PER_BUCKET; j++)
	{
    #if MRAM
	  __IO clht_val_t val = bucket->val[j];
    #else
    clht_val_t val = bucket->val[j];
    #endif
#ifdef __tile__
	  _mm_lfence();
#endif
	  if (bucket->key[j] == key)
	    {
	      if (bucket->val[j] == val) //?
		{
		  return val;
		}
	      else
		{
		  return 0;
		}
	    }
	}

      bucket = bucket->next;
    }
  while (bucket != NULL);
  return 0;
}

//Was inline previously
int
bucket_exists(bucket_t* bucket, clht_addr_t key)
{
  uint32_t j;
  do
    {
      for (j = 0; j < ENTRIES_PER_BUCKET; j++)
	{
      //printf("Key address in hashtable: %lx\n", (uint32_t *) &bucket->key[j]);
	  if (bucket->key[j] == key)
	    {
	      return true;
	    }
	}
  #if MRAM
  #ifdef MRAM_CHECKS
      uint32_t nextAddress = (uint32_t)bucket->next;
      if(nextAddress != 0 && ((nextAddress < MRAM_BANK_ADDR) || ( nextAddress > MRAM_BANK_ADDR_LIMIT_4Mb))){
    	  //printf("Bucket's next pointer has an illegal value of %lx\n Current bucket has address %lx\n", nextAddress, (uint32_t) bucket);
    	  return BUCKET_EXISTS_ERROR; //Error code
    	  /*HAL_Delay(10*1000);
    	  nextAddress = (uint32_t)bucket->next;
    	  if(nextAddress != 0 && ((nextAddress < MRAM_BANK_ADDR) || ( nextAddress > MRAM_BANK_ADDR_LIMIT_4Mb))){
    	     printf("Bucket's next pointer still has an illegal value on second try %lx\n Current bucket has address %lx\n", nextAddress, (uint32_t) bucket);
    	  }*/
      }
  #endif
  #endif


      bucket = bucket->next;
    } while (bucket != NULL);
  return false;
}


/* Insert a key-value entry into a hash table. */
int
clht_put(clht_t* h, clht_addr_t key, clht_val_t val)
{

  // Get hashtable address
  clht_hashtable_t* hashtable;
  #if MRAM
  mram_read_16bit_blocks((uint32_t) h, &hashtable, 2);
  #else
  hashtable = h->ht;
  #endif
  uint32_t bin = clht_hash(hashtable, key);
  //Read first bin address
  //uint32_t tableAddress = ;
  //mram_read_16bit_blocks((((uint32_t) hashtable) + 4), &tableAddress, 2);

  bucket_t* bucket = hashtable->table + bin;

  /*if ((uint32_t) bucket > MRAM_BANK_ADDR_LIMIT_4Mb - 32 || (uint32_t) bucket < MRAM_BANK_ADDR){
	  printf("Bug. Hash address: %lx, Bin number: %lu, size of bucket %d. Bucket address: %lx, new calculation: %lx \n", tableAddress, bin, sizeof(bucket_t), (uint32_t) bucket, (uint32_t) (tableAddress + (bin * (uint32_t) sizeof(bucket_t))));
	  return false;
  }*/

  clhtBusy = 1;
#if defined(READ_ONLY_FAIL)
  int result = bucket_exists(bucket, key);
  if (result == true)
    {
      return false;
   } else if (result == BUCKET_EXISTS_ERROR){
	   return MRAM_ISSUE_ERROR;
   }
#endif
  clht_lock_t* lock = &bucket->lock;

  clht_addr_t* empty = NULL;
  clht_val_t* empty_v = NULL;

  uint32_t j;

  //TODO: implement actual locks or switch for locks in arm-m. Original included wait mechanisms.
  LOCK_ACQ(lock);
  do
    {
      for (j = 0; j < ENTRIES_PER_BUCKET; j++)
	{

	  if (bucket->key[j] == key)
	    {
	      LOCK_RLS(lock);
	      return false;
	    }
	  else if (empty == NULL && bucket->key[j] == 0)
	    {
	      empty = &bucket->key[j];
	      empty_v = &bucket->val[j];
	      break;
	    }
	  else{
	   numCollisions++;
	  }
	}

      if (bucket->next == NULL)
	{
	  if (empty == NULL)
	    {
	      DPP(put_num_failed_expand);
        #if MRAM
	      bucket_t * new_bucket = clht_bucket_create();
	      mram_write_32bit_blocks((uint32_t) &bucket->next, &new_bucket, 1);
	      mram_write_32bit_blocks((uint32_t) &bucket->next->key[0], &key, 1);
        #else
        bucket->next = clht_bucket_create();
	      bucket->next->key[0] = key;
        #endif
#ifdef __tile__
	      _mm_sfence();
#endif
        #if MRAM
	      mram_write_32bit_blocks((uint32_t) &bucket->next->val[0], &val, 1);
        #else
        bucket->next->val[0] = val;
        #endif
	    }
	  else
	    {
        #if MRAM
	      mram_write_32bit_blocks((uint32_t) empty_v, &val, 1);
        #else
        *empty_v = val;
        #endif
#ifdef __tile__
	      _mm_sfence();
#endif
        #if MRAM
	      mram_write_32bit_blocks((uint32_t) empty, &key, 1);
        #else
        *empty = key;
        #endif
	    }

	  LOCK_RLS(lock);
	  return true;
	}
      if(clhtHardFault){
    	  clhtHardFault = 0;
    	  clhtBusy = 0;
    	  LOCK_RLS(lock);
    	  return MRAM_ISSUE_ERROR;
      }

      bucket = bucket->next;
    } while (true);
}



/* Remove a key-value entry from a hash table. */
clht_val_t
clht_remove(clht_t* h, clht_addr_t key)
{
  clht_hashtable_t* hashtable = h->ht;
  size_t bin = clht_hash(hashtable, key);
  bucket_t* bucket = hashtable->table + bin;

#if defined(READ_ONLY_FAIL)
  if (!bucket_exists(bucket, key))
    {
      return false;
    }
#endif  /* READ_ONLY_FAIL */

  clht_lock_t* lock = &bucket->lock;
  uint32_t j;

  LOCK_ACQ(lock);
  do
    {
      for (j = 0; j < ENTRIES_PER_BUCKET; j++)
	{
	  if (bucket->key[j] == key)
	    {
	      clht_val_t val = bucket->val[j];
	      bucket->key[j] = 0;
	      LOCK_RLS(lock);
	      return val;
	    }
	}
      bucket = bucket->next;
    } while (bucket != NULL);
  LOCK_RLS(lock);
  return false;
}

static uint32_t
clht_put_seq(clht_hashtable_t* hashtable, clht_addr_t key, clht_val_t val, uint64_t bin)
{
  bucket_t* bucket = hashtable->table + bin;
  clht_addr_t* empty = NULL;
  clht_val_t* empty_v = NULL;
  uint32_t j;

  do
    {
      for (j = 0; j < ENTRIES_PER_BUCKET; j++)
	{
	  if (bucket->key[j] == key)
	    {
	      return false;
	    }
	  else if (empty == NULL && bucket->key[j] == 0)
	    {
	      empty = &bucket->key[j];
	      empty_v = &bucket->val[j];
	    }
	}

      if (bucket->next == NULL)
	{
	  if (empty == NULL)
	    {
	      DPP(put_num_failed_expand);
	      bucket->next = clht_bucket_create();
	      bucket->next->key[0] = key;
	      bucket->next->val[0] = val;
	    }
	  else
	    {
	      *empty_v = val;
	      *empty = key;
	    }
	  return true;
	}

      bucket = bucket->next;
    } while (true);
}


static inline void
bucket_cpy(bucket_t* bucket, clht_hashtable_t* ht_new)
{
  LOCK_ACQ(&bucket->lock);
  uint32_t j;
  do
    {
      for (j = 0; j < ENTRIES_PER_BUCKET; j++)
	{
	  clht_addr_t key = bucket->key[j];
	  if (key != 0)
	    {
	      uint64_t bin = clht_hash(ht_new, key);
	      clht_val_t val = bucket->key[j];
	      clht_put_seq(ht_new, key, val, bin);
	    }
	}
      bucket = bucket->next;
    } while (bucket != NULL);

}

//TODO: does not delete buckets
void
clht_destroy(clht_hashtable_t* hashtable)
{
	bucket_t * bucket = NULL, * aux = NULL;
  for(int i = 0; i < hashtable->num_buckets; i++){
	  bucket = hashtable->table[i].next;
    #if MRAM
	  while(bucket != NULL && (uint32_t) bucket < MRAM_BANK_ADDR_LIMIT_4Mb && (uint32_t) bucket > MRAM_BANK_ADDR){
    #else
    while(bucket != NULL){
    #endif
		 aux = bucket;
		 bucket = aux->next;
		 free(aux);
	  }
  }
  free(hashtable->table);
  free(hashtable);
}



size_t
clht_size(clht_hashtable_t* hashtable)
{
  uint64_t num_buckets = hashtable->num_buckets;
  bucket_t* bucket = NULL;
  size_t size = 0;

  uint64_t bin;
  for (bin = 0; bin < num_buckets; bin++)
    {
      bucket = hashtable->table + bin;

      uint32_t j;
      do
	{
	  for (j = 0; j < ENTRIES_PER_BUCKET; j++)
	    {
	      if (bucket->key[j] > 0)
		{
		  size++;
		}
	    }

	  bucket = bucket->next;
	}
      while (bucket != NULL);
    }
  return size;
}

void
clht_print(clht_hashtable_t* hashtable)
{
  uint64_t num_buckets = hashtable->num_buckets;
  bucket_t* bucket;

  printf("Number of buckets: %llu\n", num_buckets);

  uint64_t bin;
  for (bin = 0; bin < num_buckets; bin++)
    {
      bucket = hashtable->table + bin;

      printf("[[%05llu]] ", bin);

      uint32_t j;
      do
	{
	  for (j = 0; j < ENTRIES_PER_BUCKET; j++)
	    {
	      if (bucket->key[j])
	      	{
		  printf("(%-5llu)-> ", (long long unsigned int) bucket->key[j]);
		}
	    }

	  bucket = bucket->next;
	  printf(" ** -> ");
	}
      while (bucket != NULL);
      printf("\n");
    }
  fflush(stdout);
}

void _map_occupy_32byte(uint8_t * memoryMap, uint32_t startAddress, uint32_t baseAddress){
	uint32_t index = (startAddress - baseAddress) / 32;
	uint8_t aligned = !((startAddress - baseAddress) % 32);

	if((memoryMap[index] == '-') && aligned){
		memoryMap[index] = '*'; // '*' character
	} else if ((memoryMap[index] == '-') &&  !aligned){
		memoryMap[index] = '/'; // '/' bucket targets this address, but unaligned, should not happen
	} else {
		memoryMap[index] = 'X';// 'X' -> Targeted by more than one bucket, should not happen
	}



}

#if STM32
//All buckets should be 32byte aligned
void print_mem_map(clht_hashtable_t* hashtable){
	bucket_t * bucket ;
	uint8_t memoryMap[NUM_BYTES_CAPACITY/32];

	for(int i = 0; i < NUM_BYTES_CAPACITY/32; i++){
		memoryMap[i] = '-';
	}

	for(int i = 0; i < hashtable->num_buckets; i++){
		_map_occupy_32byte(memoryMap, (uint32_t) &hashtable->table[i], (uint32_t)hashtable);
		bucket = hashtable->table[i].next;
		while(bucket != NULL){
			_map_occupy_32byte(memoryMap, (uint32_t)bucket, (uint32_t)hashtable);
			bucket = bucket->next;
		}
	}

	for(int i = 0; i < NUM_BYTES_CAPACITY/32; i++){
		printf("%c", memoryMap[i]);
		if((i != 0) && (i % 100 == 0)){
			printf("\n");
      #if STM32
			HAL_Delay(500);
      #else
      sleep(500);
      #endif
		}

	}
	printf("\n");

}
#endif

#ifdef DEBUG
void print_debug(){
    printf("Debug stats.\n \tPut_num_restarts: "PRINT_UNSIGNED_FORMAT";\n\tPut_num_failed_expand: "PRINT_UNSIGNED_FORMAT";\n\tPut_num_failed_on_new: "PRINT_UNSIGNED_FORMAT";\n", put_num_restarts, put_num_failed_expand, put_num_failed_on_new);
}
#endif

#ifdef STATS
void print_stats(){
	printf("Number of buckets: " PRINT_UNSIGNED_FORMAT "\n Number of collisions: "PRINT_UNSIGNED_FORMAT"\n", numBuckets, numCollisions);
}
#endif

