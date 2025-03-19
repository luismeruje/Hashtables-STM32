/*
 * file_image_array_creator.c
 *
 *  Created on: Jul 1, 2024
 *      Author: luisferreira
 */



#include <compression/jpeg-manipulator.h>
#include <image_output.h>
#include "settings.h"
#include "ol_image_generator.h"
#include "ol_image.h"
#include "ol_image_blender.h"
#include "data_generator.h"
#include "image_output.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "num_utils.h"


extern DMA2D_HandleTypeDef hdma2d;

int test_ol_JPEG_CPU(uint32_t imageWidth, uint32_t imageHeight){
	double * sineValues = (double *) malloc(sizeof(double) * imageWidth);
	uint8_t * image = (uint8_t *) malloc(sizeof(uint8_t) * imageWidth * imageHeight * 3);
	uint8_t * JPEGImage ; // Height x Width x ARGB

	memset(image, 0, sizeof(uint8_t) * imageWidth * imageHeight * 3);

	generate_sine_data(sineValues, imageWidth, 0.1, 0);
	data_to_ol_image(sineValues, image, 255,0,0,NO_ALPHA, -1, 1, imageWidth, imageHeight);
	printf("Values:\n");
	for(uint32_t i = 0; i < imageWidth; i++){
		printf("%lf,",sineValues[i]);
	}
	printf("\n\n");
	print_image_pixels(image, imageWidth, imageHeight);

	JPEGImage = encode_JPEG_from_memory_to_memory(image, imageWidth, imageHeight, 90);

	free(image);

	image = decode_JPEG_from_memory_to_memory(JPEGImage, imageWidth, imageHeight);

	printf("\n\n\n Printing image after encode/decode JPEG process.\n");
	print_image_pixels(image, imageWidth, imageHeight);

	printf("\n\nJpeg size: %lu\n", getJPEGSize(JPEGImage,  imageWidth*imageHeight*4));

	//TODO: I think free's must be done in reverse of allocation.
	free(image);
	free(JPEGImage);
	free(sineValues);
	return 0;

}

int test_ol_blend_accelerator(uint32_t imageWidth, uint32_t imageHeight){
	double * data = (double *) malloc(sizeof(double) * 300);
	uint8_t * image = (uint8_t *) malloc(sizeof(uint8_t) * 300 * 200 * 4);
	uint8_t * aux = (uint8_t *) 0x30000000;//(uint8_t *) malloc(sizeof(uint8_t) * 300 * 200 * 4);//0x30000000;

	memset(image, 0, sizeof(uint8_t) * imageWidth * imageHeight * 4);
	memset(aux, 0, sizeof(uint8_t) * imageWidth * imageHeight * 4);

	generate_sine_data(data, imageWidth, 0.1, 0);
	data_to_ol_image(data, image, 255,0,0,ALPHA, -1, 1, imageWidth, imageHeight);
	generate_sine_data(data, imageWidth, 0.1, 1);
	data_to_ol_image(data, aux, 255,0,0,ALPHA, -1, 1, imageWidth, imageHeight);
	for(uint32_t i = 0; i < imageWidth; i++){
		printf("%u,", aux[i]);
	}
	printf("\n\n");
	for(uint32_t i = 0; i < imageWidth; i++){
			printf("%u,", image[i]);
		}
	printf("\n\n");
	blend_images_DMA2D_interrupt(image, aux, image, imageWidth, imageHeight);

	print_image_pixels_alpha(image, imageWidth, imageHeight);

	//free(aux);
	free(image);
	free(data);

	return 0;
}


#if JPEG_ACCELERATOR == ON


