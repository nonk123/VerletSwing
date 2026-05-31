#pragma once

#include <SDL3/SDL_stdinc.h>

typedef enum {
    MEN_NULL,
    MEN_MAIN,
    MEN_GAME,
    MEN_MAX,
} Menu;

Menu cur_menu();
void push_menu(Menu), pop_menu();
void tick_menu();

extern bool chicken_dinner;
