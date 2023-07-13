# Linear Probing Hash Table
The linear probing Hash Table divides a zone of memory into consecutive key/value slots. During a put operation, the key is hashed into a slot. If the slot is empty, the key/value pair is inserted into the slot, otherwise an attempt is made for the following slot, and so on until an available spot is found. Slot occupation information is kept in a separate array of bytes, where each bit describes the occupation of a particular slot.

The Hash Table is available both as an STM32CubeIDE project (`MRAM-key-value` folder) and as standalone C modules that you can use to integrate into your own project (`LPHT-standalone` folder).


### Basic usage
To use the LPHT you only need the files contained in the `LPHT-standalone` folder. The basic workflow involves creating a new Hash Table and then performing put/get operations over it. For example:

```
#include "lpht.h"

int main(void){
    LPHT * hashMap = create_lpht();
    lpht_put(hashMap, "key", "value");
    char * value = lpht_get(hashMap, "key");
    printf("Retrieved value: %s\n", value);
    free(value);
}
```

### LPHT settings description
The LPHT benchmark and Hash Table can be controlled through a series of settings available in either `LPHT-standalone/inc/settings.h` for the standalone version or `MRAM-key-value/Core/Inc/settings/settings.h` for the STM32CubeIDE. It looks as follows:

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
#define KEYS_FILE "keys-32bytes.h"
#define NUM_RECORDS_TO_INSERT 4096 //25000
#define FIELD_SIZE 32 //Number of bytes for each key/value. Don't forget 1 byte is for \0

#define NUM_BYTES_CAPACITY 524288
//==================
```

The `STM32` and `MRAM` settings should be set to 0 when running on UNIX systems. The `MMAP` option can be set to 1 if you want your data to be persisted to disk, in which case a `mmap_file.data` file will be created. This option is exclusive to UNIX systems, so it should always be disabled when running on STM32 MCUs.

The `MEMORY_ADDRESS_SIZE` should be set to the size of an address in your system, i.e., the architecture type of your CPU. Most current UNIX systems use 64-bit CPUs, while the STM32 is 32-bit.

The `KEYS_FILE` indicates the file with the dataset to be used; `NUM_RECORDS_TO_INSERT` states the number of keys to insert, and should not be greater than the size of the dataset (see the [README](./README.md)); and `FIELD_SIZE` indicates the size of the keys in bytes. 

`NUM_BYTES_CAPACITY` sets the total amount of space available to the LPHT.

Furthermore, LPHT should be configured to accommodate the specific byte size you are using. In `MRAM-key-value/Core/Src/Hash_Table/lpht.c` or `LPHT-standalone/src/lpht.c` you should adjust:

```c++
//NUM_BYTES_CAPACITY minus space reserved for metadata (i.e., HashMapString struct) rounded to the nearest multiple of 32
#define USABLE_CAPACITY 521600 // Calculated as: ((double) NUM_BYTES_CAPACITY - 17)/(1 + (1/((double)KEY_VALUE_PAIR_SIZE*8))) | round(nearest 32 aligned number lower than the calculated value) . Equation from three variable system.
```

 `USABLE_CAPACITY` sets how much of the hashtable's space (i.e., `NUM_BYTES_CAPACITY`) is available for storing key/value pairs. The difference between the two, $NUM\_BYTES\_CAPACITY - USABLE\_CAPACITY$, is the amount of reserved space for slot occupation information. Currently, the value of USABLE_CAPACITY has to be set by hand, and can be calculated as:

$$ NUM\\_BYTES\\_CAPACITY - 17 \over 1 + (1 / (2 * FIELD\\_SIZE * 8)) $$

, rounded to the nearest multiple of 32 that is less than the given result.

[TODO: add values for the datasets given]


## LPHT benchmarking
The benchmark for LPHT is available at `MRAM-key-value/Core/Src/Benchmark/Hash_Table/benchmark_lpht.c` for the STM32-IDE project, and at `LPHT-standalone/src/benchmark_lpht.c` for the standalone version. It performs a series of *put* and *get* operations, outputting the performance for each of these type of operations in operations per second (ops/sec).


### Benchmarking LPHT on a UNIX machine
Before attempting to run the LPHT on the STM32 it is a good idea to test it in a normal UNIX environment to check that it runs as expected. For that we can use the standalone LPHT modules. 

First, copy the dataset you intend to use to the `LPHT-standalone` folder. In this example we will be using 32-byte sized keys.

```bash
cp MRAM-key-value/Core/Inc/keys-32bytes.h LPHT-standalone/inc/
```

Adjust the necessary settings in `LPHT-standalone/inc/settings.h` and `LPHT-standalone/src/lpht.c`. For this test, make sure the values are set as follows:

```
vim LPHT-standalone/inc/settings.h
```

```c++
#define STM32 0
#define MRAM 0
#define MMAP 0
#define BILLION  1000000000.0

//Different for 32-bit and 64-bit systems
#define MEMORY_ADDRESS_SIZE uint64_t

