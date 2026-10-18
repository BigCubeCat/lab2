#include "led_matrix.h"

#include "ws2812.pio.h"

#include "hardware/pio.h"
#include "pico/stdlib.h"

#define LED_MATRIX_PIN 28u
#define LED_COUNT      64u
#define LED_FREQUENCY  800000.0f

static PIO pio     = pio0;
static uint sm     = 0;
static uint offset = 0;

static uint32_t pixels[LED_COUNT];

static uint32_t make_pixel(uint8_t r, uint8_t g, uint8_t b) {
    /*
     * WS2812 expects color bytes in GRB order.
     * The PIO sends each 24-bit word MSB first.
     */
    return ((uint32_t)g << 16u) | ((uint32_t)r << 8u) | (uint32_t)b;
}

void led_matrix_init(void) {
    offset = pio_add_program(pio, &ws2812_program);

    /*
     * Everything timing-related is described by ws2812.pio.
     * This generated helper configures the GPIO, sideset pin,
     * shift direction, autopull, FIFO and clock divider.
     */
    ws2812_program_init(pio, sm, offset, LED_MATRIX_PIN, LED_FREQUENCY, false);

    led_matrix_fill(0, 0, 0);
    led_matrix_show();
}

void led_matrix_fill(uint8_t r, uint8_t g, uint8_t b) {
    const uint32_t pixel = make_pixel(r, g, b);

    for (uint i = 0; i < LED_COUNT; ++i) {
        pixels[i] = pixel;
    }
}

void led_matrix_show(void) {
    for (uint i = 0; i < LED_COUNT; ++i) {
        pio_sm_put_blocking(pio, sm, pixels[i] << 8u);
    }

    /*
     * WS2812 latches the received frame after the data line
     * stays low long enough.
     */
    sleep_us(80);
}