int test_ol_blend_jpeg_combo(uint32_t imageHeight, uint32_t imageWidth, uint32_t numberStackedMeasurements, uint8_t quality){
	int result;
	double * data = (double *) malloc(sizeof(double) * 300);
	uint8_t * image =  malloc(sizeof(uint8_t) * 300 * 200 * 4);
	uint8_t * Jpeg = malloc(sizeof(uint8_t) * 300 * 200 * 4);
	uint8_t * aux =  (uint8_t *) 0x30000000;
	JPEG_ConfTypeDef * jpegConf = new_jpeg_conf(imageHeight, imageWidth, quality);
	uint32_t colorIterator = 0;


	memset(image, 0, sizeof(uint8_t) * imageWidth * imageHeight * 4);
	memset(Jpeg, 0, sizeof(uint8_t) * imageWidth * imageHeight * 4);
	generate_sine_data(data, imageWidth, 0.1, 0);

	data_to_ol_image(data, image, 255,0,0,ALPHA, -1, 1, imageWidth, imageHeight);
	colorIterator++;

	printf("Values:\n");
	for(uint32_t i = 0; i < imageWidth; i++){
		printf("%lf,",data[i]);
	}
	printf("\n\n");

	for(uint32_t i = 1; i < numberStackedMeasurements; i++){
		//Each cycle CPU converts one R, one G, one B in turn. Then blender will right-shift it. If > 21 stacks, colors will get lost.
		generate_sine_data(data, imageWidth, 0.1, i);
		//double_data_to_image(data, image, 255,0,0,ALPHA, -1, 1, imageWidth, imageHeight);
		memset(aux, 0, sizeof(uint8_t) * imageHeight * imageWidth * 4);
		switch(colorIterator % 3){
		case 0:
			data_to_ol_image(data, aux, 128,0,0,ALPHA, -1, 1, imageWidth, imageHeight);
			blend_images(image, aux, image, imageWidth * imageHeight, FOREGROUND_ALPHA, BACKGROUND_ALPHA, ALPHA, true);

			break;
		case 1:
			data_to_ol_image(data, aux, 0,128,0,ALPHA, -1, 1, imageWidth, imageHeight);
			blend_images(image, aux, image, imageWidth * imageHeight, FOREGROUND_ALPHA, BACKGROUND_ALPHA, ALPHA, true);
			break;
		case 2:
			data_to_ol_image(data, aux, 0,0,128,ALPHA, -1, 1, imageWidth, imageHeight);
			blend_images(image, aux, image, imageWidth * imageHeight, FOREGROUND_ALPHA, BACKGROUND_ALPHA, ALPHA, true);
			break;

		default:
			printf("WARNING: Check test images module, this print should not have been reached\n");
			exit(-1);
		}
		colorIterator++;

	}
	printf("\n\nOriginal image:\n");
	print_image_pixels_alpha(image, imageWidth, imageHeight);

	for(uint32_t i = 0; i < imageHeight * imageWidth * 4; i++){
		if(Jpeg[i] != 0){
			printf("JPEG array not zeroed out correctly!\n");
		}
	}
	result = encode_JPEG_accelerator_MDMA(image, sizeof(uint8_t) * imageWidth * imageHeight * 4 , Jpeg, sizeof(uint8_t) * imageWidth * imageHeight * 4, jpegConf);
	if(result != 0){
		return -1;
	}
	wait_JPEG_MDMA();
	printf("\n\n\n\nJPEG output: \n");
	print_image_pixels_alpha(Jpeg, imageWidth, imageHeight);

	result = decode_JPEG_accelerator_MDMA(Jpeg, sizeof(uint8_t) * imageWidth * imageHeight * 4 , aux, sizeof(uint8_t) * imageWidth * imageHeight * 4, jpegConf);
	if(result != 0){
		return -1;
	}
	wait_JPEG_MDMA();

	printf("\n\n\n\nJPEG decoded output: \n");
	print_image_pixels_alpha(aux, imageWidth, imageHeight);

	printf("\n\nJpeg size for %lu x %lu: %lu\n", imageHeight, imageWidth, getJPEGSize(Jpeg,  imageWidth * imageHeight * 4));
	free(jpegConf);

	return 0;
}

