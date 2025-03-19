/*
 * benchmark_accelerators.c
 *
 *  Created on: Sep 2, 2024
 *      Author: luisferreira
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <test_performance_JPEG.h>
#include "settings.h"
#include "stm32u5xx_hal.h"
#include "benchmark_accelerators.h"
#include "benchmark_from_uart.h"
#include "uart.h"


extern TIM_HandleTypeDef htim3;

//TODO: test with vectorization instructions if possible?
/*
 * Image benchmarks the performance of converting RGB image data to the format required by the JPEG encoder, i.e., YCbCr.
 * Will test encoding images of the given imageType, for varying width and height sizes, as per the parameters received.
 * To get a more stable result, each image will be converted from RGB to YCbCr num_repeat_image times. This value is set for each image size.
 * For each image size, numIterations different images will be generated, and their results averaged.
 *
 */
void benchmark_YCbCr_encode_CPU(uint32_t *imageHeights, uint32_t * imageWidths, uint32_t *numRepeatImage, uint32_t numSizes, uint32_t *batchSizes, uint32_t numBatchSizes, uint32_t numIterations){
    double performance = 0;
    printf(";");
	for(uint32_t size = 0; size < numSizes; size++){
		printf("%ldx%ld;", imageWidths[size], imageHeights[size]);
	}
	printf("\n");
	for(uint32_t batch = 0; batch < numBatchSizes; batch++){
		printf("%ld;", batchSizes[batch]);
		for(uint32_t size = 0; size < numSizes; size++){
			for(uint32_t iterations = 0; iterations < numIterations; iterations++){
				performance += test_performance_YCbCr_encode_CPU(imageHeights[size], imageWidths[size], numRepeatImage[size], batchSizes[batch], ACCELERATOR_BENCHMARK_IMAGE_TYPE);
			}

			performance = performance / numIterations;
			printf("%lf;", performance);
			performance = 0;
		}
		printf("\n");
	}
	printf("\n");
}

