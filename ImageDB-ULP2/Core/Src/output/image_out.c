/*
 * image_out.c
 *
 *  Created on: Nov 25, 2024
 *      Author: luisferreira
 */

#include <stdint.h>
#include <stdio.h>

void print_image_pixels(uint8_t *image, uint32_t imageWidth, uint32_t imageHeight){
    printf("Printing image pixels...\n");
	printf("[");
	for(uint32_t i = 0; i < imageHeight * imageWidth; i++){
        if(i == 0){
            printf("[");
        } else if(i % imageWidth == 0){
            printf(",[");
        }
        printf("[%d,%d,%d]", image[i*3],image[i*3 + 1],image[i*3 + 2]);
		if((i+1) % imageWidth == 0){
			printf("]");
		} else{
		    printf(",");
        }
	}
	printf("]\n");
}

void print_image_pixels_alpha(uint8_t *image, uint32_t imageWidth, uint32_t imageHeight){
    //printf("Printing image pixels...\n");
	printf("[");
	for(uint32_t i = 0; i < imageWidth * imageHeight; i++){
        if(i == 0){
            printf("[");
        } else if(i % imageWidth == 0){
            printf(",[");
        }
        printf("[%d,%d,%d,%d]", image[i*4],image[i*4 + 1],image[i*4 + 2], image[i*4 + 3]);
		if((i+1) % imageWidth == 0){
			printf("]");
		} else{
		    printf(",");
        }
	}
	printf("]\n");
}
