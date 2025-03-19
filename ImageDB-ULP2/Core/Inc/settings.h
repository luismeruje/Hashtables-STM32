/*
 * settings.h
 *
 *  Created on: Jul 17, 2024
 *      Author: luisferreira
 */

#ifndef INC_SETTINGS_H_
#define INC_SETTINGS_H_


/* =========================================================================
 * 		  				    GENERIC SETTINGS/MACROS
 * 		 --- DO NOT MODIFY IF YOU DO NOT UNDERSTAND THEIR MEANING ---
 ========================================================================= */

#define ALPHA 1
#define NO_ALPHA 0
#define MAX_DOUBLE 0xFFFFFFFF  //Assuming 32 bits
#define MAX_UINT32 0xFFFFFFFF
#define FULL_ALPHA 255
#define ENCODE_FLAG 0
#define DECODE_FLAG 1
#define INT_24_BIT_MAX 16777215 //2^24 - 1
#define ON 1
#define OFF 0
#define JPEG_MULTIPLIER 6 // JPEG arrays will be allocated with width * height * JPEG_MULTIPLIER


//If you mess with the timers/clocks you may have to change this
#define TIM2_COUNT_PER_SEC 320000 //160MHz/0.5K prescaler = 320000
#define TIM4_COUNT_PER_SEC 640000 //160MHz/0.25k prescaler = 640000

/*==================================
 * 	PROFILING SETTINGS
 * =================================
 */

//#define PROFILE_RGB_QUANTIZATION 1

/*==================================
 * 	ACCELERATOR BENCHMARK SETTINGS
 * =================================
 */
#define ACCL_VPP 0
#define ACCELERATOR_BENCHMARK_IMAGE_TYPE ACCL_VPP
#define ACCL_BENCHMARK_YCbCr_BURST_SIZE 16384 //Multiple of 8, and likely also 32

/*==================================
 *          JPEG settings
 * =================================
 */
//Encode with CPU feeding using interrupt
#define JPEG_IN_BUFFER_SIZE		((uint32_t)(4096))  //Multiple of 4. Probably multiple of 32 is also a good idea.
#define JPEG_OUT_BUFFER_SIZE	((uint32_t)(8 * 8 * 3 * 10)) //Multiple of size of one MCU.
#define JPEG_DMA_ENCODE_SLEEP OFF

/* ===================================
 * 		  VPP IMAGE SETTINGS
 ================================== */

#define VPP_FILL_ROW_WISE 0
#define VPP_FILL_BLOCK_ZIGZAG 1
#define VPP_IMAGE_FILL_STRATEGY VPP_FILL_ROW_WISE //0 -> row-wise // 1 -> zigzag 8x8 block

#define RED_VECTOR 0
#define N_VECTOR 1
#define DIRECT_VALUE 2
#define RGB_VECTOR 3

#define VPP_ENCODE_ALPHA 255 //When image is generated with alpha, pixels will be assigned this alpha value.
#define VPP_ENCODE RGB_VECTOR //Uses value exclusively in the red color axis




//If DATASET_DATA is enabled, will allocate dataset data in memory, so comment it out if not testing with datasets

//WARNING: Don't forget to set range based on the sample/dataset that we are using (i.e., the min/max values of the dataset)
//TODO: range vs error. Bigger range should equal bigger absolute value error, since quantization "buckets" are less accurate
//WARNING: If not running VPP dataset tests, comment out this section to avoid extra memory consumption
//#define DATASET_DATA 1 //-> EDP data. 1 only possible value. Já é averaged por isso pode não ser muito interessante...
//#define DATASET_DATA_SIZE 16384
//#define DATASET_DATA_MAX 1684.0
//#define DATASET_DATA_MIN 0.0

//#define DATASET_DATA 24// 2, 22, 23, 24, 25 (Diferentes subsegmentos dos dados originais). -> Car accelerator data, from kaggle dataset. https://www.kaggle.com/datasets/jefmenegazzo/pvs-passive-vehicular-sensors-datasets/data
//#define DATASET_DATA_SIZE 16384
//#define DATASET_DATA_MAX 9.1
//#define DATASET_DATA_MIN -10.8


/* ===================================
 * 		  ACCELERATOR TESTING
 ================================== */
#define JPEG_ACCELERATOR_TEST_QUALITY 50

/* ===================================
 * 		  ACCELERATOR SETTINGS
 ================================== */
//Enables/disables the definition of certain functions
//Avoids code not compiling when certain accelerators are turned off
//The JPEG ACCELERATOR also has some extra code generation associated to it. If you want to run something and code is missing, try enabling the this flag as well as the accelerator in the ioc file.
#define JPEG_ACCELERATOR ON
#define JPEG_BUFFER_SIZE (1024 * 260) //260KB

/* ===================================
 * 		  OL IMAGE SETTINGS
 ================================== */

#define OL_ENCODE_ALPHA 128 //When image is generated with alpha, pixels which are painted will receive this alpha. Empty pixels get alpha 0.
#define DISTANCE_EQUATION 2  // 0 = EUCLIDEAN DISTANCE, 1 = CHROMA_BRIGHTNESS_DISTANCE, 2 = deltaE2000
#define FOREGROUND_ALPHA 128// When using fixed alpha.
#define BACKGROUND_ALPHA 128// When using fixed alpha.

//For Chroma Brightness distance. These values act as weights
#define CHROMA_WEIGHT 0.7
#define BRIGHTNESS_WEIGHT 0.3

/* ===================================
 * 		  Image generation
 ================================== */

#define SINE_WAVE_STEP 0.1
#define SINE_FIRST_VALUE_REFERENCE 0
#define OUTLIER_SEED_VALUE 4378972

/* ===================================
 * 		  Image generation
 * 		      OL images
 ================================== */

#define COLOR_TYPE 1 //0 = 1 bit-shift colors, i.e. fixed 128 value in alternating R/G/B colors. (Forced to use with alpha fg/bg = 1, colors will get shifted one bit to the left with each blending operation
//  				   1 = CIE Lab color space
#define BLENDING 1 // 0 = FIXED, 1 = USE IMAGE ALPHA VALUES.

//WARNING: COLOR_TYPE 1 and FIXED BLENDING ARE NOT A GOOD COMBO. Background empty pixels will fade away the foreground pixels unnecessarily. TODO: how does this affect 1-bit shift colors?

//If fixed blending, must set ALPHA fixed values
#if BLENDING == 0
#define IMAGE_BLEND_ALPHA_FG 128
#define IMAGE_BLEND_ALPHA_BG 128
#endif

//WARNING: IF COLOR_TYPE = 0, then BLENDING should be 0 and alpha fg/bg should be 1
#if COLOR_TYPE == 0
#undef BLENDING
#define BLENDING 0
#undef IMAGE_BLEND_ALPHA_FG
#undef IMAGE_BLEND_ALPHA_BG
#define IMAGE_BLEND_ALPHA_FG 1
#define IMAGE_BLEND_ALPHA_BG 1
#endif

/* ===================================
 * 		    CIELab colors (OL Images)
 ================================== */

#define NUM_ITERATIONS_CIE 10000



#endif /* INC_SETTINGS_H_ */


