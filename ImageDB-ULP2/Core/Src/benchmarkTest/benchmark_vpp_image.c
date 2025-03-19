/*
 * benchmark_image_performance.c
 *
 *  Created on: Apr 26, 2024
 *      Author: luisferreira
 */
#include "settings.h"
#include <benchmark_vpp_image.h>
#include <compression/jpeg-manipulator.h>
#include <stdlib.h>
#include <string.h>
#include "data_generator.h"
#include "stm32u5xx_hal.h"
#include <stdio.h>
#include <test_vpp_images.h>
#include <utils.h>
#include "malloc.h"
#include "stdbool.h"
#include "vpp_image.h"


extern TIM_HandleTypeDef htim3;

#if DATASET_DATA == 1
#include "edp_data.h"
#elif DATASET_DATA == 2
#include "car_data.h"
#elif DATASET_DATA == 22
#include "car_data_2.h"
#elif DATASET_DATA == 23
#include "car_data_3.h"
#elif DATASET_DATA == 24
#include "car_data_4.h"
#elif DATASET_DATA == 25
#include "car_data_5.h"
#endif

extern uint8_t jpegMDMADone; //TODO: avoid this variable in this module.

//Tests: raw accelerator power vs raw CPU power (without data generation overhead)
//		 entire image pipeline CPU alone vs CPU + DMA + accelerators


//QUESTION: how are results affected by different types of datasets? How do we show those results? A diagram showing the average from a large amount of datasets?



#if JPEG_ACCELERATOR == ON

/*
 * Benchmark the error of value per pixel (vpp) images that have been compressed using jpeg. Error given as expected pixel height vs decoded pixel height.
 * Note: Remember that values are quantized according to the number of available height pixels.
 * Image width and height values are tested as pairs.
 * (i.e., number of heights and widths should be the same). For example for widths [64, 64] and heights [64, 128] the tested images will be 64x64 and 64x128.
 * Prints results in csv format to stdout with format - header: ;;Height0, Height1, Height2, ... | rows: Quality; #Overlapped Lines; Error for WidthxHeight0; Error for WidthxHeight1; ...
 * One error measurement per quality, overlappedLines, Height x Width combination.
 * Notice that we are currently assuming imageWidth is fixed, hence why header only displays height.
 *
 */
int benchmark_error_vpp_sine(uint32_t *quality, uint32_t numQuality, uint32_t *imageHeight, uint32_t *imageWidth, uint32_t numImageSizes, uint32_t numIterationsAvg, uint32_t * amountVectors, uint32_t numAmountVectors, uint32_t * minRGBSize, uint32_t numMinRGBSizes){
//	 uint32_t quality[] = {5,10,25,50,75,80,90,95,100};
//	 uint32_t imageHeight[] = {16,32,64,128,256};
//	 uint32_t imageWidth[] = {64,64,64,64,64};
	 double mae;
	 printf(";");

#if VPP_ENCODE == N_VECTOR
	 printf(";"); //Extra column
#else
	 UNUSED(numAmountVectors);
	 UNUSED(amountVectors);
#endif
#if VPP_ENCODE == RGB_VECTOR
	 printf(";"); //Extra column
#else
	 UNUSED(minRGBSize);
	 UNUSED(numMinRGBSizes);

#endif
	 for(uint32_t size = 0; size < numImageSizes; size++){
		 printf("%ld;", imageHeight[size]);
	 }
	 printf("\n");
#if VPP_ENCODE == N_VECTOR
	 for(uint32_t v = 0; v < numAmountVectors; v++){
		 printf("%ld;", amountVectors[v]);
		 vpp_set_number_vectors(amountVectors[v]);
#endif
#if VPP_ENCODE == RGB_VECTOR
	 for(uint32_t m = 0; m < numMinRGBSizes; m++){
		 printf("%ld;", minRGBSize[m]);
		 set_min_value_RGB((uint8_t) minRGBSize[m]);
#endif
		 for(uint32_t q = 0; q < numQuality; q++){
			 printf("%lu;",quality[q]);
			 for(uint32_t size = 0; size < numImageSizes; size++){
				 mae = 0;
				 for(uint32_t iterations = 0; iterations < numIterationsAvg; iterations++){
					 mae +=  test_error_vpp(imageHeight[size], imageWidth[size], (uint8_t) quality[q], VPP_IMAGE_FILL_STRATEGY);
				 }
				 mae = mae / numIterationsAvg;
				 printf("%lf;", mae);
			 }

			 printf("\n");
		 }
#if VPP_ENCODE == N_VECTOR
		 //printf("\n");
	 }
#endif
#if VPP_ENCODE == RGB_VECTOR
	 }
#endif
	 return 0;
}

