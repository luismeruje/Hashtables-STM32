
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mram_commons.h"
#include "stm32h7xx_hal.h"


void test_memory_write_latency_elementary_operations(){
	uint32_t addr;
	int end_time;
	int start_time;
	int elapsed_time;
	int nrOperations;
	int throughput;
	float IOPs;

	//Operation sizes supported by FMC
	uint8_t op_sizes[] = {8,16,32,64};

	uint8_t len_op_sizes = sizeof(op_sizes)/sizeof(op_sizes[0]);

	start_time = HAL_GetTick();
	for(int i = 0; i < len_op_sizes; i++){
		switch(op_sizes[i]){
			case 8:
				start_time = HAL_GetTick();
				for(addr = MRAM_BANK_ADDR; addr < MRAM_BANK_ADDR_LIMIT_4Mb; addr+=1){
					*(uint8_t *) addr = (uint8_t) 0x1337;
				}
				end_time = HAL_GetTick();
				break;
			case 16:
				start_time = HAL_GetTick();
				for(addr = MRAM_BANK_ADDR; addr < MRAM_BANK_ADDR_LIMIT_4Mb; addr+=2){
					*(uint16_t *) addr = (uint16_t) 0x1337;
				}
				end_time = HAL_GetTick();
				break;
			case 32:
				start_time = HAL_GetTick();
				for(addr = MRAM_BANK_ADDR; addr < MRAM_BANK_ADDR_LIMIT_4Mb; addr+=4){
					*(uint32_t *) addr = (uint32_t) 0x1337;
				}
				end_time = HAL_GetTick();
				break;
			case 64:
				start_time = HAL_GetTick();
				for(addr = MRAM_BANK_ADDR; addr < MRAM_BANK_ADDR_LIMIT_4Mb; addr+=8){
					*(uint64_t *) addr = (uint64_t) 0x1337;
				}
				end_time = HAL_GetTick();
				break;
		}
		end_time = HAL_GetTick();
		elapsed_time = (end_time - start_time);

		nrOperations = (MRAM_BANK_ADDR_LIMIT_4Mb - MRAM_BANK_ADDR)/ (op_sizes[i]/8);

		IOPs = (nrOperations * 1000) / elapsed_time;
		printf("Write IOPs @ %d bits: %d\n", op_sizes[i] ,(int) IOPs);

		throughput = (op_sizes[i]/8) * IOPs;
		printf("Write throughput @ %d bits: %d\n\n", op_sizes[i] , throughput);
	}
}

//Read immediately after write
uint32_t test_write_read_consistency_constant_immediate(void * constant, uint8_t bit_size){
	uint8_t byte_size = bit_size / 8;
	uint32_t errors = 0;

	//Write, read and verify
	for(uint32_t addr = MRAM_BANK_ADDR; addr < MRAM_BANK_ADDR_LIMIT_4Mb; addr+=byte_size){
		switch(bit_size){
			case 8:
				* (uint8_t *) addr = * (uint8_t *) constant;
				if ((* (uint8_t *) addr) != * (uint8_t *) constant){
					printf("Error at address %lu, value at MRAM: %u, constant expected: %u\n", addr, (* (uint8_t *) addr), * (uint8_t *) constant);
					errors++;
				}
				break;
			case 16:
				* (uint16_t *) addr = * (uint16_t *) constant;
				if((* (uint16_t *) addr) != (* (uint16_t *) constant)){
					printf("Error at address %lu, value at MRAM: %u, constant expected: %u\n", addr, (* (uint16_t *) addr), * (uint16_t *) constant);
					errors++;
				}
				break;
			case 32:
				* (uint32_t *) addr = * (uint32_t *) constant;
				if((* (uint32_t *) addr) != * (uint32_t *) constant){
					printf("Error at address %lu, value at MRAM: %lu, constant expected: %lu\n", addr, (* (uint32_t *) addr), * (uint32_t *) constant);
					errors++;
				}
				break;
			case 64:
				* (uint64_t *) addr = * (uint64_t *) constant;
				if((* (uint64_t *) addr) != * (uint64_t *) constant){
					printf("Error at address %lu, value at MRAM: %llu, constant expected: %llu\n", addr, (* (uint64_t *) addr), * (uint64_t *) constant);
					errors++;
				}
				break;
			default:
				printf("Unsupported bit_size for write");
				return -1;
		}
	}
	return errors;

}

