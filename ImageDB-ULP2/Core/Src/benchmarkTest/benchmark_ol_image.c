/*
 * accelerator_raw_performance_benchmark.c
 *
 *  Created on: Aug 30, 2024
 *      Author: luisferreira
 */

#include <benchmark_ol_image.h>
#include <compression/jpeg-manipulator.h>
#include <image_output.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "settings.h"
#include "data_generator.h"
#include "vpp_image.h"
#include "ol_image.h"
#include <stdbool.h>
#include <test_vpp_images.h>
#include "ol_image_generator.h"
#include "ol_image_blender.h"
#include "test_ol_images.h"
#include "settings.h"


#define CPU_DMA2D_OP 0
#define DMA2D_OP 1

typedef int (*t_blend_images) (uint8_t * fgImage, uint8_t * bgImage, uint8_t * destination, uint32_t numPixels, uint8_t alphaFg, uint8_t alphaBg, uint8_t imageHasAlpha, uint8_t useFixedAlpha);

#if JPEG_ACCELERATOR == ON
typedef int (*t_encode_decode_accelerator_func) (uint8_t * image, uint32_t imageSize, uint8_t * JPEGOutput, uint32_t JPEGOutputSize, JPEG_ConfTypeDef * conf);
typedef uint8_t* (*t_encode_decode_cpu_func) (uint8_t * image, uint32_t width, uint32_t height, int quality);

//WARNING: make sure there is enough memory for all malloc's
uint32_t _benchmark_jpeg_encode_decode_accelerator_ol_image(uint32_t numIterations, uint32_t numImages, uint32_t imageHeight, uint32_t imageWidth, uint8_t quality,
		t_encode_decode_accelerator_func encode_decode_func, int encode_decode_flag){
	double *data = malloc(sizeof(double) * imageWidth);
	uint8_t *imagePixels = (uint8_t *) malloc(imageHeight * imageWidth * 4);
	uint8_t *outData = (uint8_t *)  0x30000000;//malloc(imageHeight * imageWidth * 4); //How to know what size to use?

	if( !data || !imagePixels || !outData){
		printf("Could not allocate sufficient space for benchmark run \n");
		return 0;
	}

	JPEG_ConfTypeDef * jpegConf = new_jpeg_conf(imageHeight, imageWidth, quality);
	uint32_t duration, start;
	double imagesPerSec;

	//Random data
	generate_sine_data(data, imageWidth, 0.1, 0);
	memset(imagePixels, 0, sizeof(uint8_t) * imageHeight * imageWidth * 4);
	data_to_ol_image(data, imagePixels, 255, 0, 0, ALPHA, -1,1, imageWidth, imageHeight);
	uint32_t numBytes = imageHeight * imageWidth * 4;

	//Must first encode the image
	if(encode_decode_flag == DECODE_FLAG){
		 encode_JPEG_accelerator(imagePixels, numBytes, imagePixels, numBytes, jpegConf);
//		 //Set Flag to 0 again.
//		 jpegMDMADone = 0;
	}


	//TODO: find a time counter that does not cause interrupts

	//TODO: Do 3 bytes per pixel?
	start = HAL_GetTick();
	for(uint32_t j = 0; j < numIterations; j++){
		for(uint32_t i = 0; i < numImages; i++){
			//start = HAL_GetTick();
			encode_decode_func(imagePixels, numBytes, outData, numBytes, jpegConf);
			//duration += wait_JPEG_MDMA() - start;
		}
	}
	duration = HAL_GetTick() - start;
	imagesPerSec =  ((double) numImages * numIterations) / ((double)duration/1000);
	printf("JPEG accelerator can %s %lu x %lu images with [%d] quality at a rate of: %lf images/sec, or %lf pixels/sec\n", encode_decode_flag == ENCODE_FLAG ? "encode" : "decode", imageHeight, imageWidth, quality, imagesPerSec, imagesPerSec * imageHeight * imageWidth);
	free(jpegConf);
	free(imagePixels);
	free(data);

	//free(outData);
	return (uint32_t)(imagesPerSec * (double)imageHeight * (double)imageWidth);

}

int _encode_JPEG_MDMA_wrapper_ol_image (uint8_t * image, uint32_t imageSize, uint8_t * JPEGOutput, uint32_t JPEGOutputSize, JPEG_ConfTypeDef * conf){
	encode_JPEG_accelerator_MDMA(image, imageSize, JPEGOutput, JPEGOutputSize, conf);
	return wait_JPEG_MDMA();
}