#if DATASET_DATA > 0
//TODO: receive pointer to data, max and min values.
int benchmark_error_vpp_dataset_image(uint32_t *quality, uint32_t numQuality, uint32_t *imageHeight, uint32_t *imageWidth, uint32_t numImageSizes, uint32_t numIterationsAvg, uint32_t * amountVectors, uint32_t numAmountVectors, uint32_t * minRGBSize, uint32_t numMinRGBSizes){
	double mae;
	uint32_t numSubImages = 0;
	printf(";");
#if VPP_ENCODE == N_VECTOR
	 printf(";"); //Extra column
#else
	 UNUSED(numAmountVectors);
	 UNUSED(amountVectors);
#endif
#if VPP_ENCODE == RGB_VECTOR
	 printf(";"); //Extra column
#else
	 UNUSED(minRGBSize);
	 UNUSED(numMinRGBSizes);

#endif
	 for(uint32_t size = 0; size < numImageSizes; size++){
		 printf("%ld;", imageHeight[size]);
	 }
	 printf("\n");
#if VPP_ENCODE == N_VECTOR
	 for(uint32_t v = 0; v < numAmountVectors; v++){
		 printf("%ld;", amountVectors[v]);
		 vpp_set_number_vectors(amountVectors[v]);
#endif
#if VPP_ENCODE == RGB_VECTOR
	 for(uint32_t m = 0; m < numMinRGBSizes; m++){
		 printf("%ld;", minRGBSize[m]);
		 set_min_value_RGB((uint8_t) minRGBSize[m]);
#endif
		 for(uint32_t q = 0; q < numQuality; q++){
			 printf("%lu;",quality[q]);
			 for(uint32_t size = 0; size < numImageSizes; size++){
				 numSubImages = DATASET_DATA_SIZE / (imageHeight[size] * imageWidth[size]);

				 //printf("\n Num sub-segments: %ld\n", numSubImages);

				 mae = 0;
				 for(uint32_t iterations = 0; iterations < numIterationsAvg; iterations++){
					 for(uint32_t imageBufferIndex = 0; imageBufferIndex < numSubImages; imageBufferIndex++){
						 mae +=  test_mse_error_vpp_dataset(data + (imageBufferIndex * imageHeight[size] * imageWidth[size]), DATASET_DATA_MIN, DATASET_DATA_MAX, imageHeight[size], imageWidth[size], (uint8_t) quality[q], VPP_IMAGE_FILL_STRATEGY);
					 }
				 }
				 mae = mae / (numIterationsAvg * numSubImages) ;
				 printf("%lf;", mae);
			 }
			 printf("\n");
		 }

#if VPP_ENCODE == N_VECTOR
		 //printf("\n");
	 }
#endif
#if VPP_ENCODE == RGB_VECTOR
	 }
#endif
	 return 0;
}

