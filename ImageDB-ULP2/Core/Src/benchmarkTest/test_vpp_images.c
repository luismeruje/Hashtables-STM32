/*
 * metrics_collector.c
 *
 *  Created on: Apr 11, 2024
 *      Author: luisferreira
 */
#include <compression/jpeg-manipulator.h>
#include <image_output.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <test_vpp_images.h>
#include <utils.h>
#include "stm32u5xx_hal.h"
#include "stdbool.h"
#include "data_generator.h"
#include "settings.h"
#include "vpp_image.h"
#include "vpp_image_generator.h"
#include "ol_image_generator.h"
#include "ol_image.h"
#include "num_utils.h"
#include "ol_image_blender.h"
#define IMAGE_SIZE 128
#define QUALITY 50

extern TIM_HandleTypeDef htim2;

uint32_t absolute_error(uint32_t *a1, uint32_t *a2, uint32_t numValues){
	double absolute_error = 0;
	int32_t difference = 0;
	for(uint32_t i = 0; i < numValues; i++){

		 difference = (int32_t) ((int64_t) a1[i] - (int64_t)a2[i]);
		 if(difference < 0){
			 difference = -difference;
		 }
		 absolute_error+= (double)difference;
	}

	return  (uint32_t) (absolute_error / numValues);
}


#if JPEG_ACCELERATOR == ON
/*
 * Compresses image and returns resulting size.
 */
uint32_t _test_size_compressed_vpp(uint8_t *image, uint32_t imageHeight, uint32_t imageWidth, uint8_t quality){
	int result = 0;
	uint32_t size = 0;
	JPEG_ConfTypeDef * jpegConf = new_jpeg_conf(imageHeight, imageWidth, quality);
	uint32_t jpegSize = (uint32_t)((double)imageWidth * (double)imageHeight * JPEG_MULTIPLIER);
	jpegSize = jpegSize - (jpegSize % 64); //Multiple of 8x8
	uint8_t * JPEGImage = malloc(jpegSize);
	memset(JPEGImage, 0, jpegSize);
	result = encode_JPEG_accelerator_MDMA(image, sizeof(uint8_t) * imageHeight * imageWidth * 4 , JPEGImage, jpegSize, jpegConf);
	if(result != 0){
		printf("Error occurred while encoding JPEG image\n");
		return 123456789;
	}
	size = getJPEGSize(JPEGImage, jpegSize);
	free(JPEGImage);
	free(jpegConf);
	return size;

}


uint32_t test_size_sine_vpp(uint32_t imageHeight, uint32_t imageWidth, uint8_t quality, uint8_t eightBlockZigZagFlag){
	uint32_t size = 0;
	uint8_t * image = generate_vpp_image( imageHeight, imageWidth, true, eightBlockZigZagFlag);
	//print_image_pixels_alpha(image, imageWidth, imageHeight);
	size = _test_size_compressed_vpp(image, imageHeight, imageWidth, quality);
//	print_image_pixels_alpha(returnImage, imageWidth, imageHeight);
	free(image);
	return size;
}


uint32_t test_size_sine_vpp_outliers(uint32_t imageHeight, uint32_t imageWidth, uint8_t quality, double outlierPercentage, uint8_t eightBlockZigZagFlag){
	uint8_t * image = generate_vpp_image_outliers(imageHeight, imageWidth, true, outlierPercentage, eightBlockZigZagFlag);
	uint32_t size = 0;

	//	print_image_pixels_alpha(image, imageWidth, imageHeight);
	size = _test_size_compressed_vpp(image, imageHeight, imageWidth, quality);


	free(image);
	return size;
}
#endif

