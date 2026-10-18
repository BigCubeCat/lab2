#pragma once

#include <stdbool.h>
#include <stdint.h>

#define FONT_HEIGHT      10
#define FONT_FIRST_GLYPH 0x0020
#define FONT_LAST_GLYPH  0x0451

// x/y denote the top-left corner of the glyph's 10 px cell.
void font_draw_char(int x, int y, uint32_t codepoint);

// UTF-8 string renderer. Unsupported codepoints are skipped.
void font_draw_text(int x, int y, const char *text);

// Returns the horizontal advance in pixels for one UTF-8 string.
int font_text_width(const char *text);
