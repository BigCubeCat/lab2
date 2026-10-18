#ifndef ENCODER_H
#define ENCODER_H

#include <stdbool.h>
#include <stdint.h>

#include "hardware/pio.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct encoder {
        PIO pio;
        uint sm;
        uint offset;
        uint pin_a;
        uint key_pin;
        bool initialized;

        int32_t last_count;
        int32_t partial_steps;

        bool button_candidate;
        bool button_stable;
        bool pressed_event;
        uint32_t button_changed_ms;
    } encoder_t;

    /*
     * Initialize a quadrature encoder.
     *
     * pin_a is S1/CLK; S2/DT must be connected to pin_a + 1.
     * key_pin is the push-button KEY input (active low).
     * The example wiring is S1=GP6, S2=GP7, KEY=GP8.
     *
     * The quadrature PIO program must be loaded at instruction offset 0 and
     * needs one free state machine. Returns true on success, false on
     * invalid/conflicting PIO resources or invalid pins.
     */
    bool encoder_init(
        encoder_t *encoder, uint pin_a, uint key_pin, PIO pio, uint sm
    );

    /* Stop the state machine, release it, and remove the loaded PIO program. */
    void encoder_deinit(encoder_t *encoder);

    /* Returns signed whole detents/clicks since the previous call (usually 4
     * PIO transitions per detent). Direction depends on S1/S2 wiring order. */
    int encoder_get_delta(encoder_t *encoder);

    /* Returns true once for each debounced KEY press. KEY is active low. */
    bool encoder_was_pressed(encoder_t *encoder);

#ifdef __cplusplus
}
#endif

#endif /* ENCODER_H */