int _decode_JPEG_MDMA_wrapper_ol_image (uint8_t * Jpeg, uint32_t JPEGSize, uint8_t * output, uint32_t outputSize, JPEG_ConfTypeDef * conf){
	decode_JPEG_accelerator_MDMA(Jpeg, JPEGSize, output, outputSize, conf);
	return wait_JPEG_MDMA();
}

uint32_t benchmark_jpeg_encode_accelerator_CPU_ol_image(uint32_t numIterations, uint32_t numImages, uint32_t imageHeight, uint32_t imageWidth, uint8_t quality){
	t_encode_decode_accelerator_func encode_func =  encode_JPEG_accelerator;
	return _benchmark_jpeg_encode_decode_accelerator_ol_image(numIterations, numImages, imageHeight, imageWidth, quality, encode_func, ENCODE_FLAG);
}

uint32_t benchmark_jpeg_encode_accelerator_MDMA_ol_image(uint32_t numIterations, uint32_t numImages, uint32_t imageHeight, uint32_t imageWidth, uint8_t quality){
	t_encode_decode_accelerator_func encode_func = _encode_JPEG_MDMA_wrapper_ol_image;
	return _benchmark_jpeg_encode_decode_accelerator_ol_image(numIterations, numImages, imageHeight, imageWidth, quality, encode_func, ENCODE_FLAG);
}

uint32_t benchmark_jpeg_decode_accelerator_CPU_ol_image(uint32_t numIterations, uint32_t numImages, uint32_t imageHeight, uint32_t imageWidth, uint8_t quality){
	t_encode_decode_accelerator_func decode_func = decode_JPEG_accelerator;
	return _benchmark_jpeg_encode_decode_accelerator_ol_image(numIterations, numImages, imageHeight, imageWidth, quality, decode_func, DECODE_FLAG);
}

uint32_t benchmark_jpeg_decode_accelerator_MDMA_ol_image(uint32_t numIterations, uint32_t numImages, uint32_t imageHeight, uint32_t imageWidth, uint8_t quality){
	t_encode_decode_accelerator_func decode_func = _decode_JPEG_MDMA_wrapper_ol_image;
	return _benchmark_jpeg_encode_decode_accelerator_ol_image(numIterations, numImages, imageHeight, imageWidth, quality, decode_func, DECODE_FLAG);
}


uint32_t _benchmark_jpeg_encode_decode_CPU_ol_image(uint32_t numIterations, uint32_t numImages, uint32_t imageHeight, uint32_t imageWidth, uint8_t quality, t_encode_decode_cpu_func encode_decode_func, int encode_decode_flag){

	double *data = malloc(sizeof(double) * imageWidth);
	uint8_t *imagePixels = (uint8_t *) malloc(imageHeight * imageWidth * 3);
	uint32_t duration, start;
	double imagesPerSec;

	generate_sine_data(data, imageWidth, 0.01, 0);
	memset(imagePixels, 0, sizeof(uint8_t) * imageHeight * imageWidth * 3);
	data_to_ol_image(data, imagePixels, 255,0,0,NO_ALPHA,-1,1, imageWidth, imageHeight);

	if(encode_decode_flag == DECODE_FLAG){
		uint8_t * pixels_aux = imagePixels;
		imagePixels = encode_JPEG_from_memory_to_memory(imagePixels,imageWidth, imageHeight, quality);
		free(pixels_aux);
	}
	//TODO: find a time counter that does not cause interrupts

	//TODO: Do 3 bytes per pixel?
	start = HAL_GetTick();
	for(uint32_t j = 0; j < numIterations; j++){
		for(uint32_t i = 0; i < numImages; i++){
			free(encode_decode_func(imagePixels,imageWidth, imageHeight, quality));
		}
	}
	duration = HAL_GetTick() - start;
	imagesPerSec =  ((double) numImages * numIterations) / ((double)duration/1000);
	printf("JPEG accelerator can %s %lu x %lu images with [%d] quality at a rate of: %lf images/sec, or %lf pixels/sec\n", encode_decode_flag == ENCODE_FLAG ? "encode" : "decode", imageHeight, imageWidth, quality, imagesPerSec, imagesPerSec * imageHeight * imageWidth);
	free(data);
	free(imagePixels);
	return (uint32_t)(imagesPerSec * (double)imageHeight * (double)imageWidth);


}

