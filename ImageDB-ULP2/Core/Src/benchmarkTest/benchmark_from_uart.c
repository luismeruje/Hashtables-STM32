/*
 * benchmark_from_uart.c
 *
 *  Created on: Feb 7, 2025
 *      Author: luisferreira
 */

#include "benchmark_from_uart.h"
#include "benchmark_accelerators.h"
#include "benchmark_vpp_image.h"
#include "benchmark_xor.h"
#include "jpeg-manipulator.h"
#include "xmodem.h"
#include "string.h"
#include <stdlib.h>
#include "logging.h"
#include <stdio.h>
#include <stdbool.h>



#define BYTES_COMPRESSION_TYPE 2
#define BYTES_MODIFIER 1
#define BYTES_METRIC 1
#define BYTES_NUM_VALUES (4 + 8 + 8) //Num values + min double + max double
#define METADATA_SIZE (BYTES_COMPRESSION_TYPE + BYTES_MODIFIER + BYTES_METRIC + BYTES_NUM_VALUES)

#define MAX_DATA_PRINT 40

#define DOUBLES_PER_ALLOC 49152 //Multiple of 256*64 image for now
#define DOUBLES_BYTE_SIZE 8



typedef enum compressionType {
	JPEG_ACC_CPU_VPP,
	JPEG_ACC_DMA_VPP,
	JPEG_CPU_VPP,
	GORILLA,
	BUFF,
	COMPRESSION_UNKNOWN
} CompressionType;

typedef enum modifier{
	NORMAL,
	RGB_YCbCr_CONVERT,
	MODIFIER_UNKNOWN
} Modifier;

typedef enum metric{
	PERFORMANCE_ENCODE,
	PERFORMANCE_DECODE,
	COMPRESSION_ERROR,
	SIZE,
	RAW,
	METRIC_UNKNOWN
} Metric;

typedef struct benchmarkMetadata{
	uint32_t numValues;
	double minValue;
	double maxValue;
	CompressionType compression;
	Modifier modifier;
	Metric metric;
} BenchmarkMetadata;


CompressionType strToCompressionType(char compressionType[]){
	Metric metric;

	if (strcmp(compressionType, "JN") == 0) {
		metric = JPEG_ACC_CPU_VPP;
	} else if (strcmp(compressionType, "JD") == 0) {
		metric = JPEG_ACC_DMA_VPP;
	} else if (strcmp(compressionType, "JC") == 0) {
		metric = JPEG_CPU_VPP;
	} else if (strcmp(compressionType, "GO") == 0) {
		metric = GORILLA;
	} else if (strcmp(compressionType, "BU") == 0) {
		metric = BUFF;
	} else {
		metric = METRIC_UNKNOWN;
	}

	return metric;
}

Modifier strToModifier(char feedingMediumStr[]){
	Modifier medium;

	switch(feedingMediumStr[0]){
	case 'N':
		medium = NORMAL;
		break;
	case 'D':
		medium = RGB_YCbCr_CONVERT;
		break;
	default:
		medium = MODIFIER_UNKNOWN;
	}

	return medium;
}

Metric strToMetric(char metricStr[]){
	Metric metric;

	switch(metricStr[0]){
	case 'C':
		metric = PERFORMANCE_ENCODE;
		break;
	case 'D':
		metric = PERFORMANCE_DECODE;
		break;
	case 'E':
		metric = COMPRESSION_ERROR;
		break;
	case 'S':
		metric = SIZE;
		break;
	case 'R':
		metric = RAW;
		break;
	default:
		metric = METRIC_UNKNOWN;
	}

	return metric;
}


void printMetadata(BenchmarkMetadata *metadata){
	printf("Compression type: %d; Feeding Medium: %d; Metric: %d\n", metadata->compression, metadata->modifier, metadata->metric);
}


