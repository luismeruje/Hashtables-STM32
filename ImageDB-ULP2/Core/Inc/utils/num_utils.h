/*
 * num_utils.h
 *
 *  Created on: Dec 3, 2024
 *      Author: luisferreira
 */

#ifndef INC_NUM_UTILS_H_
#define INC_NUM_UTILS_H_

#include "stdint.h"

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

uint32_t scale_value(float value, float valueMin, float valueMax, float dataMin, float dataMax);

float calculate_scale_factor(float valueMin, float valueMax, float dataMin, float dataMax);

uint32_t scale_value_precomputed_scale(float value, float valueMin, float dataMin, float scale);

double scale_value_double(uint32_t value, uint32_t originMin, uint32_t originMax, double targetMin, double targetMax);


double randfrom(double min, double max);
double bound_number(double value, double max, double min);
double rand_double(double min, double max);

#endif /* INC_NUM_UTILS_H_ */
