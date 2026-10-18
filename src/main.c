#include <stdbool.h>
#include <stdint.h>

#include "encoder.h"

#include "hardware/pio.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"

#define LED_PIN            12U
#define ENCODER_S1_PIN     6U
#define ENCODER_KEY_PIN    8U
#define PWM_TOP            255U
#define BRIGHTNESS_STEP    10
#define INITIAL_BRIGHTNESS 50

static int brightness = INITIAL_BRIGHTNESS;
static uint pwm_slice;

static void set_brightness(int value) {
    if (value < 0) {
        value = 0;
    }
    else if (value > 100) {
        value = 100;
    }

    brightness = value;
    pwm_set_chan_level(
        pwm_slice,
        pwm_gpio_to_channel(LED_PIN),
        (uint16_t)((brightness * (int)PWM_TOP) / 100)
    );
}

static void init_led_pwm(void) {
    pwm_config config;

    gpio_set_function(LED_PIN, GPIO_FUNC_PWM);
    pwm_slice = pwm_gpio_to_slice_num(LED_PIN);

    config = pwm_get_default_config();
    pwm_config_set_wrap(&config, PWM_TOP);
    pwm_config_set_clkdiv(&config, 4.0f);
    pwm_init(pwm_slice, &config, true);

    set_brightness(brightness);
}

int main(void) {
    encoder_t encoder = { 0 };

    stdio_init_all();
    init_led_pwm();

    if (!encoder_init(&encoder, ENCODER_S1_PIN, ENCODER_KEY_PIN, pio0, 0U)) {
        /* Initialization failed, for example because PIO0 instruction address
           zero or state machine 0 is already in use. */
        while (true) {
            tight_loop_contents();
        }
    }

    while (true) {
        const int delta = encoder_get_delta(&encoder);
        if (delta != 0) {
            set_brightness(brightness + delta * BRIGHTNESS_STEP);
        }

        if (encoder_was_pressed(&encoder)) {
            set_brightness(INITIAL_BRIGHTNESS);
        }

        sleep_ms(1);
    }

    /* Unreachable in this example, but keeps the teardown API demonstrated. */
    encoder_deinit(&encoder);
    return 0;
}