int benchmark_size_vpp_dataset(uint32_t *quality, uint32_t numQuality, uint32_t *imageHeight, uint32_t *imageWidth, uint32_t numImageSizes, uint32_t numIterationsAvg, uint32_t * amountVectors, uint32_t numAmountVectors, uint32_t * minRGBSize, uint32_t numMinRGBSizes){
	double jpegSize;
	uint32_t numSubImages = 0;
	 printf(";");
#if VPP_ENCODE == N_VECTOR
	 printf(";"); //Extra column
#else
	 UNUSED(numAmountVectors);
	 UNUSED(amountVectors);
#endif
#if VPP_ENCODE == RGB_VECTOR
	 printf(";"); //Extra column
#else
	 UNUSED(minRGBSize);
	 UNUSED(numMinRGBSizes);

#endif
	 for(uint32_t size = 0; size < numImageSizes; size++){
		 printf("%ld;", imageHeight[size]);
	 }
	 printf("\n");
#if VPP_ENCODE == N_VECTOR
	 for(uint32_t v = 0; v < numAmountVectors; v++){
		 printf("%ld;", amountVectors[v]);
		 vpp_set_number_vectors(amountVectors[v]);
#endif
#if VPP_ENCODE == RGB_VECTOR
	 for(uint32_t m = 0; m < numMinRGBSizes; m++){
		 printf("%ld;", minRGBSize[m]);
		 set_min_value_RGB((uint8_t) minRGBSize[m]);
#endif
		 for(uint32_t q = 0; q < numQuality; q++){
			 printf("%lu;",quality[q]);
			 for(uint32_t size = 0; size < numImageSizes; size++){
				 numSubImages = DATASET_DATA_SIZE / (imageHeight[size] * imageWidth[size]);

				 //printf("\n Num sub-segments: %ld\n", numSubImages);

				 jpegSize = 0;
				 for(uint32_t iterations = 0; iterations < numIterationsAvg; iterations++){
					 for(uint32_t imageBufferIndex = 0; imageBufferIndex < numSubImages; imageBufferIndex++){
						 jpegSize +=  test_size_vpp_dataset(data + (imageBufferIndex * imageHeight[size] * imageWidth[size]), DATASET_DATA_MIN, DATASET_DATA_MAX, imageHeight[size], imageWidth[size], (uint8_t) quality[q], VPP_IMAGE_FILL_STRATEGY);
					 }
				 }
				 jpegSize = jpegSize / (numIterationsAvg * numSubImages) ;
				 printf("%lf;", jpegSize);
			 }
			 printf("\n");
			 //printf("\n");
		 }
#if VPP_ENCODE == N_VECTOR
		 //printf("\n");
	 }
#endif
#if VPP_ENCODE == RGB_VECTOR
	 }
#endif
	 return 0;
}

#endif

int benchmark_error_vpp_sine_outliers(uint32_t *quality, uint32_t numQuality, uint32_t *imageHeight, uint32_t *imageWidth, uint32_t numImageSizes, double * outlierPercentage, uint32_t numOutlierPercentages, uint32_t numIterationsAvg, uint32_t * amountVectors, uint32_t numAmountVectors, uint32_t * minRGBSize, uint32_t numMinRGBSizes){
//	 uint32_t quality[] = {5,10,25,50,75,80,90,95,100};
//	 uint32_t imageHeight[] = {16,32,64,128,256};
//	 uint32_t imageWidth[] = {64,64,64,64,64};
	 double mae;

	 printf(";");
#if VPP_ENCODE == N_VECTOR
	 printf(";"); //Extra column
#else
	 UNUSED(numAmountVectors);
	 UNUSED(amountVectors);
#endif
#if VPP_ENCODE == RGB_VECTOR
	 printf(";"); //Extra column
#else
	 UNUSED(minRGBSize);
	 UNUSED(numMinRGBSizes);

#endif
	 for(uint32_t size = 0; size < numImageSizes; size++){
		 printf("%ld;", imageHeight[size]);
	 }
	 printf("\n");
#if VPP_ENCODE == N_VECTOR
	 for(uint32_t v = 0; v < numAmountVectors; v++){
		 printf("%ld;", amountVectors[v]);
		 vpp_set_number_vectors(amountVectors[v]);
#endif
#if VPP_ENCODE == RGB_VECTOR
	 for(uint32_t m = 0; m < numMinRGBSizes; m++){
		 printf("%ld;", minRGBSize[m]);
		 set_min_value_RGB((uint8_t) minRGBSize[m]);
#endif
	 for(uint32_t q = 0; q < numQuality; q++){

		 for(uint32_t out = 0; out < numOutlierPercentages; out++){
			 printf("%lu;%.3lf;", quality[q], outlierPercentage[out]);
			 for(uint32_t size = 0; size < numImageSizes; size++){
				 mae = 0;
				 for(uint32_t iterations = 0; iterations < numIterationsAvg; iterations++){
					 mae +=  test_error_vpp_outliers(imageHeight[size], imageWidth[size], (uint8_t) quality[q], outlierPercentage[out], VPP_IMAGE_FILL_STRATEGY);
				 }
				 mae = mae / numIterationsAvg;
				 printf("%.3lf;", mae);
			 }
			 printf("\n");
		 }
	 }

#if VPP_ENCODE == N_VECTOR
		 //printf("\n");
	 }
#endif
#if VPP_ENCODE == RGB_VECTOR
	 }
#endif

	 return 0;
}


