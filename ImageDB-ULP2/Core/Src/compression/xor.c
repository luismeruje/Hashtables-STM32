#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "settings.h"
#include "logging.h"

//ChatGPT
// Function to count the number of leading zeros in a 64-bit unsigned integer
int count_leading_zeros(uint64_t x) {
    if (x == 0) return 64; // Special case for zero
    int count = 0;
    while ((x & (1ULL << 63)) == 0) { // Check the most significant bit
        x <<= 1;
        count++;
    }
    return count;
}

//ChatGPT
// Function to count the number of trailing zeros in a 64-bit unsigned integer
int count_trailing_zeros(uint64_t x) {
    if (x == 0) return 64; // Special case for zero
    int count = 0;
    while ((x & 1) == 0) { // Check the least significant bit
        x >>= 1;
        count++;
    }
    return count;
}


//ChatGPT
/**
 * @brief Sets a bit to 1 at a specified bit index in a uint8_t array.
 *
 * @param arr      Pointer to the uint8_t array.
 * @param bit_index Bit index to set to 1.
 */
void set_bit(uint8_t *arr, uint32_t *bit_index) {
    if (!arr) {
        return;
    }
    uint32_t byte_pos = *bit_index / 8;
    uint32_t bit_pos = 8 - (*bit_index % 8) - 1;
    arr[byte_pos] |= (1 << bit_pos);
    *bit_index += 1;
}

//ChatGPT
/**
 * @brief Sets a bit to 0 at a specified bit index in a uint8_t array.
 *
 * @param arr      Pointer to the uint8_t array.
 * @param bit_index Bit index to set to 0.
 */
void clear_bit(uint8_t *arr, uint32_t *bit_index) {
    if (!arr) {
        return;
    }
    uint32_t byte_pos = *bit_index / 8;
    uint32_t bit_pos = 8 - (*bit_index % 8) - 1;
    arr[byte_pos] &= ~(1 << bit_pos);
    *bit_index += 1;
}


void printArray(uint8_t * array, uint32_t numBits){
    for(uint32_t i = 0; i < (numBits / 8) + 1; i++){
        printf("%x ",array[i]);
    }
    printf("; numBits: %ld\n", numBits);
}

double reconstruct_double(uint32_t leadingZeros, uint32_t numRelevantBits, uint64_t bits){
    bits = bits << (64 - leadingZeros - numRelevantBits);
    double *pointer = (double *) &bits;
    double result = *pointer;
    return result;
}

// Original slower version
// double xor_doubles(double a, double b){
//     uint64_t *intA, *intB;
//     double xor;
//     uint64_t xorInt;

//     intA = (uint64_t *) &a;
//     intB =  (uint64_t *) &b;
//     xorInt = *intA ^ *intB;
//     memcpy(&xor,&xorInt, sizeof(double));

//     return xor;
// }


double xor_doubles(double a, double b){
    union doubleInt{
        uint64_t integer;
        double decimal;
    } num1, num2, xored;


    num1.decimal = a;
    num2.decimal = b;

    xored.integer = num1.integer ^ num2.integer;

    return xored.decimal;
}



void write_n_bits(uint64_t src, uint32_t src_index, uint8_t *dest, uint32_t *dest_index, uint32_t numBits) {
    uint64_t mask = ((uint64_t)1 << numBits) - 1;
    src = (src >> src_index) & mask; // Extract the relevant bits from src

    for (int32_t i = numBits - 1; i >= 0; --i) {
        uint32_t dest_byte_index = *dest_index / 8;
        uint32_t dest_bit_offset = 7 - (*dest_index % 8);

        // Extract the i-th bit from src (starting from MSB of the extracted bits)
        uint8_t bit_to_write = (src >> i) & 1;

        // Write the bit to the correct position in dest
        dest[dest_byte_index] &= ~(1 << dest_bit_offset); // Clear the bit position
        dest[dest_byte_index] |= (bit_to_write << dest_bit_offset); // Set the bit if bit_to_write is 1

        ++(*dest_index); // Move to the next bit position in dest
    }
}

// Reads num_bits from the array arr starting at bit position *bit_index and returns the value as a uint64_t.
uint64_t read_n_bits(const uint8_t *arr, uint32_t *bit_index, uint32_t num_bits) {
    uint64_t result = 0;

    for (uint32_t i = 0; i < num_bits; ++i) {
        uint32_t arr_byte_index = *bit_index / 8;
        uint32_t arr_bit_offset = 7 - (*bit_index % 8);

        // Extract the bit from the array
        uint8_t bit = (arr[arr_byte_index] >> arr_bit_offset) & 1;

        // Set the corresponding bit in the result
        result = (result << 1) | bit;

        ++(*bit_index); // Move to the next bit position
    }

    return result;
}

