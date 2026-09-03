#ifndef TESTER_H
#define TESTER_H

#include "pad_backend.h"

#define TESTER_BUTTON_COUNT 18

typedef struct {
    uint32_t previous_buttons;
    uint32_t pressed_buttons;
    uint32_t released_buttons;
    unsigned frames;
    unsigned changed_frames;
    unsigned drift_frames;
    unsigned peak_l2;
    unsigned peak_r2;
} tester_metrics_t;

void tester_update(tester_metrics_t *metrics,
                   const controller_state_t *previous,
                   const controller_state_t *current);
int tester_axis_is_centered(uint8_t value);
int tester_should_quit(const controller_state_t *states);

#endif