/*
 * Benchmark the size of value per pixel (vpp) images, storing sine wave data, that have been compressed using jpeg.
 * Note: Remember that values are quantized according to the chosen quantization algorithm.
 * Image width and height values are tested as pairs.
 * (i.e., number of heights and widths should be the same). For example for widths [64, 64] and heights [64, 128] the tested images will be 64x64 and 64x128.
 * Prints results in csv format to stdout with format - header: ;;Height0, Height1, Height2, ... | rows: Quality; #Overlapped Lines; Size in bytes for WidthxHeight0; Size in bytes for WidthxHeight1; ...
 * One size measurement per quality, overlappedLines, Height x Width combination.
 * Notice that we are currently assuming imageWidth is fixed, hence why header only displays height.
 *
 */
int benchmark_size_sine_vpp(uint32_t *quality, uint32_t numQuality, uint32_t *imageHeight, uint32_t *imageWidth, uint32_t numImageSizes, uint32_t numIterationsAvg, uint32_t * amountVectors, uint32_t numAmountVectors, uint32_t * minRGBSize, uint32_t numMinRGBSizes){
//	 uint32_t quality[] = {5,10,25,50,75,80,90,95,100};
//	 uint32_t imageHeight[] = {16,32,64,128,256};
//	 uint32_t imageWidth[] = {64,64,64,64,64};
	 double jpegSize;

	 printf(";");
#if VPP_ENCODE == N_VECTOR
	 printf(";"); //Extra column
#else
	 UNUSED(numAmountVectors);
	 UNUSED(amountVectors);
#endif
#if VPP_ENCODE == RGB_VECTOR
	 printf(";"); //Extra column
#else
	 UNUSED(minRGBSize);
	 UNUSED(numMinRGBSizes);

#endif
	 for(uint32_t size = 0; size < numImageSizes; size++){
		 printf("%ld;", imageHeight[size]);
	 }
	 printf("\n");
#if VPP_ENCODE == N_VECTOR
	 for(uint32_t v = 0; v < numAmountVectors; v++){
		 printf("%ld;", amountVectors[v]);
		 vpp_set_number_vectors(amountVectors[v]);
#endif
#if VPP_ENCODE == RGB_VECTOR
	 for(uint32_t m = 0; m < numMinRGBSizes; m++){
		 printf("%ld;", minRGBSize[m]);
		 set_min_value_RGB((uint8_t) minRGBSize[m]);
#endif
	 for(uint32_t q = 0; q < numQuality; q++){
		 printf("%lu;",quality[q]);
		 for(uint32_t size = 0; size < numImageSizes; size++){
			 jpegSize = 0;
			 for(uint32_t iterations = 0; iterations < numIterationsAvg; iterations++){
				 jpegSize +=  (double) test_size_sine_vpp(imageHeight[size], imageWidth[size], (uint8_t) quality[q], VPP_IMAGE_FILL_STRATEGY);
			 }
			 jpegSize = jpegSize / numIterationsAvg;
			 printf("%lf;", jpegSize);
		 }
		 printf("\n");

	 }
#if VPP_ENCODE == N_VECTOR
		 //printf("\n");
	 }
#endif
#if VPP_ENCODE == RGB_VECTOR
	 }
#endif

	 return 0;
}

