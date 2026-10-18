#pragma once

#include <stdint.h>

void led_matrix_init(void);

void led_matrix_fill(
    uint8_t r,
    uint8_t g,
    uint8_t b
);

void led_matrix_show(void);

