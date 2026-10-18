#pragma once

#include <stdint.h>

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGB_t;

typedef struct {
    uint16_t h;
    uint8_t s;
    uint8_t v;
} HSV_t;

RGB_t rgb(uint8_t r, uint8_t g, uint8_t b);

HSV_t hsv(uint16_t h, uint8_t s, uint8_t v);

RGB_t hsv2rgb(HSV_t hsv);
