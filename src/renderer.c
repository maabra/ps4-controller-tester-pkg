#include "renderer.h"

#include <SDL.h>
#include <orbis/Pad.h>
#include <stddef.h>

#define UI_WIDTH 1280
#define UI_HEIGHT 720
#define PANEL_W 600
#define PANEL_H 290

static SDL_Window *window;
static SDL_Renderer *renderer;

static const uint8_t font[37][7] = {
    {14,17,17,31,17,17,17},{30,17,17,30,17,17,30},{14,17,16,16,16,17,14},
    {30,17,17,17,17,17,30},{31,16,16,30,16,16,31},{31,16,16,30,16,16,16},
    {14,17,16,23,17,17,15},{17,17,17,31,17,17,17},{31,4,4,4,4,4,31},
    {1,1,1,1,17,17,14},{17,18,20,24,20,18,17},{16,16,16,16,16,16,31},
    {17,27,21,21,17,17,17},{17,25,21,19,17,17,17},{14,17,17,17,17,17,14},
    {30,17,17,30,16,16,16},{14,17,17,17,21,18,13},{30,17,17,30,20,18,17},
    {15,16,16,14,1,1,30},{31,4,4,4,4,4,4},{17,17,17,17,17,17,14},
    {17,17,17,17,17,10,4},{17,17,17,21,21,21,10},{17,17,10,4,10,17,17},
    {17,17,10,4,4,4,4},{31,1,2,4,8,16,31},
    {0,14,17,1,14,16,31},{0,14,17,23,25,17,14},{0,30,17,30,17,17,30},
    {0,14,16,16,16,16,14},{0,30,17,17,17,17,30},{0,31,16,30,16,16,31},
    {0,14,16,22,17,17,15},{0,17,17,31,17,17,17},{0,31,4,4,4,4,31},
    {0,1,1,1,1,17,14}
};

static int glyph_index(char character) {
    if (character >= 'A' && character <= 'Z') return character - 'A';
    if (character >= '0' && character <= '9') return 26 + character - '0';
    return -1;
}

static void text(const char *value, int x, int y, int scale, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    for (; *value != '\0'; ++value, x += 6 * scale) {
        int index = glyph_index(*value);
        if (index < 0) continue;
        for (int row = 0; row < 7; ++row) {
            for (int column = 0; column < 5; ++column) {
                if ((font[index][row] & (1 << (4 - column))) != 0) {
                    SDL_Rect pixel = {x + column * scale, y + row * scale, scale, scale};
                    SDL_RenderFillRect(renderer, &pixel);
                }
            }
        }
    }
}

static void bar(int x, int y, int width, int value, SDL_Color color) {
    SDL_Rect background = {x, y, width, 12};
    SDL_Rect fill = {x, y, width * value / 255, 12};
    SDL_SetRenderDrawColor(renderer, 22, 31, 43, 255);
    SDL_RenderFillRect(renderer, &background);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    SDL_RenderFillRect(renderer, &fill);
}

static void draw_stick(int x, int y, uint8_t axis_x, uint8_t axis_y) {
    SDL_SetRenderDrawColor(renderer, 34, 47, 62, 255);
    SDL_RenderDrawLine(renderer, x - 42, y, x + 42, y);
    SDL_RenderDrawLine(renderer, x, y - 42, x, y + 42);
    SDL_SetRenderDrawColor(renderer, 75, 211, 180, 255);
    SDL_Rect dot = {x + ((int)axis_x - 128) * 36 / 128 - 5,
                    y + ((int)axis_y - 128) * 36 / 128 - 5, 10, 10};
    SDL_RenderFillRect(renderer, &dot);
}

static void button(int x, int y, int active, SDL_Color color) {
    SDL_Rect box = {x, y, 26, 26};
    SDL_SetRenderDrawColor(renderer, active ? color.r : 34, active ? color.g : 47,
                           active ? color.b : 62, 255);
    SDL_RenderFillRect(renderer, &box);
}

int renderer_open(void) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) return -1;
    window = SDL_CreateWindow("PS4 Controller Tester", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, UI_WIDTH, UI_HEIGHT,
                              SDL_WINDOW_FULLSCREEN);
    if (window == NULL) return -1;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (renderer == NULL) return -1;
    SDL_RenderSetLogicalSize(renderer, UI_WIDTH, UI_HEIGHT);
    return 0;
}