BenchmarkMetadata *parse_benchmark_metadata(char *benchmarkInfoRaw){
	BenchmarkMetadata *benchmarkMetadata = (BenchmarkMetadata*) malloc(sizeof(BenchmarkMetadata));

	//Could use union
	char compressionType[BYTES_COMPRESSION_TYPE + 1];
	char feedingMedium[BYTES_MODIFIER + 1];
	char metric[BYTES_METRIC + 1];
	char *dataPtr = benchmarkInfoRaw;
	uint32_t * intPtr = NULL;
	double *doublePtr = NULL;

	//I know it looks weird, but for some reason it doesn't work when you write it right
	strncpy(compressionType, benchmarkInfoRaw, BYTES_COMPRESSION_TYPE);
	compressionType[BYTES_COMPRESSION_TYPE] = 0; // \0
	dataPtr += BYTES_COMPRESSION_TYPE;

	strncpy(feedingMedium, benchmarkInfoRaw + BYTES_COMPRESSION_TYPE, BYTES_MODIFIER);
	feedingMedium[BYTES_MODIFIER] = 0;
	dataPtr += BYTES_MODIFIER;

	strncpy(metric, benchmarkInfoRaw + BYTES_COMPRESSION_TYPE + BYTES_MODIFIER, BYTES_METRIC);
	metric[BYTES_METRIC] = 0;
	dataPtr += BYTES_METRIC;

	benchmarkMetadata->compression = strToCompressionType(compressionType);
	benchmarkMetadata->modifier = strToModifier(feedingMedium);
	benchmarkMetadata->metric = strToMetric(metric);

	intPtr = (uint32_t *) dataPtr;
	benchmarkMetadata->numValues = *intPtr;
	dataPtr += sizeof(uint32_t);

	doublePtr = (double *) dataPtr;
	benchmarkMetadata->minValue = doublePtr[0];
	benchmarkMetadata->maxValue = doublePtr[1];

	return benchmarkMetadata;
}

double *parseDataStr(char *benchmarkInfoRaw, uint32_t metadataByteSize, uint32_t *numParsedDoubles){
	char *subtoken, **longPointer = NULL;
	double *data = (double *) malloc(sizeof(double) * DOUBLES_PER_ALLOC);
	uint32_t doublesSize = DOUBLES_PER_ALLOC;

	*numParsedDoubles = 0;
	benchmarkInfoRaw += metadataByteSize;

	while ((subtoken = strsep(&benchmarkInfoRaw, ";"))){
		data[*numParsedDoubles] = strtod(subtoken, longPointer);
		*numParsedDoubles+=1;

		if((*numParsedDoubles + 1) >= doublesSize){
			data = realloc(data, sizeof(double) * (doublesSize + DOUBLES_PER_ALLOC));
			doublesSize += DOUBLES_PER_ALLOC;
		}
	}

    return data;
}

//TODO: realloc crashes, allocating big enough size for now
double *parseDataBinary(char *benchmarkInfoRaw, uint32_t infoNumBytes, uint32_t numExpectedDoubles, uint32_t metadataByteSize, uint32_t *numParsedDoubles){
	double *data = (double *) malloc(sizeof(double) * DOUBLES_PER_ALLOC);
	uint32_t doublesSize = DOUBLES_PER_ALLOC;
	double *rawData = NULL;

	*numParsedDoubles = 0;
	benchmarkInfoRaw += metadataByteSize;
	rawData = (double *) benchmarkInfoRaw;

	for(uint32_t i = 0; (i < numExpectedDoubles) && (i < infoNumBytes/DOUBLES_BYTE_SIZE); i++){
		data[i] = rawData[i];
//		memcpy(&data[i], benchmarkInfoRaw + metadataByteSize + (i*DOUBLES_BYTE_SIZE), DOUBLES_BYTE_SIZE);
		*numParsedDoubles = *numParsedDoubles + 1;
		if(i >= doublesSize){
			data = realloc(data, sizeof(double) * (doublesSize + DOUBLES_PER_ALLOC));
			if(data == NULL){
				printf("ERROR: Was not able to alloc enough space for all values.\n");
			}
			doublesSize += DOUBLES_PER_ALLOC;
		}
	}

	return data;
}