//WARNING: Reads from odd addresses, i.e., last 8 bits of 16bit memory, are not supported.
//Adapts to size of number
//Rules out problems with individual numbers, i.e. problems with specific Address/Data lines
uint32_t test_write_read_consistency_constant(void * constant, uint8_t bit_size){
	uint8_t byte_size = bit_size / 8;
	uint32_t errors = 0;

	//Write
	for(uint32_t addr = MRAM_BANK_ADDR; addr < MRAM_BANK_ADDR_LIMIT_4Mb; addr+=byte_size){
		switch(bit_size){
			case 8:
				* (uint8_t *) addr = * (uint8_t *) constant;
				break;
			case 16:
				* (uint16_t *) addr = * (uint16_t *) constant;
				break;
			case 32:
				* (uint32_t *) addr = * (uint32_t *) constant;
				break;
			case 64:
				* (uint64_t *) addr = * (uint64_t *) constant;
				break;
			default:
				printf("Unsupported bit_size for write");
				return -1;
		}
	}

	//Read and verify
	for(uint32_t addr = MRAM_BANK_ADDR; addr < MRAM_BANK_ADDR_LIMIT_4Mb; addr+=byte_size){
		switch(bit_size){
			case 8:
				if ((* (uint8_t *) addr) != * (uint8_t *) constant){
					printf("Error at address %lu, value at MRAM: %u, constant expected: %u\n", addr, (* (uint8_t *) addr), * (uint8_t *) constant);
					errors++;
				}
				break;
			case 16:
				if((* (uint16_t *) addr) != (* (uint16_t *) constant)){
					printf("Error at address %lu, value at MRAM: %u, constant expected: %u\n", addr, (* (uint16_t *) addr), * (uint16_t *) constant);
					errors++;
				}
				break;
			case 32:
				if((* (uint32_t *) addr) != * (uint32_t *) constant){
					printf("Error at address %lu, value at MRAM: %lu, constant expected: %lu\n", addr, (* (uint32_t *) addr), * (uint32_t *) constant);
					errors++;
				}
				break;
			case 64:
				if((* (uint64_t *) addr) != * (uint64_t *) constant){
					printf("Error at address %lu, value at MRAM: %llu, constant expected: %llu\n", addr, (* (uint64_t *) addr), * (uint64_t *) constant);
					errors++;
				}
				break;
			default:
				printf("Unsupported bit_size for write");
				return -1;
		}
	}
	return errors;

}




void generate_16bit_randoms(uint16_t * randoms, uint16_t nr_elements){
	//srand(time(NULL));
	int random;

	for(int i = 0; i < nr_elements;i++){
		random = rand();
		randoms[i] = (uint16_t) random;
	}
}

void generate_32bit_randoms(uint32_t * randoms, uint16_t nr_elements){
	//srand(time(NULL));
	int random;

	for(int i = 0; i < nr_elements;i++){
		random = rand();
		randoms[i] = (uint32_t) random;
	}
}

int hasDuplicate(uint32_t arr[], uint32_t value, int size){
	for (int i = 0; i < size; i++){
		if(arr[i] == value){
			return 1;
		}
	}
	return 0;
}

void generate_random_mram_addresses(uint32_t * random_mram_addresses, uint16_t nr_elements){
	//srand(time(NULL));
	uint32_t random;
	uint32_t range = (MRAM_BANK_ADDR_LIMIT_4Mb - 2) - MRAM_BANK_ADDR;

	for(int i = 0; i < nr_elements;i++){
		random = rand();
		random_mram_addresses[i] = MRAM_BANK_ADDR + (random % range);
		random_mram_addresses[i] -= (random_mram_addresses[i] % 2);
		if(hasDuplicate(random_mram_addresses, random_mram_addresses[i] ,i - 1)){
			i--;
		}
		if(random_mram_addresses[i] < MRAM_BANK_ADDR || random_mram_addresses[i] > (MRAM_BANK_ADDR_LIMIT_4Mb - 2)){
			printf("Generated address that does not sit on MRAM\n");
			HAL_Delay(30*1000);
		}
	}
}



int countDuplicates(uint32_t arr[], int size) {
	int duplicates = 0;
    for (int i = 0; i < size; i++) {
        for (int j = i + 1; j < size; j++) {
            if (arr[i] == arr[j]) {
                duplicates++; // Return true if a duplicate is found
            }
        }
    }
    return duplicates; // Return false if no duplicates are found
}

