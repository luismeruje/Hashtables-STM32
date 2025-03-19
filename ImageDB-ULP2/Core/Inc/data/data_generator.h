/*
 * image-generator.h
 *
 *  Created on: Apr 26, 2024
 *      Author: luisferreira
 */

#ifndef INC_DATA_GENERATOR_H_
#define INC_DATA_GENERATOR_H_

#include <stdint.h>
#include "settings.h"




int generate_sine_data(double *dest, uint32_t numValues, double interval, double start);
int generate_sine_data_outliers(double *dest, uint32_t numValues, double interval, double start, double outlierPercentage);
void generate_solid_color(uint8_t * image, uint8_t alphaFlag, uint32_t imageWidth, uint32_t imageHeight, uint8_t *color);


#endif /* INC_DATA_GENERATOR_H_ */