double test_ol_decode_algorithm_accelerators(uint32_t imageWidth, uint32_t imageHeight, uint32_t numOverlappedLines, uint16_t quality){
	uint32_t * decodedHeightArray = NULL;
	double sumMAE = 0;
	double *values;
	uint8_t * color;
	uint32_t * heights;
	int result;
	JPEG_ConfTypeDef * jpegConf;
	MultilineImage * multilineImage =  generate_ol_image(imageHeight, imageWidth, numOverlappedLines, true);
	uint8_t * JPEGImage = malloc(sizeof(uint8_t) * imageHeight * imageWidth * 4);

	memset(JPEGImage, 0, sizeof(uint8_t) * imageHeight * imageWidth * 4);
//	printf("Original Image:\n");
//
//	print_image_pixels_alpha(multilineImage->image, imageWidth, imageHeight);
//
//	printf("\n\n\n\n");

	jpegConf = new_jpeg_conf(imageHeight, imageWidth, quality);

	result = encode_JPEG_accelerator(multilineImage->image, sizeof(uint8_t) * imageHeight * imageWidth * 4 , JPEGImage, sizeof(uint8_t) * imageHeight * imageWidth * 4, jpegConf);
	if(result != 0){
		printf("Error ocurred while enconding JPEG image\n");
		return -1;
	}

	memset(multilineImage->image, 0, sizeof(uint8_t) * imageHeight * imageWidth * 4);

	result = decode_JPEG_accelerator(JPEGImage, sizeof(uint8_t) * imageHeight * imageWidth * 4, multilineImage->image, sizeof(uint8_t) * imageHeight * imageWidth * 4, jpegConf);
	if(result != 0){
		printf("Error ocurred while enconding JPEG image\n");
		return -1;
	}

//	printf("Post-JPEG image:\n");
//	print_image_pixels_alpha(multilineImage->image, imageWidth , imageHeight);
	free(jpegConf);
	free(JPEGImage);

	for(uint32_t i = 0; i < numOverlappedLines; i++){
		color = multilineImage->colors[i];
		decodedHeightArray = image_to_quantized_values_array_ol_image(multilineImage->image, ALPHA, imageWidth, imageHeight, color[0], color[1], color[2]);
//		printf("Decoded height array:\n");
//		for(uint32_t i = 0; i < imageWidth; i++){
//			printf("%lu,", decodedHeightArray[i]);
//		}
//		printf("\n");
		values = get_sine_values_by_line_index(imageWidth,i);
		heights = data_to_ol_heights(values, imageHeight, imageWidth, 1, -1);

//		printf("Reference height array:\n");
//		for(uint32_t i = 0; i < imageWidth; i++){
//			printf("%lu,", heights[i]);
//		}
//		printf("\n");

		sumMAE += mean_absolute_error((int32_t *) decodedHeightArray, (int32_t *) heights, (int32_t) imageWidth);

		free(heights);
		free(values);
		free(decodedHeightArray);
		//free(color);
	}

//	printf("Average error: %lf\n", (double) sumMAE / numOverlappedLines);
	destroyMultilineImage(multilineImage);

	return (double) sumMAE / numOverlappedLines;
}

