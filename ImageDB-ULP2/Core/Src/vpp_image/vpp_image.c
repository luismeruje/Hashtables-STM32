#include "vpp_image.h"
#include "stdlib.h"
#include "string.h"
#include "settings.h"
#include "utils.h"
#include "num_utils.h"
#include <stdio.h>
#include "vpp_electrostatic_vectors.h"
#include "stm32u5xx_hal.h"

extern TIM_HandleTypeDef htim4;

static uint32_t NUMBER_VECTORS = 2;
static uint8_t MIN_VECTOR_VALUE_RGB = 0;

#define RGB_VALUE_MIN 0
#define RGB_VALUE_MAX (((256 - MIN_VECTOR_VALUE_RGB) * 3) - 1)

#define RED_VALUE_MIN 0
#define RED_VALUE_MAX 255

#define DIRECT_VALUE_MIN 0
#define DIRECT_VALUE_MAX INT_24_BIT_MAX

#define N_VECTOR_MIN 0
#define N_VECTOR_MAX ((256 * NUMBER_VECTORS) - 1)

static uint32_t zigzagOrder[64] = {
			0,  1,  8, 16,  9,  2,  3, 10,
		   17, 24, 32, 25, 18, 11,  4,  5,
		   12, 19, 26, 33, 40, 48, 41, 34,
		   27, 20, 13,  6,  7, 14, 21, 28,
		   35, 42, 49, 56, 57, 50, 43, 36,
		   29, 22, 15, 23, 30, 37, 44, 51,
		   58, 59, 52, 45, 38, 31, 39, 46,
		   53, 60, 61, 54, 47, 55, 62, 63
		};

double _direct_value_pixel_to_double(uint8_t *pixel, double dataMin, double dataMax){
	uint32_t value = 0;
	uint8_t * valueBytes = (uint8_t *) &value;
	double doubleValue = 0;

	valueBytes[0] = pixel[0];
	valueBytes[1] = pixel[1];
	valueBytes[2] = pixel[2];

	doubleValue = scale_value_double(value, 0, INT_24_BIT_MAX, dataMin, dataMax);

	return doubleValue;
}

uint8_t * _get_electrostatic_vectors_pointer(int numVectors){
	uint8_t * pointer = NULL;
	switch(numVectors){
		case 2:
			pointer = (uint8_t *) &nVectorReference2Points;
			break;
		case 4:
			pointer = (uint8_t *) &nVectorReference4Points;
			break;
		case 8:
			pointer = (uint8_t *) &nVectorReference8Points;
			break;
		case 16:
			pointer = (uint8_t *) &nVectorReference16Points;
			break;
		case 32:
			pointer = (uint8_t *) &nVectorReference32Points;
			break;
		default:
			printf("Num vectors %d not currently defined, defaulting to 64 vectors. This will most likely lead to errors.", numVectors);
			[[fallthrough]];
		case 64:
			pointer = (uint8_t *) &nVectorReference64Points;
			break;
	}
	return pointer;
}

/*
 * Points must be within valid vectors
 * TODO: optimize. Perhaps a reverse index.
 */
double mean_absolute_quantized_error(uint32_t * pointsA, uint32_t * pointsB, uint32_t numPixels){
	uint32_t valueA = 0, valueB = 0;
	int32_t differenceSum = 0, difference = 0;
	uint8_t * pixelA = NULL, *pixelB = NULL;
	uint8_t * electrostaticVectors = _get_electrostatic_vectors_pointer((int) NUMBER_VECTORS);
	double mean = 0;
	for(uint32_t p = 0; p < numPixels; p++){
		pixelA = (uint8_t *) &pointsA[p];
		pixelB = (uint8_t *) &pointsB[p];
		for(uint32_t i = 0; i < (uint32_t) NUMBER_VECTORS; i++){
			if(pixelA[0] == electrostaticVectors[i*2 + 0] && pixelA[1] == electrostaticVectors[i*2 + 1]){
				valueA = i * 255 + pixelA[2];
			}
			if(pixelB[0] == electrostaticVectors[i*2 + 0] && pixelB[1] == electrostaticVectors[i*2 + 1]){
				valueB = i * 255 + pixelB[2];
			}
		}
		difference = (int32_t) valueA -  (int32_t) valueB;
		if(difference < 0){
			difference = -difference;
		}
		differenceSum += difference;

		valueA = 0;
		valueB = 0;
	}
	mean = ((double) differenceSum) / numPixels;

	return mean;

}
/*
 *	For a set of pixels calculates the average absolute error of the direct value stored in the pixels.
 */
