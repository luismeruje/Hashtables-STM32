/*
 * vectors.h
 *
 *  Created on: Jun 19, 2024
 *      Author: luisferreira
 */

#ifndef INC_VECTORS_H_
#define INC_VECTORS_H_

#include "stdint.h"

double vector_magnitude(double va, double vb, double vc);
double * normalize_vector(uint8_t * color);
double * vector_difference(uint8_t * vector1, uint8_t * vector2);
double * cross_product(double * v1, double* v2);
double euclidean_distance_3d(uint8_t *color1, uint8_t *color2);
double inner_product(double * v1, double* v2);
double * scalar_product(double scalar, double* vector);
double * vector_sum(double * vector1, double * vector2);

#endif /* INC_VECTORS_H_ */
