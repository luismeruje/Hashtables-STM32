/*
 * file_image_array_creator.h
 *
 *  Created on: Jul 1, 2024
 *      Author: luisferreira
 */

#ifndef INC_TEST_OL_IMAGES_H_
#define INC_TEST_OL_IMAGES_H_

#include "stdint.h"

int test_ol_JPEG_CPU(uint32_t imageWidth, uint32_t imageHeight);
int test_ol_blend_accelerator(uint32_t imageWidth, uint32_t imageHeight);
int test_ol_blend_jpeg_combo(uint32_t imageHeight, uint32_t imageWidth, uint32_t numberStackedMeasurements, uint8_t quality);
int test_decode_algorithm_jpeglib(uint32_t imageWidth, uint32_t imageHeight, uint32_t numOverlappedLines);
double test_ol_decode_algorithm_accelerators(uint32_t imageWidth, uint32_t imageHeight, uint32_t numOverlappedLines, uint16_t quality);
int test_ol_JPEG_accelerator_CPU(uint32_t imageWidth, uint32_t imageHeight, uint32_t quality);
int test_ol_JPEG_accelerator_MDMA();
int test_ol_blending_single_pixels_accelerator();

void print_example_ol_images_accelerator();
void print_example_ol_images_libjpeg();

#endif /* INC_TEST_OL_IMAGES_H_ */