int benchmark_size_sine_outliers_vpp(uint32_t *quality, uint32_t numQuality, uint32_t *imageHeight, uint32_t *imageWidth, uint32_t numImageSizes, double * outlierPercentage, uint32_t numOutlierPercentages, uint32_t numIterationsAvg, uint32_t * amountVectors, uint32_t numAmountVectors, uint32_t * minRGBSize, uint32_t numMinRGBSizes){
//	 uint32_t quality[] = {5,10,25,50,75,80,90,95,100};
//	 uint32_t imageHeight[] = {16,32,64,128,256};
//	 uint32_t imageWidth[] = {64,64,64,64,64};
	double jpegSize;

	 printf(";");
#if VPP_ENCODE == N_VECTOR
	 printf(";"); //Extra column
#else
	 UNUSED(numAmountVectors);
	 UNUSED(amountVectors);
#endif
#if VPP_ENCODE == RGB_VECTOR
	 printf(";"); //Extra column
#else
	 UNUSED(minRGBSize);
	 UNUSED(numMinRGBSizes);

#endif
	 for(uint32_t size = 0; size < numImageSizes; size++){
		 printf("%ld;", imageHeight[size]);
	 }
	 printf("\n");
#if VPP_ENCODE == N_VECTOR
	 for(uint32_t v = 0; v < numAmountVectors; v++){
		 printf("%ld;", amountVectors[v]);
		 vpp_set_number_vectors(amountVectors[v]);
#endif
#if VPP_ENCODE == RGB_VECTOR
	 for(uint32_t m = 0; m < numMinRGBSizes; m++){
		 printf("%ld;", minRGBSize[m]);
		 set_min_value_RGB((uint8_t) minRGBSize[m]);
#endif

	 for(uint32_t q = 0; q < numQuality; q++){
		 for(uint32_t out = 0; out < numOutlierPercentages; out++){
			 printf("%lu;%.3lf;", quality[q], outlierPercentage[out]);
			 for(uint32_t size = 0; size < numImageSizes; size++){
				 jpegSize = 0;
				 for(uint32_t iterations = 0; iterations < numIterationsAvg; iterations++){
					 jpegSize += test_size_sine_vpp_outliers(imageHeight[size], imageWidth[size], (uint8_t) quality[q], outlierPercentage[out], VPP_IMAGE_FILL_STRATEGY);
				 }
				 jpegSize = jpegSize / numIterationsAvg;
				 printf("%.3lf;", jpegSize);
			 }
			 printf("\n");
		 }
	 }
#if VPP_ENCODE == N_VECTOR
		 //printf("\n");
	 }
#endif
#if VPP_ENCODE == RGB_VECTOR
	 }
#endif

	 return 0;
}


