#include "pad_backend.h"
#include "renderer.h"
#include "tester.h"

#include <orbis/Pad.h>
#include <orbis/libkernel.h>
#include <stddef.h>
#include <stdint.h>

static int pad_handles[CONTROLLER_SLOTS] = {-1, -1, -1, -1};

int pad_backend_open(void) {
    scePadInit();
    for (unsigned slot = 0; slot < CONTROLLER_SLOTS; ++slot) {
        pad_handles[slot] = scePadOpen(0, ORBIS_PAD_PORT_TYPE_STANDARD, (int)slot, NULL);
        if (pad_handles[slot] < 0) {
            pad_handles[slot] = -1;
        }
    }
    return 0;
}

int pad_backend_poll(unsigned slot, controller_state_t *state) {
    OrbisPadData pad;

    if (slot >= CONTROLLER_SLOTS || state == NULL || pad_handles[slot] < 0) {
        return 0;
    }
    if (scePadReadState(pad_handles[slot], &pad) < 0) {
        state->connected = 0;
        return 0;
    }
    state->buttons = pad.buttons;
    state->left_x = pad.leftStick.x;
    state->left_y = pad.leftStick.y;
    state->right_x = pad.rightStick.x;
    state->right_y = pad.rightStick.y;
    state->l2 = pad.analogButtons.l2;
    state->r2 = pad.analogButtons.r2;
    state->battery = 0;
    state->connected = pad.connected != 0;
    return state->connected;
}

void pad_backend_close(void) {
    for (unsigned slot = 0; slot < CONTROLLER_SLOTS; ++slot) {
        if (pad_handles[slot] >= 0) {
            scePadClose(pad_handles[slot]);
            pad_handles[slot] = -1;
        }
    }
}

int main(void) {
    controller_state_t states[CONTROLLER_SLOTS] = {0};
    controller_state_t previous[CONTROLLER_SLOTS] = {0};
    tester_metrics_t metrics[CONTROLLER_SLOTS] = {0};

    if (renderer_open() != 0) {
        return 1;
    }
    pad_backend_open();
    for (;;) {
        for (unsigned slot = 0; slot < CONTROLLER_SLOTS; ++slot) {
            previous[slot] = states[slot];
            pad_backend_poll(slot, &states[slot]);
            tester_update(&metrics[slot], &previous[slot], &states[slot]);
        }
        renderer_draw(states, metrics);
        if (tester_should_quit(states) || renderer_poll_quit()) {
            break;
        }
        sceKernelUsleep(16666);
    }
    pad_backend_close();
    renderer_close();
    return 0;
}