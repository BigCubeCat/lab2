#include "encoder.h"

#include <string.h>

#include "encoder.pio.h"

#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"

static int32_t encoder_read_count(encoder_t *encoder) {
    uint32_t value = 0;
    int remaining;

    if (encoder == NULL || !encoder->initialized) {
        return 0;
    }

    /* Drain queued snapshots and then read one fresh snapshot, as in the
       official Pico quadrature encoder example. */
    remaining = (int)pio_sm_get_rx_fifo_level(encoder->pio, encoder->sm) + 1;
    while (remaining > 0) {
        value = pio_sm_get_blocking(encoder->pio, encoder->sm);
        --remaining;
    }

    return (int32_t)value;
}

bool encoder_init(
    encoder_t *encoder, uint pin_a, uint key_pin, PIO pio, uint sm
) {
    pio_sm_config config;

    if (encoder == NULL || pio == NULL || sm >= 4U || pin_a > 28U
        || key_pin > 29U || key_pin == pin_a || key_pin == pin_a + 1U) {
        return false;
    }

    memset(encoder, 0, sizeof(*encoder));

    /* The PIO program uses computed jumps, so it must occupy address zero.
       Do not silently overwrite another program at that location. */
    if (pio_sm_is_claimed(pio, sm)
        || !pio_can_add_program_at_offset(
            pio, &quadrature_encoder_program, 0U
        )) {
        return false;
    }

    pio_sm_claim(pio, sm);
    encoder->offset =
        pio_add_program_at_offset(pio, &quadrature_encoder_program, 0U);
    encoder->pio     = pio;
    encoder->sm      = sm;
    encoder->pin_a   = pin_a;
    encoder->key_pin = key_pin;

    /* S1 and S2 must be adjacent GPIOs because the PIO program reads 2 pins. */
    pio_gpio_init(pio, pin_a);
    pio_gpio_init(pio, pin_a + 1U);
    pio_sm_set_consecutive_pindirs(pio, sm, pin_a, 2U, false);
    gpio_pull_up(pin_a);
    gpio_pull_up(pin_a + 1U);

    /* KEY is a separate active-low GPIO button. */
    gpio_init(key_pin);
    gpio_set_dir(key_pin, GPIO_IN);
    gpio_pull_up(key_pin);

    config = quadrature_encoder_program_get_default_config(encoder->offset);
    sm_config_set_in_pins(&config, pin_a);
    sm_config_set_jmp_pin(&config, pin_a);
    sm_config_set_in_shift(&config, false, false, 32U);
    sm_config_set_fifo_join(&config, PIO_FIFO_JOIN_NONE);
    sm_config_set_clkdiv(&config, 1.0f);

    pio_sm_init(pio, sm, encoder->offset, &config);
    encoder->initialized = true;
    pio_sm_set_enabled(pio, sm, true);

    /* Synchronize software accounting with the current PIO counter. */
    encoder->last_count    = encoder_read_count(encoder);
    encoder->partial_steps = 0;

    encoder->button_candidate  = !gpio_get(key_pin);
    encoder->button_stable     = encoder->button_candidate;
    encoder->pressed_event     = false;
    encoder->button_changed_ms = to_ms_since_boot(get_absolute_time());

    return true;
}

void encoder_deinit(encoder_t *encoder) {
    if (encoder == NULL || !encoder->initialized) {
        return;
    }

    pio_sm_set_enabled(encoder->pio, encoder->sm, false);
    pio_sm_unclaim(encoder->pio, encoder->sm);
    pio_remove_program(
        encoder->pio, &quadrature_encoder_program, encoder->offset
    );
    encoder->initialized = false;
}

int encoder_get_delta(encoder_t *encoder) {
    int32_t current;
    int32_t delta;
    int detents = 0;

    if (encoder == NULL || !encoder->initialized) {
        return 0;
    }

    current = encoder_read_count(encoder);
    /* Unsigned subtraction gives the intended modulo-2^32 difference when
       the PIO's 32-bit counter wraps around. */
    delta = (int32_t)((uint32_t)current - (uint32_t)encoder->last_count);
    encoder->last_count = current;
    encoder->partial_steps += delta;

    /* A common mechanical encoder has four quadrature transitions per click. */
    while (encoder->partial_steps >= 4) {
        ++detents;
        encoder->partial_steps -= 4;
    }
    while (encoder->partial_steps <= -4) {
        --detents;
        encoder->partial_steps += 4;
    }

    return detents;
}

bool encoder_was_pressed(encoder_t *encoder) {
    const uint32_t debounce_ms = 25U;
    uint32_t now;
    bool raw_pressed;

    if (encoder == NULL || !encoder->initialized) {
        return false;
    }

    now         = to_ms_since_boot(get_absolute_time());
    raw_pressed = !gpio_get(encoder->key_pin);

    if (raw_pressed != encoder->button_candidate) {
        encoder->button_candidate  = raw_pressed;
        encoder->button_changed_ms = now;
    }

    if ((uint32_t)(now - encoder->button_changed_ms) >= debounce_ms
        && encoder->button_stable != encoder->button_candidate) {
        encoder->button_stable = encoder->button_candidate;
        if (encoder->button_stable) {
            encoder->pressed_event = true;
        }
    }

    if (encoder->pressed_event) {
        encoder->pressed_event = false;
        return true;
    }

    return false;
}
