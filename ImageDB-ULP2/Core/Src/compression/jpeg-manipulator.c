#include <compression/jpeg-manipulator.h>
#include "settings.h"
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "jpeglib.h"
#include <fcntl.h>
#include <image_output.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <utils.h>
#include <stdbool.h>
#if JPEG_ACCELERATOR == ON
#include <jpeg_utils.h>
#endif
#include "logging.h"


//From ST's example: https://github.com/STMicroelectronics/STM32CubeH7/blob/master/Projects/STM32H743I-EVAL/Examples/JPEG/JPEG_EncodingFromFLASH_DMA/Src/encode_dma.c
typedef struct
{
  uint8_t *DataBuffer;
  uint32_t DataBufferSize;

}JPEG_Data_BufferTypeDef;

//Not used, but important!
#define MAX_DMA_TRANSFER_SIZE 64000

#define MAX_IN_SIZE 24576
#define MAX_OUT_SIZE 24576

//Possible contexts
#define JPEG_CPU 0
#define JPEG_DMA 1



//Possible status
#define JPEG_STATUS_READY	  0
#define JPEG_STATUS_ENCODING  1
#define JPEG_STATUS_DECODING  2

#if JPEG_ACCELERATOR == ON
extern JPEG_HandleTypeDef hjpeg;

volatile uint8_t jpegMDMADone = 0;
uint32_t finishTime = 0;

//Static variables for this module
static uint8_t JPEG_STATUS = JPEG_STATUS_READY;
static uint8_t JPEG_CONTEXT = JPEG_CPU;
static uint32_t jpegOutputBytesAvailable = 0;
static uint32_t jpegInputRemainingBytes = 0;
static JPEG_YCbCrToRGB_Convert_Function pYCbCrToRGB_Convert_Function = NULL;
static JPEG_RGBToYCbCr_Convert_Function pRGBToYCbCr_Convert_Function = NULL;
static uint32_t MCUDataInIndex = 0;
static uint32_t MCUDataOutIndex = 0;
static uint8_t * JPEGIn = NULL;
static uint8_t * JPEGOut = NULL;
static uint8_t * JPEGOutRef = NULL;
static bool convertRGBToYCbCrFlag = false;

//Buffers
static uint8_t Data_In_Buffer[JPEG_IN_BUFFER_SIZE] = {0};
static uint8_t Data_Out_Buffer[JPEG_OUT_BUFFER_SIZE] = {0};
JPEG_Data_BufferTypeDef Jpeg_IN_BufferTab = { Data_In_Buffer, 0};
JPEG_Data_BufferTypeDef Jpeg_OUT_BufferTab = { Data_Out_Buffer, 0};

void set_convertRGBToYCbCrFlag(bool value){
	convertRGBToYCbCrFlag = value;
}


void HAL_JPEG_EncodeCpltCallback(JPEG_HandleTypeDef *hjpeg)
{
  /* Prevent unused argument(s) compilation warning */
	//finishTime = HAL_GetTick();
	UNUSED(hjpeg);
#if LOG_LEVEL >= INFO
	printf("Encode finished \n");
#endif
	jpegMDMADone = 1;
	JPEG_STATUS = JPEG_STATUS_READY;
	//HAL_ResumeTick();
}

void HAL_JPEG_DecodeCpltCallback(JPEG_HandleTypeDef *hjpeg)
{
  /* Prevent unused argument(s) compilation warning */
	//finishTime = HAL_GetTick();
	UNUSED(hjpeg);
#if LOG_LEVEL >= INFO
	printf("Decode finished \n");
#endif
	jpegMDMADone = 1;
	JPEG_STATUS = JPEG_STATUS_READY;
	//HAL_ResumeTick();
}

void HAL_JPEG_InfoReadyCallback(JPEG_HandleTypeDef *hjpeg, JPEG_ConfTypeDef *pInfo){
	UNUSED(hjpeg);
	UNUSED(pInfo);
#if LOG_LEVEL >= INFO
	printf("Info Ready callback\n");
#endif
}

void HAL_JPEG_ErrorCallback(JPEG_HandleTypeDef *hjpeg){
	UNUSED(hjpeg);
#if LOG_LEVEL >= ERROR
	printf("ERROR callback!\n");
#endif
}

/*
 * Description: Asserted for both encoding and decoding
         operations to inform the application that the input buffer has been
         consumed. Considers JPEGIn is already in YCbCr format.
 * */
