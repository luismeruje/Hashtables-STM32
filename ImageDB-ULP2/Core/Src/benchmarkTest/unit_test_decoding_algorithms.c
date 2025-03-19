/*
 * unit_test_decoding_algorithms.c
 *
 *  Created on: Jul 2, 2024
 *      Author: luisferreira
 */


#include <vectors.h>
#include <stdio.h>

#define EUCLIDEAN_ERROR_MARGIN 0.02

uint8_t test_euclidean_distance(){
	uint8_t color1[3] = {0,0,0};
	uint8_t color2[3] = {255,255,255};
	uint8_t color3[3] = {128,128,128};
	uint8_t color4[3] = {0,255,0};
	uint8_t color5[3] = {3,15,30};

	uint8_t test_ok = 1;

	double expectedResults[5] = {0.0, 0.0, 255.0, 219.87, 241.89};
	double results[5] = {0.0};

	results[0] = euclidean_distance_3d(color1, color1);
	results[1] = euclidean_distance_3d(color2, color2);
	results[2] = euclidean_distance_3d(color1, color4);
	results[3] = euclidean_distance_3d(color2, color3);
	results[3] = euclidean_distance_3d(color4, color5);

	for(uint32_t i = 0; i < sizeof(expectedResults) / sizeof(double); i++){
		if((results[i] + EUCLIDEAN_ERROR_MARGIN) < expectedResults[i] || (results[i] - EUCLIDEAN_ERROR_MARGIN) > expectedResults[i]){
			test_ok = 0;
			printf("Unexpected result on operation nr. %lu. Expected result: %lf. Obtained result: %lf.", i, expectedResults[i], results[i]);
		}
	}
	return test_ok;

}

//uint8_t test_rgb_distance(){
//	return 0;
//}

void run_tests_decoding_algorithms(){
	test_euclidean_distance();
}
