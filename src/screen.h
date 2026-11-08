#pragma once

#include <stdint.h>

typedef uint8_t ScreenIndex;
enum {
    SCR_NONE,
    SCR_JOIN,
    SCR_PLAY,
    SCR_MAX,
};

double timestep();

void goto_screen(ScreenIndex idx);
void update_screen();
