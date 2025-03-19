/*
 * ol_image.c
 *
 *  Created on: Aug 6, 2024
 *      Author: luisferreira
 */

#include <stdlib.h>
#include <utils.h>
#include "ol_image.h"
#include "settings.h"
#include "num_utils.h"
#include "vectors.h"
#include "cielab_colors.h"
/*
 * Converts a data array to the heights those data points would occupy in an OL image.
 *
 */
uint32_t * data_to_ol_heights(double * data, uint32_t imageHeight, uint32_t imageWidth, double dataMax, double dataMin){
	uint32_t * heights = malloc(sizeof(uint32_t) * imageWidth);
	double valueMax = imageHeight - 1, valueMin = 0;
	for(uint32_t i = 0; i < imageWidth; i++){
		heights[i] = scale_value(data[i], valueMin, valueMax, dataMin, dataMax);
	}
	return heights;
}


int data_to_ol_image(double* values, uint8_t * image, uint8_t r, uint8_t g, uint8_t b, uint8_t alphaFlag, int32_t dataMin, int32_t dataMax, uint32_t imageWidth, uint32_t imageHeight){
	double valueMax = imageHeight - 1, valueMin = 0;
    volatile uint32_t height = 0;
    uint8_t numParams = 3;

    if(alphaFlag == ALPHA){
        numParams = 4;
    }
    //Each row is of size imageWidth, there are imageHeight rows
    //Select row: height*imageWidth +
    for(uint32_t i = 0; i < imageWidth; i++){
		height = (uint32_t) ((((double)values[i] - dataMin ) / (dataMax - dataMin)) * (valueMax - valueMin) + valueMin);
		image[height * imageWidth * numParams + i * numParams] = r; //R
        image[height * imageWidth * numParams + i * numParams + 1] = g; //G
        image[height * imageWidth * numParams + i * numParams + 2] = b; //B
        if(alphaFlag == ALPHA){
            image[height * imageWidth * numParams + i * numParams + 3] = OL_ENCODE_ALPHA; //A
        }
	}
    return 0;
}


// Was in file "data_decoder.c"


//From: Online Full Body Human Motion Tracking Based on Dense Volumetric 3D Reconstructions from Multi Camera Setups
//Splits color difference into luminance and chromatic differences. Weight of each must be carefully chosen.
double _chroma_brightness_distance(uint8_t * color1, uint8_t * color2){
	double chromaDistance = 0, brightnessDistance = 0;
    double *normal = NULL, *vectorDifference = NULL, *crossProduct, *scalarProduct, *r, innerProduct;
    double doubleColor1[3] = {color1[0], color1[1], color1[2]};


    normal = normalize_vector(color1);
	vectorDifference = vector_difference(color2, color1);
	crossProduct = cross_product(vectorDifference, normal);
	chromaDistance = vector_magnitude(crossProduct[0], crossProduct[1], crossProduct[2]);

	innerProduct = inner_product(vectorDifference, normal);
	scalarProduct = scalar_product(innerProduct, normal);
	r = vector_sum(scalarProduct, doubleColor1);
	brightnessDistance = vector_magnitude(doubleColor1[0], doubleColor1[1], doubleColor1[2]) / vector_magnitude(r[0],r[1],r[2]);


	free(r);
	free(scalarProduct);
	free(crossProduct);
	free(vectorDifference);
	free(normal);
    //If zero, euclidean distance suffices

    chromaDistance = euclidean_distance_3d(color1, color2);

    //printf("RGB Distance: %lf\n", rgbDistance);


	return (chromaDistance * CHROMA_WEIGHT) + (brightnessDistance * BRIGHTNESS_WEIGHT);
}


uint32_t _decode_timestamp_value_height(uint8_t * image, uint8_t alphaFlag, uint32_t timeStamp, uint32_t imageWidth, uint32_t imageHeight, uint8_t target_r, uint8_t target_g, uint8_t target_b){
	//TODO: Search in adjacent columns as well. Set a threshold for color distance and max num adjacent positions to search.
	uint8_t numComponents = 3;
	double minDistance = MAX_DOUBLE;
	double distance =  MAX_DOUBLE;
	uint8_t targetColor[3] = {target_r, target_g, target_b};
	uint32_t valueHeight = 0;
	uint8_t * pixel = NULL;
	if (alphaFlag == 1){
		numComponents = 4;
	}

	//TODO: vectorize
	for(uint32_t i = 0; i < imageHeight; i++){
		pixel = &image[i * imageWidth * numComponents + timeStamp * numComponents];
#if DISTANCE_EQUATION == 1
		distance = _chroma_brightness_distance(targetColor, pixel);
#elif DISTANCE_EQUATION == 2
        ColorLab c1, c2;
        rgb_to_lab(targetColor[0], targetColor[1], targetColor[2], &c1);
        rgb_to_lab(pixel[0],pixel[1],pixel[2],&c2);
        distance = deltaE2000_distance(c1,c2);
#else
        distance = (uint32_t) euclidean_distance_3d(targetColor, pixel);
#endif
		if (distance < minDistance){
			minDistance = distance;
			valueHeight = i;
		}
	}

	return valueHeight;
}