void HAL_JPEG_GetDataCallback(JPEG_HandleTypeDef *hjpeg, uint32_t NbDecodedData){
	if(JPEG_STATUS == JPEG_STATUS_ENCODING || JPEG_STATUS == JPEG_STATUS_DECODING){
//		__HAL_JPEG_CLEAR_FLAG(hjpeg, JPEG_FLAG_IFTF);
//		__HAL_JPEG_CLEAR_FLAG(hjpeg, JPEG_FLAG_IFNFF);
		jpegInputRemainingBytes -= NbDecodedData;


		if(jpegInputRemainingBytes > 0){
			//memset(Jpeg_IN_BufferTab.DataBuffer, 0, JPEG_IN_BUFFER_SIZE);
			JPEGIn += NbDecodedData;
			if(jpegInputRemainingBytes > MAX_IN_SIZE){
				HAL_JPEG_ConfigInputBuffer(hjpeg, JPEGIn, MAX_IN_SIZE);
			} else {
				HAL_JPEG_ConfigInputBuffer(hjpeg, JPEGIn, jpegInputRemainingBytes);
			}


		} else {
	#if LOG_LEVEL >= INFO
			printf("INFO: Input image exhausted while encoding JPEG image.\n");

	#endif
		}
	#if LOG_LEVEL >= INFO
			printf("INFO: Get data callback for encoding.\n");
	#endif
	} else {
#if LOG_LEVEL >= ERROR
		printf("WARNING: GetData callback called with an unexpected STATUS\n");
#endif
	}
}

//void HAL_JPEG_DataReadyCallback_Encoding (JPEG_HandleTypeDef *hjpeg, uint8_t *pDataOut, uint32_t OutDataLength)
//{
//  /* Update JPEG encoder output buffer address*/
//	UNUSED(pDataOut);
//	jpegOutputBytesAvailable -= OutDataLength;
//	JPEGOut += OutDataLength;
//	if(jpegOutputBytesAvailable > 0){
//		HAL_JPEG_ConfigOutputBuffer(hjpeg, JPEGOut, jpegOutputBytesAvailable);
//	} else {
//#if LOG_LEVEL >= INFO
//		printf("INFO: exhausted JPEG output buffer space while encoding\n");
//#endif
//	}
//	//Save power
//	//HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON,PWR_SLEEPENTRY_WFI);
//}
//
//
//void HAL_JPEG_DataReadyCallback_Decoding(JPEG_HandleTypeDef *hjpeg, uint8_t *pDataOut, uint32_t OutDataLength){
//	uint32_t convertedBytes = 0;
//	if(OutDataLength > 0 && JPEG_CONTEXT != JPEG_DMA){
//		MCUDataOutIndex += pYCbCrToRGB_Convert_Function(pDataOut,
//				JPEGOut,
//				MCUDataOutIndex,
//				OutDataLength,
//				&convertedBytes);
//		memset(Data_Out_Buffer, 0, JPEG_OUT_BUFFER_SIZE);
//		jpegOutputBytesAvailable -= convertedBytes;
//		//TODO: check for jpetOutputBytesAvailable
////		printf("WARNING: DataReady callback converted bytes (RGB size): %lu\n", convertedBytes);
////		print_image_pixels_alpha(JPEGOutRef, hjpeg->Conf.ImageWidth, hjpeg->Conf.ImageHeight);
////		printf("Pointer difference: %lu\n", (uint32_t) JPEGOut - (uint32_t) JPEGOutRef);
//		HAL_JPEG_ConfigOutputBuffer(hjpeg,Data_Out_Buffer, JPEG_OUT_BUFFER_SIZE);
//	}
//
//	//Save power
//	//HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON,PWR_SLEEPENTRY_WFI);
//}

// Must convert to
void HAL_JPEG_DataReadyCallback(JPEG_HandleTypeDef *hjpeg, uint8_t *pDataOut, uint32_t OutDataLength){
//	if(JPEG_STATUS == JPEG_STATUS_ENCODING){
//		HAL_JPEG_DataReadyCallback_Encoding(hjpeg, pDataOut, OutDataLength);
//	} else if(JPEG_STATUS == JPEG_STATUS_DECODING) {
//		HAL_JPEG_DataReadyCallback_Decoding(hjpeg, pDataOut, OutDataLength);
//	} else {
	UNUSED(pDataOut);
	if(JPEG_STATUS == JPEG_STATUS_ENCODING || JPEG_STATUS == JPEG_STATUS_DECODING){
		jpegOutputBytesAvailable -= OutDataLength;
		if(jpegOutputBytesAvailable > 0){
			JPEGOut += OutDataLength;
			if(jpegOutputBytesAvailable > MAX_OUT_SIZE){
				HAL_JPEG_ConfigOutputBuffer(hjpeg,JPEGOut, MAX_OUT_SIZE);
			} else {
				HAL_JPEG_ConfigOutputBuffer(hjpeg,JPEGOut, jpegOutputBytesAvailable);
			}
		} else {
		#if LOG_LEVEL >= INFO
				printf("INFO: exhausted JPEG output buffer space while encoding\n");
		#endif
		}
	} else{
		printf("WARNING: GetData callback called with an unexpected STATUS\n");
	}
	//printf("Data Ready Callback, out data length: %lu\n", OutDataLength);
}

