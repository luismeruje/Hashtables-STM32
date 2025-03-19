/*
 * test_accelerators.c
 *
 *  Created on: Sep 2, 2024
 *      Author: luisferreira
 */
#include <compression/jpeg-manipulator.h>
#include "settings.h"

#if JPEG_ACCELERATOR == ON
#include <jpeg_utils.h>
#endif


#include <stdio.h>
#include "stm32u5xx_hal.h"
#include <test_performance_JPEG.h>
#include "stdbool.h"
#include "vpp_image_generator.h"
#include <stdlib.h>
#include "stm32u5xx_hal.h"
#include <string.h>
#include <image_output.h>
#include <utils.h>
#include "settings.h"
#include "benchmark_from_uart.h"

extern DMA2D_HandleTypeDef hdma2d;
extern TIM_HandleTypeDef htim2;

#if JPEG_ACCELERATOR == ON
/*
 * Make sure ACCL_BENCHMARK_YCbCr_BURST_SIZE is a multiple of 4 and corresponds to N MCU blocks. I.e., should be a multiple of 8x8x4
 */

//TODO: Make sure image matches MCU size, otherwise would have to add padding
//TODO: Use RGB in original image, we do not need alpha since we are not blending images.
double test_performance_YCbCr_encode_CPU(uint32_t imageHeight, uint32_t imageWidth, uint32_t numImageRepeats, uint32_t batchSize, uint8_t imageType){
	uint32_t duration, start, MCU_TotalNb;
	JPEG_RGBToYCbCr_Convert_Function pRGBToYCbCr_Convert_Function = NULL;
	JPEG_ConfTypeDef * conf = new_jpeg_conf(imageHeight, imageWidth, JPEG_ACCELERATOR_TEST_QUALITY);
	JPEG_GetEncodeColorConvertFunc(conf, &pRGBToYCbCr_Convert_Function, &MCU_TotalNb);
	double performance = 0;
	uint32_t convertedBytes;

	volatile uint8_t *outBuffer = malloc((batchSize / 4) * 3); //From ARGB to YCbCr goes from 4 bytes to 3.
	if((uint32_t) outBuffer < 0x24000000 || (uint32_t) outBuffer > 0x2407ffff){
		printf("Was not able to allocate space for YCbCr output buffer \n");
		free(conf);
		return 0.0;
	}

	volatile uint8_t *image = NULL, *imageOgPointer = NULL;

	if(imageType == ACCL_VPP){
		image = (volatile uint8_t *) generate_vpp_image( imageHeight, imageWidth, true,  VPP_IMAGE_FILL_STRATEGY);
		if(image == NULL){
			free( (uint8_t *) outBuffer);
			free(conf);
			return 0.0;
		}
		imageOgPointer = image;
	} else {
		printf("Image type for accelerator test unknown: %d\n", imageType);
		return 0;
	}

	start = HAL_GetTick();

	uint32_t numBatches = (imageHeight * imageWidth * 4) / batchSize;
	//When image is smaller than input buffer
	if(numBatches == 0 && imageHeight > 0 && imageWidth > 0){
		numBatches = 1;
	}


	for(uint32_t imageRepetition = 0; imageRepetition < numImageRepeats; imageRepetition++){
		image = imageOgPointer;
		for(uint32_t batch = 0; batch < numBatches; batch++){
			pRGBToYCbCr_Convert_Function((uint8_t *) image, (uint8_t *) outBuffer, 0, batchSize,(uint32_t*)(&convertedBytes));
			image += batchSize;
		}
	}
	duration = HAL_GetTick() - start;
	performance =  ((double) numImageRepeats * imageHeight * imageWidth) / ((double)duration/1000);


	free( (uint8_t *)imageOgPointer);
	free( (uint8_t *) outBuffer);
	free(conf);

	return performance;

}

/*
 * Encodes ARGB image into YCbCr and then decodes it numImageRepeats amount of times using library provided by stm
 * Calculates decoding performance in pixels/second.
 */
