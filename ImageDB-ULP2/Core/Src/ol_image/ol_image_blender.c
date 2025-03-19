/*
 * image_blender.c
 *
 *  Created on: Jun 19, 2024
 *      Author: luisferreira
 */

#include <ol_image_blender.h>
#include "stm32u5xx_hal.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
extern DMA2D_HandleTypeDef hdma2d;

/*
 *
 *  Performs a blending operation where each pixel of images 1 and 2 are applied an OR operation, by pairs that occupy the same position.
 *  The result is stored in image1.
 *
 */

int image_or_blend(uint8_t *image1, uint8_t *image2, uint32_t size){
    for(uint32_t i=0; i<size; i++){
    	image1[i] = image1[i] | image2[i];
    }
    return 0;
}



/*
 * Performs image blending in the CPU.
 * Follows the same blending function as the STM32H743 DMA2D controller.
 *
 */
int blend_images(uint8_t * fgImage, uint8_t * bgImage, uint8_t * destination, uint32_t numPixels, uint8_t alphaFg, uint8_t alphaBg, uint8_t imageHasAlpha, uint8_t useFixedAlpha){
    uint32_t alphaMult = 0, alphaOut = 0;
    uint8_t numComponents = imageHasAlpha ? 4 : 3;
    if(useFixedAlpha){
        alphaMult = (uint32_t)(alphaFg * alphaBg) / 255;
        alphaOut = alphaFg + alphaBg - alphaMult;
    }
	for(uint32_t i = 0; i < numPixels; i++){
		//TODO: missing case for !useFixedAlpha and !imageHasAlpha
        if(!useFixedAlpha){
            alphaFg = fgImage[(i * numComponents) + 3];
            alphaBg = bgImage[(i * numComponents) + 3];
            alphaMult = ((uint32_t)alphaFg * (uint32_t) alphaBg) / 255;
            alphaOut = alphaFg + alphaBg - alphaMult;
        }
        //Set RGB
        for(uint32_t c = 0; c < 3; c++){
            if(alphaOut != 0){
            	/* Since values are limited to 255, cast will not change the signal */
            	int32_t bgDifference = (int32_t) ((int32_t) bgImage[(i * numComponents) + c])*alphaBg - ((int32_t) bgImage[(i * numComponents) + c])*alphaMult;
            	destination[(i * numComponents) + c] = (uint8_t) ((fgImage[(i * numComponents) + c]*alphaFg + (uint32_t) bgDifference) / alphaOut);
            } else{
                destination[(i * numComponents) + c] = 0;
            }
        }
        if(imageHasAlpha == true){
            //Set alpha
            destination[(i * numComponents) + 3] = (uint8_t) alphaOut;
        }

	}

	return 0;
}


/*
 * Blend images using DMA2D controller. Waits until interrupt is received before returning (i.e., until blending operation is finished).
 */
int blend_images_accelerator_interrupt(uint8_t * fgImage, uint8_t * bgImage, uint8_t * destination, uint32_t imageWidth, uint32_t imageHeight){
	HAL_StatusTypeDef hal_status = HAL_OK;
	hal_status = HAL_DMA2D_BlendingStart_IT(&hdma2d,
		(uint32_t)fgImage,   /* Foreground image */
		(uint32_t)bgImage,   /* Background image */
		(uint32_t)destination,   /* Destination address */
		imageWidth,  /* width in pixels   */
		imageHeight);  /* height in pixels   */

	if(hal_status != HAL_OK){
		printf("Hal error, status: %d\n", hal_status);
		printf("Request to blend images failed.\n");
		return -1;
	}

	hal_status = HAL_DMA2D_PollForTransfer(&hdma2d, 10);
	if(hal_status != HAL_OK){
		printf("Hal error, status: %d\n", hal_status);
		printf("Timed out while waiting for DMA2D blend operation.\n");
		return -2;
	}
	return 0;
}