int benchmark_performance_w_quantization_outliers_vpp(uint32_t *quality, uint32_t numQuality, uint32_t *imageHeight, uint32_t *imageWidth, uint32_t numImageSizes, uint32_t *numImageRepeats, double * outlierPercentage, uint32_t numOutlierPercentages, uint32_t numIterationsAvg, uint32_t * amountVectors, uint32_t numAmountVectors, uint32_t * minRGBSize, uint32_t numMinRGBSizes){
//	 uint32_t quality[] = {5,10,25,50,75,80,90,95,100};
//	 uint32_t imageHeight[] = {16,32,64,128,256};
//	 uint32_t imageWidth[] = {64,64,64,64,64};
	double performance = 0;
	double stddev;
	 printf(";");
#if VPP_ENCODE == N_VECTOR
	 printf(";"); //Extra column
#else
	 UNUSED(numAmountVectors);
	 UNUSED(amountVectors);
#endif
#if VPP_ENCODE == RGB_VECTOR
	 printf(";"); //Extra column
#else
	 UNUSED(minRGBSize);
	 UNUSED(numMinRGBSizes);

#endif
	 for(uint32_t size = 0; size < numImageSizes; size++){
		 printf("%ld;", imageHeight[size]);
	 }
	 printf("\n");
#if VPP_ENCODE == N_VECTOR
	 for(uint32_t v = 0; v < numAmountVectors; v++){
		 printf("%ld;", amountVectors[v]);
		 vpp_set_number_vectors(amountVectors[v]);
#endif
#if VPP_ENCODE == RGB_VECTOR
	 for(uint32_t m = 0; m < numMinRGBSizes; m++){
		 printf("%ld;", minRGBSize[m]);
		 set_min_value_RGB((uint8_t) minRGBSize[m]);
#endif

	 for(uint32_t q = 0; q < numQuality; q++){
		 for(uint32_t out = 0; out < numOutlierPercentages; out++){
			 printf("%lu;%.3lf;", quality[q], outlierPercentage[out]);
			 for(uint32_t size = 0; size < numImageSizes; size++){
				 performance = 0;
				 for(uint32_t iterations = 0; iterations < numIterationsAvg; iterations++){
					 performance += test_vpp_performance_JPEG_encode_accelerator_DMA_w_quantization(imageHeight[size], imageWidth[size], numImageRepeats[size], (uint8_t) quality[q], outlierPercentage[out], VPP_IMAGE_FILL_STRATEGY, &stddev);
				 }
				 performance = performance / numIterationsAvg;
				 printf("%.3lf;", performance);
				 //To create clear break in energy trace
				if (HAL_TIM_Base_Start_IT(&htim3) != HAL_OK)
				{
					/* Starting Error */
					printf("Error starting interrupt timer 3\n");
				}
				HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON,PWR_SLEEPENTRY_WFI);
				if (HAL_TIM_Base_Stop_IT(&htim3) != HAL_OK)
				{
					/* Starting Error */
					printf("Error stoping interrupt timer 3\n");
				}
			 }
			 printf("\n");
		 }
	 }
#if VPP_ENCODE == N_VECTOR
		 //printf("\n");
	 }
#endif
#if VPP_ENCODE == RGB_VECTOR
	 }
#endif

	 return 0;
}

