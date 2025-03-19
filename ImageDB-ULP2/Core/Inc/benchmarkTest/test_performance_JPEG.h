/*
 * test_accelerators.h
 *
 *  Created on: Sep 2, 2024
 *      Author: luisferreira
 */

#ifndef INC_TEST_PERFORMANCE_JPEG_H_
#define INC_TEST_PERFORMANCE_JPEG_H_
#include <stdint.h>

double test_performance_YCbCr_encode_CPU(uint32_t imageHeight, uint32_t imageWidth, uint32_t numImageRepeats, uint32_t batchSize, uint8_t imageType);

double test_performance_YCbCr_decode_CPU(uint32_t imageHeight, uint32_t imageWidth, uint32_t numImageRepeats, uint32_t batchSize, uint8_t imageType);

void test_performance_JPEG_encode_accelerator_DMA(uint32_t imageHeight, uint32_t imageWidth, uint32_t numImageRepeats, uint8_t imageQuality, uint8_t imageType, double *performance, double *stddev, uint8_t timeQuantization);

void test_performance_JPEG_decode_accelerator_DMA(uint32_t imageHeight, uint32_t imageWidth, uint32_t numImageRepeats, uint8_t imageQuality, uint8_t imageType, double *performance, double *stddev);

void test_performance_JPEG_encode_accelerator_IT(uint32_t imageHeight, uint32_t imageWidth, uint32_t numImageRepeats, uint8_t imageQuality, uint8_t imageType, double *performance, double *stddev, uint8_t timeQuantization);

void test_performance_JPEG_decode_accelerator_IT(uint32_t imageHeight, uint32_t imageWidth, uint32_t numImageRepeats, uint8_t imageQuality, uint8_t imageType, double *performance, double *stddev);

void test_performance_JPEG_encode_CPU(uint32_t imageHeight, uint32_t imageWidth, uint32_t numImageRepeats, uint8_t imageQuality, uint8_t imageType, double *performance, double *stddev, uint8_t timeQuantization);

void test_performance_JPEG_decode_CPU(uint32_t imageHeight, uint32_t imageWidth, uint32_t numImageRepeats, uint8_t imageQuality, uint8_t imageType, double *performance, double *stddev);

double test_performance_JPEG_encode_accelerator_DMA_raw_dataset(double * data, double dataMin, double dataMax, uint32_t numValues, uint32_t imageHeight, uint32_t imageWidth, uint32_t numImageRepeats, uint8_t imageQuality, uint8_t eightBlockZigZagFlag, double *stddev);

#endif /* INC_TEST_PERFORMANCE_JPEG_H_ */
