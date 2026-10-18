#pragma once

#include <stdint.h>

#include "output/colors.h"

typedef enum SelectedMode {
    HUE        = 0,
    SATURATION = 1,
    VALUE      = 2,
} ESelectedMode;

typedef struct ColorController {
    HSV_t color;
    uint8_t current;
    int use_cos;
    float k;
    ESelectedMode mode;
} TColorController;

char mode2char(ESelectedMode mode);

TColorController *color_controller();

void color_controller_next();

void color_controller_change(int direction);

void color_controller_update_cos();
