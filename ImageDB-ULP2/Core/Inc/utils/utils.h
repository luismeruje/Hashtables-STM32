/*
 * commons.h
 *
 *  Created on: May 3, 2024
 *      Author: luisferreira
 */

#ifndef INC_UTILS_H_
#define INC_UTILS_H_

#include <stdint.h>


double clock_cycles_mean_time (uint32_t * clockCycles, uint32_t numValues);

double clock_cycles_time_standard_deviation (uint32_t *values, uint32_t numValues, double mean);

double euclidean_distance_2D(uint8_t * a, uint8_t * b);

double timer_mean_time (uint32_t * timerCounts, uint32_t numValues, uint32_t countsPerSec);

double timer_standard_deviation(uint32_t * timerCounts, uint32_t numValues, uint32_t countsPerSec, double mean);

#endif /* INC_UTILS_H_ */
