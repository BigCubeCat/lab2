#include "encoder.h"

#include <hardware/gpio.h>

TEncoderState *get_encoder() {
    static TEncoderState state;
    return &state;
}

void setup_gpio(int gpio) {
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_IN);
    gpio_pull_up(gpio);
}

void init_encoder(int gpio_s1, int gpio_s2, int gpio_key) {
    TEncoderState *state = get_encoder();

    state->direction   = 0;
    state->key_pressed = false;

    state->gpio_key = gpio_key;
    state->gpio_s1  = gpio_s1;
    state->gpio_s2  = gpio_s2;

    setup_gpio(state->gpio_key);
    setup_gpio(state->gpio_s1);
    setup_gpio(state->gpio_s2);

    state->states.delta = 0;

    state->states.prev =
        (gpio_get(state->gpio_s1) << 1) | gpio_get(state->gpio_s2);
}

void process_state() {
    TEncoderState *state = get_encoder();

    state->direction   = 0;
    state->key_pressed = false;

    state->states.next =
        (gpio_get(state->gpio_s1) << 1) | gpio_get(state->gpio_s2);

    if (state->states.next != state->states.prev) {
        if ((state->states.prev == 0b00 && state->states.next == 0b01)
            || (state->states.prev == 0b01 && state->states.next == 0b11)
            || (state->states.prev == 0b11 && state->states.next == 0b10)
            || (state->states.prev == 0b10 && state->states.next == 0b00)) {
            state->states.delta++;
        }
        else if (
            (state->states.prev == 0b00 && state->states.next == 0b10)
            || (state->states.prev == 0b10 && state->states.next == 0b11)
            || (state->states.prev == 0b11 && state->states.next == 0b01)
            || (state->states.prev == 0b01 && state->states.next == 0b00)
        ) {
            state->states.delta--;
        }

        state->states.prev = state->states.next;
        // Полный щелчок энкодера
        if (state->states.delta >= 4) {
            state->states.delta = 0;
            state->direction    = -1;
        }
        else if (state->states.delta <= -4) {
            state->states.delta = 0;
            state->direction    = 1;
        }
    }

    // Кнопка:
    // 1 = отпущена
    // 0 = нажата
    static bool previous_key = true;

    bool current_key = gpio_get(state->gpio_key);
    // Событие только на переходе 1 -> 0
    if (previous_key && !current_key) {
        state->key_pressed = true;
    }
    previous_key = current_key;
}
