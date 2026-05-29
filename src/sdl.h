#pragma once

#include <SDL3/SDL_stdinc.h>

#include "vec2.h"

int w_width(), w_height();

Vec2 mouse_pos_v();
void mouse_pos(double*, double*);

bool is_pressed();