void printData(const double *data, uint32_t numDoubles){

	for(uint32_t i = 0; i < numDoubles && i < MAX_DATA_PRINT; i++){
		printf("%lf;",data[i]);
	}
	printf("\n");

}


void print_benchmark_details(BenchmarkMetadata *metadata, double* data, uint32_t numParsedDoubles, uint8_t *benchmarkInfoRaw, uint32_t numReceivedBytes){
	printf("Num received doubles: %ld\n", numParsedDoubles);
	printf("First bytes of received data: ");
	for(uint32_t i = 0; i < numReceivedBytes && i < MAX_DATA_PRINT; i++){
		printf("%x ", benchmarkInfoRaw[i]);
	}
	printf("\n");
	printMetadata(metadata);
	printData(data, numParsedDoubles);
}

void setModifiers(Modifier modifier){
	switch(modifier){
	case NORMAL:
		set_convertRGBToYCbCrFlag(false);
		break;
	case RGB_YCbCr_CONVERT:
		set_convertRGBToYCbCrFlag(true);
		break;
	case MODIFIER_UNKNOWN:
		printf("ERROR: Unknown modifier\n");
		break;
	}
}

void benchmark_from_uart(BenchmarkSettings *settings){

	uint32_t numReceivedBytes = 0, numParsedDoubles = 0;
	char *benchmarkInfoRaw = xmodem_receive(&numReceivedBytes);
	if(numReceivedBytes > 0){
		BenchmarkMetadata *metadata = parse_benchmark_metadata(benchmarkInfoRaw);
		double* data = parseDataBinary(benchmarkInfoRaw, numReceivedBytes, metadata->numValues, METADATA_SIZE, &numParsedDoubles);
		//Fill in settings details
		settings->data = data;
		settings->numValues = numParsedDoubles; //Metadata numValues is only the expected number of values
		settings->minValue = metadata->minValue;
		settings->maxValue = metadata->maxValue;

		if(LOG_LEVEL >= INFO){
			print_benchmark_details(metadata, data, numParsedDoubles, (uint8_t *) benchmarkInfoRaw, numReceivedBytes);
		}
		setModifiers(metadata->modifier);

		switch(metadata->compression){
		case JPEG_ACC_DMA_VPP:
			switch(metadata->metric){
			case PERFORMANCE_ENCODE:
				benchmark_performance_encode_accelerator_jpeg_vpp(settings);
				break;
			case PERFORMANCE_DECODE:
				//benchmark_performance_JPEG_decode_CPU(settings->imageHeight, settings->imageWidth, settings->numRepeatImage, settings->numSizes, settings->quality, settings->numQuality, settings->numIterations);
				break;
			case COMPRESSION_ERROR:
				benchmark_JPEG_error_vpp_dataset(settings);
				break;
			case RAW:
				benchmark_raw_JPEG_accelerator_encode_performance_dataset(settings);
				break;
			case SIZE:
				break;
			default:
				printf("FAILED TO BENCHMARK FROM UART, unknown metric type: %d\n", metadata->metric);
			}
			break;
		case GORILLA:
			switch(metadata->metric){
			case PERFORMANCE_ENCODE:
				benchmark_performance_encode_UART_xor(settings);
				break;
			case PERFORMANCE_DECODE:
				//benchmark_performance_JPEG_decode_CPU(settings->imageHeight, settings->imageWidth, settings->numRepeatImage, settings->numSizes, settings->quality, settings->numQuality, settings->numIterations);
				break;
			case COMPRESSION_ERROR:
				benchmark_error_UART_xor(settings);
				break;
			case SIZE:
				benchmark_compressed_size_UART_xor(settings);
				break;
			default:
				printf("FAILED TO BENCHMARK FROM UART, unknown metric type: %d\n", metadata->metric);
			}
			break;
		}



		free(data);
		free(metadata);
		free(benchmarkInfoRaw);
		//Special character, signals end of benchmark so sender can proceed with more data.
		(void)uart_transmit_ch('@');
	} else {
		printf("Failed to receive data, retrying.\n");
	}
}