JPEG_ConfTypeDef * new_jpeg_conf(uint32_t height, uint32_t width, uint32_t imageQuality){
	JPEG_ConfTypeDef *conf = malloc(sizeof(JPEG_ConfTypeDef));
	conf->ColorSpace = JPEG_YCBCR_COLORSPACE;
	conf->ChromaSubsampling = JPEG_444_SUBSAMPLING; //No subsampling
	conf->ImageHeight = height;
	conf->ImageWidth = width;
	conf->ImageQuality = imageQuality; // 1-100
	//Initialize YCbCr conversion tables
	JPEG_InitColorTables();
	//HAL_JPEG_DisableHeaderParsing(&hjpeg);
	return conf;
}



//TODO: does too big jpegSize increase execution time?
int decode_JPEG_accelerator(uint8_t * JPEGInput, uint32_t jpegSize, uint8_t * imageOutput, uint32_t imageOutputSize, JPEG_ConfTypeDef * conf){
	JPEG_STATUS = JPEG_STATUS_DECODING;
	JPEG_CONTEXT = JPEG_CPU;

	MCUDataInIndex = 0;
	MCUDataOutIndex = 0;
	HAL_StatusTypeDef hal_status = HAL_OK;
	HAL_JPEG_ConfigEncoding(&hjpeg,conf);
	uint32_t MCU_TotalNb = 0;
	JPEG_GetDecodeColorConvertFunc(conf, &pYCbCrToRGB_Convert_Function, &MCU_TotalNb);
	JPEGIn = JPEGInput;
	JPEGOut = imageOutput;
	JPEGOutRef = imageOutput; //debugging
	jpegInputRemainingBytes = jpegSize;
	jpegOutputBytesAvailable = imageOutputSize;


	hal_status = HAL_JPEG_Decode(&hjpeg, JPEGInput, jpegSize, Jpeg_OUT_BufferTab.DataBuffer, JPEG_OUT_BUFFER_SIZE, 1000000);

	if(hal_status != HAL_OK){
		printf("Hal error decoding, status: %d\n", hal_status);
		return -1;
	}

	//Reset global variables. TODO: avoid calling this function as this is not an MDMA call.
	wait_JPEG_MDMA();


	return 0;
}


int rearrange_YCbCr_64_block_into_pixels(uint8_t * input, uint32_t num_pixels, uint8_t *output, uint32_t *output_size){
	if (num_pixels % 64 != 0) {
		printf("Error: num_pixels must be a multiple of 64.\n");
		output = NULL;
		*output_size = 0;
		return -1;
	}

	size_t block_size = 64;
	size_t num_blocks = num_pixels / block_size;

	for (size_t block = 0; block < num_blocks; block++) {
		size_t input_base = block * (block_size * 3);
		size_t output_base = block * (block_size * 4);

		for (size_t i = 0; i < block_size; i++) {
			output[output_base + i * 4] = input[input_base + i];                // R
			output[output_base + i * 4 + 1] = input[input_base + block_size + i]; // G
			output[output_base + i * 4 + 2] = input[input_base + 2 * block_size + i]; // B
			output[output_base + i * 4 + 3] = 255; // Alpha (set to 255)
		}
	}

	return 0;
}

//WARNING: Assuming RGBA input
int rearrange_YCbCr_into_64_block(uint8_t * rgba_pixels, uint32_t num_pixels, uint8_t *output, uint32_t *output_size){

	if (num_pixels % 64 != 0) {
		printf("Error: num_pixels must be a multiple of 64.\n");
		output = NULL;
		output_size = 0;
		return -1;
	}

	size_t block_size = 64;
	size_t num_blocks = num_pixels / block_size;
	*output_size = num_pixels * 3; // Only R, G, B components (ignoring Alpha)

	for (size_t block = 0; block < num_blocks; block++) {
		size_t output_base = block * (block_size * 3);
		size_t input_idx = 0;
		for (size_t i = 0; i < block_size; i++) {
			input_idx = (block * block_size + i) * 4;

			output[output_base + i] = rgba_pixels[input_idx];          // R
			output[output_base + block_size + i] = rgba_pixels[input_idx + 1]; // G
			output[output_base + 2 * block_size + i] = rgba_pixels[input_idx + 2]; // B
		}
	}

	return 0;
}

