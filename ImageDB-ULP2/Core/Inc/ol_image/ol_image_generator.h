/*
 * ol_image_generator.h
 *
 *  Created on: Aug 6, 2024
 *      Author: luisferreira
 */

#ifndef INC_OL_IMAGE_GENERATOR_H_
#define INC_OL_IMAGE_GENERATOR_H_

#include <stdint.h>

typedef struct multilineImage{
    uint8_t * image;
    uint8_t ** colors;
    uint32_t numLines;
} MultilineImage;

//Not implemented
uint8_t * get_color_by_index(uint32_t i);

double * get_sine_values_by_line_index(uint32_t imageWidth, uint32_t index);
MultilineImage * generate_ol_image(uint32_t imageHeight, uint32_t imageWidth, uint32_t numberStackedMeasurements, uint8_t alphaFlag);
void destroyMultilineImage(MultilineImage * multilineImage);


#endif /* INC_OL_IMAGE_GENERATOR_H_ */
