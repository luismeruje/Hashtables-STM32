#ifndef JPEG_MANIPULATOR_H
#define JPEG_MANIPULATOR_H
#include "settings.h"
#include <stdint.h>
#include "stm32u5xx_hal.h"
#include <stdbool.h>

void set_convertRGBToYCbCrFlag(bool value);
#if JPEG_ACCELERATOR == ON
JPEG_ConfTypeDef * new_jpeg_conf(uint32_t height, uint32_t width, uint32_t imageQuality);
int encode_JPEG_accelerator(uint8_t * image, uint32_t imageSize, uint8_t * JPEGOutput, uint32_t JPEGOutputSize, JPEG_ConfTypeDef * jpegConf);
int encode_JPEG_accelerator_IT(uint8_t * image, uint32_t imageSize, uint8_t * JPEGOutput, uint32_t JPEGOutputSize, JPEG_ConfTypeDef * conf);
int encode_JPEG_accelerator_MDMA(uint8_t * image, uint32_t imageSize, uint8_t * JPEGOutput, uint32_t JPEGOutputSize, JPEG_ConfTypeDef * conf);
int decode_JPEG_accelerator(uint8_t * JPEGInput, uint32_t jpegSize, uint8_t * imageOutput, uint32_t imageOutputSize, JPEG_ConfTypeDef * jpegConf);
int decode_JPEG_accelerator_MDMA(uint8_t * Jpeg, uint32_t JPEGSize, uint8_t * output, uint32_t outputSize, JPEG_ConfTypeDef * conf);
int decode_JPEG_accelerator_IT(uint8_t * Jpeg, uint32_t JPEGSize, uint8_t * output, uint32_t outputSize, JPEG_ConfTypeDef * conf);
//Prepares image by converting it into YCbCr and ordering to format expected by the accelerator.
int prepare_RGB_to_YCbCr_image(uint8_t * image, uint32_t imageSize, uint8_t **YCbCrImage, uint32_t *YCbCrImageSize, JPEG_ConfTypeDef * conf);
//Compresses image, using accelerator fed by MDMA, without converting image from RGB to YCbCr and ordering it properly for the accelerator (must already be converted).
int encode_JPEG_accelerator_MDMA_YCbCr(uint8_t * YCbCrImage, uint32_t YCbCrImageSize, uint8_t * JPEGOutput, uint32_t JPEGOutputSize, JPEG_ConfTypeDef * conf);
#endif

//WARNING: CPU JPEG operates with RGB (without alpha).
uint8_t * encode_JPEG_from_memory_to_memory(uint8_t * image, uint32_t width, uint32_t height, int quality);
unsigned char * decode_JPEG_from_memory_to_memory(uint8_t * encoded_image, uint32_t width, uint32_t height);

////TODO
//unsigned char * decode_JPEG_from_file_to_memory(const char * filename);
////TODO
//int write_JPEG_from_memory_to_file (const char * filename, int quality, int width, int height, uint8_t* raw_image);
int wait_JPEG_MDMA();
uint32_t getJPEGSize(uint8_t * image, uint32_t maxNumBytes);

//int encode_JPEG_accelerator_MDMA_direct(uint8_t * YCbCrImage, uint32_t YCbCrImageSize, uint8_t * JPEGOutput, uint32_t JPEGOutputSize, JPEG_ConfTypeDef * conf);
//int rearrange_YCbCr_64_block_into_pixels(uint8_t * input, uint32_t num_pixels, uint8_t *output, uint32_t *output_size);


#endif //JPEG_MANIPULATOR_H
