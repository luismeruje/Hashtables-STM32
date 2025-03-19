/*
 * metrics_collector.h
 *
 *  Created on: Apr 15, 2024
 *      Author: luisferreira
 */

#ifndef INC_TEST_VPP_IMAGES_H_
#define INC_TEST_VPP_IMAGES_H_


double test_error_vpp(uint32_t imageHeight, uint32_t imageWidth, uint8_t quality, uint8_t eightBlockZigZagFlag);
#if DATASET_DATA > 0
double test_size_vpp_dataset(double * data, double dataMin, double dataMax, uint32_t imageHeight, uint32_t imageWidth, uint8_t quality,  uint8_t eightBlockZigZagFlag);
#endif
double test_mse_error_vpp_dataset(double * data, double dataMin, double dataMax, uint32_t imageHeight, uint32_t imageWidth, uint8_t quality,  uint8_t eightBlockZigZagFlag);
double test_error_vpp_outliers(uint32_t imageHeight, uint32_t imageWidth, uint8_t quality, double outlierPercentage, uint8_t eightBlockZigZagFlag);
uint32_t test_size_sine_vpp(uint32_t imageHeight, uint32_t imageWidth, uint8_t quality, uint8_t eightBlockZigZagFlag);
uint32_t test_size_sine_vpp_outliers(uint32_t imageHeight, uint32_t imageWidth, uint8_t quality, double outlierPercentage, uint8_t eightBlockZigZagFlag);
double test_vpp_performance_JPEG_encode_accelerator_DMA_w_quantization(uint32_t imageHeight, uint32_t imageWidth, uint32_t numImageRepeats, uint8_t imageQuality,double outlierPercentage, uint8_t eightBlockZigZagFlag, double *stddev);
double test_vpp_performance_JPEG_encode_accelerator_DMA_w_quantization_dataset(double * data, double dataMin, double dataMax, uint32_t dataValues, uint32_t imageHeight, uint32_t imageWidth, uint32_t numImageRepeats, uint8_t imageQuality, uint8_t eightBlockZigZagFlag, double *stddev);

#endif /* INC_TEST_VPP_IMAGES_H_ */
