#include "tester.h"

#include <orbis/Pad.h>
#include <stddef.h>

int tester_axis_is_centered(uint8_t value) {
    return value >= 120 && value <= 136;
}

void tester_update(tester_metrics_t *metrics,
                   const controller_state_t *previous,
                   const controller_state_t *current) {
    if (metrics == NULL || previous == NULL || current == NULL) {
        return;
    }
    metrics->previous_buttons = previous->buttons;
    metrics->pressed_buttons = current->buttons & ~previous->buttons;
    metrics->released_buttons = previous->buttons & ~current->buttons;
    metrics->frames++;
    if (metrics->pressed_buttons != 0 || metrics->released_buttons != 0) {
        metrics->changed_frames++;
    }
    if (!tester_axis_is_centered(current->left_x) ||
        !tester_axis_is_centered(current->left_y) ||
        !tester_axis_is_centered(current->right_x) ||
        !tester_axis_is_centered(current->right_y)) {
        metrics->drift_frames++;
    }
    if (current->l2 > metrics->peak_l2) {
        metrics->peak_l2 = current->l2;
    }
    if (current->r2 > metrics->peak_r2) {
        metrics->peak_r2 = current->r2;
    }
}

int tester_should_quit(const controller_state_t *states) {
    for (unsigned slot = 0; slot < CONTROLLER_SLOTS; ++slot) {
        if (states[slot].connected &&
            (states[slot].buttons & ORBIS_PAD_BUTTON_OPTIONS) != 0) {
            return 1;
        }
    }
    return 0;
}