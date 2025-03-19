///*
// * commons.c
// *
// *  Created on: May 29, 2024
// *      Author: luisferreira
// */
//
//#include "commons.h"
//#include <stdlib.h>
//#include <math.h>
//#include "stm32u5xx_hal.h"
//
//double mean_absolute_error(int32_t * a1, int32_t * a2, int32_t size){
//	int32_t sum = 0;
//	for(int32_t i = 0; i < size; i++){
////		printf("Difference between %ld and %ld: %ld\n", a1[i], a2[i], abs(a1[i] - a2[i]));
//		sum += abs(a1[i] - a2[i]);
//
//	}
//
//	return ((double) sum) / size;
//}
//
///*
// * Takes integer value and scales and converts it to a double interval
// */
//double scale_value_double(uint32_t value, uint32_t originMin, uint32_t originMax, double targetMin, double targetMax){
//	double scaledValue = 0;
//	scaledValue = (((double)value - originMin) / ((double) originMax - originMin)) * (targetMax-targetMin) + targetMin;
//
//	return scaledValue;
//}
//
//uint32_t scale_value(double value, double valueMin, double valueMax, double dataMin, double dataMax){
//	uint32_t scaledValue = 0;
//	scaledValue = (uint32_t) ((((double)value - dataMin ) / (dataMax - dataMin)) * (valueMax - valueMin) + valueMin);
//
//	return scaledValue;
//}
//
///* generate a random floating point number from min to max */
////From: https://ubuntuforums.org/showthread.php?t=1717717&p=10618266#post10618266
//double randfrom(double min, double max)
//{
//    double range = (max - min);
//    double div = RAND_MAX / range;
//    return min + (rand() / div);
//}
//
//
////Duration in seconds
//double clock_cycles_mean_time (uint32_t * clockCycles, uint32_t numValues){
//	uint64_t sum  = 0;
//	uint32_t clockFreq = HAL_RCC_GetSysClockFreq(); //In some instances may not give correct value, but it is correct for 480MHz.
//	double duration = 0;
//
//	for(uint32_t i = 0; i < numValues; i++){
//		sum += clockCycles[i];
//	}
//
//    duration = ((double)sum / numValues) / (double)clockFreq;
//
//	return duration;
//}
//
//double timer_mean_time (uint32_t * timerCounts, uint32_t numValues, uint32_t countsPerSec){
//	uint64_t sum  = 0;
//	double duration = 0;
//
//	for(uint32_t i = 0; i < numValues; i++){
//		sum += timerCounts[i];
//	}
//	duration = ((double)sum / numValues) / (double)countsPerSec;
//	return duration;
//
//}
//
//double timer_standard_deviation(uint32_t * timerCounts, uint32_t numValues, uint32_t countsPerSec, double mean){
//	double result = 0;
//	double sum = 0;
//
//	for(uint32_t i = 0; i < numValues; i++){
//		result = ((double) timerCounts[i]) /  countsPerSec; //To pixels/sec
//		sum += pow((result - (double) mean), 2);
//	}
//	result = sqrt(sum / numValues);
//	return result;
//}
//
//double clock_cycles_time_standard_deviation (uint32_t *values, uint32_t numValues, double mean){
//	double result = 0;
//	double sum = 0;
//	uint32_t clockFreq = HAL_RCC_GetSysClockFreq();
//	for(uint32_t i = 0; i < numValues; i++){
//		result = ((double) values[i]) /  clockFreq; //To pixels/sec
//		sum += pow((result - (double) mean), 2);
//	}
//	result = sqrt(sum / numValues);
//	return result;
//}
//
//double euclidean_distance_2D(uint8_t * a, uint8_t * b){
//	double distance = 0;
//	distance = sqrt((double) pow(a[0] - b[0], 2) + (double)pow(a[1] - b[1], 2));
//	return distance;
//}
//
//