//TODO: When JPEG conf is passed, there is no need to pass imageSize.
int encode_JPEG_accelerator_MDMA(uint8_t * image, uint32_t imageSize, uint8_t * JPEGOutput, uint32_t JPEGOutputSize, JPEG_ConfTypeDef * conf){
	MCUDataInIndex = 0;
	MCUDataOutIndex = 0;
	HAL_StatusTypeDef hal_status = HAL_OK;
	uint32_t MCU_TotalNb           = 0;
	uint32_t convertedBytes = 0;
	//HAL_PWR_EnableSleepOnExit();
	uint32_t YCbCrImageSize = (imageSize/4)*3;
	uint8_t *YCbCrImage = malloc(YCbCrImageSize);
	memset((void *)YCbCrImage, 0 , YCbCrImageSize);

	if(JPEG_STATUS != JPEG_STATUS_READY){
		printf("WARNING: Could not start JPEG encoding because a JPEG process is already ongoing.\n");
		return -1;
	}

	JPEG_STATUS = JPEG_STATUS_ENCODING;
	JPEG_CONTEXT = JPEG_DMA;

	//Set module static variables
	HAL_JPEG_ConfigEncoding(&hjpeg,conf);
	JPEGIn = (uint8_t *)YCbCrImage;
	JPEGOut = JPEGOutput;
	jpegInputRemainingBytes = imageSize;
	jpegOutputBytesAvailable = JPEGOutputSize;

	if(convertRGBToYCbCrFlag){
		JPEG_GetEncodeColorConvertFunc(conf, &pRGBToYCbCr_Convert_Function, &MCU_TotalNb);
		MCUDataInIndex += pRGBToYCbCr_Convert_Function(image, (uint8_t *) YCbCrImage, 0, imageSize,(uint32_t*)(&convertedBytes));
	} else {
		rearrange_YCbCr_into_64_block(image, conf->ImageHeight * conf->ImageWidth, YCbCrImage, &YCbCrImageSize);
	}
	Jpeg_IN_BufferTab.DataBufferSize = 0;




//	printf("\n\n\nYCbCr before JPEG encoding:");
//	print_image_pixels(Jpeg_IN_BufferTab.DataBuffer, 64, 64);
//	printf("\n\n\n");

	//Compress to JPEG

	hal_status = HAL_JPEG_Encode_DMA(&hjpeg, (uint8_t *) YCbCrImage, MAX_IN_SIZE, JPEGOutput, MAX_OUT_SIZE);

	if(hal_status != HAL_OK){
		printf("Hal error encoding JPEG with MDMA, status: %d\n", hal_status);
		printf("Error occurred while compressing image with JPEG codec.\n");
		JPEG_STATUS = JPEG_STATUS_READY;
		HAL_Delay(5000);
		jpegMDMADone = 0;
		return -1;
	}
#if JPEG_DMA_ENCODE_SLEEP == ON
	HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON,PWR_SLEEPENTRY_WFI);
#endif
	//printf("Omw to sleep\n");
	//HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON,PWR_SLEEPENTRY_WFI);

	wait_JPEG_MDMA();
//	printf("Output remaining: %ld\n", jpegOutputBytesAvailable);
//	printf("Input remaining: %ld\n",jpegInputRemainingBytes);
	free((uint8_t *) YCbCrImage);


	return 0;
}

int prepare_RGB_to_YCbCr_image(uint8_t * image, uint32_t imageSize, uint8_t **YCbCrImage, uint32_t *YCbCrImageSize, JPEG_ConfTypeDef * conf){
	uint32_t convertedBytes = 0;
	uint32_t MCU_TotalNb = 0;
	*YCbCrImageSize = (imageSize/4)*3;
	*YCbCrImage = malloc(*YCbCrImageSize);
	memset((void *)*YCbCrImage, 0 , *YCbCrImageSize);
	JPEG_GetEncodeColorConvertFunc(conf, &pRGBToYCbCr_Convert_Function, &MCU_TotalNb);

	Jpeg_IN_BufferTab.DataBufferSize = 0;


	MCUDataInIndex += pRGBToYCbCr_Convert_Function(image, (uint8_t *) *YCbCrImage, 0, imageSize,(uint32_t*)(&convertedBytes));

	return 0;
}