uint32_t benchmark_jpeg_decode_CPU_ol_image(uint32_t numIterations, uint32_t numImages, uint32_t imageHeight, uint32_t imageWidth, uint8_t quality){
	return _benchmark_jpeg_encode_decode_CPU_ol_image(numIterations, numImages, imageWidth, imageHeight, quality,encode_JPEG_from_memory_to_memory, ENCODE_FLAG);
}


uint32_t _benchmark_blending_generic_ol_image(uint32_t numIterations, uint32_t numImages, uint32_t imageHeight, uint32_t imageWidth, t_blend_images blend_func, int entity_flag){
	double *dataBg = malloc(sizeof(double) * imageWidth);
	double *dataFg = malloc(sizeof(double) * imageWidth);
	uint8_t *imageBgPixels = (uint8_t *) malloc(imageHeight * imageWidth * 4);
	uint8_t *imageFgPixels = (uint8_t *) malloc(imageHeight * imageWidth * 4);
	uint8_t *imageDestinationPixels = (uint8_t *) 0x30000000;//(uint8_t *) malloc(imageHeight * imageWidth * 4); //Not enough memory
	uint32_t duration, start;
	double imagesPerSec;



	start = HAL_GetTick();
	for(uint32_t j = 0; j < numIterations; j++){
		memset(imageBgPixels, 0, sizeof(uint8_t) * imageHeight * imageWidth * 4);
		memset(imageFgPixels, 0, sizeof(uint8_t) * imageHeight * imageWidth * 4);
		memset(imageDestinationPixels, 0, sizeof(uint8_t) * imageHeight * imageWidth * 4);
		generate_sine_data(dataBg, imageWidth, 0.01, 0 + j);
		generate_sine_data(dataFg, imageWidth, 0.01, 6 + j);
		data_to_ol_image(dataBg, imageBgPixels,255,0,0,ALPHA,-1,1, imageWidth, imageHeight);
		data_to_ol_image(dataFg, imageFgPixels,0,255,0,ALPHA,-1,1, imageWidth, imageHeight);

		for(uint32_t i = 0; i < numImages; i++){
			blend_func(imageFgPixels, imageBgPixels, imageDestinationPixels, imageWidth * imageHeight, FOREGROUND_ALPHA, BACKGROUND_ALPHA, ALPHA, true);
		}
	}
	duration = HAL_GetTick() - start;
	imagesPerSec =  ((double) numImages * numIterations) / ((double)duration/1000);
	printf("%s can blend %lu x %lu images at a rate of: %lf image pairs/sec, or %lf destination pixels/sec\n", entity_flag == CPU_DMA2D_OP ? "CPU simulating DMA2D" : "DMA2D accelerator", imageHeight, imageWidth, imagesPerSec, imagesPerSec * imageHeight * imageWidth);
	//WARNING: Freeing order matters!
	free(imageFgPixels);
	free(imageBgPixels);
	free(dataFg);
	free(dataBg);



	//free(imageDestinationPixels);
	return (uint32_t)(imagesPerSec * (double)imageHeight * (double)imageWidth);



	return 0;
}

//uint32_t benchmark_blending_DMA2D_ol_image(uint32_t numIterations, uint32_t numImages, uint32_t imageHeight, uint32_t imageWidth){
//	t_blend_images blend_func = blend_images_DMA2D_interrupt;
//	_benchmark_blending_generic_ol_image(numIterations, numImages, imageHeight, imageWidth, blend_func, DMA2D_OP);
//	return 0;
//}

uint32_t benchmark_blending_CPU_ol_image(uint32_t numIterations, uint32_t numImages, uint32_t imageHeight, uint32_t imageWidth){
	t_blend_images blend_func = blend_images;
	_benchmark_blending_generic_ol_image(numIterations, numImages, imageHeight, imageWidth, blend_func, CPU_DMA2D_OP);
	return 0;

}

//uint32_t benchmark_blending_CPU_simple_ol_image(uint32_t numIterations, uint32_t numImages, uint32_t imageHeight, uint32_t imageWidth){
//	t_blend_images blend_func = blend_images_simple;
//	_benchmark_blending_generic_ol_image(numIterations, numImages, imageHeight, imageWidth, blend_func, CPU_DMA2D_OP);
//	return 0;
//
//}

/*
 * Benchmark the size of compressed jpeg images for the overlapping lines (OL image) method (i.e., drawing the values as images). Image width and height values are tested as pairs
 * (i.e., number of heights and widths should be the same). For example for widths 64, 64 and heights 64, 128 the tested images will be 64x64 and 64x128.
 * Prints results in csv format to stdout with format - header: ;;Height0, Height1, Height2, ... | rows: Quality; #Overlapped Lines; Size for WidthxHeight0; Size for WidthxHeight1; ...
 * One row per quality, overlappedLines, Height x Width combination.
 * Notice that we are currently assuming imageWidth is fixed, hence why header only displays height.
 */
