# Cache Line Hash Table
The Cache Line Hash Table (CLHT) is organized into buckets, where each bucket contains a set of key/value pairs, a lock, and a pointer to the next bucket. Keys are hashed into buckets, which when full add a new bucket in a linked list configuration. CLHT leverages the CPU's cache lines to improve performance by matching the bucket's size to the CPU's cache line size. 

This CLHT implementation is an adaptation from [LPD-EPFL's implementation](https://github.com/LPD-EPFL/CLHT) (lock-based variant). Since STM32 devices are predominantly single-threaded we disabled the locking mechanism.

The Hash Table is available both as an STM32CubeIDE project (`MRAM-key-value` folder) and as standalone C modules that you can use to integrate into your own project (`CLHT-standalone` folder). Furthermore, we provide a simple benchmark that showcases basic usage and allows for performance evaluation of the Hash Table.

### Basic usage
To use the CLHT you only need the files contained in the `CLHT-standalone` folder. The basic workflow involves creating a new Hash Table and then performing put/get operations over it. For example:

```c++
#include "clht.h"

int main(void){
    initialNumBuckets = 100;
    clht_t *hash = clht_create(initialNumBuckets);
    int result = 0;
    uint64_t value = 3214, returned_value = 0;
    result = clht_put(hash, value, value);
    returned_value = clht_get(hash, value);
    printf("Put value: %lu\nGet value: %lu\n", value, returned_value);
}   
```

### CLHT settings description
The CLHT Hash Table and benchmark can be controlled through a series of settings available in either `CLHT-standalone/inc/settings.h` for the standalone version or `MRAM-key-value/Core/Inc/settings/settings.h` for the STM32CubeIDE. It looks as follows:

```c++
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
#define KEYS_FILE "keys-4bytes.h"
#define NUM_RECORDS_TO_INSERT 4096 //25000
#define FIELD_SIZE 4 //Number of bytes for each key/value. Don't forget 1 byte is for \0

#define NUM_BYTES_CAPACITY 524288
//==================
```

The `STM32` and `MRAM` settings should be set to 0 when running on UNIX systems. The first setting determines whether we are running on an STM32 device or not, and the second enables storage in an external MRAM memory device connected to an STM32. The `MMAP` option is not used for CLHT.

The `MEMORY_ADDRESS_SIZE` should be set to the size of an address in your system, i.e., the architecture type of your CPU. Most current UNIX systems use 64-bit CPUs, while the STM32 is 32-bit.

The `KEYS_FILE` indicates the file with the dataset to be used; `NUM_RECORDS_TO_INSERT` states the number of keys to insert, and should not be greater than the size of the dataset (see the [README](./README.md)); and `FIELD_SIZE` indicates the size of the keys in bytes. For the case of CLHT, the dataset and key-size should be set to 4 bytes. 

`NUM_BYTES_CAPACITY` sets the total amount of space available to the CLHT.

In `CLHT-standalone/inc/clht_lb.h` for the standalone version or `MRAM-key-value/Core/Inc/clht_lb.h` for the STM32CubeIDE project you can set the cache line size and number of key/value pairs per bucket. The default values are set for keys and values with 4 bytes each, and a cache line size of 32 bytes.

```c++
#define CACHE_LINE_SIZE 32
#define ENTRIES_PER_BUCKET 3

[...]

typedef struct ALIGNED(CACHE_LINE_SIZE) bucket_s
{
  clht_lock_t lock; //32-bit each
  clht_addr_t key[ENTRIES_PER_BUCKET]; //32-bit each
  clht_val_t  val[ENTRIES_PER_BUCKET]; //32-bit each
  struct bucket_s* next; //64-bit or 32-bit depending on platform
} bucket_t;
```

The `CACHE_LINE_SIZE`, as the name implies, sets the cache line size for the CPU being used, and the `ENTRIES_PER_BUCKET` sets the number of key/value pairs per bucket. If you want to adjust the CLHT for a system with a different cache size then you must take into account both of these parameters and the parameters in `settings.h`. It can also help to look at how the bucket's structure, `bucket_t`, is defined. Each bucket should fit into a single cache line.

## CLHT benchmarking
The benchmark for CLHT is available at `MRAM-key-value/Core/Src/Benchmark/Hash_Table/benchmark_clht.c` for the STM32-IDE project, and at `CLHT-standalone/src/benchmark_clht.c` for the standalone version. It performs a series of *put* and *get* operations, outputting the performance for each of these type of operations in operations per second (ops/sec).

### Benchmarking CLHT on a UNIX machine
Before attempting to run the CLHT on the STM32 it is a good idea to test it in a normal UNIX environment to check that it runs as expected. For that we can use the standalone CLHT modules.

First, copy the dataset you intend to use to the `CLHT-standalone` folder. In this example we will be using 32-byte sized keys.

```bash
cp MRAM-key-value/Core/Inc/keys-32bytes.h CLHT-standalone/inc/
```

Adjust the necessary settings in `CLHT-standalone/inc/settings.h` and `CLHT-standalone/src/clht.h`. For this test, we chose the following values, assuming a 128 byte cache line size, and a 64-bit system:

```bash
vim CLHT-standalone/inc/settings.h
```

```c++
#ifndef INC_SETTINGS_H_
#define INC_SETTINGS_H_

#define STM32 0
#define MRAM 0
#define BILLION  1000000000.0

//Different for 32-bit and 64-bit systems
#define MEMORY_ADDRESS_SIZE uint64_t

//For clht, select only 4 byte-sized keys/values.
#define KEYS_FILE "keys-4bytes.h"
#define NUM_RECORDS_TO_INSERT 20000 //25000
#define FIELD_SIZE 4 //Number of bytes for each key/value. Don't forget 1 byte is for \0

#define NUM_BYTES_CAPACITY 524288
```

```bash
vim CLHT-standalone/inc/settings/clht_lb.h
```
```c++
#define CACHE_LINE_SIZE 128
#define ENTRIES_PER_BUCKET 29
```

Compile the code and run.

```bash
cd CLHT-standalone
make
./CLHT-standalone-test
```

If everything runs as expected, you should see an output similar to this:

```
  Example value: .RQ
  Hashtable base address: 0x143704200
  Put operation duplicates: 273
  Mram errors: 0
  Took 0.001274 to insert 20000 records
  Took 0.000577 to read 20000 records
  Number of buckets: 13332
  Number of collisions: 14694
  Consistency check.
          Keys not found: 0
          Non-matching values: 0
```

### Benchmarking CLHT on STM32
Start by opening the root folder of the repository as an STM32CubeIDE workspace, or just import the project `MRAM-key-value` to your STM32CubeIDE. 

The standard output is redirected to the **SWV ITM Data Console**, so make sure you have SWV correctly enabled. For further details, see [this tutorial](https://www.steppeschool.com/pages/blog?p=stm32-printf-function-swv-stm32cubeide).

Make sure to adjust your settings in the `MRAM-key-value/Core/Inc/settings/settings.h` and `MRAM-key-value/Core/Inc/clht.h` files. Make sure to enable STM32 and leave the rest of the settings disabled for now. Configure the dataset you wanna use, and set the correct size accordingly. Also, make sure to set `MEMORY_ADDRESS_SIZE` to `uint32_t`. The settings files should look similar to this:

```bash
vim MRAM-key-value/Core/Inc/settings/settings.h
```

```c++
#define STM32 1
#define MRAM 0
#define MMAP 0
#define BILLION  1000000000.0

//Different for 32-bit and 64-bit systems
#define MEMORY_ADDRESS_SIZE uint32_t
//==============

//BENCHMARK Settings
//==================
//For clht, select only 4 byte-sized keys/values.
#define KEYS_FILE "keys-4bytes.h"
#define NUM_RECORDS_TO_INSERT 5000 //CLHT takes up more space, so we won't be able to fit the full dataset with the given capacity
#define FIELD_SIZE 4 //Number of bytes for each key/value. Don't forget 1 byte is for \0
//WARNING: Only tested with this specific NUM_BYTES_CAPACITY
#define NUM_BYTES_CAPACITY 524288
```

And the CLHT file, adjusted to a 32 byte cache line size:

```bash
vim MRAM-key-value/Core/Inc/clht_lb.h
```

```c++
#define CACHE_LINE_SIZE 32
#define ENTRIES_PER_BUCKET 3
```

Make sure that the main function is calling the clht benchmark:

```bash
vim MRAM-key-value/Core/Src/main.c
```

```c++
int main(void)
{
    ...
  benchmark_clht_write_throughput();
	//test_clht();
	//benchmark_lpht_throughput();
    ...
}
```

Run a debug session. The output should be similar to this:

```
Program starting... 
Keys pointer: 0x24000000
First string pointer: 0x24000000
First string: Ofe
Example value: Ofe
Hashtable base address: 0x240210e0
Put operation duplicates: 14
Mram errors: 0
Took 4 ms to insert 5000 records 
Took 2 ms to read 5000 records 
Number of buckets: 3571
 Number of collisions: 3791
Consistency check.
 	Keys not found: 0
 	Non-matching values: 0
```

### Running CLHT on STM32 with external memory device (MRAM)

The project includes code to run the Hash Table on an external MRAM device set at address `0xc0000000`.

To enable the use of MRAM enable its setting in `MRAM-key-value/Core/Inc/settings/settings.h`:

```c++
#ifndef INC_SETTINGS_H_
#define INC_SETTINGS_H_

//LPHT settings
//=============
#define STM32 1
#define MRAM 1  // <---- SET MRAM OPTION
#define MMAP 0
#define BILLION  1000000000.0

//Different for 32-bit and 64-bit systems
#define MEMORY_ADDRESS_SIZE uint32_t
//==============

//BENCHMARK Settings
//==================
//For clht, select only 4 byte-sized keys/values.
#define KEYS_FILE "keys-4bytes.h"
#define NUM_RECORDS_TO_INSERT 5000 //25000
#define FIELD_SIZE 4 //Number of bytes for each key/value. Don't forget 1 byte is for \0
//WARNING: Only tested with this specific NUM_BYTES_CAPACITY
#define NUM_BYTES_CAPACITY 524288
//==================

#if STM32
#define PRINT_UNSIGNED_FORMAT "%lu"
#else
#define PRINT_UNSIGNED_FORMAT "%u"
#endif

//WARNING: For hashmap_string, must set USABLE_CAPACITY to correct value according to PAIR_SIZE ,i.e. (2 * FIELD_SIZE)

#endif /* INC_SETTINGS_H_ */
```

This will enact a series of changes in the `MRAM-key-value/Core/Src/Hash_Table/clht_lb.c` file. Firstly, CLHT pointers are set to point to the external memory device (in this case at 0xc0000000). Secondly, a different heap implementation is used that wastes less space on aligned memory allocation. However, at the moment, it is only able to allocate space, it cannot free previous allocations. Finally, all write and read operations are redirected through a dedicated MRAM driver module. 

To see how to adapt this CLHT implementation to other external devices look for CLHT's MRAM code blocks that follow this format:

```bash
vim MRAM-key-value/Core/Src/Hash_Table/clht_lb.c
```

```c++
#if MRAM
  [MRAM code] <-- Look for these code blocks and see how you can adapt them to your own device
#else
  [Onboard storage code]
#endif
```
