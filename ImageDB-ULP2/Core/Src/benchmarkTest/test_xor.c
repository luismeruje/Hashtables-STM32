/*
 * test_xor.c
 *
 *  Created on: Feb 25, 2025
 *      Author: luisferreira
 */
#include "xor.h"
#include <stdint.h>
#include <stdlib.h>
#include "stm32u5xx_hal.h"
#include "settings.h"
#include "utils.h"
#include <string.h>

extern TIM_HandleTypeDef htim2;

double test_xor_performance_dataset(double * data, uint32_t numValues, uint32_t numDataRepeats, double *stddev){
	double performance = 0;
	uint32_t timerCounts[numDataRepeats];
	memset(timerCounts, 0, numDataRepeats * sizeof(uint32_t));
	uint8_t *dataCompressed = NULL;
	uint32_t numBitsCompressed = 0;


	//start = HAL_GetTick();
	for(uint32_t dataRepetition = 0; dataRepetition < numDataRepeats; dataRepetition++){
		__HAL_TIM_SetCounter(&htim2,0);
		dataCompressed = xor_compression(data, numValues, &numBitsCompressed);
		timerCounts[dataRepetition] =__HAL_TIM_GetCounter(&htim2); //Get current clock cycle count
		free(dataCompressed);
		dataCompressed = NULL;
	}

	double meanDuration = timer_mean_time (timerCounts, numDataRepeats, TIM2_COUNT_PER_SEC);

	performance =  ((double) numValues) / meanDuration;


	*stddev = timer_standard_deviation(timerCounts, numDataRepeats, TIM2_COUNT_PER_SEC, meanDuration);
	*stddev = ((*stddev) / meanDuration) * performance;

	//printf("Performance: %lf pixels/sec with stddev: %lf \n", *performance, *stddev);
	return performance;
}

// @return Dataset compressed size in bytes
uint32_t test_xor_compressed_size_dataset(double * data, uint32_t numValues){
	uint8_t *dataCompressed = NULL;
	uint32_t numBitsCompressed = 0;
	uint32_t compressedSize = 0;

	dataCompressed = xor_compression(data, numValues, &numBitsCompressed);
	compressedSize = numBitsCompressed / 8;
	free(dataCompressed);
	dataCompressed = NULL;

	return compressedSize;
}

//Just for sanity check
uint32_t test_xor_error_dataset(double * data, uint32_t numValues){
	uint8_t *dataCompressed = NULL;
	uint32_t numBitsCompressed = 0, numDifferentValues = 0;
	double *decompressedData = NULL;

	dataCompressed = xor_compression(data, numValues, &numBitsCompressed);
	decompressedData = xor_decompression(dataCompressed, numBitsCompressed, numValues);

	for(uint32_t i = 0; i < numValues; i++){
		if(data[i] != decompressedData[i]){
			numDifferentValues++;
		}
	}
	free(dataCompressed);
	free(decompressedData);
	dataCompressed = NULL;
	decompressedData = NULL;

	return numDifferentValues;
}