int benchmark_size_jpeg_ol_image(uint32_t *quality, uint32_t numQuality, uint32_t *overlappedLines, uint32_t numOverlappedLines, uint32_t *imageHeight, uint32_t *imageWidth, uint32_t numImageSizes){
//	uint32_t quality[]={5,10,25,50,75,90,100};
//	uint32_t overlappedLines[] = {1,2,5,10,20};
//	uint32_t imageHeight[] = {16,32,64,128, 256};
//	uint32_t imageWidth[] = {64,64,64,64,64};
	uint8_t * jpeg;

//	printf("\n\n");
	//print_image_pixels_alpha(buffers->image, imageWidth, imageHeight);

	JPEG_ConfTypeDef * jpegConf = NULL;
	MultilineImage * multilineImage;

	printf(";;");
	for(uint32_t size = 0; size < numImageSizes; size++){
		printf("%ld;", imageHeight[size]);
	}
	for(uint32_t q = 0; q < numQuality; q++){
		printf("\n");
		for(uint32_t ol = 0; ol < numOverlappedLines; ol++){
			printf("%lu;%lu;",quality[q],overlappedLines[ol]);
			for(uint32_t size = 0; size < numImageSizes; size++){
				multilineImage =  generate_ol_image(imageHeight[size], imageWidth[size], overlappedLines[ol], true);
				jpeg = (uint8_t *) malloc(sizeof(uint8_t) * imageHeight[size] * imageWidth[size]* 4);
				jpegConf = new_jpeg_conf(imageHeight[size], imageWidth[size], quality[q]);
				memset(jpeg, 0, sizeof(uint8_t) * imageHeight[size] * imageWidth[size] * 4);
				encode_JPEG_accelerator(multilineImage->image, sizeof(uint8_t) * imageHeight[size] * imageWidth[size] * 4 , jpeg, sizeof(uint8_t) * imageHeight[size] * imageWidth[size] * 4, jpegConf);
				printf("%lu;", getJPEGSize(jpeg, imageHeight[size] * imageWidth[size] * 4));
				free(jpeg);
				destroyMultilineImage(multilineImage);
			}
			printf("\n");
		}

	}


	return 0;

}
#endif

/*
 * Benchmark the error of overlapping line (OL) images that have been compressed using jpeg. Error given as expected pixel height vs decoded pixel height.
 * Note: Remember that values are quantized according to the number of available height pixels.
 * Image width and height values are tested as pairs.
 * (i.e., number of heights and widths should be the same). For example for widths 64, 64 and heights 64, 128 the tested images will be 64x64 and 64x128.
 * Prints results in csv format to stdout with format - header: ;;Height0, Height1, Height2, ... | rows: Quality; #Overlapped Lines; Error for WidthxHeight0; Error for WidthxHeight1; ...
 * One error measurement per quality, overlappedLines, Height x Width combination.
 * Notice that we are currently assuming imageWidth is fixed, hence why header only displays height.
 *
 */
int benchmark_error_ol_image(uint32_t *quality, uint32_t numQuality, uint32_t *overlappedLines, uint32_t numOverlappedLines, uint32_t *imageHeight, uint32_t *imageWidth, uint32_t numImageSizes){
//    uint32_t quality[] = {5, 10, 25, 50, 75,80,90,95,100};
//    uint32_t imageHeight[] = {16,32,64,128, 256};
//    uint32_t imageWidth[] = {64,64,64,64,64};
//    uint32_t overlappedLines[] = {1,2,5,10,20};
    double mae;
    //TODO: register error distribution by color.
    //Header, to separate each surface in csv
    printf(";;");
    for(uint32_t size = 0; size < numImageSizes; size++){
        printf("%ld;", imageHeight[size]);
    }
    //Que atrocidade
    for(uint32_t q = 0; q < numQuality; q++){
        printf("\n");
        for(uint32_t ol = 0; ol < numOverlappedLines; ol++){
            printf("%lu;%lu;",quality[q],overlappedLines[ol]);
            for(uint32_t size = 0; size < numImageSizes; size++){
                mae = test_ol_decode_algorithm_accelerators(imageWidth[size], imageHeight[size], overlappedLines[ol], (uint16_t) quality[q]);
                printf("%lf;", mae);
            }
            printf("\n");
        }

    }
    return 0;

}



