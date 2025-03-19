/*
 * image_manipulator.h
 *
 *  Created on: Apr 19, 2024
 *      Author: luisferreira
 */

#ifndef INC_IMAGE_OUTPUT_H_
#define INC_IMAGE_OUTPUT_H_

#include "stdint.h"

//TODO: erase module
void print_image_pixels(uint8_t *image, uint32_t imageWidth, uint32_t imageHeight);
void print_image_pixels_alpha(uint8_t *image, uint32_t imageWidth, uint32_t imageHeight);



#endif /* INC_IMAGE_OUTPUT_H_ */
