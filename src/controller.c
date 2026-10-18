#include "controller.h"

#include <math.h>

#include "pico/stdlib.h"


char mode2char(ESelectedMode mode) {
    switch (mode) {
    case HUE:
        return 'H';
    case SATURATION:
        return 'S';
    case VALUE:
    default:
        return 'V';
    }
}

TColorController *color_controller() {
    static TColorController controller;
    static int init = 0;
    if (init == 0) {
        controller.current = 0;
        controller.k       = 1.0;
        controller.use_cos = 0;
        controller.mode    = HUE;
        init               = 1;
    }
    return &controller;
}

void color_controller_next() {
    TColorController *c = color_controller();
    int loop            = (c->use_cos == 0) ? 4 : 6;
    c->current          = ((c->current) + 1) % loop;
}

void color_controller_change(int direction) {
    if (direction == 0) {
        return;
    }
    TColorController *c = color_controller();
    int tmp;
    switch (c->current) {
        // меняем HUE
    case 0: {
        tmp = (int)(c->color.h + direction);
        if (tmp <= 0) {
            c->color.h = 0;
        }
        else {
            c->color.h = tmp % 360;
        }
        break;
    }
    // SATURATION
    case 1: {
        tmp = (int)(c->color.s + direction);
        if (tmp <= 0) {
            c->color.s = 0;
        }
        else {
            c->color.s = tmp % 101;
        }
        break;
    }
        // VALUE
    case 2: {
        tmp = (int)(c->color.v + direction);
        if (tmp <= 0) {
            c->color.v = 0;
        }
        else {
            c->color.v = tmp % 101;
        }
        break;
    }
    case 3: {
        c->use_cos = !c->use_cos;
        break;
    }
    case 4: {
        c->k += 0.01 * direction;
        if (c->k < 0) {
            c->k = 0;
        }
        else if (c->k > 100) {
            c->k = 100;
        }
        break;
    }
    case 5: {
        c->mode = (c->mode + direction + 3) % 3;
        break;
    }
    default:
        break;
    }
}


void color_controller_update_cos() {
    TColorController *c = color_controller();

    if (!c->use_cos)
        return;

    float t     = get_absolute_time() / 1000000.0f;
    float value = cosf(c->k * t);

    switch (c->mode) {
    case HUE:
        c->color.h = (uint16_t)((value + 1.0f) * 180.0f);
        break;

    case SATURATION:
        c->color.s = (uint8_t)((value + 1.0f) * 50.0f);
        break;

    case VALUE:
        c->color.v = (uint8_t)((value + 1.0f) * 50.0f);
        break;
    }
}
