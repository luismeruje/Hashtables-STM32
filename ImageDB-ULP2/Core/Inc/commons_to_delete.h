/*
 * commons.h
 *
 *  Created on: May 3, 2024
 *      Author: luisferreira
 */

#ifndef INC_COMMONS_H_
#define INC_COMMONS_H_

#include <stdint.h>

#define ALPHA 1
#define NO_ALPHA 0


double mean_absolute_error(int32_t * a1, int32_t * a2, int32_t size);

/*
 * @brief Function to scale a double from a [dataMin, dataMax] range of values to a [scaledMin, scaledMax] range
 * @param value Value to be scaled
 * @param scaledMin Minimum value for the number to be scaled.
 * @param scaledMax Maximum value for the number to be scaled.
 * @param dataMin Minimum value of the target scaled interval.
 * @param dataMax Maximum value of the target scaled interval.
 * @return The scaled value converted to an unsigned 32-bit integer.
 */

uint32_t scale_value(double value, double scaledMin, double scaledMax, double dataMin, double dataMax);


double scale_value_double(uint32_t value, uint32_t originMin, uint32_t originMax, double targetMin, double targetMax);


double randfrom(double min, double max);


double clock_cycles_mean_time (uint32_t * clockCycles, uint32_t numValues);

double clock_cycles_time_standard_deviation (uint32_t *values, uint32_t numValues, double mean);

double euclidean_distance_2D(uint8_t * a, uint8_t * b);

double timer_mean_time (uint32_t * timerCounts, uint32_t numValues, uint32_t countsPerSec);

double timer_standard_deviation(uint32_t * timerCounts, uint32_t numValues, uint32_t countsPerSec, double mean);

#endif /* INC_COMMONS_H_ */