double test_performance_YCbCr_decode_CPU(uint32_t imageHeight, uint32_t imageWidth, uint32_t numImageRepeats, uint32_t batchSize, uint8_t imageType){
	uint32_t duration, start,  MCU_TotalNb, MCU_TotalNb_Decode;
	JPEG_RGBToYCbCr_Convert_Function pRGBToYCbCr_Convert_Function = NULL;
	JPEG_YCbCrToRGB_Convert_Function pYCbCrToRGB_Convert_Function = NULL;
	JPEG_ConfTypeDef * conf = new_jpeg_conf(imageHeight, imageWidth, JPEG_ACCELERATOR_TEST_QUALITY);
	JPEG_GetEncodeColorConvertFunc(conf, &pRGBToYCbCr_Convert_Function, &MCU_TotalNb);
	JPEG_GetDecodeColorConvertFunc(conf, &pYCbCrToRGB_Convert_Function, &MCU_TotalNb_Decode);
	double performance = 0;
	uint32_t convertedBytes = 0;
	//HAL_StatusTypeDef hal_status = HAL_OK;

	volatile uint8_t *outBuffer = malloc(imageHeight * imageWidth * 4); //From ARGB to YCbCr goes from 4 bytes to 3, but giving 3 just in case
	volatile uint8_t *outBufferOgPointer = outBuffer;
	if((uint32_t) outBuffer < 0x24000000 || (uint32_t) outBuffer > 0x2407ffff){
		printf("Was not able to allocate space for YCbCr output buffer \n");
		return 0.0;
	}
	memset((uint8_t *) outBuffer, 0 , imageHeight * imageWidth * 4);

	volatile uint8_t *image = NULL, *imageOgPointer = 0;

	if(imageType == ACCL_VPP){
		image = (volatile uint8_t *) generate_vpp_image( imageHeight, imageWidth, true,  VPP_IMAGE_FILL_STRATEGY);
		if(image == NULL){
			free( (uint8_t *) outBuffer);
			free(conf);
			return 0.0;
		}
		imageOgPointer = (uint8_t *) image;
	} else {
		printf("Image type for accelerator test unknown: %d\n", imageType);
		return 0;
	}
	uint32_t numBatches = (imageHeight * imageWidth * 4) / batchSize;


	if(numBatches == 0 && imageHeight > 0 && imageWidth > 0){
		numBatches = 1;
	}

	//Create our base YCbCr array
	for(uint32_t batch = 0; batch < numBatches; batch++){
		pRGBToYCbCr_Convert_Function((uint8_t *) image, (uint8_t *) outBuffer, 0, batchSize,(uint32_t*)(&convertedBytes));
		//pRGBToYCbCr_Convert_Function(image, Jpeg_IN_BufferTab.DataBuffer, 0, JPEG_IN_BUFFER_SIZE,(uint32_t*)(&Jpeg_IN_BufferTab.DataBufferSize));
		image += batchSize;
		outBuffer += (batchSize / 4) * 3;
	}

	outBuffer = outBufferOgPointer;
	image = (volatile uint8_t *)imageOgPointer;
	print_image_pixels_alpha((uint8_t *) image, imageWidth, imageHeight);
	memset((uint8_t *) image, 0, imageWidth * imageHeight * 4);

	//Calculate performance
	batchSize = (batchSize / 4) * 3;
	start = HAL_GetTick();
	for(uint32_t imageRepetition = 0; imageRepetition < numImageRepeats; imageRepetition++){
		for(uint32_t batch = 0; batch < numBatches; batch++){
			pYCbCrToRGB_Convert_Function((uint8_t *) outBuffer, (uint8_t *) image, 0, batchSize,(uint32_t*)(&convertedBytes));
			outBuffer += batchSize;
			image += (batchSize / 3) * 4;
		}
		image = imageOgPointer;
		outBuffer = outBufferOgPointer;
		print_image_pixels_alpha((uint8_t *) image, imageWidth, imageHeight);
//		HAL_DMA2D_Start_IT(&hdma2d, (uint32_t) outBuffer, (uint32_t) image, imageWidth, imageHeight);
//		if(hal_status != HAL_OK){
//			printf("Hal error, status: %d\n", hal_status);
//			printf("Request to blend images failed.\n");
//			return -1;
//		}
//
//		hal_status = HAL_DMA2D_PollForTransfer(&hdma2d, 10);
//		if(hal_status != HAL_OK){
//			printf("Hal error, status: %d\n", hal_status);
//			printf("Timed out while waiting for DMA2D blend operation.\n");
//			return -2;
//		}
	}



	duration = HAL_GetTick() - start;
	performance =  ((double) numImageRepeats * imageHeight * imageWidth) / ((double)duration/1000);



	free( (uint8_t *)image);
	free( (uint8_t *) outBufferOgPointer);
	free(conf);
	return performance;

}