double mean_direct_value_error(uint32_t * pointsA, uint32_t * pointsB, uint32_t numPixels){
	int64_t differenceSum = 0, difference = 0;
	int64_t valueA  = 0, valueB = 0;
	double mean = 0;
	for(uint32_t p = 0; p < numPixels; p++){
		//Must ignore alpha to calculate difference!
		valueA = (int64_t) (pointsA[p] & 0xFFFFFF); //Zero out alpha
		valueB = (int64_t) (pointsB[p] & 0xFFFFFF); //Zero out alpha
		difference = valueA-valueB;
		if(difference < 0){
			difference = -difference;
		}
		differenceSum += difference;
	}
	mean = ((double) differenceSum) / numPixels;
	return mean;

}

uint32_t get_number_vectors(){
	return NUMBER_VECTORS;
}

void vpp_set_number_vectors(uint32_t numVectors){
	NUMBER_VECTORS = numVectors;
	return;
}

/*
 *	Sets pixel to the nearest valid position from its current position,
 *	according to the electrostatic vectors passed as argument. For N_VECTOR encoding
 */
void set_to_nearest_valid_pixel(uint8_t *pixel){
	uint8_t * electrostaticVectors = _get_electrostatic_vectors_pointer((int)NUMBER_VECTORS);
	double minDistance = MAX_DOUBLE, distance = 0;
	int closestVectorIndex = 0;
	for(uint8_t i = 0; i < NUMBER_VECTORS; i++){
		distance = euclidean_distance_2D(&electrostaticVectors[i * 2], pixel);
		if(distance < minDistance){
			minDistance = distance;
			closestVectorIndex = i;
		}
	}
	//Set pixel to the nearest vector
	pixel[0] = electrostaticVectors[closestVectorIndex * 2 + 0];
	pixel[1] = electrostaticVectors[closestVectorIndex * 2 + 1];
	return;
}

/*
 *  Sets pixel to the nearest valid position according to RGB vector encoding.
 */
void set_to_nearest_valid_rgb_pixel(uint8_t *pixel){
	if (pixel[0] > pixel[1] && pixel[0] > pixel[2]){
		if(pixel[0] <  MIN_VECTOR_VALUE_RGB){
			pixel[0] =  MIN_VECTOR_VALUE_RGB;
		}
		pixel[1] = 0;
		pixel[2] = 0;
	} else if (pixel[1] > pixel[2]){
		if(pixel[1] <  MIN_VECTOR_VALUE_RGB){
			pixel[1] =  MIN_VECTOR_VALUE_RGB;
		}
		pixel[0] = 0;
		pixel[2] = 0;
	} else {
		if(pixel[2] <  MIN_VECTOR_VALUE_RGB){
			pixel[2] =  MIN_VECTOR_VALUE_RGB;
		}
		pixel[0] = 0;
		pixel[1] = 0;
	}
	return;
}




//TODO: 255 or 256??
void fill_n_vector_value(uint8_t * pixel, double value, double dataMin, double dataMax, float scaleFactor){
	//uint32_t scaledValue = scale_value(value, 0, (256 * NUMBER_VECTORS) - 1, dataMin, dataMax);
	uint32_t scaledValue = scale_value_precomputed_scale(value, N_VECTOR_MIN, dataMin, scaleFactor);

	uint8_t * electrostaticVectors = _get_electrostatic_vectors_pointer((int)NUMBER_VECTORS);

	pixel[0] = electrostaticVectors[(scaledValue/256) * 2 + 0]; //R
	pixel[1] = electrostaticVectors[(scaledValue/256) * 2 + 1]; //G
	pixel[2] = scaledValue % 256; //B

	return;

}