//TODO: version with alternating image buffers
int decode_JPEG_accelerator_MDMA(uint8_t * Jpeg, uint32_t JPEGSize, uint8_t * output, uint32_t outputSize, JPEG_ConfTypeDef * conf){

	uint32_t YCbCrImageSize = (outputSize / 4) * 3;
	uint8_t *YCbCrImage = malloc(YCbCrImageSize); //TODO: MAX_OUT_SIZE would be sufficient if we do YCbCr conversion iteratively
	memset((void *)YCbCrImage, 0 , YCbCrImageSize);
	uint32_t convertedBytes = 0;

	JPEG_STATUS = JPEG_STATUS_DECODING;
	JPEG_CONTEXT = JPEG_DMA;

	MCUDataInIndex = 0;
	MCUDataOutIndex = 0;
	HAL_StatusTypeDef hal_status = HAL_OK;
	HAL_JPEG_ConfigEncoding(&hjpeg,conf);
	uint32_t MCU_TotalNb = 0;
	JPEGIn = Jpeg;
	JPEGOut = (uint8_t *)YCbCrImage;
	JPEGOutRef = output; //debugging
	jpegInputRemainingBytes = JPEGSize;
	jpegOutputBytesAvailable = YCbCrImageSize;



	//Decompress JPEG
	hal_status = HAL_JPEG_Decode_DMA(&hjpeg, Jpeg, MAX_IN_SIZE,
			(uint8_t *)  YCbCrImage, MAX_OUT_SIZE);
	if(hal_status != HAL_OK){
		printf("Hal error decoding JPEG with MDMA, status: %d\n", hal_status);
		printf("Error occurred while decompressing image with JPEG codec.");
		JPEG_STATUS = JPEG_STATUS_READY;
		HAL_Delay(5000);
		return -1;
	}
	wait_JPEG_MDMA();

	if(convertRGBToYCbCrFlag){
		JPEG_GetDecodeColorConvertFunc(conf, &pYCbCrToRGB_Convert_Function, &MCU_TotalNb);
		MCUDataInIndex += pYCbCrToRGB_Convert_Function((uint8_t *) YCbCrImage, output, 0, YCbCrImageSize,(uint32_t*)(&convertedBytes));
	} else {
		rearrange_YCbCr_64_block_into_pixels(YCbCrImage, conf->ImageHeight * conf->ImageWidth, output, &outputSize);
	}

	free((uint8_t *) YCbCrImage);
	return 0;
}





//TODO: does too big jpegSize increase execution time?
int decode_JPEG_accelerator_IT(uint8_t * JPEGInput, uint32_t jpegSize, uint8_t * imageOutput, uint32_t imageOutputSize, JPEG_ConfTypeDef * conf){
	JPEG_STATUS = JPEG_STATUS_DECODING;
	JPEG_CONTEXT = JPEG_CPU;

	MCUDataInIndex = 0;
	MCUDataOutIndex = 0;
	HAL_StatusTypeDef hal_status = HAL_OK;
	HAL_JPEG_ConfigEncoding(&hjpeg,conf);
	uint32_t MCU_TotalNb = 0;
	JPEG_GetDecodeColorConvertFunc(conf, &pYCbCrToRGB_Convert_Function, &MCU_TotalNb);
	JPEGIn = JPEGInput;
	JPEGOut = imageOutput;
	JPEGOutRef = imageOutput; //debugging
	jpegInputRemainingBytes = jpegSize;
	jpegOutputBytesAvailable = imageOutputSize;

	hal_status = HAL_JPEG_Decode_IT(&hjpeg, JPEGInput, jpegSize, Jpeg_OUT_BufferTab.DataBuffer, JPEG_OUT_BUFFER_SIZE);

	if(hal_status != HAL_OK){
		printf("Hal error decoding, status: %d\n", hal_status);
		return -1;
	}

	//Reset global variables. TODO: avoid calling this function as this is not an MDMA call.
	wait_JPEG_MDMA();


	return 0;
}


