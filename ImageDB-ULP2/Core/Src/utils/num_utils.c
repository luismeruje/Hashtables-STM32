#include <math.h>
#include <stdlib.h>
#include "num_utils.h"
#include "settings.h"


double bound_number(double value, double max, double min){
    if (value > max) return max;
    if (value < min) return min;
    return value;
}

// Function to generate a random double between min and max
double rand_double(double min, double max) {
    return min + (rand() / (double)RAND_MAX) * (max - min);
}

double mean_absolute_error(int32_t * a1, int32_t * a2, int32_t size){
	int32_t sum = 0;
	for(int32_t i = 0; i < size; i++){
//		printf("Difference between %ld and %ld: %ld\n", a1[i], a2[i], abs(a1[i] - a2[i]));
		sum += abs(a1[i] - a2[i]);

	}

	return ((double) sum) / size;
}

/*
 * Takes integer value and scales and converts it to a double interval
 */
double scale_value_double(uint32_t value, uint32_t originMin, uint32_t originMax, double targetMin, double targetMax){
	double scaledValue = 0;
	scaledValue = (((double)value - originMin) / ((double) originMax - originMin)) * (targetMax-targetMin) + targetMin;

	return scaledValue;
}

float calculate_scale_factor(float valueMin, float valueMax, float dataMin, float dataMax){
	return (valueMax - valueMin) / (dataMax - dataMin);
}

uint32_t scale_value_precomputed_scale(float value, float valueMin, float dataMin, float scale){
	return (uint32_t)((value - dataMin) * scale + valueMin);
}


uint32_t scale_value(float value, float valueMin, float valueMax, float dataMin, float dataMax){
	uint32_t scaledValue = 0;
	scaledValue = (uint32_t) ((((float)value - dataMin ) / (dataMax - dataMin)) * (valueMax - valueMin) + valueMin);

	return scaledValue;
}

/* generate a random floating point number from min to max */
//From: https://ubuntuforums.org/showthread.php?t=1717717&p=10618266#post10618266
double randfrom(double min, double max)
{
    double range = (max - min);
    double div = RAND_MAX / range;
    return min + (rand() / div);
}