// Compressed values can take up to 2 times the original size of the values.
// @return Size in bits of compressed data
uint8_t *xor_compression(double * data, uint32_t numValues, uint32_t *numBitsCompressedValues){
	uint8_t * compressedValues = calloc(1,sizeof(double) * numValues * 2);
	uint32_t compressedValuesBitIndex = 0;
	uint64_t xorValue = 0;
	int trailingZeros = 0, trailingZerosPrevious = -1, leadingZeros = 0, leadingZerosPrevious = -1;
    uint64_t *cur = NULL, *prev = NULL;
    uint32_t relevantBits = 0;

    if(numValues > 0){
		memcpy(&compressedValues[0], &data[0], sizeof(double));
        //printf("%lf\n", data[0]);
		compressedValuesBitIndex += 8 * 8;
        if(LOG_LEVEL >= INFO){
            printArray(compressedValues, compressedValuesBitIndex);
        }
        trailingZeros = count_trailing_zeros((uint64_t) data[0]);
        leadingZeros = count_leading_zeros((uint64_t) data[0]);

        for(uint32_t i = 1; i < numValues; i++){
            prev = (uint64_t *)(&data[i-1]);
            cur = (uint64_t *)(&data[i]);

            xorValue = *cur ^ *prev;
            if(xorValue == 0){
            	clear_bit(compressedValues, &compressedValuesBitIndex);
            } else {
            	trailingZeros = count_trailing_zeros(xorValue);
            	leadingZeros = count_leading_zeros(xorValue);

                set_bit(compressedValues, &compressedValuesBitIndex);
                if(trailingZeros != trailingZerosPrevious || leadingZeros != leadingZerosPrevious){
                    set_bit(compressedValues, &compressedValuesBitIndex);
                    //Copy number of leading zeroes
                    write_n_bits(leadingZeros, 0, compressedValues, &compressedValuesBitIndex, 6);
                    //Copy number of meaningful bits (6 bits)
                    relevantBits = (uint32_t) 8 * 8 - trailingZeros - leadingZeros;
					write_n_bits(relevantBits, 0, compressedValues, &compressedValuesBitIndex, 6);
                    if(LOG_LEVEL >= INFO){
                        printArray(compressedValues, compressedValuesBitIndex);
                    }
                } else {
                   clear_bit(compressedValues, &compressedValuesBitIndex);
                }
                write_n_bits(xorValue, trailingZeros, compressedValues, &compressedValuesBitIndex, relevantBits);

                //Copy the meaningful bits

                if(LOG_LEVEL >= INFO){
                   printArray(compressedValues, compressedValuesBitIndex);
                }
            }
            if(LOG_LEVEL >= INFO){
               printf("Trailing: %d, Leading: %d, Meaningful bits: %u, XOR: %llu\n", trailingZeros, leadingZeros, (sizeof(double) * 8) - leadingZeros - trailingZeros, xorValue);
            }
            trailingZerosPrevious = trailingZeros;
            leadingZerosPrevious = leadingZeros;
        }
    }
    //printf("\n");
    *numBitsCompressedValues = compressedValuesBitIndex;
    return compressedValues;
}

double * xor_decompression(uint8_t * compressed, uint32_t maxBitIndex, uint32_t numValues){
    double *values = calloc(numValues, sizeof(double));
    uint32_t valuesIndex = 0;
    uint32_t bitIndex = 0, leadingZeros = 0, numBits = 0;
    uint64_t bits = 0;
    double prev_number = 0;
    uint8_t controlBit1, controlBit2;

    memcpy(&values[0], &compressed[0], sizeof(double));
    memcpy(&prev_number, &compressed[0], sizeof(double));

    valuesIndex++;

    bitIndex += sizeof(double) * 8;
    if(LOG_LEVEL >= INFO){
        printf("Decompressed: %lf\n", values[0]);
    }
    prev_number = values[0];

    while(bitIndex < maxBitIndex && valuesIndex < numValues){
        controlBit1 = read_n_bits(compressed, &bitIndex,1);
        if(LOG_LEVEL >= INFO){
            printf("Control bit 1: %x\n", controlBit1);
        }

        if(controlBit1 == 1){
            controlBit2 = read_n_bits(compressed, &bitIndex,1);
            if(LOG_LEVEL >= INFO){
                printf("Control bit 2: %x\n", controlBit2);
            }
            if(controlBit2 == 1){
                if(LOG_LEVEL >= INFO){
                    printf("New leading/trailing zeros info.");
                }
                leadingZeros = read_n_bits(compressed, &bitIndex, 6);
                numBits = read_n_bits(compressed, &bitIndex, 6);
            }
            bits = read_n_bits(compressed, &bitIndex, numBits);
            if(LOG_LEVEL >= INFO){
                printf("Leading zeros: %ld, #different bits: %ld, relevant bits as uint64_t: %llu\n",leadingZeros, numBits, bits);
            }
            values[valuesIndex] = reconstruct_double(leadingZeros, numBits, bits);
            if(LOG_LEVEL >= INFO){
                printf("Relevant bits after reconstruction, but before XOR: %llu", (uint64_t) *((uint64_t*) &values[valuesIndex]));
            }
            values[valuesIndex] = xor_doubles(values[valuesIndex], prev_number);
            prev_number  = values[valuesIndex];
            valuesIndex++;
            if(LOG_LEVEL >= INFO){
                printf("Reconstructed double: %lf\n", values[valuesIndex-1]);
            }
        } else {
            //Same number
            values[valuesIndex] = values[valuesIndex-1];
            valuesIndex++;
        }
    }

    if(bitIndex != maxBitIndex){
        printf("Bit index: %lu, maxBitIndex: %lu\n", bitIndex, maxBitIndex);
        printf("Not all compressed bits were read\n");
    }
    return values;
}