void benchmark_performance_JPEG_accelerator_DMA(uint32_t *imageHeights, uint32_t * imageWidths, uint32_t *numRepeatImage, uint32_t numSizes, uint32_t *quality, uint32_t numQuality, uint32_t numIterations, uint8_t timeQuantization){
	double performance = 0, runPerformance = 0, runStddev = 0, stddev=0;
	printf(";");
	for(uint32_t size = 0; size < numSizes; size++){
		printf("%ldx%ld;;", imageWidths[size], imageHeights[size]);
	}
	for(uint32_t q = 0; q < numQuality; q++){
		printf("\n");
		printf("%lu;",quality[q]);
		for(uint32_t size = 0; size < numSizes; size++){
			for(uint32_t iterations = 0; iterations < numIterations; iterations++){
				runPerformance = 0;
				runStddev = 0;
				test_performance_JPEG_encode_accelerator_DMA(imageHeights[size], imageWidths[size], numRepeatImage[size], (uint8_t) quality[q], ACCELERATOR_BENCHMARK_IMAGE_TYPE, &runPerformance, &runStddev, timeQuantization);
				performance += runPerformance;
				stddev += runStddev;
			}

			performance = performance / numIterations;
			stddev = stddev / numIterations;
			printf("%lf;%lf;", performance, stddev);
			performance = 0;
			stddev = 0;
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
	}
	printf("\n");

}

#define CHAR_BUFF_SIZE 50
//From: https://stackoverflow.com/questions/23191203/convert-float-to-string-without-sprintf + debug (fixed)
static char * _float_to_char(float x, char *p) {
    char *s = p + CHAR_BUFF_SIZE; // go to end of buffer
    uint16_t decimals;  // variable to store the decimals
    int units;  // variable to store the units (part to left of decimal place)
    if (x < 0) { // take care of negative numbers
        decimals = (int)(x * -100) % 100; // make 1000 for 3 decimals etc.
        units = (int)(-1 * x);
    } else { // positive numbers
        decimals = (int)(x * 100) % 100;
        units = (int)x;
    }
    *--s = '\0';
    *--s = (decimals % 10) + '0';
    decimals /= 10; // repeat for as many decimal places as you need
    *--s = (decimals % 10) + '0';
    *--s = '.';

    while (units > 0) {
        *--s = (units % 10) + '0';
        units /= 10;
    }
    if (x < 0) *--s = '-'; // unary minus sign for negative numbers
    return s;
}

//JPEG ONLY! No quantization or YCbCr conversion
void benchmark_raw_JPEG_accelerator_encode_performance_dataset(BenchmarkSettings *settings){
	double performance = 0, runStddev = 0, stddev=0;
	char performanceStr[CHAR_BUFF_SIZE], *strP;
	printf(";");
	for(uint32_t size = 0; size < settings->numSizes; size++){
		printf("%ldx%ld;;", settings->imageWidth[size], settings->imageHeight[size]);
	}
	for(uint32_t q = 0; q < settings->numQuality; q++){
		printf("\n");
		strP = _float_to_char((float)settings->quality[q], performanceStr);
		uart_transmit_str((uint8_t *) strP);
		uart_transmit_ch(';');
		//printf("%lu;",settings->quality[q]);
		for(uint32_t size = 0; size < settings->numSizes; size++){
			for(uint32_t iterations = 0; iterations < settings->numIterations; iterations++){
				runStddev = 0;
				performance += test_performance_JPEG_encode_accelerator_DMA_raw_dataset(settings->data, settings->minValue, settings->maxValue, settings->numValues, settings->imageHeight[size], settings->imageWidth[size], settings->numRepeatImageQuantization[size], settings->quality[q], false, &runStddev);
				stddev += runStddev;
			}

			performance = performance / settings->numIterations;
			stddev = stddev / settings->numIterations;

			//printf and sprintf were not working here. Perhaps UART has a limited buffer space hence printf not working? sprintf I believe has known issues on stm32 also.
			strP = _float_to_char((float)performance, performanceStr);
			uart_transmit_str((uint8_t *) strP);
			uart_transmit_ch(';');
			strP = _float_to_char((float)stddev, performanceStr);
			uart_transmit_str((uint8_t *) strP);
			uart_transmit_ch(';');
			//uart_transmit_ch('\n');
			performance = 0;
			stddev = 0;
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
	}
	printf("\n");
}


void benchmark_performance_JPEG_accelerator_DMA_decode(uint32_t *imageHeights, uint32_t * imageWidths, uint32_t *numRepeatImage, uint32_t numSizes, uint32_t *quality, uint32_t numQuality, uint32_t numIterations){
	double performance = 0, runPerformance = 0, runStddev = 0, stddev=0;
	printf(";");
	for(uint32_t size = 0; size < numSizes; size++){
		printf("%ldx%ld;;", imageWidths[size], imageHeights[size]);
	}
	for(uint32_t q = 0; q < numQuality; q++){
		printf("\n");
		printf("%lu;",quality[q]);
		for(uint32_t size = 0; size < numSizes; size++){
			for(uint32_t iterations = 0; iterations < numIterations; iterations++){
				runPerformance = 0;
				runStddev = 0;
				test_performance_JPEG_decode_accelerator_DMA(imageHeights[size], imageWidths[size], numRepeatImage[size], (uint8_t) quality[q], ACCELERATOR_BENCHMARK_IMAGE_TYPE, &runPerformance, &runStddev);
				performance += runPerformance;
				stddev += runStddev;
			}

			performance = performance / numIterations;
			stddev = stddev / numIterations;
			printf("%lf;%lf;", performance, stddev);
			performance = 0;
			stddev = 0;
		}
	}
	printf("\n");

}

void benchmark_performance_JPEG_accelerator_IT(uint32_t *imageHeights, uint32_t * imageWidths, uint32_t *numRepeatImage, uint32_t numSizes, uint32_t *quality, uint32_t numQuality, uint32_t numIterations, uint8_t timeQuantization){
	double performance = 0, runPerformance = 0, runStddev = 0, stddev=0;
	printf(";");
	for(uint32_t size = 0; size < numSizes; size++){
		printf("%ldx%ld;;", imageWidths[size], imageHeights[size]);
	}
	for(uint32_t q = 0; q < numQuality; q++){
		printf("\n");
		printf("%lu;",quality[q]);
		for(uint32_t size = 0; size < numSizes; size++){
			for(uint32_t iterations = 0; iterations < numIterations; iterations++){
				runPerformance = 0;
				runStddev = 0;
				test_performance_JPEG_encode_accelerator_IT(imageHeights[size], imageWidths[size], numRepeatImage[size], (uint8_t) quality[q], ACCELERATOR_BENCHMARK_IMAGE_TYPE, &runPerformance, &runStddev, timeQuantization);
				performance += runPerformance;
				stddev += runStddev;
			}

			performance = performance / numIterations;
			stddev = stddev / numIterations;
			printf("%lf;%lf;", performance, stddev);
			performance = 0;
			stddev = 0;
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
	}
	printf("\n");

}

void benchmark_performance_JPEG_accelerator_IT_decode(uint32_t *imageHeights, uint32_t * imageWidths, uint32_t *numRepeatImage, uint32_t numSizes, uint32_t *quality, uint32_t numQuality, uint32_t numIterations){
	double performance = 0, runPerformance = 0, runStddev = 0, stddev=0;
	printf(";");
	for(uint32_t size = 0; size < numSizes; size++){
		printf("%ldx%ld;;", imageWidths[size], imageHeights[size]);
	}
	for(uint32_t q = 0; q < numQuality; q++){
		printf("\n");
		printf("%lu;",quality[q]);
		for(uint32_t size = 0; size < numSizes; size++){
			for(uint32_t iterations = 0; iterations < numIterations; iterations++){
				runPerformance = 0;
				runStddev = 0;
				test_performance_JPEG_decode_accelerator_IT(imageHeights[size], imageWidths[size], numRepeatImage[size], (uint8_t) quality[q], ACCELERATOR_BENCHMARK_IMAGE_TYPE, &runPerformance, &runStddev);
				performance += runPerformance;
				stddev += runStddev;
			}

			performance = performance / numIterations;
			stddev = stddev / numIterations;
			printf("%lf;%lf;", performance, stddev);
			performance = 0;
			stddev = 0;

		}
	}
	printf("\n");

}


//TODO: need better code reuse
//TODO: remove timeQuantization
//Time quantization - if true, takes into account the time to generate image -> makes no sense...
void benchmark_performance_JPEG_encode_CPU(uint32_t *imageHeights, uint32_t * imageWidths, uint32_t *numRepeatImage, uint32_t numSizes, uint32_t *quality, uint32_t numQuality, uint32_t numIterations, uint8_t timeQuantization){
	double performance = 0, runPerformance = 0, runStddev = 0, stddev=0;
	printf(";");
	for(uint32_t size = 0; size < numSizes; size++){
		printf("%ldx%ld;;", imageWidths[size], imageHeights[size]);
	}
	for(uint32_t q = 0; q < numQuality; q++){
		printf("\n");
		printf("%lu;",quality[q]);
		for(uint32_t size = 0; size < numSizes; size++){
			for(uint32_t iterations = 0; iterations < numIterations; iterations++){
				runPerformance = 0;
				runStddev = 0;
				test_performance_JPEG_encode_CPU(imageHeights[size], imageWidths[size], numRepeatImage[size], (uint8_t) quality[q], ACCELERATOR_BENCHMARK_IMAGE_TYPE, &runPerformance, &runStddev, timeQuantization);
				performance += runPerformance;
				stddev += runStddev;
			}

			performance = performance / numIterations;
			stddev = stddev / numIterations;
			printf("%lf;%lf;", performance, stddev);
			performance = 0;
			stddev = 0;
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
	}
	printf("\n");
}



void benchmark_performance_JPEG_decode_CPU(uint32_t *imageHeights, uint32_t * imageWidths, uint32_t *numRepeatImage, uint32_t numSizes, uint32_t *quality, uint32_t numQuality, uint32_t numIterations){

	double performance = 0, runPerformance = 0, runStddev = 0, stddev=0;
		printf(";");
		for(uint32_t size = 0; size < numSizes; size++){
			printf("%ldx%ld;;", imageWidths[size], imageHeights[size]);
		}
		for(uint32_t q = 0; q < numQuality; q++){
			printf("\n");
			printf("%lu;",quality[q]);
			for(uint32_t size = 0; size < numSizes; size++){
				for(uint32_t iterations = 0; iterations < numIterations; iterations++){
					runPerformance = 0;
					runStddev = 0;
					test_performance_JPEG_decode_CPU(imageHeights[size], imageWidths[size], numRepeatImage[size], (uint8_t) quality[q], ACCELERATOR_BENCHMARK_IMAGE_TYPE, &runPerformance, &runStddev);
					performance += runPerformance;
					stddev += runStddev;
				}

				performance = performance / numIterations;
				stddev = stddev / numIterations;
				printf("%lf;%lf;", performance, stddev);
				performance = 0;
				stddev = 0;
			}
		}
		printf("\n");


}

