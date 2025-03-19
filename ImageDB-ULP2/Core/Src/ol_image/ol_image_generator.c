/*
 * ol_image_generator.c
 *
 *  Created on: Aug 6, 2024
 *      Author: luisferreira
 */

#include <ol_image_blender.h>
#include <stdlib.h>
#include <string.h>
#include "ol_image.h"
#include "ol_image_generator.h"
#include "data_generator.h"
#include "cielab_colors.h"
#include <stdbool.h>
#include "settings.h"



//When generating overlapped line (ol) image, each sine wave is generated with a specific difference in phase, and with a specific sequence. This function provides the sine wave values associated to a given index
//WARNING: this function is only for images generated without outliers! TODO: add mechanism to prevent/warn when calling this function and images were generated with outliers
double * get_sine_values_by_line_index(uint32_t imageWidth, uint32_t index){
	//UNUSED(imageHeight);
	double * data = (double *) malloc(sizeof(double) * imageWidth);
	generate_sine_data(data, imageWidth, SINE_WAVE_STEP, SINE_FIRST_VALUE_REFERENCE + index);
	return data;
}

//Generates blended image from sine waves. Generated waves start at different points, according to the values creation function. All done with software (no accelerators).
MultilineImage * generate_ol_image(uint32_t imageHeight, uint32_t imageWidth, uint32_t numberStackedMeasurements, uint8_t imageHasAlpha){
	double * data = NULL;
	uint8_t numComponents = imageHasAlpha ? 4 : 3;
	MultilineImage * multilineImage = malloc(sizeof(MultilineImage));
	uint8_t * image =  malloc(sizeof(uint8_t) * imageWidth * imageHeight * numComponents);
	uint8_t * aux =  malloc(sizeof(uint8_t) * imageWidth * imageHeight * numComponents);
	uint8_t * color;

	multilineImage->image = image;
	multilineImage->colors = malloc(sizeof(uint8_t *) * numberStackedMeasurements);
	for(uint32_t i = 0; i < numberStackedMeasurements; i++){
		multilineImage->colors[i] = malloc(sizeof(uint8_t) * 3);
		memset(multilineImage->colors[i], 0, sizeof(uint8_t) * 3);
	}
	multilineImage->numLines = numberStackedMeasurements;

    memset(image, 0, sizeof(uint8_t) * imageWidth * imageHeight * numComponents);
	memset(aux, 0, sizeof(uint8_t) * imageWidth * imageHeight * numComponents);
	//We do not use colorByIndex in this case because the blender always shifts existing colors to the right by 1 in each operation
#if COLOR_TYPE == 1
	find_max_dist_colors_CIE(numberStackedMeasurements, multilineImage->colors, NUM_ITERATIONS_CIE);
#endif
    for(uint32_t i = 0; i < numberStackedMeasurements; i++){
		//color = get_color_by_index(i);
		data = get_sine_values_by_line_index(imageWidth, i);
		#if COLOR_TYPE == 0
		switch(i % 3){
			case 0:
				data_to_ol_image(data, aux, 128,0,0,imageHasAlpha, -1, 1, imageWidth, imageHeight);
				multilineImage->colors[i][0] = 128;
				break;
			case 1:
				data_to_ol_image(data, aux, 0,128,0,imageHasAlpha, -1, 1, imageWidth, imageHeight);
				multilineImage->colors[i][1] = 128;
				break;
			case 2:
				data_to_ol_image(data,aux, 0,0,128, imageHasAlpha, -1, 1, imageWidth, imageHeight);
				multilineImage->colors[i][2] = 128;

				#if BLENDING == 0
				blend_images(aux, image, image, imageWidth * imageHeight, IMAGE_BLEND_ALPHA_FG, IMAGE_BLEND_ALPHA_BG, imageHasAlpha, true);
				#else
				blend_images(aux, image, image, imageWidth * imageHeight, IMAGE_BLEND_ALPHA_FG, IMAGE_BLEND_ALPHA_BG, imageHasAlpha, false);
				#endif

				//Shift colors
				for(uint32_t c = 0; c < i + 1; c++){
					multilineImage->colors[c][0] = multilineImage->colors[c][0] >> 1;
					multilineImage->colors[c][1] = multilineImage->colors[c][1] >> 1;
					multilineImage->colors[c][2] = multilineImage->colors[c][2] >> 1;
				}
				memset(aux, 0, sizeof(uint8_t) * imageWidth * imageHeight * numComponents);
				break;
			default:
				printf("WARNING: Check data generator module, this print should not have been reached\n");
				exit(-1);
		}
		#else
		color = multilineImage->colors[i];
		data_to_ol_image(data, aux, color[0],color[1],color[2],imageHasAlpha, -1, 1, imageWidth, imageHeight);
		if(imageHasAlpha){
		#if BLENDING == 0
			blend_images(aux, image, image, imageWidth * imageHeight, IMAGE_BLEND_ALPHA_FG, IMAGE_BLEND_ALPHA_BG, imageHasAlpha, true);
			//If image has alpha, use it to do blending.
		#else
			blend_images(aux, image, image, imageWidth * imageHeight, 0, 0, imageHasAlpha, false);
		#endif
		}
		memset(aux, 0, sizeof(uint8_t) * imageWidth * imageHeight * numComponents);
		//free(color);
		#endif

		free(data);

		//free(color);
	}
	#if COLOR_TYPE == 0
	if(numberStackedMeasurements % 3 != 0){
		#if BLENDING == 0
		blend_images(aux, image, image, imageWidth * imageHeight, IMAGE_BLEND_ALPHA_FG, IMAGE_BLEND_ALPHA_BG, imageHasAlpha, true);
		#else
		blend_images(aux, image, image, imageWidth * imageHeight, IMAGE_BLEND_ALPHA_FG, IMAGE_BLEND_ALPHA_BG, imageHasAlpha, false);
		#endif
		for(uint32_t c = 0; c < numberStackedMeasurements; c++){
			multilineImage->colors[c][0] = multilineImage->colors[c][0] >> 1;
			multilineImage->colors[c][1] = multilineImage->colors[c][1] >> 1;
			multilineImage->colors[c][2] = multilineImage->colors[c][2] >> 1;
		}
	}
	#endif
	free(aux);

    return multilineImage;
}

void destroyMultilineImage(MultilineImage * multilineImage){
	free(multilineImage->image);
	for(uint32_t l = 0; l < multilineImage->numLines; l++){
		free(multilineImage->colors[l]);
	}
	free(multilineImage->colors);
	free(multilineImage);
}
