#pragma once

#include <SDL3/SDL_stdinc.h>

extern uint64_t score, hiscore;
extern bool chicken_dinner;

void restart(), update(), draw_game();