//double absolute_error_zigzag(uint8_t *image, uint32_t *values, uint32_t imageWidth, uint32_t imageHeight){
//	double absoluteError = 0;
//	int32_t difference = 0;
//	uint32_t zigzagOrder[64] = {
//		0,  1,  8, 16,  9,  2,  3, 10,
//	   17, 24, 32, 25, 18, 11,  4,  5,
//	   12, 19, 26, 33, 40, 48, 41, 34,
//	   27, 20, 13,  6,  7, 14, 21, 28,
//	   35, 42, 49, 56, 57, 50, 43, 36,
//	   29, 22, 15, 23, 30, 37, 44, 51,
//	   58, 59, 52, 45, 38, 31, 39, 46,
//	   53, 60, 61, 54, 47, 55, 62, 63
//	};
//	uint32_t valuesIndex = 0;
//
//	for (uint32_t blockRow = 0; blockRow < imageHeight; blockRow += 8) {
//		for (uint32_t blockCol = 0; blockCol < imageWidth; blockCol += 8) {
//			for (uint32_t i = 0; i < 64; ++i) {
//				uint32_t zigzagIndex = zigzagOrder[i];
//				uint32_t localRow = zigzagIndex / 8;
//				uint32_t localCol = zigzagIndex % 8;
//				uint32_t imageRow = blockRow + localRow;
//				uint32_t imageCol = blockCol + localCol;
//				uint32_t pixelIndex = (imageRow * imageWidth + imageCol);
//				difference = (int32_t) (image[pixelIndex + 0] - values[valuesIndex + 0]); //0 index for red
//				if(difference < 0){
//					difference = -difference;
//				}
//				absoluteError += (double) difference;
//				valuesIndex++;
//			}
//		}
//	}
//	return (double) (absoluteError / valuesIndex);
//}

double absolute_error_red(uint32_t *a1, uint32_t *a2, uint32_t numValues){
	double absolute_error = 0;
	int32_t difference = 0;
	uint8_t * image1 = (uint8_t *) a1;
	uint8_t * image2 = (uint8_t *) a2;
	for(uint32_t i = 0; i < numValues; i++){
		 //printf("Red value: %u\n", image1[i*4 + 0]);
		 difference = (int32_t) (image1[i*4 + 0] - image2[i*4 + 0]); //0 index for red
		 if(difference < 0){
			 difference = -difference;
		 }
		 absolute_error+= (double)difference;
	}

	return  (double) (absolute_error / numValues);
}

//Move to vpp_image module
uint32_t get_rgb_vector_value(uint8_t *pixel, uint8_t minVectorValue){
	uint32_t value = 0;
	if(pixel[0] > 0){
		value = pixel[0] - minVectorValue;
	} else if(pixel[1] > 0) {
		value = 255 + (uint32_t) pixel[1] - ((uint32_t)minVectorValue * 2);
	} else if(pixel[2] > 0){
		value = 511 + (uint32_t) pixel[2] - ((uint32_t) minVectorValue * 3);
	}
	return value;
}

/*
 * Return image is the one that must be corrected
 */
double mean_rgb_value_error(uint32_t * a1, uint32_t * a2, uint32_t numValues){
	double absolute_error = 0;
	int32_t difference = 0;
	uint8_t minVectorValue = get_min_value_RGB();
	uint8_t * image1 = (uint8_t *) a1;
	uint8_t * image2 = (uint8_t *) a2;
	uint32_t valueA = 0;
	uint32_t valueB = 0;
	for(uint32_t i = 0; i < numValues; i++){
		valueA = get_rgb_vector_value(&image1[i*4], minVectorValue);
		valueB = get_rgb_vector_value(&image2[i*4], minVectorValue);
		//printf("Red value: %u\n", image1[i*4 + 0]);
		difference = (int32_t) (valueA - valueB); //0 index for red
		if(difference < 0){
		    difference = -difference;
		}
		absolute_error += (double)difference;
	}

	return  (double) (absolute_error / numValues);
}


#if DATASET_DATA > 0

double test_size_vpp_dataset(double * data, double dataMin, double dataMax, uint32_t imageHeight, uint32_t imageWidth, uint8_t quality,  uint8_t eightBlockZigZagFlag){
	JPEG_ConfTypeDef * jpegConf = new_jpeg_conf(imageHeight, imageWidth, quality);
	uint8_t* vppImage = generate_vpp_image_dataset(data, dataMin, dataMax, imageHeight, imageWidth, true, eightBlockZigZagFlag);
	int result = 0;
	uint32_t jpegSize = (uint32_t) ((double)imageWidth * (double)imageHeight * JPEG_MULTIPLIER);
	jpegSize = jpegSize - (jpegSize % 64); //Multiple of 8x8
	uint8_t * JPEGImage = malloc(jpegSize);
	memset(JPEGImage, 0, jpegSize);
	uint32_t size = 0;

	result = encode_JPEG_accelerator_MDMA(vppImage, sizeof(uint8_t) * imageHeight * imageWidth * 4 , JPEGImage, jpegSize, jpegConf);
	if(result != 0){
		printf("Error occurred while encoding JPEG image\n");
		return -1;
	}

	size = getJPEGSize(JPEGImage, jpegSize);

	free(JPEGImage);
	free(vppImage);
	free(jpegConf);
	return size;
}
#endif


