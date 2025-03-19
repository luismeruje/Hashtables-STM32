/*
 * accelerator_raw_performance_benchmark.h
 *
 *  Created on: Aug 28, 2024
 *      Author: luisferreira
 */

#ifndef INC_BENCHMARK_OL_IMAGE_H_
#define INC_BENCHMARK_OL_IMAGE_H_

#include <stdint.h>

//TODO: change all these functions to a single one with configuration structure
/*
 * @brief Benchmark the size of compressed jpeg format when data is converted to a series of overlapping plot lines.
 * @param quality List of quality jpeg compression values to test.
 * @param numQuality Number of elements in quality list
 * @param overlappedLines List of number of overlappedLines per image to test.
 * @param numOverlappedLines Number of elements in overlappedLines list.
 * @param imageHeight List of image heights to test. Should have the same number of elements as imageWidth.
 * @param imageWidth List of image widths to test. Should have the same number of elements as imageHeight.
 * @param numImageSizes Number of Height x Width pairs.
 * @return Returns 0 when all tests run successfully.
 */
int benchmark_size_jpeg_ol_image(uint32_t *quality, uint32_t numQuality, uint32_t *overlappedLines, uint32_t numOverlappedLines, uint32_t *imageHeight, uint32_t *imageWidth, uint32_t numImageSizes);


/*
 * @brief Benchmark the decoding error associated to overlapping plot lines (ol) images.
 * @param quality List of quality jpeg compression values to test.
 * @param numQuality Number of elements in quality list
 * @param overlappedLines List of number of overlappedLines per image to test.
 * @param numOverlappedLines Number of elements in overlappedLines list.
 * @param imageHeight List of image heights to test. Should have the same number of elements as imageWidth.
 * @param imageWidth List of image widths to test. Should have the same number of elements as imageHeight.
 * @param numImageSizes Number of Height x Width pairs.
 * @return Returns 0 when all tests run successfully.
 */
int benchmark_error_ol_image(uint32_t *quality, uint32_t numQuality, uint32_t *overlappedLines, uint32_t numOverlappedLines, uint32_t *imageHeight, uint32_t *imageWidth, uint32_t numImageSizes);

uint32_t benchmark_jpeg_encode_accelerator_MDMA(uint32_t numIterations, uint32_t numImages, uint32_t imageHeight, uint32_t imageWidth, uint8_t quality);
uint32_t benchmark_jpeg_encode_CPU(uint32_t numIterations, uint32_t numImages, uint32_t imageHeight, uint32_t imageWidth, uint8_t quality);
uint32_t benchmark_jpeg_decode_CPU_ol_image(uint32_t numIterations, uint32_t numImages, uint32_t imageHeight, uint32_t imageWidth, uint8_t quality);
uint32_t benchmark_jpeg_decode_accelerator_MDMA_ol_image(uint32_t numIterations, uint32_t numImages, uint32_t imageHeight, uint32_t imageWidth, uint8_t quality);
uint32_t benchmark_jpeg_decode_accelerator_CPU_ol_image(uint32_t numIterations, uint32_t numImages, uint32_t imageHeight, uint32_t imageWidth, uint8_t quality);
uint32_t benchmark_blending_DMA2D_ol_image(uint32_t numIterations, uint32_t numImages, uint32_t imageHeight, uint32_t imageWidth);
uint32_t benchmark_blending_CPU_ol_image(uint32_t numIterations, uint32_t numImages, uint32_t imageHeight, uint32_t imageWidth);
uint32_t benchmark_blending_CPU_simple_ol_image(uint32_t numIterations, uint32_t numImages, uint32_t imageHeight, uint32_t imageWidth);


#endif /* INC_BENCHMARK_OL_IMAGE_H_ */
