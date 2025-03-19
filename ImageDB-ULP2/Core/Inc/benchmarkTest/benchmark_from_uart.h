/*
 * benchmark_from_uart.h
 *
 *  Created on: Feb 7, 2025
 *      Author: luisferreira
 */

#ifndef INC_BENCHMARKTEST_BENCHMARK_FROM_UART_H_
#define INC_BENCHMARKTEST_BENCHMARK_FROM_UART_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct benchmarkSettings{
	double * data;
	uint32_t numValues;
	double minValue;
	double maxValue;
	uint32_t *quality;
	uint32_t numQuality;
	uint32_t *imageHeight;
	uint32_t *imageWidth;
	uint32_t *numRepeatImage;
	uint32_t *numRepeatImageQuantization;
	uint32_t numSizes; //Num of height/width to test
	uint32_t numIterations;
	uint32_t *amountVectors;
	uint32_t numAmountVectors;
	uint32_t *minRGBVectorSize;
	uint32_t numMinRGBSizes;
	double *outliers;
	bool timedQuantization; //WARNING: in the case of JPEG_CPU it is not the quantization being accounted for, its the time to generate image.
} BenchmarkSettings;

void benchmark_from_uart(BenchmarkSettings *settings);


#endif /* INC_BENCHMARKTEST_BENCHMARK_FROM_UART_H_ */