uint8_t * _generate_image(uint32_t imageHeight, uint32_t imageWidth, uint8_t imageType){
	uint8_t * image = NULL;
//TODO: Check if we want to keep this macro
	if(imageType == ACCL_VPP){
		image = generate_vpp_image( imageHeight, imageWidth, true,  VPP_IMAGE_FILL_STRATEGY);
		if(image == NULL){
			printf("Was not able to allocate space for output image while testing performance of CPU only JPEG encoding\n");
			return NULL;
		}
	} else {
		printf("ERROR: Image type for accelerator test unknown: %d\n", imageType);
	}
	return image;
}


void test_performance_JPEG_encode_accelerator_DMA(uint32_t imageHeight, uint32_t imageWidth, uint32_t numImageRepeats, uint8_t imageQuality, uint8_t imageType, double *performance, double *stddev, uint8_t timeQuantization){
	uint32_t timerCounts[numImageRepeats];
	memset(timerCounts, 0, numImageRepeats * sizeof(uint32_t));
	uint8_t *image = NULL;
	uint8_t *jpegOutput = (uint8_t *) 0x30000000; //RAM_D2
	uint32_t jpegOutputSize = 1024 * 200; //200KB
	JPEG_ConfTypeDef * conf = new_jpeg_conf(imageHeight, imageWidth, imageQuality);
	memset(jpegOutput, 0, imageHeight * imageWidth * 4); //ARGB
	if(!timeQuantization){
		image = _generate_image(imageHeight, imageWidth, imageType);
	}
	//start = HAL_GetTick();
	for(uint32_t imageRepetition = 0; imageRepetition < numImageRepeats; imageRepetition++){
		__HAL_TIM_SetCounter(&htim2,0);
		if(timeQuantization){
			image = _generate_image(imageHeight, imageWidth, imageType);
		}
		encode_JPEG_accelerator_MDMA(image, imageHeight * imageWidth * 4, jpegOutput, jpegOutputSize, conf);
		timerCounts[imageRepetition] =__HAL_TIM_GetCounter(&htim2); //Get current clock cycle count
		if(timeQuantization){
			free(image);
			image = NULL;
		}
	}

	double meanDuration = timer_mean_time (timerCounts, numImageRepeats, TIM2_COUNT_PER_SEC);

	*performance =  ((double) imageWidth * imageHeight) / meanDuration;


	*stddev = timer_standard_deviation(timerCounts, numImageRepeats, TIM2_COUNT_PER_SEC, meanDuration);
	*stddev = ((*stddev) / meanDuration) * (*performance);


	//printf("Performance: %lf pixels/sec with stddev: %lf \n", *performance, *stddev);

	//HAL_Delay(5000);
	if(!timeQuantization){
		free(image);
	}

}

