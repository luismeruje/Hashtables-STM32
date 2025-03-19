/*
 * vpp_image.h
 *
 *  Created on: Aug 6, 2024
 *      Author: luisferreira
 */

#ifndef INC_VPP_IMAGE_H_
#define INC_VPP_IMAGE_H_

#include "stdint.h"

/*
 * @brief Converts data array to vpp image.
 * @param values Array of values to convert.
 * @param dataMin Minimum possible value for a data value.
 * @param dataMax Maximum possible value for a data value.
 * @param numValues Number of values to convert.
 * @param imageHasAlpha Flag that signals whether data should be generated with alpha value.
 * @return Pointer to value per point (vpp) image. Array must be later freed by caller using the returned pointer.
 */
uint8_t * create_vpp_image(double *values, uint32_t valuesSize, double dataMin, double dataMax, uint32_t numValues, uint32_t imageWidth, uint8_t imageHasAlpha, uint8_t eightBlockZigzagFlag);

/*
 * Mean absolute error of quantized vectors for N_VECTOR quantization method
 */
double mean_absolute_quantized_error(uint32_t * pointsA, uint32_t * pointsB, uint32_t numPixels);


double mean_direct_value_error(uint32_t * pointsA, uint32_t * pointsB, uint32_t numPixels);

/*
 * Convert pixel to nearest correct pixel within allowed N vectors.
 */
void set_to_nearest_valid_pixel(uint8_t *pixel);

/*
 * Convert pixel to nearest correct pixel within allowed R G or B vectors.
 */
void set_to_nearest_valid_rgb_pixel(uint8_t *pixel);

/*
 * Set N for N vector encoding scheme
 */
void vpp_set_number_vectors(uint32_t numVectors);

/*
 * Get number of vectors from N vector encoding scheme
 */
uint32_t get_number_vectors();


double calculate_mean_scaled_absolute_error_vpp_image(uint8_t *image, double * data, uint32_t numValues, double dataMin, double dataMax);

double calculate_mean_square_error_vpp_image(uint8_t *image, double * data, uint32_t numValues, double dataMin, double dataMax);


/*
 * Convert a value that was encoded with DIRECT_VALUE encoding back to its corresponding double value
 */
double direct_value_pixel_to_double(uint8_t *pixel, double dataMin, double dataMax);


/*
 * Set minimum vector value for R G B encoding method
 */
void set_min_value_RGB(uint8_t min_value);


/*
 * Get minimum vector value set for R G B encoding method
 */
uint8_t get_min_value_RGB();



#endif /* INC_VPP_IMAGE_H_ */