double test_mse_error_vpp_dataset(double * data, double dataMin, double dataMax, uint32_t imageHeight, uint32_t imageWidth, uint8_t quality,  uint8_t eightBlockZigZagFlag){
	double mse = 0;
	JPEG_ConfTypeDef * jpegConf = new_jpeg_conf(imageHeight, imageWidth, quality);
	//uint8_t * image ; Uses  extern

	//print_image_pixels_alpha(vppImage, imageWidth, imageHeight);
	uint32_t jpegSize = (uint32_t)((double)imageWidth * (double)imageHeight * JPEG_MULTIPLIER);
	jpegSize = jpegSize - (jpegSize % 64); //Multiple of 8x8
	uint32_t returnImageSize = sizeof(uint8_t) * imageWidth * imageHeight * 4;

	int result = 0;
	uint8_t * JPEGImage = malloc(jpegSize);

	uint8_t * returnImage = malloc(returnImageSize + 4); //Hand aligned memory to 32 bit
	while((uint32_t)returnImage % 4 != 0){
		printf("ReturnImage was not aligned\n");
		returnImage += 1;
	}
	uint8_t* vppImage = generate_vpp_image_data(data, dataMin, dataMax, imageHeight * imageWidth, imageHeight, imageWidth, true, eightBlockZigZagFlag);



	result = encode_JPEG_accelerator_MDMA(vppImage, returnImageSize , JPEGImage, jpegSize, jpegConf);
	if(result != 0){
		printf("Error occurred while encoding JPEG image\n");
		return -1;
	}
	free(vppImage);

	memset(returnImage, 0, returnImageSize);
	result = decode_JPEG_accelerator_MDMA(JPEGImage, jpegSize, returnImage, returnImageSize, jpegConf);
	if(result != 0){
		printf("Error occurred while decoding JPEG image\n");
		return -1;
	}


#if VPP_ENCODE == RGB_VECTOR
	for(uint32_t i = 0; i < imageHeight * imageWidth; i++){
		set_to_nearest_valid_rgb_pixel(&returnImage[i*4]);
	}
#elif VPP_ENCODE == N_VECTOR
	for(uint32_t i = 0; i < imageHeight * imageWidth; i++){
		set_to_nearest_valid_pixel(&returnImage[i*4]);
	}
#endif
    mse = calculate_mean_scaled_absolute_error_vpp_image(returnImage, data, imageHeight * imageWidth, dataMin, dataMax);

	free(returnImage);
	free(JPEGImage);
	free(jpegConf);
	return mse;
}