double _get_pixel_n_value(uint8_t * pixel, double dataMin, double dataMax){
	uint32_t value = 0 ;
	double scaledValue = 0;
	uint8_t * electrostaticVectors = _get_electrostatic_vectors_pointer((int)NUMBER_VECTORS);
	for(uint32_t i = 0; i < (uint32_t) NUMBER_VECTORS; i++){
		if(pixel[0] == electrostaticVectors[i*2 + 0] && pixel[1] == electrostaticVectors[i*2 + 1]){
			value = i * 255 + pixel[2];
		}
	}
	scaledValue = scale_value_double( value, 0, NUMBER_VECTORS * 256 - 1, dataMin, dataMax);
	return scaledValue;
}

double _get_pixel_rgb_value(uint8_t * pixel, double dataMin, double dataMax){
	int64_t unscaledValue = 0;

	if(pixel[0] > 0){
		unscaledValue = pixel[0] - MIN_VECTOR_VALUE_RGB;
	} else if(pixel[1] > 0){
		unscaledValue = (int64_t) pixel[1] + 256 - (2 * (uint32_t)MIN_VECTOR_VALUE_RGB);
	} else if (pixel[2] > 0){
		unscaledValue = (int64_t) pixel[2] + (256 * 2) - (3 * MIN_VECTOR_VALUE_RGB);
	}
	if(unscaledValue < 0){
		printf("Found invalid value on RGB encoded image\n");
	}

	double scaledValue = scale_value_double((uint32_t) unscaledValue, 0, (uint32_t)(3*256 - (3 * MIN_VECTOR_VALUE_RGB)) - 1, dataMin, dataMax);
	return scaledValue;
}

double _get_pixel_red_value(uint8_t * pixel, double dataMin, double dataMax){
	uint32_t red = pixel[0];
	double scaledValue = scale_value_double(red, 0, 255, dataMin, dataMax);
	return scaledValue;
}

double _get_pixel_direct_value(uint8_t * pixel, double dataMin, double dataMax){
	uint32_t * unscaledValueP = (uint32_t *) pixel;
	uint32_t unscaledValue = 0;
	unscaledValue = (*unscaledValueP) & 0xFFFFFF; //Zero out alpha
	double scaledValue = scale_value_double(unscaledValue, 0, INT_24_BIT_MAX, dataMin, dataMax);
	return scaledValue;
}

float determine_scale_factor(float dataMin, float dataMax){
#if VPP_ENCODE == DIRECT_VALUE
		value = calculate_scale_factor(DIRECT_VALUE_MIN, DIRECT_VALUE_MAX, dataMin, dataMax);
#elif VPP_ENCODE == RED_VECTOR
		return calculate_scale_factor(RED_VALUE_MIN, RED_VALUE_MAX, dataMin, dataMax);
#elif VPP_ENCODE == RGB_VECTOR
		return calculate_scale_factor(RGB_VALUE_MIN, RGB_VALUE_MAX, dataMin, dataMax);
#elif VPP_ENCODE == N_VECTOR
		return calculate_scale_factor(N_VECTOR_MIN, N_VECTOR_MAX, dataMin, dataMax);
#else
		printf("No error function available for this VPP ENCODE MODE!\n");
#endif
}

uint8_t _set_pixel_direct_value(uint8_t * pixel, double value, double dataMin, double dataMax, float scaleFactor){
	//Scale to a value within 0 - 2^24, i.e., a 24-bit integer. Max: 16777215
	//uint32_t scaledValue = scale_value(value, 0, INT_24_BIT_MAX, dataMin, dataMax);
	uint32_t scaledValue = scale_value_precomputed_scale(value, DIRECT_VALUE_MIN, dataMin, scaleFactor);
	uint8_t * scaledValueBytes = (uint8_t *) &scaledValue;
	pixel[0] = scaledValueBytes[0];
	pixel[1] = scaledValueBytes[1];
	pixel[2] = scaledValueBytes[2];

	return 0;
}

