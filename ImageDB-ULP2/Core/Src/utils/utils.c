/*
 * commons.c
 *
 *  Created on: May 29, 2024
 *      Author: luisferreira
 */

#include <stdlib.h>
#include <math.h>
#include <utils.h>
#include "stm32u5xx_hal.h"




//Duration in seconds
double clock_cycles_mean_time (uint32_t * clockCycles, uint32_t numValues){
	uint64_t sum  = 0;
	uint32_t clockFreq = HAL_RCC_GetSysClockFreq(); //In some instances may not give correct value, but it is correct for 480MHz.
	double duration = 0;

	for(uint32_t i = 0; i < numValues; i++){
		sum += clockCycles[i];
	}

    duration = ((double)sum / numValues) / (double)clockFreq;

	return duration;
}

double timer_mean_time (uint32_t * timerCounts, uint32_t numValues, uint32_t countsPerSec){
	uint64_t sum  = 0;
	double duration = 0;

	for(uint32_t i = 0; i < numValues; i++){
		sum += timerCounts[i];
	}
	duration = ((double)sum / numValues) / (double)countsPerSec;
	return duration;

}

double timer_standard_deviation(uint32_t * timerCounts, uint32_t numValues, uint32_t countsPerSec, double mean){
	double result = 0;
	double sum = 0;

	for(uint32_t i = 0; i < numValues; i++){
		result = ((double) timerCounts[i]) /  countsPerSec; //To pixels/sec
		sum += pow((result - (double) mean), 2);
	}
	result = sqrt(sum / numValues);
	return result;
}

double clock_cycles_time_standard_deviation (uint32_t *values, uint32_t numValues, double mean){
	double result = 0;
	double sum = 0;
	uint32_t clockFreq = HAL_RCC_GetSysClockFreq();
	for(uint32_t i = 0; i < numValues; i++){
		result = ((double) values[i]) /  clockFreq; //To pixels/sec
		sum += pow((result - (double) mean), 2);
	}
	result = sqrt(sum / numValues);
	return result;
}

double euclidean_distance_2D(uint8_t * a, uint8_t * b){
	double distance = 0;
	distance = sqrt((double) pow(a[0] - b[0], 2) + (double)pow(a[1] - b[1], 2));
	return distance;
}


