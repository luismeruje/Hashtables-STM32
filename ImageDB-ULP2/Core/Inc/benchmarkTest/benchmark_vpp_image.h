/*
 * benchmark_image_performance.h
 *
 *  Created on: Apr 26, 2024
 *      Author: luisferreira
 */


#ifndef INC_BENCHMARK_IMAGE_PERFORMANCE_H_
#define INC_BENCHMARK_IMAGE_PERFORMANCE_H_
#include "settings.h"
#include <stdint.h>
#include "benchmark_from_uart.h"
//
//typedef struct buffers Buffers;
//
//
//typedef enum EnergyInterval{
//	ENERGY_INTERVAL_ON,
//	ENERGY_INTERVAL_OFF
//} ENERGY_INTERVAL_FLAG;
//
//typedef enum loopIntervalType{
//	ARRAY_VALUES,
//	INTERVAL
//} LoopIntervalType;
//
//typedef enum imageSource{
//	SINE,
//	SINE_OUTLIERS,
//	DATASET
//}ImageSource;
//
//typedef enum jpegOperations{
//	ENCODE,
//	ENCODE_DECODE
//} JpegOperations;


//typedef struct benchmarkLoopConfig{
//	LoopIntervalType loopType;
//	void (*loopInitFunc) (int d);
//	int *interval; //Either NULL, or list of values, depending on loopType
//	int intervalSize;
//
//} BenchmarkLoopConfig;

//typedef struct imageGenerator{
//
//}ImageGenerator;


//Note: VPP image -> value per pixel image
//		OL image -> data converted to overlapping visual plot lines

//TODO: change to single function with extra config structure


/*
 * @brief Benchmark the decoding error associated to value per pixel (vpp) images.
 * @param quality List of quality jpeg compression values to test.
 * @param numQuality Number of elements in quality list
 * @param imageHeight List of image heights to test. Should have the same number of elements as imageWidth.
 * @param imageWidth List of image widths to test. Should have the same number of elements as imageHeight.
 * @param numImageSizes Number of Height x Width pairs.
 * @param numIterationsAvg Number of times each combination of parameters is tested to reach an average.
 * @return Returns 0 when all tests run successfully.
 */
int benchmark_error_vpp_sine(uint32_t *quality, uint32_t numQuality, uint32_t *imageHeight, uint32_t *imageWidth, uint32_t numImageSizes, uint32_t numIterationsAvg,  uint32_t * amountVectors, uint32_t numAmountVectors, uint32_t * minRGBSize, uint32_t numMinRGBSizes);

/*
 * @brief Benchmark the decoding error associated to value per pixel (vpp) images considering outliers.
 * @param quality List of quality jpeg compression values to test.
 * @param numQuality Number of elements in quality list
 * @param imageHeight List of image heights to test. Should have the same number of elements as imageWidth.
 * @param imageWidth List of image widths to test. Should have the same number of elements as imageHeight.
 * @param numImageSizes Number of Height x Width pairs.
 * @param outlierPercentage List of outlier percentages to test.
 * @param numOutlierPercentages Number of outlier percentage values.
 * @param numIterationsAvg Number of times each combination of parameters is tested to reach an average.
 * @return Returns 0 when all tests run successfully.
 */
int benchmark_error_vpp_sine_outliers(uint32_t *quality, uint32_t numQuality, uint32_t *imageHeight, uint32_t *imageWidth, uint32_t numImageSizes, double * outlierPercentage, uint32_t numOutlierPercentages, uint32_t numIterationsAvg,  uint32_t * amountVectors, uint32_t numAmountVectors, uint32_t * minRGBSize, uint32_t numMinRGBSizes);

#if DATASET_DATA > 0

/*
 * @brief Benchmark the decoding error associated to value per pixel (vpp) images from dataset data.
 * @param quality List of quality jpeg compression values to test.
 * @param numQuality Number of elements in quality list
 * @param imageHeight List of image heights to test. Should have the same number of elements as imageWidth.
 * @param imageWidth List of image widths to test. Should have the same number of elements as imageHeight.
 * @param numImageSizes Number of Height x Width pairs.
 * @param outlierPercentage List of outlier percentages to test.
 * @param numOutlierPercentages Number of outlier percentage values.
 * @param numIterationsAvg Number of times each combination of parameters is tested to reach an average.
 * @param numAmountVectors Number of vectors to consider for quantization. Only applied when VPP_ENCODE is set to N_VECTOR
 * @return Returns 0 when all tests run successfully.
 */