uint8_t _set_pixel_r_g_b_value(uint8_t * pixel, double value, double dataMin, double dataMax, float scaleFactor){
#if PROFILE_RGB_QUANTIZATION
	uint32_t scalingCycles = 0, vectorCycles = 0;
	__HAL_TIM_SetCounter(&htim4,0);
#endif

	//uint32_t scaledValue = scale_value(value, RGB_VALUE_MIN, RGB_VALUE_MAX, dataMin, dataMax);
	uint32_t scaledValue = scale_value_precomputed_scale(value, RGB_VALUE_MIN, dataMin, scaleFactor);
#if PROFILE_RGB_QUANTIZATION
	scalingCycles = __HAL_TIM_GetCounter(&htim4);

	__HAL_TIM_SetCounter(&htim4,0);
#endif
	if(scaledValue < (256 - (uint32_t)MIN_VECTOR_VALUE_RGB)){
		pixel[0] = (uint8_t) scaledValue + MIN_VECTOR_VALUE_RGB;
		pixel[1] = 0;
		pixel[2] = 0;
	} else if (scaledValue < (512 - (2 * (uint32_t) MIN_VECTOR_VALUE_RGB))){
		pixel[0] = 0;
		pixel[1] = (uint8_t)(scaledValue % (256 - MIN_VECTOR_VALUE_RGB)) + MIN_VECTOR_VALUE_RGB;
		pixel[2] = 0;
	} else{
		pixel[0] = 0;
	    pixel[1] = 0;
		pixel[2] = (uint8_t)(scaledValue % (256 - MIN_VECTOR_VALUE_RGB)) + MIN_VECTOR_VALUE_RGB;
	}
#if PROFILE_RGB_QUANTIZATION
	vectorCycles = __HAL_TIM_GetCounter(&htim4);
	printf("Time distribution quantization. Scaling: %lu; Vector value: %lu\n",  scalingCycles, vectorCycles);
#endif
	return 0;
}

void set_min_value_RGB(uint8_t min_value){
	MIN_VECTOR_VALUE_RGB = min_value;
	return;
}

uint8_t get_min_value_RGB(){
	return MIN_VECTOR_VALUE_RGB;
}

/*
 * 	Paints a given vpp image pixel according to the color encoding scheme set by the VPP_ENCODE macro.
 */
uint8_t paint_pixel(uint8_t * pixel, double value, double dataMin, double dataMax, uint8_t imageHasAlpha, float scaleFactor){

#if VPP_ENCODE == RED_VECTOR
	//pixel[0] = (uint8_t) scale_value_(value, RED_VALUE_MIN, RED_VALUE_MAX, dataMin, dataMax);
	pixel[0] = (uint8_t) scale_value_precomputed_scale(value, RED_VALUE_MIN, dataMin, scaleFactor);
#elif VPP_ENCODE == N_VECTOR
	fill_n_vector_value(pixel, value, dataMin, dataMax, scaleFactor);
#elif VPP_ENCODE == DIRECT_VALUE
	_set_pixel_direct_value(pixel, value, dataMin, dataMax, scaleFactor);
#elif VPP_ENCODE == RGB_VECTOR
	_set_pixel_r_g_b_value(pixel, value, dataMin, dataMax, scaleFactor);
#else
	printf("Unknown encoding scheme for vpp image, check VPP_ENCODE macro\n");
#endif
	if(imageHasAlpha){
		pixel[3] = VPP_ENCODE_ALPHA;
	}
	return 0;
}


/*
 * Scales image pixels back to thei supposed values, then scales the absolute error according to the range of possible values for the data used.
 */
double calculate_mean_scaled_absolute_error_vpp_image(uint8_t *image, double * data, uint32_t numValues, double dataMin, double dataMax){
	double value = 0;
	double interval = dataMax - dataMin;
	double difference = 0, scaledDifferenceSum = 0;

	for(uint32_t i = 0; i < numValues; i++){
#if VPP_ENCODE == DIRECT_VALUE
		value = _get_pixel_direct_value(&image[i*4], dataMin, dataMax);
#elif VPP_ENCODE == RED_VECTOR
		value = _get_pixel_red_value(&image[i*4], dataMin, dataMax);
#elif VPP_ENCODE == RGB_VECTOR
		value = _get_pixel_rgb_value(&image[i*4], dataMin, dataMax);
#elif VPP_ENCODE == N_VECTOR
		value = _get_pixel_n_value(&image[i*4], dataMin, dataMax);
#else
		printf("No error function available for this VPP ENCODE MODE!\n");
#endif

		difference = value - data[i];
		if(difference < 0){
			difference = -difference;
		}
		scaledDifferenceSum += difference / interval;
	}
	return scaledDifferenceSum / numValues;
}

