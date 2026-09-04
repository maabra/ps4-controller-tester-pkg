#ifndef RENDERER_H
#define RENDERER_H

#include "pad_backend.h"
#include "tester.h"

int renderer_open(void);
void renderer_draw(const controller_state_t *states,
                   const tester_metrics_t *metrics);
int renderer_poll_quit(void);
void renderer_close(void);

#endif