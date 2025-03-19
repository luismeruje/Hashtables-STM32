/*
 * data_generator.c
 *
 *  Created on: Apr 19, 2024
 *      Author: luisferreira
 */
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "utils.h"
#include "data_generator.h"
#include "cielab_colors.h"
#include "num_utils.h"


int generate_sine_data_outliers(double *dest, uint32_t numValues, double interval, double start, double outlierPercentage){
	srand(OUTLIER_SEED_VALUE); //In case we need to replicate values
	double x = start;

	for(uint32_t i = 0; i < numValues; i++, x+= interval){
		if((double) rand() / RAND_MAX <= outlierPercentage){
			dest[i] = randfrom(-1, 1);
		}
		else {
			dest[i] =  sin(x);
		}

	}

	return 0;
}

int generate_sine_data(double *dest, uint32_t numValues, double interval, double start){
	return generate_sine_data_outliers(dest, numValues, interval, start, 0);
}


void generate_solid_color(uint8_t * image, uint8_t alphaFlag, uint32_t imageWidth, uint32_t imageHeight, uint8_t *color){
	uint8_t numParams = 3;
	if(alphaFlag){
		numParams = 4;
	}
	for(uint32_t h = 0; h < imageHeight; h++){
		for(uint32_t w = 0; w < imageWidth; w++){
			image[h * imageWidth * numParams + w * numParams] = color[0]; //R
			image[h * imageWidth * numParams + w * numParams + 1] = color[1]; //G
			image[h * imageWidth * numParams + w * numParams + 2] = color[2]; //B
			if(alphaFlag){
				image[h * imageWidth * numParams + w * numParams + 3] = FULL_ALPHA; //A
			}
		}
	}
}