//TODO: version with alternating image buffers
//int decode_JPEG_accelerator_IT(uint8_t * Jpeg, uint32_t JPEGSize, uint8_t * output, uint32_t outputSize, JPEG_ConfTypeDef * conf){
//	uint32_t YCbCrImageSize = (outputSize / 4) * 3;
//	volatile uint8_t *YCbCrImage = calloc((outputSize / 4) * 3, 1);
//	HAL_StatusTypeDef hal_status = HAL_OK;
//	uint32_t convertedBytes = 0;
//	uint32_t MCU_TotalNb           = 0;
//
//
//	JPEG_STATUS = JPEG_STATUS_DECODING;
//	JPEG_CONTEXT = JPEG_CPU;
//
//	MCUDataInIndex = 0;
//	MCUDataOutIndex = 0;
//	HAL_JPEG_ConfigEncoding(&hjpeg,conf);
//	JPEG_GetDecodeColorConvertFunc(conf, &pYCbCrToRGB_Convert_Function, &MCU_TotalNb);
//	JPEGIn = Jpeg;
//	JPEGOut = imageOutput;
//	JPEGOutRef = imageOutput; //debugging
//	jpegInputRemainingBytes = jpegSize;
//	jpegOutputBytesAvailable = imageOutputSize;
//
//	MCUDataInIndex = 0;
//	MCUDataOutIndex = 0;
//
//	//Is this needed or does JPEG get it from the header?
//	HAL_JPEG_ConfigEncoding(&hjpeg,conf);
//
//	JPEG_STATUS = JPEG_STATUS_DECODING;
//	JPEG_CONTEXT = JPEG_CPU;
//
//	//Decompress JPEG
//	hal_status = HAL_JPEG_Decode_IT(&hjpeg, Jpeg, JPEGSize,
//			(uint8_t *)  YCbCrImage, YCbCrImageSize);
//	if(hal_status != HAL_OK){
//		printf("Hal error decoding JPEG with MDMA, status: %d\n", hal_status);
//		printf("Error occurred while decompressing image with JPEG codec.");
//		JPEG_STATUS = JPEG_STATUS_READY;
//		HAL_Delay(5000);
//		return -1;
//	}
//	wait_JPEG_MDMA();
//
//	JPEG_GetDecodeColorConvertFunc(conf, &pYCbCrToRGB_Convert_Function, &MCU_TotalNb);
//
//	MCUDataInIndex += pYCbCrToRGB_Convert_Function((uint8_t *) YCbCrImage, output, 0, YCbCrImageSize,(uint32_t*)(&convertedBytes));
//
//	free((uint8_t *) YCbCrImage);
//	return 0;
//}

//TODO: add low-power flag
//TODO: add wait flag, which only waits for end of MDMA if desired.
//TODO: convert everything to YCbCr before JPEG encoding. At the moment is only doing a few bytes at a time.
int encode_JPEG_accelerator_IT(uint8_t * image, uint32_t imageSize, uint8_t * JPEGOutput, uint32_t JPEGOutputSize, JPEG_ConfTypeDef * conf){
	MCUDataInIndex = 0;
	MCUDataOutIndex = 0;
	HAL_StatusTypeDef hal_status = HAL_OK;
	uint32_t MCU_TotalNb           = 0;
	//HAL_PWR_EnableSleepOnExit();

	if(JPEG_STATUS != JPEG_STATUS_READY){
		printf("WARNING: Could not start JPEG encoding because a JPEG process is already ongoing.\n");
		return -1;
	}

	JPEG_STATUS = JPEG_STATUS_ENCODING;
	JPEG_CONTEXT = JPEG_CPU;

	//Set module static variables
	HAL_JPEG_ConfigEncoding(&hjpeg,conf);
	JPEGIn = image;
	JPEGOut = JPEGOutput;
	jpegInputRemainingBytes = imageSize;
	jpegOutputBytesAvailable = JPEGOutputSize;
	JPEG_GetEncodeColorConvertFunc(conf, &pRGBToYCbCr_Convert_Function, &MCU_TotalNb);

	Jpeg_IN_BufferTab.DataBufferSize = 0;


	//WARNING: Input buffer must have enough space to hold JPEG_IN_BUFFER_SIZE RGB bytes converted to the YCbCr format selected. //TODO: Check this requirement.
	if(imageSize > JPEG_IN_BUFFER_SIZE){
		MCUDataInIndex += pRGBToYCbCr_Convert_Function(image, Jpeg_IN_BufferTab.DataBuffer, 0, JPEG_IN_BUFFER_SIZE,(uint32_t*)(&Jpeg_IN_BufferTab.DataBufferSize));
		JPEGIn += JPEG_IN_BUFFER_SIZE;
		jpegInputRemainingBytes -= JPEG_IN_BUFFER_SIZE;

	} else{
		MCUDataInIndex += pRGBToYCbCr_Convert_Function(image, Jpeg_IN_BufferTab.DataBuffer, 0, imageSize,(uint32_t*)(&Jpeg_IN_BufferTab.DataBufferSize));
		JPEGIn += imageSize;
		jpegInputRemainingBytes = 0;
	}

//	printf("\n\n\nYCbCr before JPEG encoding:");
//	print_image_pixels(Jpeg_IN_BufferTab.DataBuffer, 64, 64);
//	printf("\n\n\n");

	//Compress to JPEG
	//HAL_SuspendTick();
	hal_status = HAL_JPEG_Encode_IT(&hjpeg, Jpeg_IN_BufferTab.DataBuffer, Jpeg_IN_BufferTab.DataBufferSize,	JPEGOutput, JPEGOutputSize);

	if(hal_status != HAL_OK){
		printf("Hal error encoding JPEG with IT, status: %d\n", hal_status);
		printf("Error occurred while compressing image with JPEG codec.\n");
		jpegMDMADone = 0;
		JPEG_STATUS = JPEG_STATUS_READY;
		HAL_Delay(5000);
		return -1;
	}

	//printf("Omw to sleep\n");
	//HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON,PWR_SLEEPENTRY_WFI);


	wait_JPEG_MDMA();

//	}



	return 0;
}