//From: https://barrgroup.com/embedded-systems/how-to/memory-test-suite-c
uint8_t test_MRAM_data_bus(){
	uint8_t error = 0;
	for(int i=0;i<16;i++){
		*(uint16_t *) MRAM_BANK_ADDR = pow(2,i);
		if(*(uint16_t *) MRAM_BANK_ADDR != pow(2,i)){
			error = 1;
			break;
		}

	}
	return error;
}


void test_memory_consistency_random_16bits(){
	uint16_t random_array_size = 1000;
	uint16_t random_16bits[random_array_size];
	uint32_t random_mram_addresses [random_array_size];
	uint32_t errors = 0;

	generate_16bit_randoms(random_16bits,random_array_size);
	generate_random_mram_addresses(random_mram_addresses, random_array_size);

	printf("Number DUPLICATES: %d\n", countDuplicates(random_mram_addresses, random_array_size));

	for(int i = 0; i < random_array_size; i++){
		* (uint16_t *) random_mram_addresses[i] = random_16bits[i];
	}

	for(int i = 0; i < random_array_size; i++){
		if(* (uint16_t *) random_mram_addresses[i] != random_16bits[i]){
			errors++;
			printf("Memory test error with value %d, at address: %lx\n Value stored: %u\n ", random_16bits[i], random_mram_addresses[i], * (uint16_t *) random_mram_addresses[i]);
			if(random_mram_addresses[i] < MRAM_BANK_ADDR || random_mram_addresses[i] > MRAM_BANK_ADDR_LIMIT_4Mb){
				printf("Address was invalid\n");

			} else{
				printf("Address was valid\n");
			}
			if(* (uint16_t *) random_mram_addresses[i] != random_16bits[i]){
				printf("Second test also came out wrong\n");
			}
		}
	}

	printf("Random-write test errors: %ld\n",errors);
}

void test_memory_consistency_random_32bits(){
	uint16_t random_array_size = 1000;
	uint32_t random_32bits[random_array_size];
	uint32_t random_mram_addresses [random_array_size];
	uint32_t errors = 0;

	generate_32bit_randoms(random_32bits,random_array_size);
	generate_random_mram_addresses(random_mram_addresses, random_array_size);

	printf("Number DUPLICATES: %d\n", countDuplicates(random_mram_addresses, random_array_size));

	for(int i = 0; i < random_array_size; i++){
		* (uint32_t *) random_mram_addresses[i] = random_32bits[i];
	}

	for(int i = 0; i < random_array_size; i++){
		if(* (uint32_t *) random_mram_addresses[i] != random_32bits[i]){
			errors++;
			printf("Memory test error with value %lu, at address: %lx\n Value stored: %lu\n ", random_32bits[i], random_mram_addresses[i], * (uint32_t *) random_mram_addresses[i]);
			if(random_mram_addresses[i] < MRAM_BANK_ADDR || random_mram_addresses[i] > MRAM_BANK_ADDR_LIMIT_4Mb){
				printf("Address was invalid\n");

			} else{
				printf("Address was valid\n");
			}
			if(* (uint32_t *) random_mram_addresses[i] != random_32bits[i]){
				printf("Second test also came out wrong\n");
			}
		}
	}

	printf("Random-write test errors: %ld\n",errors);
}



void test_memory_consistency_random(uint16_t bitSize){
	switch(bitSize){
		case 16:
			test_memory_consistency_random_16bits();
			break;
		case 32:
			test_memory_consistency_random_32bits();
			break;
		default:
			printf("Random memory consistency test with %d bit size not supported.\n", bitSize);
	}

}

int test_memory_consistency (uint16_t bitSize){
	int memory_consistent = 1;
	uint16_t constant;
	uint16_t max = ~0; //Max int for 16 bits
	uint32_t errors = 0;
	uint8_t any_error = 0;

	//NOTE: Tested up to number 26800
	//Constant test
	for(constant = 0;constant != max ; constant++){
		errors = test_write_read_consistency_constant_immediate((void *) & constant, bitSize);
		if(errors > 0){
			printf("Memory test errors with constant %d, #errors: %ld\n", constant, errors);
			any_error = 1;
		}
		if((constant % 100) == 0){
			printf("Tested %d different constants\n",constant);
		}
	}

	//Test last one
	errors = test_write_read_consistency_constant_immediate((void *) & constant, bitSize);
	if(errors > 0){
		printf("Memory test errors with constant %d, #errors: %ld\n", constant, errors);
		any_error = 1;
	}


	if(!any_error){
		printf("Memory showing no errors\n");
		memory_consistent = -1;
	}

	return memory_consistent;
}