//Tests only the time it takes for the accelerator to compress image, without quantization or conversion to YCbCr
double test_performance_JPEG_encode_accelerator_DMA_raw_dataset(double * data, double dataMin, double dataMax, uint32_t numValues, uint32_t imageHeight, uint32_t imageWidth, uint32_t numImageRepeats, uint8_t imageQuality, uint8_t eightBlockZigZagFlag, double *stddev){
	uint32_t timerCounts[numImageRepeats];
	uint32_t imageSize = imageHeight * imageWidth * 4; //32-bit per pixel
	double performance = 0;
	memset(timerCounts, 0, numImageRepeats * sizeof(uint32_t));
	uint8_t *image = NULL;
	uint8_t *jpegOutput = (uint8_t *) malloc(JPEG_BUFFER_SIZE);
	uint32_t jpegOutputSize = JPEG_BUFFER_SIZE; //200KB
	JPEG_ConfTypeDef * conf = new_jpeg_conf(imageHeight, imageWidth, imageQuality);
	memset(jpegOutput, 0, imageSize); //ARGB
	uint8_t * YCbCrImage = NULL;
	uint32_t YCbCrImageSize;

	image = generate_vpp_image_data(data, dataMin, dataMax, numValues, imageHeight, imageWidth, true, eightBlockZigZagFlag);

	printf("RGB: ");
	//Debug. To figure out how data is organized after being converted to YCbCr
	for(uint32_t i = 0; i < 16; i++){
		printf("%u;", image[i]);
	}
	printf("\n");


	prepare_RGB_to_YCbCr_image(image, imageSize, &YCbCrImage, &YCbCrImageSize, conf);

	printf("YCbCr: ");
	for(uint32_t i = 0; i < 3000; i++){
		printf("%u;", YCbCrImage[i]);
	}
	printf("\n");

	//start = HAL_GetTick();
	for(uint32_t imageRepetition = 0; imageRepetition < numImageRepeats; imageRepetition++){
		__HAL_TIM_SetCounter(&htim2,0);
		encode_JPEG_accelerator_MDMA(YCbCrImage, YCbCrImageSize, jpegOutput, jpegOutputSize, conf);
		//encode_JPEG_accelerator_MDMA_(image, imageSize, jpegOutput, jpegOutputSize, conf);
		timerCounts[imageRepetition] =__HAL_TIM_GetCounter(&htim2); //Get current clock cycle count
		memset(jpegOutput, 0, imageSize); //ARGB
	}

	double meanDuration = timer_mean_time (timerCounts, numImageRepeats, TIM2_COUNT_PER_SEC);

	performance =  ((double) imageWidth * imageHeight) / meanDuration;


	*stddev = timer_standard_deviation(timerCounts, numImageRepeats, TIM2_COUNT_PER_SEC, meanDuration);
	*stddev = ((*stddev) / meanDuration) * performance;


	//printf("Performance: %lf pixels/sec with stddev: %lf \n", *performance, *stddev);

	//HAL_Delay(5000);
	free(YCbCrImage);
	free(image);
	free(conf);
	free(jpegOutput);

	return performance;

}

void test_performance_JPEG_decode_accelerator_DMA(uint32_t imageHeight, uint32_t imageWidth, uint32_t numImageRepeats, uint8_t imageQuality, uint8_t imageType, double *performance, double *stddev){
	uint32_t timerCounts[numImageRepeats];
	memset(timerCounts, 0, numImageRepeats * sizeof(uint32_t));
	uint8_t *image = NULL;
	uint8_t *jpegOutput = (uint8_t *) 0x30000000; //RAM_D2
	uint32_t jpegOutputSize = 1024 * 200; //200KB
	JPEG_ConfTypeDef * conf = new_jpeg_conf(imageHeight, imageWidth, imageQuality);
	memset(jpegOutput, 0, imageHeight * imageWidth * 4);


    image = _generate_image(imageHeight, imageWidth, imageType);

	//Encode image to JPEG
	encode_JPEG_accelerator_MDMA(image, imageHeight * imageWidth * 4, jpegOutput, jpegOutputSize, conf);

	//Clear original image data to ensure no overlap of data
	memset(image, 0, imageHeight * imageWidth * 4);

	//start = HAL_GetTick();
	for(uint32_t imageRepetition = 0; imageRepetition < numImageRepeats; imageRepetition++){
		__HAL_TIM_SetCounter(&htim2,0);
		decode_JPEG_accelerator_MDMA(jpegOutput, imageHeight * imageWidth * 3, image, imageHeight * imageWidth * 4, conf);
		timerCounts[imageRepetition] =__HAL_TIM_GetCounter(&htim2); //Get current clock cycle count
		//print_image_pixels_alpha(image, imageWidth, imageHeight);
		//HAL_Delay(5000);

	}

	double meanDuration = timer_mean_time (timerCounts, numImageRepeats, TIM2_COUNT_PER_SEC);

	*performance =  ((double) imageWidth * imageHeight) / meanDuration;


	*stddev = timer_standard_deviation(timerCounts, numImageRepeats, TIM2_COUNT_PER_SEC, meanDuration);
	*stddev = ((*stddev) / meanDuration) * (*performance);

	//printf("Performance for clock speed %ld: %lf pixels/sec\n", clockFreq, performance);

	//HAL_Delay(5000);
	free(image);
}