//Returns HAL time at which transfer finished.
int wait_JPEG_MDMA(){
	while(!jpegMDMADone){
	}
	jpegMDMADone = 0;
	JPEG_STATUS = JPEG_STATUS_READY;
	//return finishTime;
	return 0;
}

//TODO: add a status global variable to avoid starting new encoding/decoding operations before the previous operation has been completed.
int encode_JPEG_accelerator(uint8_t * image, uint32_t imageSize, uint8_t * JPEGOutput, uint32_t JPEGOutputSize, JPEG_ConfTypeDef * conf){
	MCUDataInIndex = 0;
	MCUDataOutIndex = 0;
	HAL_StatusTypeDef hal_status = HAL_OK;
	uint32_t MCU_TotalNb           = 0;

	if(JPEG_STATUS != JPEG_STATUS_READY){
		printf("WARNING: Could not start JPEG encoding because a JPEG process is already ongoing.\n");
		return -1;
	}

	JPEG_STATUS = JPEG_STATUS_ENCODING;
	JPEG_CONTEXT = JPEG_CPU;

	//Set module static variables
	HAL_JPEG_ConfigEncoding(&hjpeg,conf);
	JPEGIn = image;
	JPEGOut = JPEGOutput;
	jpegInputRemainingBytes = imageSize;
	jpegOutputBytesAvailable = JPEGOutputSize;
	JPEG_GetEncodeColorConvertFunc(conf, &pRGBToYCbCr_Convert_Function, &MCU_TotalNb);

	//Initialize YCrCb conversion process
	JPEG_InitColorTables();
	Jpeg_IN_BufferTab.DataBufferSize = 0;


	//WARNING: Input buffer must have enough space to hold JPEG_IN_BUFFER_SIZE RGB bytes converted to the YCbCr format selected. //TODO: Check this requirement.
	if(imageSize > JPEG_IN_BUFFER_SIZE){
		MCUDataInIndex += pRGBToYCbCr_Convert_Function(image, Jpeg_IN_BufferTab.DataBuffer, 0, JPEG_IN_BUFFER_SIZE,(uint32_t*)(&Jpeg_IN_BufferTab.DataBufferSize));
		JPEGIn += JPEG_IN_BUFFER_SIZE;
		jpegInputRemainingBytes -= JPEG_IN_BUFFER_SIZE;

	} else{
		MCUDataInIndex += pRGBToYCbCr_Convert_Function(image, Jpeg_IN_BufferTab.DataBuffer, 0, imageSize,(uint32_t*)(&Jpeg_IN_BufferTab.DataBufferSize));
		JPEGIn += imageSize;
		jpegInputRemainingBytes = 0;
	}

//	printf("\n\n\nYCbCr before JPEG encoding:");
//	print_image_pixels(Jpeg_IN_BufferTab.DataBuffer, 64, 64);
//	printf("\n\n\n");

	//Compress to JPEG
	hal_status = HAL_JPEG_Encode(&hjpeg, Jpeg_IN_BufferTab.DataBuffer, Jpeg_IN_BufferTab.DataBufferSize, JPEGOutput, JPEGOutputSize, 1000);

	if(hal_status != HAL_OK){
		printf("Hal error encoding JPEG, status: %d\n", hal_status);
		printf("Error occurred while compressing image with JPEG codec.\n");
		JPEG_STATUS = JPEG_STATUS_READY;
		HAL_Delay(5000);
		return -1;
	}



	//Makes sure variables are reset.
	wait_JPEG_MDMA();

	return 0;
}
#endif


uint32_t getJPEGSize(uint8_t * image, uint32_t maxImageSize){
	uint32_t size = 0;
	for(uint32_t i = 0; i < maxImageSize; i++){
		if(image[i] != 0){
			size = i;
		}
	}
	return size;
}




//TODO: where is subsampling set?
uint8_t * encode_JPEG_from_memory_to_memory(uint8_t * image, uint32_t width, uint32_t height, int quality){

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    
    JSAMPROW row_pointer[1];
    JSAMPROW jpegOutput = NULL;// = (JSAMPROW) malloc(sizeof(unsigned char) * width * height * 3);
    //memset(jpegOutput, 0, sizeof(unsigned char) * width * height * 3);
    unsigned long size = 0; //sizeof(unsigned char) * width * height * 3;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_mem_dest(&cinfo,&jpegOutput,&size);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults( &cinfo );
    jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);
    jpeg_start_compress( &cinfo, TRUE );

    while( cinfo.next_scanline < cinfo.image_height )
    {
        row_pointer[0] = &image[ cinfo.next_scanline * (unsigned int) cinfo.image_width *  (unsigned int) cinfo.input_components];
        jpeg_write_scanlines( &cinfo, row_pointer, 1 );
    }

    jpeg_finish_compress( &cinfo );
    jpeg_destroy_compress( &cinfo );

    return jpegOutput;

}


