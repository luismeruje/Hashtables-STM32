/*
 * vpp_image_generator.h
 *
 *  Created on: Aug 6, 2024
 *      Author: luisferreira
 */

#ifndef INC_VPP_IMAGE_GENERATOR_H_
#define INC_VPP_IMAGE_GENERATOR_H_

#include "settings.h"
#include <stdint.h>
/*
 * @brief Generate vpp image based on a sine wave pattern with a size of imageWidth x imageHeight
 * @param imageHeight Image height in pixels.
 * @param imageWidth Image width in pixels.
 * @param imageHasAlpha Flag to signal whether image should be generated with alpha field.
 * @param outliersPercentage Percentage of outlier values in final image.
 * @return Pointer to generated vpp image.
 */
uint8_t * generate_vpp_image(uint32_t imageHeight, uint32_t imageWidth, uint8_t imageHasAlpha,  uint8_t eightBlockZigZagFlag);

/*
 * @brief Generate vpp image based on a sine wave pattern with a given percentage of outliers and a size of imageWidth x imageHeight
 * @param imageHeight Image height in pixels.
 * @param imageWidth Image width in pixels.
 * @param imageHasAlpha Flag to signal whether image should be generated with alpha field.
 * @param outliersPercentage Percentage of outlier values in final image.
 * @return Pointer to generated vpp image.
 */
uint8_t *generate_vpp_image_outliers(uint32_t imageHeight, uint32_t imageWidth, uint8_t imageHasAlpha, double outliersPercentage,  uint8_t eightBlockZigZagFlag);

/* Fills in the data */
uint8_t *generate_vpp_image_outliers_data(double ** data, uint32_t imageHeight, uint32_t imageWidth, uint8_t imageHasAlpha, double outliersPercentage, uint8_t eightBlockZigZagFlag);

uint8_t *generate_vpp_image_from_sine_data(double * data, uint32_t imageHeight, uint32_t imageWidth,  uint8_t imageHasAlpha,  uint8_t eightBlockZigZagFlag);

double *generate_vpp_sine_data(uint32_t imageHeight, uint32_t imageWidth, double outliersPercentage);

/*
 * @brief Generate vpp image from a predefined dataset array and a size of imageWidth x imageHeight
 * @param imageHeight Image height in pixels.
 * @param imageWidth Image width in pixels.
 * @param imageHasAlpha Flag to signal whether image should be generated with alpha field.
 * @param outliersPercentage Percentage of outlier values in final image.
 * @return Pointer to generated vpp image.
 */
uint8_t *generate_vpp_image_data(double * data, double dataMin, double dataMax, uint32_t numValues, uint32_t imageHeight, uint32_t imageWidth, uint8_t imageHasAlpha, uint8_t eightBlockZigZagFlag);

#endif /* INC_VPP_IMAGE_GENERATOR_H_ */
