/*
 * cielab_colors.c
 *
 *  Created on: Jun 19, 2024
 *      Author: luisferreira
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <float.h>
#include "cielab_colors.h"
#include "num_utils.h"

static double k = 903.3; //CIE standard
static double e = 0.008856; //CIE standard

double refX =  0.9504; //Reference CIE D65 white
double refY = 1.0;  //
double refZ = 1.0888;

#define MAX_L 95
#define MIN_L 5

//TODO: determine colors such that distance between colors is maximized, and distance between blended colors and its original colors is smaller than between the blended colors and the remaining colors.

//Note: a good part of the code was generated using Chat-GPT


//Done by hand, but based on: http://www.brucelindbloom.com/index.html?Equations.html
void lab_to_xyz_2(ColorLab lab, double *X, double *Y, double *Z) {
    double xr, yr, zr;

    double fx, fy, fz;
    fy = (lab.L + 16) / 116;
    fx = (lab.a /500) + fy;
    fz = fy - (lab.b / 200);

    xr = pow(fx, 3);
    if ( xr <= e ){
        xr = pow(((116 * fx) - 16) / 116, 3);
    }

    yr = pow((lab.L + 16) / 116, 3);
    if( yr <= k * e){
        yr = lab.L / k;
    }

    zr = pow(fz,3);
    if(zr <= e){
        zr = ((116 * fz) - 16)/k;
    }

    *X = xr * refX;
    if (*X < 0) *X = 0;
    if (*X > 1) *X = 1;
    *Y = yr * refY;
    if (*Y < 0) *Y = 0;
    if (*Y > 1) *Y = 1;
    *Z = zr * refZ;
    if (*Z < 0) *Z = 0;
    if (*Z > 1) *Z = 1;
}

double sRGB_companding(double v){
    if(v > 0.0031308){
        v = 1.055 * pow(v, 1/2.4) - 0.055;
    } else{
        v = 12.92 * v;
    }
    return v;
}

// Function to convert XYZ to RGB. Done by hand, based on: http://www.brucelindbloom.com/index.html?Equations.html
void xyz_to_rgb_2(double X, double Y, double Z, uint8_t *r, uint8_t *g, uint8_t *b) {
//M^-1 matrix for sRGB using D65 reference white
//    [ 3.2404542 -1.5371385 -0.4985314]  [X]
//    [-0.9692660  1.8760108  0.0415560]  [Y]
//    [ 0.0556434 -0.2040259  1.0572252]  [Z]

    double r_aux, g_aux, b_aux;

//XYZ to linear RGB
    r_aux = ( 3.240452  * X) + (-1.5371385 * Y) + (-0.4985314 * Z);
    g_aux = (-0.9692660 * X) + ( 1.8760108 * Y) + ( 0.0415560 * Z);
    b_aux = ( 0.0556434 * X) + (-0.2040259 * Y) + ( 1.0572252 * Z);

//sRGB companding
    r_aux = sRGB_companding(r_aux);
    g_aux = sRGB_companding(g_aux);
    b_aux = sRGB_companding(b_aux);

//Scale to 255
    *r = (uint8_t)(r_aux * (double)255);
    *g = (uint8_t)(g_aux * (double)255);
    *b = (uint8_t)(b_aux * (double)255);
}

// Function to convert CIELAB to RGB
void lab_to_rgb(ColorLab lab, uint8_t *r, uint8_t *g, uint8_t *b) {
    double X, Y, Z;
    lab_to_xyz_2(lab, &X, &Y, &Z);
    xyz_to_rgb_2(X, Y, Z, r, g, b);
}


// Helper function to perform the linearization of sRGB values
double pivot_rgb(double value) {
    if (value > 0.04045) {
        return pow((value + 0.055) / 1.055, 2.4);
    } else {
        return value / 12.92;
    }
}

// Function to convert RGB to XYZ
void rgb_to_xyz(uint8_t r, uint8_t g, uint8_t b, double *X, double *Y, double *Z) {
    double R = pivot_rgb(r / 255.0);
    double G = pivot_rgb(g / 255.0);
    double B = pivot_rgb(b / 255.0);

    // Observer = 2°, Illuminant = D65
    *X = R * 0.4124564 + G * 0.3575761 + B * 0.1804375;
    *Y = R * 0.2126729 + G * 0.7151522 + B * 0.0721750;
    *Z = R * 0.0193339 + G * 0.1191920 + B * 0.9503041;
}

// Helper function to perform the pivot operation for XYZ to LAB conversion
double pivot_xyz(double value) {
    if (value > e) {
        return pow(value, 1.0 / 3.0);
    } else {
        return ((k*value) + 16.0) / 116.0;
    }
}

// Function to convert XYZ to LAB
//TODO: make ref white, k and e values as global variables.
void xyz_to_lab(double X, double Y, double Z, ColorLab *lab) {

    double fx, fy, fz;
    fx = X / refX;
    fy = Y / refY;
    fz = Z / refZ;

    fx = pivot_xyz(fx);
    fy = pivot_xyz(fy);
    fz = pivot_xyz(fz);


    lab->L = (116.0 * fy) - 16.0;
    lab->a = 500.0 * (fx - fy);
    lab->b = 200.0 * (fy - fz);
}

// Function to convert RGB to LAB
void rgb_to_lab(uint8_t r, uint8_t g, uint8_t b, ColorLab *lab) {
    double X, Y, Z;
    rgb_to_xyz(r, g, b, &X, &Y, &Z);
    xyz_to_lab(X, Y, Z, lab);
}

// Function to calculate the Euclidean distance in CIELAB color space
double lab_distance(ColorLab c1, ColorLab c2) {
    return sqrt((c1.L - c2.L) * (c1.L - c2.L) + (c1.a - c2.a) * (c1.a - c2.a) + (c1.b - c2.b) * (c1.b - c2.b));
}

double deltaE2000_distance(ColorLab c1, ColorLab c2){
    double k_L = 1.0, k_C = 1.0, k_H = 1.0;

    double deltaL_prime = c2.L - c1.L;

    double L_bar = (c1.L + c2.L) / 2.0;

    double C1 = sqrt(c1.a * c1.a + c1.b * c1.b);
    double C2 = sqrt(c2.a * c2.a + c2.b * c2.b);

    double C_bar = (C1 + C2) / 2.0;

    double a1_prime = c1.a + c1.a / 2.0 * (1 - sqrt(pow(C_bar, 7) / (pow(C_bar, 7) + pow(25.0, 7))));
    double a2_prime = c2.a + c2.a / 2.0 * (1 - sqrt(pow(C_bar, 7) / (pow(C_bar, 7) + pow(25.0, 7))));

    double C1_prime = sqrt(a1_prime * a1_prime + c1.b * c1.b);
    double C2_prime = sqrt(a2_prime * a2_prime + c2.b * c2.b);

    double C_bar_prime = (C1_prime + C2_prime) / 2.0;

    double deltaC_prime = C2_prime - C1_prime;

    double h1_prime = atan2(c1.b, a1_prime);
    if (h1_prime < 0) h1_prime += 2.0 * M_PI;

    double h2_prime = atan2(c2.b, a2_prime);
    if (h2_prime < 0) h2_prime += 2.0 * M_PI;

    double deltah_prime;
    if (fabs(h1_prime - h2_prime) <= M_PI) {
        deltah_prime = h2_prime - h1_prime;
    } else if (h2_prime <= h1_prime) {
        deltah_prime = h2_prime - h1_prime + 2.0 * M_PI;
    } else {
        deltah_prime = h2_prime - h1_prime - 2.0 * M_PI;
    }

    double deltaH_prime = 2.0 * sqrt(C1_prime * C2_prime) * sin(deltah_prime / 2.0);

    double H_bar_prime;
    if (fabs(h1_prime - h2_prime) > M_PI) {
        H_bar_prime = (h1_prime + h2_prime + 2.0 * M_PI) / 2.0;
    } else {
        H_bar_prime = (h1_prime + h2_prime) / 2.0;
    }

    double T = 1.0 - 0.17 * cos(H_bar_prime - M_PI / 6.0) +
               0.24 * cos(2.0 * H_bar_prime) +
               0.32 * cos(3.0 * H_bar_prime + M_PI / 30.0) -
               0.20 * cos(4.0 * H_bar_prime - 63.0 * M_PI / 180.0);

    double S_L = 1.0 + ((0.015 * (L_bar - 50.0) * (L_bar - 50.0)) / sqrt(20.0 + (L_bar - 50.0) * (L_bar - 50.0)));
    double S_C = 1.0 + 0.045 * C_bar_prime;
    double S_H = 1.0 + 0.015 * C_bar_prime * T;

    double deltaTheta = (30.0 * M_PI / 180.0) * exp(-pow((H_bar_prime - 275.0 * M_PI / 180.0) / (25.0 * M_PI / 180.0), 2.0));
    double R_C = 2.0 * sqrt(pow(C_bar_prime, 7.0) / (pow(C_bar_prime, 7.0) + pow(25.0, 7.0)));
    double R_T = -sin(2.0 * deltaTheta) * R_C;

    double deltaE = sqrt(
        pow(deltaL_prime / (k_L * S_L), 2.0) +
        pow(deltaC_prime / (k_C * S_C), 2.0) +
        pow(deltaH_prime / (k_H * S_H), 2.0) +
        R_T * (deltaC_prime / (k_C * S_C)) * (deltaH_prime / (k_H * S_H))
    );

    return deltaE;
}

// Function to calculate the minimum distance between any pair of selected colors
double min_distance(ColorLab *colors, int num_colors) {
    double min_dist = DBL_MAX;
    for (int i = 0; i < num_colors; i++) {
        for (int j = i + 1; j < num_colors; j++) {
            double dist = lab_distance(colors[i], colors[j]);
            if (dist < min_dist) {
                min_dist = dist;
            }
        }
    }
    return min_dist;
}

/*TODO: use https://mokole.com/palette.html method
    Start with X11 palette
    Exclude colors which are outside of max/min L value
    Create random palettes from the reamaining colors. Calculate min_distance between any two colors.
    Do this n times, keep the palette with the biggest min_distance.
*/