int test_ol_JPEG_accelerator_CPU(uint32_t imageWidth, uint32_t imageHeight, uint32_t numOverlappedLines, uint32_t quality){
	UNUSED(numOverlappedLines);
//	double * sineValues = malloc(sizeof(uint8_t) * imageWidth);
//	memset(sineValues, 0, sizeof(uint8_t) * imageWidth);
//	uint8_t * image = (uint8_t *) malloc(sizeof(uint8_t) * imageHeight * imageWidth * 4);
//	memset(image,0,sizeof(uint8_t) * imageHeight * imageWidth * 4);
	uint8_t * JPEGImage = (uint8_t *) malloc(sizeof(uint8_t) * imageHeight * imageWidth * 4); // Height x Width x ARGB
	memset(JPEGImage, 0, sizeof(uint8_t) * imageHeight * imageWidth * 4);
	MultilineImage * multilineImage =  generate_ol_image(imageHeight, imageWidth, numOverlappedLines, true);
	int result;
//	uint8_t color[3] = {255,0,0};
	JPEG_ConfTypeDef * jpegConf = new_jpeg_conf(imageHeight, imageWidth, quality);

//	generate_sine_data(sineValues, imageWidth, 0.1, 0);
//    double_data_to_image(sineValues, image, 255,0,0,ALPHA, -1, 1, imageWidth, imageHeight);
	//solid_color(image, true, imageWidth, imageHeight,color);
//	printf("Values:\n");
//	for(uint32_t i = 0; i < imageWidth; i++){
//		printf("%lf,",sineValues[i]);
//	}
//	printf("\n\n");
	printf("Original image: \n");
	print_image_pixels_alpha(multilineImage->image, imageWidth, imageHeight);
	printf("\n\n\n");

	result = encode_JPEG_accelerator(multilineImage->image, sizeof(uint8_t) * imageHeight * imageWidth * 4 , JPEGImage, sizeof(uint8_t) * imageHeight * imageWidth * 4, jpegConf);
	if(result != 0){
		free(jpegConf);
		free(JPEGImage);
		destroyMultilineImage(multilineImage);
//		free(JPEGImage);
//		free(image);
		//free(sineValues);
		return -1;
	}

	memset(multilineImage->image, 0, sizeof(uint8_t) * imageHeight * imageWidth * 4);
	result = decode_JPEG_accelerator(JPEGImage, sizeof(uint8_t) * imageHeight * imageWidth * 4, multilineImage->image, sizeof(uint8_t) * imageHeight * imageWidth * 4, jpegConf);
	if(result != 0){
		free(jpegConf);
		free(JPEGImage);
		destroyMultilineImage(multilineImage);
//		free(image);
		//free(sineValues);
		return -1;
	}

	printf("\n\n\n Printing image after encode/decode JPEG process.\n");
	print_image_pixels_alpha(multilineImage->image, imageWidth, imageHeight);


	free(jpegConf);
	free(JPEGImage);
	destroyMultilineImage(multilineImage);

//	free(image);
	//free(sineValues);
	return 0;

}


int test_ol_blending_single_pixels_accelerator(){
	HAL_StatusTypeDef hal_status = HAL_OK;
	uint8_t * pixel1 = (uint8_t *) malloc(sizeof(uint8_t) * 1 * 1 * 4); // Height x Width x ARGB
	uint8_t * pixel2 = (uint8_t *) malloc(sizeof(uint8_t) * 1 * 1 * 4); // Height x Width x ARGB
	pixel1[0] = 1; //B
	pixel1[1] = 1; //G
	pixel1[2] = 1; //R
	pixel1[3] = 1; //A
	pixel2[0] = 2; //B
	pixel2[1] = 2; //G
	pixel2[2] = 2; //R
	pixel2[3] = 1; //A

	hal_status = HAL_DMA2D_BlendingStart_IT(&hdma2d,
		(uint32_t)pixel1,   /* Foreground image */
		(uint32_t)pixel2,   /* Background image */
		(uint32_t)pixel2,   /* Destination address */
		 1 ,  /* width in pixels   */
		 1);  /* height in pixels   */
	//OnError_Handler(hal_status != HAL_OK);
	if(hal_status != HAL_OK){
		printf("Hal error, status: %d\n", hal_status);
		return -1;
	}
	//sum_images(newImage, currentImage, IMAGE_SIZE * IMAGE_SIZE * 3);
	hal_status = HAL_DMA2D_PollForTransfer(&hdma2d, 10);
	if(hal_status != HAL_OK){
		printf("Hal error, status: %d\n", hal_status);
		return -1;
	}

	printf("Result of pixel blending - R:%d G:%d B:%d A:%d\n", pixel2[2], pixel2[1], pixel2[0], pixel2[3]);

	return 0;

}