void test_performance_JPEG_encode_accelerator_IT(uint32_t imageHeight, uint32_t imageWidth, uint32_t numImageRepeats, uint8_t imageQuality, uint8_t imageType, double *performance, double *stddev, uint8_t timeQuantization){
	uint32_t timerCounts[numImageRepeats];
	memset(timerCounts, 0, numImageRepeats * sizeof(uint32_t));
	uint8_t *image = NULL;
	uint8_t *jpegOutput = (uint8_t *) 0x30000000; //RAM_D2
	uint32_t jpegOutputSize = 1024 * 200; //200KB
	JPEG_ConfTypeDef * conf = new_jpeg_conf(imageHeight, imageWidth, imageQuality);
	memset(jpegOutput, 0, imageHeight * imageWidth * 4); //ARGB

	if(!timeQuantization){
		image = _generate_image(imageHeight, imageWidth, imageType);
	}

	//start = HAL_GetTick();
	for(uint32_t imageRepetition = 0; imageRepetition < numImageRepeats; imageRepetition++){
		memset(jpegOutput, 0, imageHeight * imageWidth * 4); //ARGB
		__HAL_TIM_SetCounter(&htim2,0);
		if(timeQuantization){
			image = _generate_image(imageHeight, imageWidth, imageType);
		}
		encode_JPEG_accelerator_IT(image, imageHeight * imageWidth * 4, jpegOutput, jpegOutputSize, conf);

		timerCounts[imageRepetition] =__HAL_TIM_GetCounter(&htim2); //Get current clock cycle count
		if(timeQuantization){
			free(image);
			image = NULL;
		}

	}
	//print_image_pixels_alpha(jpegOutput, imageWidth, imageHeight);
	double meanDuration = timer_mean_time (timerCounts, numImageRepeats, TIM2_COUNT_PER_SEC);

	*performance =  ((double) imageWidth * imageHeight) / meanDuration;


	*stddev = timer_standard_deviation(timerCounts, numImageRepeats, TIM2_COUNT_PER_SEC, meanDuration);
	*stddev = ((*stddev) / meanDuration) * (*performance);


	//printf("Performance: %lf pixels/sec with stddev: %lf \n", *performance, *stddev);

	//HAL_Delay(5000);
	if(!timeQuantization){
		free(image);
	}

}

void test_performance_JPEG_decode_accelerator_IT(uint32_t imageHeight, uint32_t imageWidth, uint32_t numImageRepeats, uint8_t imageQuality, uint8_t imageType, double *performance, double *stddev){
	uint32_t timerCounts[numImageRepeats];
	memset(timerCounts, 0, numImageRepeats * sizeof(uint32_t));
	uint8_t *image = NULL;
	uint8_t *jpegOutput = (uint8_t *) 0x30000000; //RAM_D2
	uint32_t jpegOutputSize = 1024 * 200; //200KB

	JPEG_ConfTypeDef * conf = new_jpeg_conf(imageHeight, imageWidth, imageQuality);
	memset(jpegOutput, 0, imageHeight * imageWidth * 4);

	if(imageType == ACCL_VPP){
		image = generate_vpp_image( imageHeight, imageWidth, true,  VPP_IMAGE_FILL_STRATEGY);
//		//DEBUG
//		print_image_pixels_alpha(image, imageWidth, imageHeight);
//		HAL_Delay(5000);
//		//
		if(image == NULL){
			free(conf);
		}
	} else {
		printf("Image type for accelerator test unknown: %d\n", imageType);
	}

	//Encode image to JPEG
	encode_JPEG_accelerator_IT(image, imageHeight * imageWidth * 4, jpegOutput, jpegOutputSize, conf);

	//Clear original image data to ensure no overlap of data
	memset(image, 0, imageHeight * imageWidth * 4);

	//start = HAL_GetTick();
	for(uint32_t imageRepetition = 0; imageRepetition < numImageRepeats; imageRepetition++){
		__HAL_TIM_SetCounter(&htim2,0);
		decode_JPEG_accelerator_IT(jpegOutput, imageHeight * imageWidth * 3, image, imageHeight * imageWidth * 4, conf);
		timerCounts[imageRepetition] =__HAL_TIM_GetCounter(&htim2); //Get current clock cycle count
//		print_image_pixels_alpha(image, imageWidth, imageHeight);
//		HAL_Delay(5000);

	}

	double meanDuration = timer_mean_time (timerCounts, numImageRepeats, TIM2_COUNT_PER_SEC);

	*performance =  ((double) imageWidth * imageHeight) / meanDuration;


	*stddev = timer_standard_deviation(timerCounts, numImageRepeats, TIM2_COUNT_PER_SEC, meanDuration); //In time
	*stddev = ((*stddev) / meanDuration) * (*performance); //In pixels/sec

	//printf("Performance for clock speed %ld: %lf pixels/sec\n", clockFreq, performance);

	//HAL_Delay(5000);
	free(image);
}
#endif