unsigned char * decode_JPEG_from_memory_to_memory(uint8_t * encoded_image, uint32_t width, uint32_t height){
//    struct stat file_info;
//	unsigned long jpg_size;
//	unsigned char *jpg_buffer;
    JSAMPARRAY buffer;
    uint32_t row_stride;
    unsigned char * outputBuffer = (unsigned char *) malloc(sizeof(unsigned char) * height * width * 3);
    memset(outputBuffer, 0, sizeof(uint8_t) * height * width * 3);

    // Variables for the decompressor itself
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, encoded_image, width * height*3);
    (void) jpeg_read_header(&cinfo, TRUE);
    (void) jpeg_start_decompress(&cinfo);
    row_stride = width * 3;
    buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    while (cinfo.output_scanline < cinfo.output_height) {
        (void) jpeg_read_scanlines(&cinfo, buffer, 1);
        memcpy(&outputBuffer[row_stride * (cinfo.output_scanline-1)], buffer[0], row_stride);
    }

    jpeg_finish_decompress( &cinfo );
    jpeg_destroy_decompress( &cinfo );

    return outputBuffer;
}


// NOT USED FOR NOW, BUT MIGHT COME IN HANDY

//unsigned char * decode_JPEG_from_file_to_memory(const char * filename){
//    JSAMPARRAY buffer;
//
//
//    // Variables for the decompressor itself
//	struct jpeg_decompress_struct cinfo;
//	struct jpeg_error_mgr jerr;
//
//	int row_stride;
//    FILE * infile;
//
//    if ((infile = fopen(filename, "rb")) == NULL) {
//        fprintf(stderr, "can't open %s\n", filename);
//        return 0;
//    }
//
//    cinfo.err = jpeg_std_error(&jerr);
//    jpeg_create_decompress(&cinfo);
//    jpeg_stdio_src(&cinfo, infile);
//    (void) jpeg_read_header(&cinfo, TRUE);
//
//    unsigned char * outputBuffer = (unsigned char *) malloc(sizeof(unsigned char) * cinfo.output_width * cinfo.output_height * cinfo.output_components);
//    memset(outputBuffer, 0, sizeof(unsigned char) * cinfo.output_width * cinfo.output_height * cinfo.output_components);
//
//    (void) jpeg_start_decompress(&cinfo);
//    row_stride = cinfo.output_width * cinfo.output_components;
//    buffer = (*cinfo.mem->alloc_sarray)
//		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
//
//    while (cinfo.output_scanline < cinfo.output_height) {
//        (void) jpeg_read_scanlines(&cinfo, buffer, 1);
//        memcpy(&outputBuffer[row_stride * cinfo.output_scanline], buffer[0], row_stride);
//    }
//
//    return outputBuffer;
//}

//int write_JPEG_from_memory_to_file (const char * filename, int quality, int width, int height, uint8_t* raw_image)
//{
//    struct jpeg_compress_struct cinfo;
//    struct jpeg_error_mgr jerr;
//
//    JSAMPROW row_pointer[1];
//    FILE *outfile = fopen( filename, "wb" );
//
//    if ( !outfile )
//    {
//        printf("Error opening output jpeg file %s\n!", filename );
//        return -1;
//    }
//    cinfo.err = jpeg_std_error( &jerr );
//    jpeg_create_compress(&cinfo);
//    jpeg_stdio_dest(&cinfo, outfile);
//
//
//    cinfo.image_width = width;
//    cinfo.image_height = height;
//    cinfo.input_components = 3;
//    cinfo.in_color_space = JCS_RGB;
//
//    jpeg_set_defaults( &cinfo );
//
//    jpeg_start_compress( &cinfo, TRUE );
//
//    while( cinfo.next_scanline < cinfo.image_height )
//    {
//        row_pointer[0] = &raw_image[ cinfo.next_scanline * cinfo.image_width *  cinfo.input_components];
//        jpeg_write_scanlines( &cinfo, row_pointer, 1 );
//    }
//
//    jpeg_finish_compress( &cinfo );
//    jpeg_destroy_compress( &cinfo );
//    fclose(outfile);
//
//    return 1;
//}