/*
 * @brief Encodes a sine wave image using the JPEG codec (i.e., accelerator) and then decodes it. Prints the original image as well as the decoded image post-JPEG compression.
 *
 * Creates a sineWave image with multiple lines. Then encodes and decodes it using the JPEG accelerator via MDMA.
 * Both the original and post-JPEG decoding images are printed to stdout in the form of an RGBA array.
 * Notes: Image in must be multiple of inXfrSize (32 bytes)
 * 		  Image out must be multiple of outXfrSize (4 bytes)
 * 		  It must also be a multiple of MAX block size, or smaller than it (confirm if true)
 *
 */
int test_ol_JPEG_accelerator_MDMA(uint32_t imageWidth, uint32_t imageHeight, uint32_t quality){
	double * sineValues = (double *) malloc(sizeof(double) * imageWidth * imageHeight);
	uint8_t * image = (uint8_t *) malloc(sizeof(uint8_t) * imageWidth * imageHeight * 4);
	uint8_t * JPEGImage = (uint8_t *) malloc(sizeof(uint8_t) * imageWidth * imageHeight * 4); // Height x Width x ARGB
	int result;
	JPEG_ConfTypeDef * jpegConf = new_jpeg_conf(imageHeight, imageWidth, quality);

	memset(image, 0, sizeof(uint8_t) * imageWidth * imageHeight * 4);
	memset(JPEGImage, 0, sizeof(uint8_t) * imageWidth * imageHeight * 4);
	generate_sine_data(sineValues, imageWidth * imageHeight, 0.1, 0);
	data_to_ol_image(sineValues, image, 255,0,0,ALPHA, -1, 1, imageWidth, imageHeight);
	printf("Values:\n");
	for(uint32_t i = 0; i < imageWidth * imageHeight; i++){
		printf("%lf,",sineValues[i]);
	}
	printf("\n\n");
	print_image_pixels_alpha(image, imageWidth, imageHeight);

	result = encode_JPEG_accelerator_MDMA(image, sizeof(uint8_t) * imageWidth * imageHeight * 4 , JPEGImage, sizeof(uint8_t) * imageWidth * imageHeight * 4, jpegConf);
	if(result != 0){
		free(jpegConf);
		free(JPEGImage);
		free(image);
		free(sineValues);
		return -1;
	}
	//wait_JPEG_MDMA();
	printf("\n\n\n\nJPEG output: \n");
	print_image_pixels(JPEGImage, imageWidth, imageHeight);

	memset(image, 0, sizeof(uint8_t) * imageWidth * imageHeight * 4);

	result = decode_JPEG_accelerator_MDMA(JPEGImage, sizeof(uint8_t) * imageWidth * imageHeight * 4, image, sizeof(uint8_t) * imageWidth * imageHeight * 4, jpegConf);
	if(result != 0){
		free(jpegConf);
		free(JPEGImage);
		free(image);
		free(sineValues);
		return -1;
	}
	//wait_JPEG_MDMA();
	printf("\n\n\n Printing image after encode/decode JPEG process.\n");
	print_image_pixels_alpha(image, imageWidth, imageHeight);
	printf("\n\nJpeg size: %lu\n", getJPEGSize(JPEGImage,  imageWidth * imageHeight * 4));
	free(jpegConf);
	free(JPEGImage);
	free(image);
	free(sineValues);

	return 0;

}

