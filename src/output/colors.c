#include "colors.h"

const uint8_t max_whiteness = 15;
const uint8_t max_value     = 17;

const uint8_t sixth_hue       = 16;
const uint8_t third_hue       = sixth_hue * 2;
const uint8_t half_hue        = sixth_hue * 3;
const uint8_t two_thirds_hue  = sixth_hue * 4;
const uint8_t five_sixths_hue = sixth_hue * 5;
const uint8_t full_hue        = sixth_hue * 6;
const RGB_t black             = { 0, 0, 0 };

RGB_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return (RGB_t) { r, g, b };
}

HSV_t hsv(uint16_t h, uint8_t s, uint8_t v) {
    return (HSV_t) { h, s, v };
}

RGB_t hsv2rgb(HSV_t hsv) {
    float s = (float)(hsv.s) / 100.0;
    float v = (float)(hsv.v) / 100.0;

    if (s <= 0.0) {
        uint8_t value = (int)(v * 255.0f + 0.5f);
        return rgb(value, value, value);
    }

    float h_prime    = hsv.h / 60.0f;
    int sector       = (int)h_prime;
    float fractional = h_prime - sector;

    float p = v * (1.0f - s);
    float q = v * (1.0f - (s * fractional));
    float t = v * (1.0f - (s * (1.0f - fractional)));

    RGB_t result;

    switch (sector) {
    case 0:
        result.r = (uint8_t)(v * 255.0f + 0.5f);
        result.g = (uint8_t)(t * 255.0f + 0.5f);
        result.b = (uint8_t)(p * 255.0f + 0.5f);
        break;

    case 1:
        result.r = (uint8_t)(q * 255.0f + 0.5f);
        result.g = (uint8_t)(v * 255.0f + 0.5f);
        result.b = (uint8_t)(p * 255.0f + 0.5f);
        break;

    case 2:
        result.r = (uint8_t)(p * 255.0f + 0.5f);
        result.g = (uint8_t)(v * 255.0f + 0.5f);
        result.b = (uint8_t)(t * 255.0f + 0.5f);
        break;

    case 3:
        result.r = (uint8_t)(p * 255.0f + 0.5f);
        result.g = (uint8_t)(q * 255.0f + 0.5f);
        result.b = (uint8_t)(v * 255.0f + 0.5f);
        break;

    case 4:
        result.r = (uint8_t)(t * 255.0f + 0.5f);
        result.g = (uint8_t)(p * 255.0f + 0.5f);
        result.b = (uint8_t)(v * 255.0f + 0.5f);
        break;

    default:
        result.r = (uint8_t)(v * 255.0f + 0.5f);
        result.g = (uint8_t)(p * 255.0f + 0.5f);
        result.b = (uint8_t)(q * 255.0f + 0.5f);
        break;
    }
    return result;
}