#if JPEG_ACCELERATOR == ON
double test_error_vpp_outliers(uint32_t imageHeight, uint32_t imageWidth, uint8_t quality, double outlierPercentage,  uint8_t eightBlockZigZagFlag){
	JPEG_ConfTypeDef * jpegConf = new_jpeg_conf(imageHeight, imageWidth, quality);
	//print_image_pixels_alpha(image, imageWidth, imageHeight);
	int result = 0;
	double * data = NULL;
	uint32_t jpegSize = (uint32_t)((double)imageWidth * (double)imageHeight * JPEG_MULTIPLIER);
	jpegSize = jpegSize - (jpegSize % 64); //Multiple of 8x8
	uint8_t * JPEGImage = malloc(jpegSize);
	uint8_t * image = generate_vpp_image_outliers_data(&data, imageHeight, imageWidth, true, outlierPercentage,  eightBlockZigZagFlag);
	uint8_t * returnImage = NULL;
	double averageAbsoluteError = 0;

	//print_image_pixels_alpha(image, imageWidth, imageHeight);
	memset(JPEGImage, 0, jpegSize);

	result = encode_JPEG_accelerator_MDMA(image, sizeof(uint8_t) * imageHeight * imageWidth * 4 , JPEGImage, jpegSize, jpegConf);
	if(result != 0){
		printf("Error occurred while encoding JPEG image\n");
		return -1;
	}

	//free(image);
	returnImage = malloc(sizeof(uint8_t) * imageWidth * imageHeight * 4);
	memset(returnImage, 0, sizeof(uint8_t) * imageWidth * imageHeight * 4);
	//printf("JPEG size: %ld\n", getJPEGSize(JPEGImage, jpegSize));

	result = decode_JPEG_accelerator_MDMA(JPEGImage, jpegSize, returnImage, sizeof(uint8_t) * imageHeight * imageWidth * 4, jpegConf);
	if(result != 0){
		printf("Error ocurred while enconding JPEG image\n");
		return -1;
	}
	//print_image_pixels_alpha(returnImage, imageWidth, imageHeight);

//	//reset alpha value
//	for(uint32_t i = 0; i < imageHeight * imageWidth; i++){
//		returnImage[i*4 + 3] = 255;
//	}
	//print_image_pixels_alpha(returnImage, imageWidth, imageHeight);
#if VPP_QUANTIZED_ERROR
	#if VPP_ENCODE == RED_VECTOR
		averageAbsoluteError = absolute_error_red((uint32_t *) image, (uint32_t *) returnImage, imageHeight * imageWidth);
	#elif VPP_ENCODE == N_VECTOR
		//Adjust pixels to valid vectors
		//print_image_pixels_alpha(returnImage, imageWidth, imageHeight);
		for(uint32_t i = 0; i < imageHeight * imageWidth; i++){
			set_to_nearest_valid_pixel(&returnImage[i*4]);
		}
		//print_image_pixels_alpha(returnImage, imageWidth, imageHeight);
		averageAbsoluteError = mean_absolute_quantized_error((uint32_t *) image, (uint32_t *) returnImage, imageHeight * imageWidth);

	#elif VPP_ENCODE == RGB_VECTOR
		//print_image_pixels_alpha(returnImage, imageWidth, imageHeight);
		for(uint32_t i = 0; i < imageHeight * imageWidth; i++){
			set_to_nearest_valid_rgb_pixel(&returnImage[i*4]);
		}
		//print_image_pixels_alpha(returnImage, imageWidth, imageHeight);
		averageAbsoluteError = mean_rgb_value_error((uint32_t *)image, (uint32_t *) returnImage, imageHeight * imageWidth);
	#elif VPP_ENCODE == DIRECT_VALUE
		//print_image_pixels_alpha(returnImage, imageWidth, imageHeight);
		averageAbsoluteError = mean_direct_value_error((uint32_t *)image, (uint32_t *) returnImage, imageHeight * imageWidth);
	#else
		printf("VPP ENCODE MODE NOT SUPPORTED!\n");
#endif

#else
#if VPP_ENCODE == RGB_VECTOR
	for(uint32_t i = 0; i < imageHeight * imageWidth; i++){
		set_to_nearest_valid_rgb_pixel(&returnImage[i*4]);
	}
#elif VPP_ENCODE == N_VECTOR
	for(uint32_t i = 0; i < imageHeight * imageWidth; i++){
		set_to_nearest_valid_pixel(&returnImage[i*4]);
	}
#endif
	averageAbsoluteError = calculate_mean_scaled_absolute_error_vpp_image(returnImage, data, imageHeight * imageWidth, -1, 1);
#endif
	free(returnImage);
	free(data);
	free(image);
	free(JPEGImage);
	free(jpegConf);
	return averageAbsoluteError;
}
#endif

double test_error_vpp(uint32_t imageHeight, uint32_t imageWidth, uint8_t quality, uint8_t eightBlockZigZagFlag){
	return test_error_vpp_outliers(imageHeight, imageWidth, quality, 0, eightBlockZigZagFlag);
}