//TODO: some functions ask for width first, others ask for height (in arguments). Uniformize and find a way to distinguish between them using typedef
//TODO: not sure this order of free's is good
int test_decode_algorithm_jpeglib(uint32_t imageWidth, uint32_t imageHeight, uint32_t numOverlappedLines){
	uint8_t * JPEGImage = NULL; // Height x Width x ARGB
	uint32_t * decodedHeightArray = NULL;
	double mae = 0;
	double *valuesLine0;
	uint8_t * image, color0[3];
	uint32_t * heights;

	valuesLine0 = get_sine_values_by_line_index(imageWidth, 0);
	MultilineImage * multiLineImage =  generate_ol_image(imageHeight, imageWidth, numOverlappedLines, false);

	color0[0] = multiLineImage->colors[0][0]; color0[1] = multiLineImage->colors[0][1]; color0[2] = multiLineImage->colors[0][2];
	printf("Original Image:\n");

	print_image_pixels(multiLineImage->image, imageWidth, imageHeight);

	printf("\n\n\n\n");

	JPEGImage = encode_JPEG_from_memory_to_memory(multiLineImage->image, imageWidth , imageHeight, 90);

	destroyMultilineImage(multiLineImage);

	image = (uint8_t *) decode_JPEG_from_memory_to_memory(JPEGImage, imageHeight , imageWidth);

	printf("Post-JPEG image:\n");
	print_image_pixels(image, imageWidth , imageHeight);
	free(JPEGImage);

	decodedHeightArray = image_to_quantized_values_array_ol_image(image, NO_ALPHA, imageWidth, imageHeight, color0[0], color0[1], color0[2]);
	printf("Decoded array:\n");
	for(uint32_t i = 0; i < imageWidth; i++){
		printf("%ld,",decodedHeightArray[i]);
	}
	printf("\n\n");
	heights = data_to_ol_heights(valuesLine0, imageHeight, imageWidth, 1, -1);
	printf("Reference array:\n");
	for(uint32_t i = 0; i < imageWidth; i++){
		printf("%ld,",heights[i]);
	}
	printf("\n\n");
	mae = mean_absolute_error((int32_t *) decodedHeightArray, (int32_t *) heights, (int32_t) imageWidth);

	printf("Mean absolute error for height after decode is: %lf\n", mae);


	return 0;
}

void _print_image_original_and_jpeg_accelerator(uint32_t imageWidth, uint32_t imageHeight, uint32_t numOverlappedLines, uint16_t quality){
	int result;
	JPEG_ConfTypeDef * jpegConf;
	MultilineImage * multilineImage =  generate_ol_image(imageHeight, imageWidth, numOverlappedLines, true);
	uint8_t * JPEGImage = malloc(sizeof(uint8_t) * imageHeight * imageWidth * 4);

	memset(JPEGImage, 0, sizeof(uint8_t) * imageHeight * imageWidth * 4);
	//printf("Original Image:\n");

	print_image_pixels_alpha(multilineImage->image, imageWidth, imageHeight);
	printf(",\n");
	//printf("\n\n\n\n");

	jpegConf = new_jpeg_conf(imageHeight, imageWidth, quality);

	result = encode_JPEG_accelerator(multilineImage->image, sizeof(uint8_t) * imageHeight * imageWidth * 4 , JPEGImage, sizeof(uint8_t) * imageHeight * imageWidth * 4, jpegConf);
	if(result != 0){
		printf("Error occurred while encoding JPEG image\n");
		return;
	}

	memset(multilineImage->image, 0, sizeof(uint8_t) * imageHeight * imageWidth * 4);

	result = decode_JPEG_accelerator(JPEGImage, sizeof(uint8_t) * imageHeight * imageWidth * 4, multilineImage->image, sizeof(uint8_t) * imageHeight * imageWidth * 4, jpegConf);
	if(result != 0){
		printf("Error occurred while encoding JPEG image\n");
		return;
	}

	//printf("Post-JPEG image:\n");
	print_image_pixels_alpha(multilineImage->image, imageWidth , imageHeight);

	free(jpegConf);
	free(JPEGImage);

	destroyMultilineImage(multilineImage);

}

//Prints to stdout an array of images compatible with python, to be offloaded to a file. Array has shape: [[[height, quality,#lines], [originalImage], [jpegImage]], ...]