void renderer_draw(const controller_state_t *states,
                   const tester_metrics_t *metrics) {
    SDL_Color white = {235, 242, 248, 255};
    SDL_Color muted = {137, 157, 177, 255};
    SDL_Color green = {75, 211, 180, 255};
    SDL_Color orange = {255, 180, 84, 255};

    SDL_SetRenderDrawColor(renderer, 10, 16, 24, 255);
    SDL_RenderClear(renderer);
    text("PS4 CONTROLLER TESTER", 42, 26, 4, white);
    text("FIRMWARE 11 02  /  FOUR SLOT DIAGNOSTICS", 45, 67, 2, muted);
    for (unsigned slot = 0; slot < CONTROLLER_SLOTS; ++slot) {
        int x = 32 + (slot % 2) * 616;
        int y = 105 + (slot / 2) * 310;
        const controller_state_t *state = &states[slot];
        const tester_metrics_t *stat = &metrics[slot];
        SDL_SetRenderDrawColor(renderer, 18, 27, 38, 255);
        SDL_Rect panel = {x, y, PANEL_W, PANEL_H};
        SDL_RenderFillRect(renderer, &panel);
        SDL_SetRenderDrawColor(renderer, state->connected ? green.r : 62,
                               state->connected ? green.g : 76,
                               state->connected ? green.b : 91, 255);
        SDL_Rect edge = {x, y, PANEL_W, 4};
        SDL_RenderFillRect(renderer, &edge);
        text(slot == 0 ? "CONTROLLER 1" : slot == 1 ? "CONTROLLER 2" :
             slot == 2 ? "CONTROLLER 3" : "CONTROLLER 4", x + 20, y + 18, 2, white);
        text(state->connected ? "CONNECTED" : "NOT CONNECTED", x + 20, y + 47, 2,
             state->connected ? green : muted);
        text("L STICK", x + 22, y + 89, 1, muted);
        text("R STICK", x + 150, y + 89, 1, muted);
        draw_stick(x + 52, y + 140, state->left_x, state->left_y);
        draw_stick(x + 180, y + 140, state->right_x, state->right_y);
        text("L2", x + 270, y + 90, 1, muted);
        text("R2", x + 270, y + 141, 1, muted);
        bar(x + 310, y + 91, 150, state->l2, orange);
        bar(x + 310, y + 142, 150, state->r2, orange);
           text("BUTTONS", x + 270, y + 188, 1, muted);
           button(x + 350, y + 180, state->buttons & ORBIS_PAD_BUTTON_CROSS, green);
           button(x + 382, y + 180, state->buttons & ORBIS_PAD_BUTTON_CIRCLE, green);
           button(x + 414, y + 180, state->buttons & ORBIS_PAD_BUTTON_SQUARE, green);
           button(x + 446, y + 180, state->buttons & ORBIS_PAD_BUTTON_TRIANGLE, green);
           button(x + 350, y + 214, state->buttons & ORBIS_PAD_BUTTON_UP, green);
           button(x + 382, y + 214, state->buttons & ORBIS_PAD_BUTTON_DOWN, green);
           button(x + 414, y + 214, state->buttons & ORBIS_PAD_BUTTON_LEFT, green);
           button(x + 446, y + 214, state->buttons & ORBIS_PAD_BUTTON_RIGHT, green);
           button(x + 478, y + 180, state->buttons & ORBIS_PAD_BUTTON_L1, orange);
           button(x + 510, y + 180, state->buttons & ORBIS_PAD_BUTTON_R1, orange);
           button(x + 478, y + 214, state->buttons & ORBIS_PAD_BUTTON_L3, orange);
           button(x + 510, y + 214, state->buttons & ORBIS_PAD_BUTTON_R3, orange);
           text(stat->drift_frames ? "DRIFT MOVE" : "DRIFT OK", x + 20,
               y + 250, 1, stat->drift_frames ? orange : green);
    }
    text("OPTIONS  QUIT", 1080, 26, 2, muted);
    SDL_RenderPresent(renderer);
}

int renderer_poll_quit(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) return 1;
    }
    return 0;
}

void renderer_close(void) {
    if (renderer != NULL) SDL_DestroyRenderer(renderer);
    if (window != NULL) SDL_DestroyWindow(window);
    SDL_Quit();
}