double test_vpp_performance_JPEG_encode_accelerator_DMA_w_quantization(uint32_t imageHeight, uint32_t imageWidth, uint32_t numImageRepeats, uint8_t imageQuality,double outlierPercentage, uint8_t eightBlockZigZagFlag, double *stddev){
	double performance = 0;
	uint32_t timerCounts[numImageRepeats];
	memset(timerCounts, 0, numImageRepeats * sizeof(uint32_t));
	uint8_t *image = NULL;
	uint8_t *jpegOutput = (uint8_t *) malloc(JPEG_BUFFER_SIZE);
	uint32_t jpegOutputSize = JPEG_BUFFER_SIZE; //200KB
	JPEG_ConfTypeDef * conf = new_jpeg_conf(imageHeight, imageWidth, imageQuality);
	memset(jpegOutput, 0, imageHeight * imageWidth * 4); //ARGB
	double * data = generate_vpp_sine_data(imageHeight, imageWidth, outlierPercentage);


	//start = HAL_GetTick();
	for(uint32_t imageRepetition = 0; imageRepetition < numImageRepeats; imageRepetition++){
		__HAL_TIM_SetCounter(&htim2,0);
		image = generate_vpp_image_from_sine_data(data, imageHeight, imageWidth,  true,  eightBlockZigZagFlag);
		encode_JPEG_accelerator_MDMA(image, imageHeight * imageWidth * 4, jpegOutput, jpegOutputSize, conf);
		timerCounts[imageRepetition] =__HAL_TIM_GetCounter(&htim2); //Get current clock cycle count
		free(image);
		image = NULL;
	}

	double meanDuration = timer_mean_time (timerCounts, numImageRepeats, TIM2_COUNT_PER_SEC);

	performance =  ((double) imageWidth * imageHeight) / meanDuration;


	*stddev = timer_standard_deviation(timerCounts, numImageRepeats, TIM2_COUNT_PER_SEC, meanDuration);
	*stddev = ((*stddev) / meanDuration) * performance;

	free(data);
	free(conf);
	free(jpegOutput);

	//printf("Performance: %lf pixels/sec with stddev: %lf \n", *performance, *stddev);
	return performance;
}


double test_vpp_performance_JPEG_encode_accelerator_DMA_w_quantization_dataset(double * data, double dataMin, double dataMax, uint32_t numValues, uint32_t imageHeight, uint32_t imageWidth, uint32_t numImageRepeats, uint8_t imageQuality, uint8_t eightBlockZigZagFlag, double *stddev){
	double performance = 0;
	uint32_t timerCounts[numImageRepeats];
	uint32_t timerSum = 0;

	uint32_t elapsedCycles = 0;
	memset(timerCounts, 0, numImageRepeats * sizeof(uint32_t));
	uint8_t *image = NULL;
	uint8_t *jpegOutput = (uint8_t *) malloc(JPEG_BUFFER_SIZE);
	uint32_t jpegOutputSize = JPEG_BUFFER_SIZE; //200KB
	JPEG_ConfTypeDef * conf = new_jpeg_conf(imageHeight, imageWidth, imageQuality);
	memset(jpegOutput, 0, imageHeight * imageWidth * 4); //ARGB


	//start = HAL_GetTick();
	for(uint32_t imageRepetition = 0; imageRepetition < numImageRepeats; imageRepetition++){
		timerSum = 0;
		__HAL_TIM_SetCounter(&htim2,0);
		encode_JPEG_accelerator_MDMA(image, imageHeight * imageWidth * 4, jpegOutput, jpegOutputSize, conf);
		timerSum += elapsedCycles;



		timerCounts[imageRepetition] =timerSum; //Get current clock cycle count
		free(image);
		image = NULL;

	}

	double meanDuration = timer_mean_time (timerCounts, numImageRepeats, TIM2_COUNT_PER_SEC);

	performance =  ((double) imageWidth * imageHeight) / meanDuration;


	*stddev = timer_standard_deviation(timerCounts, numImageRepeats, TIM2_COUNT_PER_SEC, meanDuration);
	*stddev = ((*stddev) / meanDuration) * performance;

	free(conf);
	free(jpegOutput);

	//uint32_t totalTime = quantizationSum + YCbCrConversionSum + compressionSum;
	//printf("Quantization time perc: %lf; Conversion to YCbCr time perc: %lf; Compression time perc: %lf\n", ((double) quantizationSum) / totalTime, ((double) YCbCrConversionSum) / totalTime,((double) compressionSum) / totalTime);
	//printf("Performance: %lf pixels/sec with stddev: %lf \n", *performance, *stddev);
	return performance;
}





