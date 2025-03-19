/*
 * benchmark_xor.c
 *
 *  Created on: Feb 25, 2025
 *      Author: luisferreira
 */

#include "benchmark_from_uart.h"
#include "benchmark_xor.h"
#include "test_xor.h"
#include <stdio.h>

int benchmark_performance_encode_UART_xor(BenchmarkSettings *settings){
	double performance = 0;
	double stddev;
	uint32_t numSubImages = 0;

	for(uint32_t size = 0; size < settings->numSizes; size++){
	 printf("%ld;", settings->imageHeight[size]);
	}
	printf("\n");


		 for(uint32_t size = 0; size < settings->numSizes; size++){
			numSubImages = settings->numValues / (settings->imageHeight[size] * settings->imageWidth[size]);

			 //printf("\n Num sub-segments: %ld\n", numSubImages);

			performance = 0;
			for(uint32_t iterations = 0; iterations < settings->numIterations; iterations++){
				for(uint32_t imageBufferIndex = 0; imageBufferIndex < numSubImages; imageBufferIndex++){
					performance += test_xor_performance_dataset(settings->data + (imageBufferIndex * settings->imageHeight[size] * settings->imageWidth[size]), settings->imageHeight[size] * settings->imageWidth[size], settings->numRepeatImageQuantization[size], &stddev);
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


	return 0;
}

int benchmark_compressed_size_UART_xor(BenchmarkSettings *settings){
	uint32_t compressedSize = 0;
	uint32_t numSubImages = 0;

	for(uint32_t size = 0; size < settings->numSizes; size++){
	 printf("%ld;", settings->imageHeight[size]);
	}
	printf("\n");


	 for(uint32_t size = 0; size < settings->numSizes; size++){
		numSubImages = settings->numValues / (settings->imageHeight[size] * settings->imageWidth[size]);

		 //printf("\n Num sub-segments: %ld\n", numSubImages);

		compressedSize = 0;

		for(uint32_t imageBufferIndex = 0; imageBufferIndex < numSubImages; imageBufferIndex++){
			compressedSize += test_xor_compressed_size_dataset(settings->data + (imageBufferIndex * settings->imageHeight[size] * settings->imageWidth[size]), settings->imageHeight[size] * settings->imageWidth[size]);
		}

		compressedSize = compressedSize / (settings->numIterations * numSubImages) ;
		printf("%ld;", compressedSize);

	 }
	 printf("\n");


	return 0;
}

int benchmark_error_UART_xor(BenchmarkSettings *settings){
	uint32_t nonMatchingValues = 0;
	uint32_t numSubImages = 0;

	for(uint32_t size = 0; size < settings->numSizes; size++){
		printf("%ld;", settings->imageHeight[size]);
	}
	printf("\n");

	for(uint32_t size = 0; size < settings->numSizes; size++){
		numSubImages = settings->numValues / (settings->imageHeight[size] * settings->imageWidth[size]);

		 //printf("\n Num sub-segments: %ld\n", numSubImages);

		nonMatchingValues = 0;

		for(uint32_t imageBufferIndex = 0; imageBufferIndex < numSubImages; imageBufferIndex++){
			nonMatchingValues += test_xor_error_dataset(settings->data + (imageBufferIndex * settings->imageHeight[size] * settings->imageWidth[size]), settings->imageHeight[size] * settings->imageWidth[size]);
		}

		nonMatchingValues = nonMatchingValues / (settings->numIterations * numSubImages) ;
		printf("%ld;", nonMatchingValues);

	 }
	 printf("\n");


	return 0;

}