// Quantized values = height of pixel. For each column, i.e. timestamp, will look for pixel with most similar color and return its height.
uint32_t * image_to_quantized_values_array_ol_image(uint8_t * image, uint8_t alphaFlag, uint32_t imageWidth, uint32_t imageHeight, uint8_t target_r, uint8_t target_g, uint8_t target_b){
	uint32_t * heightArray = malloc(sizeof(uint32_t) * imageWidth);
	for(uint32_t i = 0; i < imageWidth; i++){
		heightArray[i] = _decode_timestamp_value_height(image, alphaFlag, i, imageWidth, imageHeight, target_r, target_g, target_b);
	}
	return heightArray;
}

// ==== Taken from old image_manipulator.c =====

//Transform values from an array with range dataMin to dataMax (negative or positive values) into a positive 0 to imageHeight range. Then projects those values into a 2D image.
//int double_data_to_image(double* values, uint8_t * image, uint8_t r, uint8_t g, uint8_t b, uint8_t alphaFlag, int32_t dataMin, int32_t dataMax, uint32_t imageWidth, uint32_t imageHeight){
//	double valueMax = imageHeight - 1, valueMin = 0;
//    volatile uint32_t height = 0;
//    uint8_t numParams = 3;
//
//    if(alphaFlag == ALPHA){
//        numParams = 4;
//    }
//    //Each row is of size imageWidth, there are imageHeight rows
//    //Select row: height*imageWidth +
//    for(uint32_t i = 0; i < imageWidth; i++){
//		height = (uint32_t) ((((double)values[i] - dataMin ) / (dataMax - dataMin)) * (valueMax - valueMin) + valueMin);
//		image[height * imageWidth * numParams + i * numParams] = b; //B
//        image[height * imageWidth * numParams + i * numParams + 1] = g; //G
//        image[height * imageWidth * numParams + i * numParams + 2] = r; //R
//        if(alphaFlag == ALPHA){
//            image[height * imageWidth * numParams + i * numParams + 3] = 255; //A
//        }
//	}
//    return 0;
//}

//uint32_t _square(uint32_t integer){
//	return integer * integer;
//}
//
//uint32_t _colorDistance(uint32_t colorA, uint32_t colorB){
//	uint8_t * colorComponentsA = (uint8_t *)&colorA;
//	uint8_t * colorComponentsB = (uint8_t *)&colorB;
//
//	//Mean square root (MQS), but without the root to save computation cost. Named color distance.
//	uint32_t distance = _square(colorComponentsA[0] - colorComponentsB[0]) + _square(colorComponentsA[1] - colorComponentsB[1]) + _square(colorComponentsA[2] - colorComponentsB[2]);
//	return distance;
//}

//TODO: rotate images 90º again, to improve data locality
//uint32_t _decode_image_value_height(uint8_t * image, uint8_t alphaFlag, uint32_t timeStamp, uint32_t imageWidth, uint32_t imageHeight){
//	//TODO: For now assuming we are looking for 255 on Blue, will test mixed colors later
//	//TODO: Search in adjacent columns as well. Set a thresholds for color distance and max num adjacent positions to search.
//	uint8_t numComponents = 3;
//	uint32_t minDistance = MAX_COLOR_DISTANCE_PLUS_1;
//	uint32_t distance =  MAX_COLOR_DISTANCE_PLUS_1;
//	uint32_t targetColor = FULL_BLUE;
//	uint32_t valueHeight = 0;
//	uint8_t * pixel = NULL;
//	if (alphaFlag == ALPHA){
//		numComponents = 4;
//	}
//
//	//TODO: vectorize
//	for(uint32_t i = 0; i < imageHeight; i++){
//		pixel = &image[i * imageWidth * numComponents + timeStamp * numComponents];
//		//Sorry for the non-intuitive pointer manipulation
//		distance = _colorDistance(targetColor, *((uint32_t *) pixel));
//		if (distance < minDistance){
//			minDistance = distance;
//			valueHeight = i;
//		}
//	}
//
//	return valueHeight;
//}

//uint32_t * data_to_heights(double * data, uint32_t imageHeight, uint32_t imageWidth, double dataMax, double dataMin){
//	uint32_t * heights = malloc(sizeof(uint32_t) * imageWidth);
//	double valueMax = imageHeight - 1, valueMin = 0;
//	for(uint32_t i = 0; i < imageWidth; i++){
//		heights[i] = (uint32_t) ((((double)data[i] - dataMin ) / (dataMax - dataMin)) * (valueMax - valueMin) + valueMin);
//	}
//	return heights;
//
//}
