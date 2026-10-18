#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct States {
    uint8_t prev;
    uint8_t next;
    int delta;
} TStates;

typedef struct EncoderState {
    int gpio_s1;
    int gpio_s2;
    int gpio_key;
    /// Направление: 1 для увеличения, -1 для уменьшения, 0 не менять
    int direction;
    bool key_pressed;
    TStates states;
} TEncoderState;

TEncoderState *get_encoder();

void init_encoder(int gpio_s1, int gpio_s2, int gpio_key);

void process_state();
