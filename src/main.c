#include <stdint.h>
#include <stdio.h>

#include "controller.h"
#include "encoder.h"

#include "hardware/pwm.h"
#include "output/font.h"
#include "output/led_matrix.h"
#include "output/oled.h"
#include "pico/stdlib.h"

#define ENC_S1          6
#define ENC_S2          7
#define ENC_KEY         8

#define PWM_TOP         100
#define BRIGHTNESS_STEP 10

#define OLED_POWER      12

#define BRIGHTNESS_Y    20
#define TEXT_X          10
#define TEXT_Y          30


static void draw_data() {
    oled_clear();
    TColorController *c = color_controller();
    uint8_t current     = c->current;
    char str[128];
    snprintf(
        str, sizeof(str), "h s v=%d %d %d", c->color.h, c->color.s, c->color.v
    );
    if (current == 0) {
        str[0] = 'H';
    }
    else if (current == 1) {
        str[2] = 'S';
    }
    else if (current == 2) {
        str[4] = 'V';
    }
    font_draw_text(1, 0, str);
    bool use_cos = (c->use_cos != 0);
    snprintf(str, sizeof(str), "use cos: %c", use_cos ? 'Y' : 'N');
    if (current == 3) {
        str[0] = 'U';
    }
    font_draw_text(1, 16, str);
    if (!use_cos) {
        return;
    }
    snprintf(str, sizeof(str), "k: %.2f", c->k);
    if (current == 4) {
        str[0] = 'K';
    }
    font_draw_text(10, 32, str);
    snprintf(str, sizeof(str), "cos(k*%c)", mode2char(c->mode));
    if (current == 5) {
        str[0] = 'C';
    }
    font_draw_text(10, 48, str);
}


int main(void) {
    init_encoder(ENC_S1, ENC_S2, ENC_KEY);

    TEncoderState *encoder = get_encoder();

    gpio_init(OLED_POWER);
    gpio_set_dir(OLED_POWER, GPIO_OUT);
    gpio_put(OLED_POWER, 1);

    bool enabled      = true;
    bool screen_dirty = true;

    TColorController *color = color_controller();
    color->color            = hsv(0, 90, 90);

    oled_init();
    led_matrix_init();

    RGB_t initial_rgb = hsv2rgb(color->color);
    led_matrix_fill(initial_rgb.r, initial_rgb.g, initial_rgb.b);
    led_matrix_show();

    draw_data();
    oled_update();


    while (true) {
        process_state();

        if (encoder->key_pressed) {
            color_controller_next();
            screen_dirty = true;
        }

        if (encoder->direction != 0) {
            color_controller_change(encoder->direction);
            screen_dirty = true;
        }

        if (color->use_cos) {
            color_controller_update_cos();
            screen_dirty = true;
        }

        if (screen_dirty) {
            RGB_t color_rgb = hsv2rgb(color->color);

            if (enabled)
                led_matrix_fill(color_rgb.r, color_rgb.g, color_rgb.b);
            else
                led_matrix_fill(0, 0, 0);

            led_matrix_show();

            draw_data();
            oled_update();

            screen_dirty = false;
        }

        sleep_ms(10);
    }
}
