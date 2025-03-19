/*
 * cielab_colors.h
 *
 *  Created on: Jun 19, 2024
 *      Author: luisferreira
 */

#ifndef INC_CIELAB_COLORS_H_
#define INC_CIELAB_COLORS_H_

#include <stdint.h>
typedef struct {
    double L;
    double a;
    double b;
} ColorLab;

void find_max_dist_colors_CIE(uint32_t num_colors, uint8_t **colors, uint32_t max_iterations);
void rgb_to_lab(uint8_t r, uint8_t g, uint8_t b, ColorLab *lab);
double deltaE2000_distance(ColorLab c1, ColorLab c2);

#endif /* INC_CIELAB_COLORS_H_ */