double calculate_mean_square_error_vpp_image(uint8_t *image, double * data, uint32_t numValues, double dataMin, double dataMax){
	double value = 0;
	double squaredDifference = 0, squaredDifferenceSum = 0;

	for(uint32_t i = 0; i < numValues; i++){
#if VPP_ENCODE == DIRECT_VALUE
		value = _get_pixel_direct_value(&image[i*4], dataMin, dataMax);
#elif VPP_ENCODE == RED_VECTOR
		value = _get_pixel_red_value(&image[i*4], dataMin, dataMax);
#elif VPP_ENCODE == RGB_VECTOR
		value = _get_pixel_rgb_value(&image[i*4], dataMin, dataMax);
#elif VPP_ENCODE == N_VECTOR
		value = _get_pixel_n_value(&image[i*4], dataMin, dataMax);
#else
		printf("No error function available for this VPP ENCODE MODE!\n");
#endif

		squaredDifference = value - data[i];
		squaredDifference = squaredDifference * squaredDifference;
		squaredDifferenceSum += squaredDifference;
	}
	return squaredDifferenceSum / numValues;
}


//TODO: separate zigzag traversal logic from this function

/*
 * Converts an array of data to a vpp (value per pixel) image. The specific color encoding scheme will depend on the macro set in the VPP_ENCODE macro
 * Will rotate dataset if not enough values
 */
uint8_t *create_vpp_image(double *values, uint32_t valuesSize, double dataMin, double dataMax, uint32_t numValues, uint32_t imageWidth, uint8_t imageHasAlpha, uint8_t eightBlockZigzagFlag){
	uint8_t *vppImage = NULL;
	uint8_t numComponents = 3;
	uint32_t imageHeight = numValues / imageWidth;
	if(imageHasAlpha){
		vppImage= (uint8_t *)malloc(sizeof(uint8_t) * numValues * 4 + 4);
		if((uint32_t)vppImage %4 != 0){
			printf("Vpp image not aligned\n");
			vppImage += 1;
		}

		if(vppImage == NULL ){
			printf("Was not able to allocate space for vpp image \n");
			return NULL;
		}
		numComponents = 4;
		memset(vppImage, 0, sizeof(uint8_t) * numValues * 4);
	} else {
		vppImage= (uint8_t *)malloc(sizeof(uint8_t) * numValues * 3);
		if(vppImage == NULL ){
			printf("Was not able to allocate space for vpp image \n");
			return NULL;
		}
		memset(vppImage, 0, sizeof(uint8_t) * numValues * 3);
	}

	float scaleFactor = determine_scale_factor(dataMin, dataMax);
	if(eightBlockZigzagFlag){
		//uint32_t numBlocks = numValues / JPEG_BLOCK_SIZE;

//		for(uint32_t i = 0; i < numBlocks; i++){
//			for(uint32_t j = 0; j < JPEG_BLOCK_SIZE; j++){
//				uint32_t zzindex = zigzagOrder[j];
//				uint32_t index = zzindex * (1 + (imageWidth / 8)) + (8 * i) + (zzindex % 8);
//				paint_pixel(&vppImage[index * numComponents], values[(i * JPEG_BLOCK_SIZE) + j], dataMin, dataMax, imageHasAlpha);
//			}
//        }
		uint32_t valuesIndex = 0;
		for (uint32_t blockRow = 0; blockRow < imageHeight; blockRow += 8) {
			for (uint32_t blockCol = 0; blockCol < imageWidth; blockCol += 8) {
				for (uint32_t i = 0; i < 64; ++i) {
					uint32_t zigzagIndex = zigzagOrder[i];
					uint32_t localRow = zigzagIndex / 8;
					uint32_t localCol = zigzagIndex % 8;
					uint32_t imageRow = blockRow + localRow;
					uint32_t imageCol = blockCol + localCol;
					uint32_t pixelIndex = (imageRow * imageWidth + imageCol);
					paint_pixel(&vppImage[pixelIndex * numComponents], values[valuesIndex%valuesSize], dataMin, dataMax, imageHasAlpha, scaleFactor);
					valuesIndex++;
				}
			}
		}
	} else {
		for(uint32_t i = 0; i < numValues; i++){
			paint_pixel(&vppImage[i * numComponents], values[i%valuesSize], dataMin, dataMax, imageHasAlpha, scaleFactor);
		}
	}

	return vppImage;
}

