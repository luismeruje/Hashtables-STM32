/*
 *   File: clht_lb.h
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: lock-based cache-line hash table with no resizing
 *   clht_lb.h is part of ASCYLIB
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

#ifndef _CLHT_LB_H_
#define _CLHT_LB_H_

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "stm32h7xx.h"


#define true 1
#define false 0

#define READ_ONLY_FAIL
#define STATS
#define BUCKET_EXISTS_ERROR 500
#define MRAM_ISSUE_ERROR 500

#if defined(DEBUG)
#  define DPP(x)	x++
#else
#  define DPP(x)
#endif

#define CACHE_LINE_SIZE 32
#define ENTRIES_PER_BUCKET 3
//#define VANILLA_CLHT


#ifndef ALIGNED
#  if __GNUC__ && !SCC
#    define ALIGNED(N) __attribute__ ((aligned (N)))
#  else
#    define ALIGNED(N)
#  endif
#endif

#if defined(__sparc__)
#  define PREFETCHW(x)
#  define PREFETCH(x)
#  define PREFETCHNTA(x)
#  define PREFETCHT0(x)
#  define PREFETCHT1(x)
#  define PREFETCHT2(x)

#  define PAUSE    asm volatile("rd    %%ccr, %%g0\n\t" \
				::: "memory")
#  define _mm_pause() PAUSE
#  define _mm_mfence() __asm__ __volatile__("membar #LoadLoad | #LoadStore | #StoreLoad | #StoreStore");
#  define _mm_lfence() __asm__ __volatile__("membar #LoadLoad | #LoadStore");
#  define _mm_sfence() __asm__ __volatile__("membar #StoreLoad | #StoreStore");


#elif defined(__tile__)
#  define _mm_lfence() arch_atomic_read_barrier()
#  define _mm_sfence() arch_atomic_write_barrier()
#  define _mm_mfence() arch_atomic_full_barrier()
#  define _mm_pause() cycle_relax()
#endif

#define CAS_U64_BOOL(a, b, c) (CAS_U64(a, b, c) == b)
//inline int is_power_of_two(unsigned int x);

typedef uint32_t clht_addr_t;
typedef uint32_t clht_val_t;

//TODO: reducing size may increase performance
typedef uint32_t clht_lock_t;

typedef struct ALIGNED(CACHE_LINE_SIZE) bucket_s
{
  clht_lock_t lock;
  clht_addr_t key[ENTRIES_PER_BUCKET];
  clht_val_t  val[ENTRIES_PER_BUCKET];
  struct bucket_s* next;
} bucket_t;

typedef struct ALIGNED(CACHE_LINE_SIZE) clht
{
  union
  {
    struct
    {
      struct clht_hashtable_s* ht;
      uint8_t next_cache_line[CACHE_LINE_SIZE - (sizeof(void*))];
    };
    uint8_t padding[2 * CACHE_LINE_SIZE];
  };
} clht_t;

typedef struct ALIGNED(CACHE_LINE_SIZE) clht_hashtable_s
{
  union
  {
    struct
    {
      uint32_t num_buckets ;
      bucket_t* table;
    };
    uint8_t padding[1 * CACHE_LINE_SIZE]; //TODO: what is the purpose of this padding?
  };
} clht_hashtable_t;


//TODO: Revise these 2 definitions
#define LOCK_ACQ(lock) *lock = 1;
#define LOCK_RLS(lock) *lock = 0;
/* Create a new hashtable. */
clht_hashtable_t* clht_hashtable_create(uint64_t num_buckets );
clht_t* clht_create(uint64_t num_buckets);

/* Hash a key for a particular hashtable. */
uint32_t clht_hash(clht_hashtable_t* hashtable, clht_addr_t key );

/* Insert a key-value pair into a hashtable. */
int clht_put(clht_t* h, clht_addr_t key, clht_val_t val);

/* Retrieve a key-value pair from a hashtable. */
clht_val_t clht_get(clht_hashtable_t* hashtable, clht_addr_t key);

/* Remove a key-value pair from a hashtable. */
clht_val_t clht_remove(clht_t* hashtable, clht_addr_t key);

/* Dealloc the hashtable */
void clht_destroy(clht_hashtable_t* hashtable);

size_t clht_size(clht_hashtable_t* hashtable);

void clht_print(clht_hashtable_t* hashtable);

bucket_t* clht_bucket_create();

const char* clht_type_desc();

#ifdef DEBUG
void print_debug();
#endif

#ifdef STATS
void print_stats();
#endif

void print_mem_map(clht_hashtable_t* hashtable);

//TODO: print a map that shows how hashtable is allocated within memory.

#endif /* _CLHT_LB_H_ */