#if DATASET_DATA > 0
int benchmark_performance_w_quantization_dataset_vpp(uint32_t *quality, uint32_t numQuality, uint32_t *imageHeight, uint32_t *imageWidth, uint32_t numImageSizes, uint32_t *numImageRepeats, uint32_t numIterationsAvg, uint32_t * amountVectors, uint32_t numAmountVectors, uint32_t * minRGBSize, uint32_t numMinRGBSizes){
//	 uint32_t quality[] = {5,10,25,50,75,80,90,95,100};
//	 uint32_t imageHeight[] = {16,32,64,128,256};
//	 uint32_t imageWidth[] = {64,64,64,64,64};
	double performance = 0;
	double stddev;
	uint32_t numSubImages = 0;
	 printf(";");
#if VPP_ENCODE == N_VECTOR
	 printf(";"); //Extra column
#else
	 UNUSED(numAmountVectors);
	 UNUSED(amountVectors);
#endif
#if VPP_ENCODE == RGB_VECTOR
	 printf(";"); //Extra column
#else
	 UNUSED(minRGBSize);
	 UNUSED(numMinRGBSizes);

#endif
	 for(uint32_t size = 0; size < numImageSizes; size++){
		 printf("%ld;", imageHeight[size]);
	 }
	 printf("\n");
#if VPP_ENCODE == N_VECTOR
	 for(uint32_t v = 0; v < numAmountVectors; v++){
		 printf("%ld;", amountVectors[v]);
		 vpp_set_number_vectors(amountVectors[v]);
#endif
#if VPP_ENCODE == RGB_VECTOR
	 for(uint32_t m = 0; m < numMinRGBSizes; m++){
		 printf("%ld;", minRGBSize[m]);
		 set_min_value_RGB((uint8_t) minRGBSize[m]);
#endif

	 for(uint32_t q = 0; q < numQuality; q++){
		 printf("%lu;", quality[q]);
		 for(uint32_t size = 0; size < numImageSizes; size++){
			numSubImages = DATASET_DATA_SIZE / (imageHeight[size] * imageWidth[size]);

			 //printf("\n Num sub-segments: %ld\n", numSubImages);

			performance = 0;
			for(uint32_t iterations = 0; iterations < numIterationsAvg; iterations++){
				for(uint32_t imageBufferIndex = 0; imageBufferIndex < numSubImages; imageBufferIndex++){
					performance += test_vpp_performance_JPEG_encode_accelerator_DMA_w_quantization_dataset(data + (imageBufferIndex * imageHeight[size] * imageWidth[size]), DATASET_DATA_MIN, DATASET_DATA_MAX, imageHeight[size], imageWidth[size], numImageRepeats[size], (uint8_t) quality[q], VPP_IMAGE_FILL_STRATEGY, &stddev);

				}
			}
			performance = performance / (numIterationsAvg * numSubImages) ;
			printf("%lf;", performance);
//			//To create clear break in energy trace
//			if (HAL_TIM_Base_Start_IT(&htim3) != HAL_OK)
//			{
//				/* Starting Error */
//				printf("Error starting interrupt timer 3\n");
//			}
//			HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON,PWR_SLEEPENTRY_WFI);
//			if (HAL_TIM_Base_Stop_IT(&htim3) != HAL_OK)
//			{
//				/* Starting Error */
//				printf("Error stopping interrupt timer 3\n");
//			}

		 }
		 printf("\n");
	 }
#if VPP_ENCODE == N_VECTOR
		 //printf("\n");
	 }
#endif
#if VPP_ENCODE == RGB_VECTOR
	 }
#endif

	 return 0;
}
#endif

int benchmark_JPEG_error_vpp_dataset(BenchmarkSettings *settings){
	double mse;
	uint32_t numSubImages = 0;
	printf(";");
#if VPP_ENCODE == N_VECTOR
	 printf(";"); //Extra column
#endif
#if VPP_ENCODE == RGB_VECTOR
	 printf(";"); //Extra column
#endif
	 for(uint32_t size = 0; size < settings->numSizes; size++){
		 printf("%ld;", settings->imageHeight[size]);
	 }
	 printf("\n");
#if VPP_ENCODE == N_VECTOR
	 for(uint32_t v = 0; v < settings->numAmountVectors; v++){
		 printf("%ld;", settings->amountVectors[v]);
		 vpp_set_number_vectors(settings->amountVectors[v]);
#endif
#if VPP_ENCODE == RGB_VECTOR
	 for(uint32_t m = 0; m < settings->numMinRGBSizes; m++){
		 printf("%ld;", settings->minRGBVectorSize[m]);
		 set_min_value_RGB((uint8_t) settings->minRGBVectorSize[m]);
#endif
		 for(uint32_t q = 0; q < settings->numQuality; q++){
			 printf("%lu;",settings->quality[q]);
			 for(uint32_t size = 0; size < settings->numSizes; size++){
				 numSubImages = settings->numValues / (settings->imageHeight[size] * settings->imageWidth[size]);

				 //printf("\n Num sub-segments: %ld\n", numSubImages);

				 mse = 0;
				 for(uint32_t iterations = 0; iterations < settings->numIterations; iterations++){
					 for(uint32_t imageBufferIndex = 0; imageBufferIndex < numSubImages; imageBufferIndex++){
						 mse +=  test_mse_error_vpp_dataset(settings->data + (imageBufferIndex * settings->imageHeight[size] * settings->imageWidth[size]), settings->minValue, settings->maxValue, settings->imageHeight[size], settings->imageWidth[size], (uint8_t) settings->quality[q], VPP_IMAGE_FILL_STRATEGY);
					 }
				 }
				 mse = mse / (settings->numIterations * numSubImages) ;
				 printf("%lf;", mse);
			 }
			 printf("\n");
		 }

#if VPP_ENCODE == N_VECTOR
		 //printf("\n");
	 }
#endif
#if VPP_ENCODE == RGB_VECTOR
	 }
#endif
	 return 0;
}