int benchmark_error_vpp_dataset_image(uint32_t *quality, uint32_t numQuality, uint32_t *imageHeight, uint32_t *imageWidth, uint32_t numImageSizes, uint32_t numIterationsAvg, uint32_t * amountVectors, uint32_t numAmountVectors, uint32_t * minRGBSize, uint32_t numMinRGBSizes);
/*
 * @brief Benchmark the compressed size associated of value per pixel (vpp) images from dataset data.
 * @param quality List of quality jpeg compression values to test.
 * @param numQuality Number of elements in quality list
 * @param imageHeight List of image heights to test. Should have the same number of elements as imageWidth.
 * @param imageWidth List of image widths to test. Should have the same number of elements as imageHeight.
 * @param numImageSizes Number of Height x Width pairs.
 * @param outlierPercentage List of outlier percentages to test.
 * @param numOutlierPercentages Number of outlier percentage values.
 * @param numIterationsAvg Number of times each combination of parameters is tested to reach an average.
 * @return Returns 0 when all tests run successfully.
 */

int benchmark_size_vpp_dataset(uint32_t *quality, uint32_t numQuality, uint32_t *imageHeight, uint32_t *imageWidth, uint32_t numImageSizes, uint32_t numIterationsAvg,  uint32_t * amountVectors, uint32_t numAmountVectors, uint32_t * minRGBSize, uint32_t numMinRGBSizes);

#endif

/*
 * @brief Benchmark the size of value per pixel (vpp) images after JPEG encoding. Uses sine wave as source data.
 * @param quality List of quality jpeg compression values to test.
 * @param numQuality Number of elements in quality list
 * @param imageHeight List of image heights to test. Should have the same number of elements as imageWidth.
 * @param imageWidth List of image widths to test. Should have the same number of elements as imageHeight.
 * @param numImageSizes Number of Height x Width pairs.
 * @param numIterationsAvg Number of times each combination of parameters is tested to reach an average.
 * @return Returns 0 when all tests run successfully.
 */
int benchmark_size_sine_vpp(uint32_t *quality, uint32_t numQuality, uint32_t *imageHeight, uint32_t *imageWidth, uint32_t numImageSize, uint32_t numIterationsAvg, uint32_t * amountVectors, uint32_t numAmountVectors, uint32_t * minRGBSize, uint32_t numMinRGBSizes);

int benchmark_JPEG_error_vpp_dataset(BenchmarkSettings *settings);

/*
 * @brief Benchmark the size of value per pixel (vpp) images after JPEG encoding. Uses sine wave with a given percentage of outliers as source data.
 * @param quality List of quality jpeg compression values to test.
 * @param numQuality Number of elements in quality list
 * @param imageHeight List of image heights to test. Should have the same number of elements as imageWidth.
 * @param imageWidth List of image widths to test. Should have the same number of elements as imageHeight.
 * @param numImageSizes Number of Height x Width pairs.
 * @param numIterationsAvg Number of times each combination of parameters is tested to reach an average.
 * @return Returns 0 when all tests run successfully.
 */
int benchmark_size_sine_outliers_vpp(uint32_t *quality, uint32_t numQuality, uint32_t *imageHeight, uint32_t *imageWidth, uint32_t numImageSizes, double * outlierPercentage, uint32_t numOutlierPercentages, uint32_t numIterationsAvg, uint32_t * amountVectors, uint32_t numAmountVectors, uint32_t * minRGBSize, uint32_t numMinRGBSizes);

int benchmark_performance_w_quantization_outliers_vpp(uint32_t *quality, uint32_t numQuality, uint32_t *imageHeight, uint32_t *imageWidth, uint32_t numImageSizes, uint32_t *numImageRepeats, double * outlierPercentage, uint32_t numOutlierPercentages, uint32_t numIterationsAvg, uint32_t * amountVectors, uint32_t numAmountVectors, uint32_t * minRGBSize, uint32_t numMinRGBSizes);

#if DATASET_DATA > 0
int benchmark_performance_w_quantization_dataset_vpp(uint32_t *quality, uint32_t numQuality, uint32_t *imageHeight, uint32_t *imageWidth, uint32_t numImageSizes, uint32_t *numImageRepeats, uint32_t numIterationsAvg, uint32_t * amountVectors, uint32_t numAmountVectors, uint32_t * minRGBSize, uint32_t numMinRGBSizes);
#endif

int benchmark_performance_encode_accelerator_jpeg_vpp(BenchmarkSettings *settings);

#endif /* INC_BENCHMARK_IMAGE_PERFORMANCE_H_ */
