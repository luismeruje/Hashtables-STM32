/*
 * xor_compress.h
 *
 *  Created on: Dec 10, 2024
 *      Author: luisferreira
 */

#ifndef INC_COMPRESSION_XOR_COMPRESS_H_
#define INC_COMPRESSION_XOR_COMPRESS_H_
#include <stdint.h>

uint8_t *xor_compression(double * data, uint32_t numValues, uint32_t *numBitsCompressedValues);
double * xor_decompression(uint8_t * compressed, uint32_t maxBitIndex, uint32_t numValues);

#endif /* INC_COMPRESSION_XOR_COMPRESS_H_ */