//For clht, select only 4 byte-sized keys/values.
#define KEYS_FILE "keys-32bytes.h"
#define NUM_RECORDS_TO_INSERT 4096 //25000
#define FIELD_SIZE 32 //Number of bytes for each key/value. Don't forget 1 byte is for \0

#define NUM_BYTES_CAPACITY 524288
```

```
vim LPHT-standalone/src/lpht.c
```

```c++
//NUM_BYTES_CAPACITY minus space reserved for metadata (i.e., HashMapString struct) rounded to the nearest multiple of 32
#define USABLE_CAPACITY 521600
```

Compile the code and run.

```bash
cd LPHT-standalone
make
./LPHT-standalone-test
```

If everything runs as expected, you should see an output similar to this:
```
Keys pointer: 0x104cf8000
First string pointer: 0x104cf8000
First string: >X4L8K7=n&oex]j$iF(w&i6BI8WPQ9l
Creating Hash Table with total bytes: 524288
Usable capacity: 521600
Max pairs available: 8150
Size reserved for block occupation and other: 2688
Took 0.000868 to insert 4096 records
Number of collisions 1981
 Number of replacements 0 
Took 0.001235 to read 4096 records
Consistency check.
        Keys not found: 0
        Non-matching values: 0
```



### Benchmarking LPHT on STM32
Start by opening the root folder of the repository as an STM32CubeIDE workspace, or just import the project `MRAM-key-value` to your STM32CubeIDE. 

The standard output is redirected to the **SWV ITM Data Console**, so make sure you have SWV correctly enabled. For further details, see [this tutorial](https://www.steppeschool.com/pages/blog?p=stm32-printf-function-swv-stm32cubeide).

Make sure to adjust your settings in the `MRAM-key-value/Core/Inc/settings/settings.h` and `MRAM-key-value/Core/Src/Hash_Table/lpht.c` files. Make sure to enable STM32 and leave the rest of the settings disabled for now. Configure the dataset you wanna use, and set the correct size accordingly. Also, make sure to set `MEMORY_ADDRESS_SIZE` to `uint32_t`. The settings files should look similar to this:

```
vim MRAM-key-value/Core/Inc/settings/settings.h
```

```c++
#define STM32 1
#define MRAM 0
#define MMAP 0
#define BILLION  1000000000.0

//Different for 32-bit and 64-bit systems
#define MEMORY_ADDRESS_SIZE uint32_t

//For clht, select only 4 byte-sized keys/values.
#define KEYS_FILE "keys-32bytes.h"
#define NUM_RECORDS_TO_INSERT 4096 //25000
#define FIELD_SIZE 32 //Number of bytes for each key/value. Don't forget 1 byte is for \0

#define NUM_BYTES_CAPACITY 300000
```

And the LPHT file:

```
vim MRAM-key-value/Core/Src/Hash_Table/lpht.c
```

```c++

//NUM_BYTES_CAPACITY minus space reserved for metadata (i.e., HashMapString struct) rounded to the nearest multiple of 32
#define USABLE_CAPACITY 299392
```


Make sure that the main function is calling the lpht benchmark:

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
First string: >X4L8K7=n&oex]j$iF(w&i6BI8WPQ9l
Keys pointer: 0x24000000
First string pointer: 0x24000000
First string: >X4L8K7=n&oex]j$iF(w&i6BI8WPQ9l
Creating Hash Table with total bytes: 524288
Usable capacity: 521600
Max pairs available: 8150
Size reserved for block occupation and other: 2688
Took 30 ms to insert 4096 records 
Number of collisions 301991873
 Number of replacements 0 
Took 34 ms to read 4096 records 
Consistency check.
 	Keys not found: 0
 	Non-matching values: 0
```


### Running LPHT on STM32 with external memory device (MRAM)

Running LPHT on an external memory device is simple. You only have to change the LPHT pointers to match the location of the external device, and adjust the memory capacity of the table to be in accordance with the limitations of the device. The project includes code to run the Hash Table on an external MRAM device set at address `0xc0000000`. 

To enable the use of MRAM change the settings at `MRAM-key-value/Core/Inc/settings/settings.h` :

```c++
//LPHT settings
//=============
#define STM32 1
#define MRAM 1  // <---- Set MRAM option
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

#define NUM_BYTES_CAPACITY 524288 //Value for a 4Mb device
```

This will enable a series of code blocks in `MRAM-key-value/Core/Src/Hash_Table/lpht.c` which set the Hash Map to the appropriate address, and redirect all read/write operations through the `MRAM-key-value/Core/Src/Drivers/MRAM_driver.c` driver. Look out for the `#if MRAM` flags to see how this is done.

Also, make sure to adjust the space settings in `MRAM-key-value/Core/Src/Hash_Table/lpht.c`:

```

//NUM_BYTES_CAPACITY minus space reserved for metadata (i.e., HashMapString struct) rounded to the nearest multiple of 32
#define USABLE_CAPACITY 521600
```