//Time quantization - if true, takes into account the time that the data to image generation process takes
//TODO: remove time quantization value

void test_performance_JPEG_encode_CPU(uint32_t imageHeight, uint32_t imageWidth, uint32_t numImageRepeats, uint8_t imageQuality, uint8_t imageType, double *performance, double *stddev, uint8_t timeQuantization){
	uint32_t timerCounts[numImageRepeats];
	memset(timerCounts, 0, numImageRepeats * sizeof(uint32_t));
	uint8_t *image = NULL;
	uint8_t *jpeg = NULL;

	if(!timeQuantization){
		image = _generate_image(imageHeight, imageWidth, imageType);
	}

	//start = HAL_GetTick();
	for(uint32_t imageRepetition = 0; imageRepetition < numImageRepeats; imageRepetition++){
		__HAL_TIM_SetCounter(&htim2,0);
		if(timeQuantization){
			image = _generate_image(imageHeight, imageWidth, imageType);
		}
		jpeg = encode_JPEG_from_memory_to_memory(image, imageWidth, imageHeight, imageQuality);
		timerCounts[imageRepetition] =__HAL_TIM_GetCounter(&htim2); //Get current clock cycle count
		free(jpeg);
		if(timeQuantization){
			free(image);
		}
	}

	double meanDuration = timer_mean_time (timerCounts, numImageRepeats, TIM2_COUNT_PER_SEC);

	*performance =  ((double) imageWidth * imageHeight) / meanDuration;


	*stddev = timer_standard_deviation(timerCounts, numImageRepeats, TIM2_COUNT_PER_SEC, meanDuration);
	*stddev = ((*stddev) / meanDuration) * (*performance);


	//printf("Performance: %lf pixels/sec with stddev: %lf \n", *performance, *stddev);

	//HAL_Delay(5000);
	if(!timeQuantization){
		free(image);
	}

}

void test_performance_JPEG_decode_CPU(uint32_t imageHeight, uint32_t imageWidth, uint32_t numImageRepeats, uint8_t imageQuality, uint8_t imageType, double *performance, double *stddev){
	uint32_t timerCounts[numImageRepeats];
	memset(timerCounts, 0, numImageRepeats * sizeof(uint32_t));
	uint8_t *image = NULL;
	uint8_t *jpeg = NULL;

	if(imageType == ACCL_VPP){
		image = generate_vpp_image( imageHeight, imageWidth, true,  VPP_IMAGE_FILL_STRATEGY);
		if(image == NULL){
			printf("Was not able to allocate space for output image while testing performance of CPU only JPEG encoding\n");
			return;
		}
	} else {
		printf("ERROR: Image type for accelerator test unknown: %d\n", imageType);
	}

	//Encode the image
	jpeg = encode_JPEG_from_memory_to_memory(image, imageWidth, imageHeight, imageQuality);
	//Free original image
	free(image);
	image = NULL;

	//start = HAL_GetTick();
	for(uint32_t imageRepetition = 0; imageRepetition < numImageRepeats; imageRepetition++){
		__HAL_TIM_SetCounter(&htim2,0);
		image = decode_JPEG_from_memory_to_memory(jpeg, imageWidth, imageHeight);
		timerCounts[imageRepetition] =__HAL_TIM_GetCounter(&htim2); //Get current clock cycle count

		free(image);
	}

	double meanDuration = timer_mean_time (timerCounts, numImageRepeats, TIM2_COUNT_PER_SEC);

	*performance =  ((double) imageWidth * imageHeight) / meanDuration;


	*stddev = timer_standard_deviation(timerCounts, numImageRepeats, TIM2_COUNT_PER_SEC, meanDuration);
	*stddev = ((*stddev) / meanDuration) * (*performance);


	//printf("Performance: %lf pixels/sec with stddev: %lf \n", *performance, *stddev);

	//HAL_Delay(5000);
	free(jpeg);

}

