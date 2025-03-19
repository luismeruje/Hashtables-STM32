/*
 * vpp_image_generator.c
 *
 *  Created on: Aug 6, 2024
 *      Author: luisferreira
 */

#include "vpp_image_generator.h"
#include <stdlib.h>
#include "data_generator.h"
#include "vpp_image.h"

/*
 * Generate a value per pixel image with a given height x width size, and outlierPercentage % of outlier values. There is an option to generate the image with alpha values.
 */
uint8_t *generate_vpp_image_from_sine_data(double * data, uint32_t imageHeight, uint32_t imageWidth,  uint8_t imageHasAlpha,  uint8_t eightBlockZigZagFlag){
	uint8_t * vppImage = create_vpp_image(data,imageWidth * imageHeight, -1, 1, imageWidth * imageHeight, imageWidth, imageHasAlpha, eightBlockZigZagFlag);

	return vppImage;
}

double *generate_vpp_sine_data(uint32_t imageHeight, uint32_t imageWidth, double outliersPercentage){
	double * data = (double *) malloc(sizeof(double) * imageWidth * imageHeight);
	generate_sine_data_outliers(data, imageWidth * imageHeight, SINE_WAVE_STEP, rand(), outliersPercentage);

	return data;
}



/*
 * Generate a value per pixel image with a given height x width size, and outlierPercentage % of outlier values. There is an option to generate the image with alpha values.
 */
uint8_t *generate_vpp_image_outliers(uint32_t imageHeight, uint32_t imageWidth, uint8_t imageHasAlpha, double outliersPercentage, uint8_t eightBlockZigZagFlag){
	double * data = (double *) malloc(sizeof(double) * imageWidth * imageHeight);
	generate_sine_data_outliers(data, imageWidth * imageHeight, SINE_WAVE_STEP, rand(), outliersPercentage);
	uint8_t * vppImage = create_vpp_image(data,imageWidth * imageHeight, -1, 1, imageWidth * imageHeight, imageWidth, imageHasAlpha, eightBlockZigZagFlag);

	free(data);
	return vppImage;
}

uint8_t *generate_vpp_image_outliers_data(double ** data, uint32_t imageHeight, uint32_t imageWidth, uint8_t imageHasAlpha, double outliersPercentage, uint8_t eightBlockZigZagFlag){
	*data = (double *) malloc(sizeof(double) * imageWidth * imageHeight);
	generate_sine_data_outliers(*data, imageWidth * imageHeight, SINE_WAVE_STEP, rand(), outliersPercentage);
	uint8_t * vppImage = create_vpp_image(*data,imageWidth * imageHeight, -1, 1, imageWidth * imageHeight, imageWidth, imageHasAlpha, eightBlockZigZagFlag);

	return vppImage;
}

/*
 * Generate a value per pixel image with a given height x width size, given an external source of data. There is an option to generate the image with alpha values.
 */

uint8_t *generate_vpp_image_data(double * data, double dataMin, double dataMax, uint32_t numValues, uint32_t imageHeight, uint32_t imageWidth, uint8_t imageHasAlpha, uint8_t eightBlockZigZagFlag){
	uint8_t * vppImage = create_vpp_image(data, numValues, dataMin, dataMax, imageWidth * imageHeight, imageWidth, imageHasAlpha, eightBlockZigZagFlag);

	return vppImage;
}

/*
 * Generate a value per pixel image with a given height x width size. There is an option to generate the image with alpha values.
 */
uint8_t *generate_vpp_image(uint32_t imageHeight, uint32_t imageWidth, uint8_t imageHasAlpha, uint8_t eightBlockZigZagFlag){
	return generate_vpp_image_outliers(imageHeight, imageWidth, imageHasAlpha, 0, eightBlockZigZagFlag);
}
