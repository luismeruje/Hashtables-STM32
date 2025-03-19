/*
 * ol_image.h
 *
 *  Created on: Aug 6, 2024
 *      Author: luisferreira
 */

#ifndef INC_OL_IMAGE_H_
#define INC_OL_IMAGE_H_

#include <stdint.h>
/*
 * @brief Creates an overlapped line image (OL) from an array of data
 * @param values Values to be converted to image
 * @param image Pointer to destination image array
 * @param r Red value for pixels' RGB color
 * @param g Green value for pixels' RGB color
 * @param b Blue value for pixels' RGB color
 * @param alphaFlag Flag signaling whether to set alpha value.
 * @param dataMin Minimum possible value for each value in the "values" array
 * @param dataMax Maximum possible value for each value in the "values" array
 * @param imageWidth Destination image width
 * @param imageHeight Destination image height
 * @return Returns 0 if successful
 */

int data_to_ol_image(double* values, uint8_t * image, uint8_t r, uint8_t g, uint8_t b, uint8_t alphaFlag, int32_t dataMin, int32_t dataMax, uint32_t imageWidth, uint32_t imageHeight);

/*
 * @brief Converts a data array to the heights those data points would occupy in an OL image.
 * @param data Array of data values to convert
 * @param imageHeight Height of OL image.
 * @param imageWidth  Width of OL image.
 * @param dataMax	  Max possible value for data
 * @param dataMin	  Min possible value for data
 * @return Array of heights resulting from conversion.
 */
uint32_t * data_to_ol_heights(double * data, uint32_t imageHeight, uint32_t imageWidth, double dataMax, double dataMin);


uint32_t * image_to_quantized_values_array_ol_image(uint8_t * image, uint8_t alphaFlag, uint32_t imageWidth, uint32_t imageHeight, uint8_t target_r, uint8_t target_g, uint8_t target_b);

#endif /* INC_OL_IMAGE_H_ */
