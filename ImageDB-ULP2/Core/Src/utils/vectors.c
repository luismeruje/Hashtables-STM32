/*
 * vectors.c
 *
 *  Created on: Jun 19, 2024
 *      Author: luisferreira
 */

#include "vectors.h"
#include <math.h>
#include <stdlib.h>

double vector_magnitude(double va, double vb, double vc){
    double magnitude = sqrt(pow((va), 2.0) + pow((vb), 2.0) + pow((vc), 2.0));
    return magnitude;
}

double * normalize_vector(uint8_t * color){
    double * normalizedVector = (double * ) malloc(sizeof(double) * 3);
    double r = (double) color[0];
    double g = (double) color[1];
    double b = (double) color[2];

    double magnitude = vector_magnitude(r,g,b);

    if(magnitude != 0){
    	normalizedVector[0] = r / magnitude;
    	normalizedVector[1] = g / magnitude;
    	normalizedVector[2] = b / magnitude;
    } else {
    	//Return the null vector itself
    	normalizedVector[0] = 0;
    	normalizedVector[1] = 0;
    	normalizedVector[2] = 0;

    }

    return normalizedVector;
}

double * vector_difference(uint8_t * vector1, uint8_t * vector2){
    double * difference = (double *) malloc(sizeof(double) * 3);
    difference[0] = (double)vector1[0] - vector2[0];
    difference[1] = (double)vector1[1] - vector2[1];
    difference[2] = (double)vector1[2] - vector2[2];

    return difference;
}

double * cross_product(double * v1, double* v2){
    double * crossProduct = (double *) malloc(sizeof(double) * 3);
    crossProduct[0] = pow((v1[1] * v2[2]) - (v1[2] * v2[1]), 2);
    crossProduct[1] = pow((v1[2] * v2[0]) - (v1[0] * v2[2]), 2);
    crossProduct[2] = pow((v1[0] * v2[1]) - (v1[1] * v2[0]), 2);

    return crossProduct;
}

double inner_product(double * v1, double* v2){
	double innerProduct;
	innerProduct = v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
	return innerProduct;
}

double * scalar_product(double scalar, double* vector){
	double * scalarProduct = (double *) malloc(sizeof(double) * 3);
	scalarProduct[0] = scalar * vector[0];
	scalarProduct[1] = scalar * vector[1];
	scalarProduct[2] = scalar * vector[2];
	return scalarProduct;
}

double * vector_sum(double * vector1, double * vector2){
    double * sum = (double *) malloc(sizeof(double) * 3);
    sum[0] = (double)vector1[0] + vector2[0];
    sum[1] = (double)vector1[1] + vector2[1];
    sum[2] = (double)vector1[2] + vector2[2];

    return sum;
}

double euclidean_distance_3d(uint8_t *color1, uint8_t *color2){
    double result = sqrt(pow((double) color1[0] - color2[0], 2.0) + pow((double) color1[1] - color2[1], 2.0) + pow((double) color1[2] - color2[2], 2.0));
    return result;

}
