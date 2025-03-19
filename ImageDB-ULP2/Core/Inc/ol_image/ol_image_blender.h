/*
 * image_blender.h
 *
 *  Created on: Jun 19, 2024
 *      Author: luisferreira
 */

#ifndef INC_OL_IMAGE_BLENDER_H_
#define INC_OL_IMAGE_BLENDER_H_


#include <stdint.h>

//TODO: What happens if useFixedAlpha = 0 and pixels do not have alpha value?
/*
 * @brief In CPU function for image blending. Performs the same blending operation as the DMA2D controller.
 * @param fgImage Byte pointer to foreground image.
 * @param bgImage Byte pointer to background image.
 * @param destination Byte pointer to location where blended image will be saved. Can be the same as fgImage or bgImage.
 * @param numPixels Number of pixels to be blended.
 * @param alphaFg Alpha value for foreground image.
 * @param alphaBg Alpha value for background image.
 * @param imageHasAlpha True/False flag parameter. If true (1) will consider image pixels as RGBA (32-bit). If false (0) pixels are treated as RGB (24-bit).
 * @param usedFixedAlpha True/False flag parameter. If true (1) will use ALPHA_FG and ALPHA_BG macros as the alpha values for all foreground and background image pixels, respectively. Otherwise, will consider pixel's alpha values.
 * @return Returns 0 if successful.
 */
int blend_images(uint8_t * fgImage, uint8_t * bgImage, uint8_t * destination, uint32_t numPixels, uint8_t alphaFg, uint8_t alphaBg, uint8_t imageHasAlpha, uint8_t useFixedAlpha);



/*
* @brief Perform or operation between image pixels.
* @param image1 Byte pointer to the first image to be blended. This is the image where the result will be stored.
* @param image2 Byte pointer to the second image to be blended.
* @param size Number of bytes to be blended.
* @return Returns 0 if successful.
*/
int image_or_blend(uint8_t *image1, uint8_t *image2, uint32_t size);


/*
 * @brief Blend images using DMA2D controller. Uses interrupt to signal end of blending operation.
 * @param fgImage Foreground image
 * @param bgImage Background image
 * @param destination Destination image. May be the same as fgImage or bgImage.
 * @param imageWidth Width of images.
 * @param imageHeight Height of images.
 * @return Returns 0 if successful.
 */
int blend_images_DMA2D_interrupt(uint8_t * fgImage, uint8_t * bgImage, uint8_t * destination, uint32_t imageWidth, uint32_t imageHeight);

#endif /* INC_OL_IMAGE_BLENDER_H_ */
