#pragma once

#include <stdint.h>

#define OLED_WIDTH  128
#define OLED_HEIGHT 64

void oled_init(void);

void oled_clear(void);
void oled_update(void);

void oled_draw_pixel(int x, int y, uint8_t value);

void oled_draw_char(int x, int y, char c);

void oled_draw_string(int x, int y, const char *text);
