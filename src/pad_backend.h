#ifndef PAD_BACKEND_H
#define PAD_BACKEND_H

#include <stdint.h>

#define CONTROLLER_SLOTS 4

typedef struct {
    uint32_t buttons;
    uint8_t left_x;
    uint8_t left_y;
    uint8_t right_x;
    uint8_t right_y;
    uint8_t l2;
    uint8_t r2;
    uint8_t battery;
    uint8_t connected;
} controller_state_t;

int pad_backend_open(void);
int pad_backend_poll(unsigned slot, controller_state_t *state);
void pad_backend_close(void);

#endif