// Function to find a set of colors with the maximum distance in CIELAB color space using simulated annealing
void find_max_dist_colors_CIE(uint32_t num_colors, uint8_t **colors, uint32_t max_iterations) {
    ColorLab selected_colors[num_colors];

    // Generate initial random colors
    for (uint32_t i = 0; i < num_colors; i++) {
        selected_colors[i].L = rand_double(MIN_L, MAX_L);
        selected_colors[i].a = rand_double(-128, 127);
        selected_colors[i].b = rand_double(-128, 127);
    }

    double current_min_dist = min_distance(selected_colors, (int) num_colors);
    double temperature = 100.0;
    double cooling_rate = 0.99;

    for (uint32_t iter = 0; iter < max_iterations; iter++) {
        // Generate a random modification to a random color
        uint32_t index = (uint32_t)(rand() % (int)num_colors);
        ColorLab new_color = selected_colors[index];
        new_color.L += rand_double(-20, 20);
        new_color.a += rand_double(-40, 40);
        new_color.b += rand_double(-40, 40);

        // Ensure the new color is within valid bounds
        new_color.L = bound_number(new_color.L, MAX_L, MIN_L);
        new_color.a = bound_number(new_color.a, 127, -128);
        new_color.b = bound_number(new_color.b, 127, -128);

        ColorLab old_color = selected_colors[index];
        selected_colors[index] = new_color;

        double new_min_dist = min_distance(selected_colors, (int) num_colors);
        double acceptance_probability = exp((new_min_dist - current_min_dist) / temperature);

        if (new_min_dist > current_min_dist || rand_double(0, 1) < acceptance_probability) {
            current_min_dist = new_min_dist;
        } else {
            selected_colors[index] = old_color;
        }

        temperature *= cooling_rate;
    }

    // Convert the selected colors from CIELAB to RGB
    for (uint32_t i = 0; i < num_colors; i++) {
        lab_to_rgb(selected_colors[i], &colors[i][0], &colors[i][1], &colors[i][2]);
        //printf("\nSelected color %d: L:%lf A:%lf B:%lf\n", i, selected_colors[i].L,selected_colors[i].a, selected_colors[i].b);
        //printf("\nSelected color %d: 0x%02X%02X%02X\n", i, colors[i][0], colors[i][1], colors[i][2]);
    }
}