int benchmark_performance_encode_accelerator_jpeg_vpp(BenchmarkSettings *settings){
	double performance = 0;
	double stddev;
	uint32_t numSubImages = 0;
	printf(";");
	#if VPP_ENCODE == N_VECTOR
		 printf(";"); //Extra column
	#else
		 //UNUSED(numAmountVectors);
		 //UNUSED(amountVectors);
	#endif
	#if VPP_ENCODE == RGB_VECTOR
		 printf(";"); //Extra column
	#else
		 UNUSED(settings->minRGBVectorSize);
		 UNUSED(settings->numMinRGBSizes);

	#endif
		 for(uint32_t size = 0; size < settings->numSizes; size++){
			 printf("%ld;", settings->imageHeight[size]);
		 }
		 printf("\n");
	#if VPP_ENCODE == N_VECTOR
		 for(uint32_t v = 0; v < settings->numAmountVectors; v++){
			 printf("%ld;", settings->amountVectors[v]);
			 vpp_set_number_vectors(settings->amountVectors[v]);
	#endif
	#if VPP_ENCODE == RGB_VECTOR
		 for(uint32_t m = 0; m < settings->numMinRGBSizes; m++){
			 printf("%ld;", settings->minRGBVectorSize[m]);
			 set_min_value_RGB((uint8_t) settings->minRGBVectorSize[m]);
	#endif

		 for(uint32_t q = 0; q < settings->numQuality; q++){
			 printf("%lu;", settings->quality[q]);
			 for(uint32_t size = 0; size < settings->numSizes; size++){
				numSubImages = settings->numValues / (settings->imageHeight[size] * settings->imageWidth[size]);

				 //printf("\n Num sub-segments: %ld\n", numSubImages);

				performance = 0;
				for(uint32_t iterations = 0; iterations < settings->numIterations; iterations++){
					for(uint32_t imageBufferIndex = 0; imageBufferIndex < numSubImages; imageBufferIndex++){
						performance += test_vpp_performance_JPEG_encode_accelerator_DMA_w_quantization_dataset(settings->data + (imageBufferIndex * settings->imageHeight[size] * settings->imageWidth[size]), settings->minValue, settings->maxValue, settings->numValues, settings->imageHeight[size], settings->imageWidth[size], settings->numRepeatImageQuantization[size], (uint8_t) settings->quality[q], VPP_IMAGE_FILL_STRATEGY, &stddev);

					}
				}
				performance = performance / (settings->numIterations * numSubImages) ;
				printf("%lf;", performance);
	//			//To create clear break in energy trace
	//			if (HAL_TIM_Base_Start_IT(&htim3) != HAL_OK)
	//			{
	//				/* Starting Error */
	//				printf("Error starting interrupt timer 3\n");
	//			}
	//			HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON,PWR_SLEEPENTRY_WFI);
	//			if (HAL_TIM_Base_Stop_IT(&htim3) != HAL_OK)
	//			{
	//				/* Starting Error */
	//				printf("Error stopping interrupt timer 3\n");
	//			}

			 }
			 printf("\n");
		 }
	#if VPP_ENCODE == N_VECTOR
			 //printf("\n");
		 }
	#endif
	#if VPP_ENCODE == RGB_VECTOR
		 }
	#endif

	return 0;
}


#endif

//int benchmark(uint32_t *iterators, BeginIterationFunc *beginIterationFuncs, uint32_t numIterators, TestFunc testFunc, Measurement measurement, ENERGY_INTERVAL_FLAG energyIntervalFlag);
