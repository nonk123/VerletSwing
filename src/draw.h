#pragma once

#include <SDL3/SDL_render.h>

#include "vec2.h"

#define RGB(_r, _g, _b) ((SDL_Color){.r = (_r), .g = (_g), .b = (_b), .a = SDL_ALPHA_OPAQUE})

bool sdl_load_font();

Vec2 camera_pos();
void set_camera_target(Vec2);
void update_camera();

void fill_square(Vec2, double, SDL_Color);

double text_width(double, const char*);
void draw_text(Vec2, double, const char*);