void print_example_ol_images_accelerator(){
    uint32_t quality[] = {5, 50, 75, 90, 100};//{5, 10, 25, 50, 75,80,90,95,100};
    uint32_t imageHeight[] = {16, 64, 256};//{16,32,64,128, 256};
    uint32_t imageWidth[] = {64,64,64};//{64,64,64,64,64};
    uint32_t overlappedLines[] = {1,5,10,20};//{1,2,5,10,20};


	printf("images = [");
	printf("\n");
	for(uint32_t q = 0; q < (sizeof(quality) / sizeof(uint32_t)); q++){
		for(uint32_t ol = 0; ol < (sizeof(overlappedLines) / sizeof(uint32_t)); ol++){
			for(uint32_t size = 0; size < (sizeof(imageHeight) / sizeof(uint32_t)); size++){
				printf("[[%lu,%lu,%lu],\n", imageHeight[size],quality[q],overlappedLines[ol]);
				_print_image_original_and_jpeg_accelerator(imageWidth[size], imageHeight[size], overlappedLines[ol], (uint16_t) quality[q]);
				if(((size + 1)  < (sizeof(imageHeight) / sizeof(uint32_t))) || ((ol + 1) < (sizeof(overlappedLines) / sizeof(uint32_t))) || ((q + 1) < (sizeof(quality) / sizeof(uint32_t)))){
					printf("],\n");
				} else {
					printf("]");
				}
			}
			printf("\n");
		}

	}
	printf("]\n");


}
#endif
void _print_ol_image_original_and_libjpeg(uint32_t imageWidth, uint32_t imageHeight, uint32_t numOverlappedLines, uint16_t quality){
	uint8_t * JPEGImage = NULL; // Height x Width x ARGB
	uint8_t * image;
	MultilineImage * multiLineImage =  generate_ol_image(imageHeight, imageWidth, numOverlappedLines, false);


	//printf("Original Image:\n");

	print_image_pixels(multiLineImage->image, imageWidth, imageHeight);

	//printf("\n\n\n\n");
	printf(",\n");

	JPEGImage = encode_JPEG_from_memory_to_memory(multiLineImage->image, imageWidth , imageHeight, quality);

	destroyMultilineImage(multiLineImage);

	image = (uint8_t *) decode_JPEG_from_memory_to_memory(JPEGImage, imageHeight , imageWidth);

//	printf("Post-JPEG image:\n");
	print_image_pixels(image, imageWidth , imageHeight);
	free(JPEGImage);

}

void print_example_ol_images_libjpeg(){
	uint32_t quality[] = {5, 50, 75, 90, 100};//{5, 10, 25, 50, 75,80,90,95,100};
	uint32_t imageHeight[] = {16,32, 64};//{16,32,64,128, 256};
	uint32_t imageWidth[] = {64,64,64};//{64,64,64,64,64};
	uint32_t overlappedLines[] = {1,5,10,20};//{1,2,5,10,20};

	printf("images = [");
	printf("\n");
	for(uint32_t q = 0; q < (sizeof(quality) / sizeof(uint32_t)); q++){
		for(uint32_t ol = 0; ol < (sizeof(overlappedLines) / sizeof(uint32_t)); ol++){
			for(uint32_t size = 0; size < (sizeof(imageHeight) / sizeof(uint32_t)); size++){
				printf("[[%lu,%lu,%lu],\n", imageHeight[size],quality[q],overlappedLines[ol]);
				_print_ol_image_original_and_libjpeg(imageWidth[size], imageHeight[size], overlappedLines[ol], (uint16_t) quality[q]);
				if(((size + 1)  < (sizeof(imageHeight) / sizeof(uint32_t))) || ((ol + 1) < (sizeof(overlappedLines) / sizeof(uint32_t))) || ((q + 1) < (sizeof(quality) / sizeof(uint32_t)))){
					printf("],\n");
				} else {
					printf("]");
				}
			}
			printf("\n");
		}

	}
	printf("]\n");
}