//Optimized operations, possibly vectorized.
//uint8_t *create_vpp_image_CMSIS(double *values, uint32_t valuesSize, double dataMin, double dataMax, uint32_t numValues, uint32_t imageWidth, uint8_t imageHasAlpha, uint8_t eightBlockZigzagFlag){
//	uint8_t *vppImage = NULL;
//	uint8_t numComponents = 3;
//	uint32_t imageHeight = numValues / imageWidth;
//	if(imageHasAlpha){
//		vppImage= (uint8_t *)malloc(sizeof(uint8_t) * numValues * 4 + 4);
//		if((uint32_t)vppImage %4 != 0){
//			printf("Vpp image not aligned\n");
//			vppImage += 1;
//		}
//
//		if(vppImage == NULL ){
//			printf("Was not able to allocate space for vpp image \n");
//			return NULL;
//		}
//		numComponents = 4;
//		memset(vppImage, 0, sizeof(uint8_t) * numValues * 4);
//	} else {
//		vppImage= (uint8_t *)malloc(sizeof(uint8_t) * numValues * 3);
//		if(vppImage == NULL ){
//			printf("Was not able to allocate space for vpp image \n");
//			return NULL;
//		}
//		memset(vppImage, 0, sizeof(uint8_t) * numValues * 3);
//	}
//
//	float scaleFactor = determine_scale_factor(dataMin, dataMax);
//	if(eightBlockZigzagFlag){
//		//uint32_t numBlocks = numValues / JPEG_BLOCK_SIZE;
//
////		for(uint32_t i = 0; i < numBlocks; i++){
////			for(uint32_t j = 0; j < JPEG_BLOCK_SIZE; j++){
////				uint32_t zzindex = zigzagOrder[j];
////				uint32_t index = zzindex * (1 + (imageWidth / 8)) + (8 * i) + (zzindex % 8);
////				paint_pixel(&vppImage[index * numComponents], values[(i * JPEG_BLOCK_SIZE) + j], dataMin, dataMax, imageHasAlpha);
////			}
////        }
//		uint32_t valuesIndex = 0;
//		for (uint32_t blockRow = 0; blockRow < imageHeight; blockRow += 8) {
//			for (uint32_t blockCol = 0; blockCol < imageWidth; blockCol += 8) {
//				for (uint32_t i = 0; i < 64; ++i) {
//					uint32_t zigzagIndex = zigzagOrder[i];
//					uint32_t localRow = zigzagIndex / 8;
//					uint32_t localCol = zigzagIndex % 8;
//					uint32_t imageRow = blockRow + localRow;
//					uint32_t imageCol = blockCol + localCol;
//					uint32_t pixelIndex = (imageRow * imageWidth + imageCol);
//					paint_pixel(&vppImage[pixelIndex * numComponents], values[valuesIndex%valuesSize], dataMin, dataMax, imageHasAlpha, scaleFactor);
//					valuesIndex++;
//				}
//			}
//		}
//	} else {
//		for(uint32_t i = 0; i < numValues; i++){
//			paint_pixel(&vppImage[i * numComponents], values[i%valuesSize], dataMin, dataMax, imageHasAlpha, scaleFactor);
//		}
//	}
//
//	return vppImage;
//}
