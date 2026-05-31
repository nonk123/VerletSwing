#pragma once

#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_storage.h>

int w_width(), w_height();
extern bool left_press, right_press, tap;

double timestep(), delta();

SDL_Storage* open_user_storage();
void load_hi(), maybe_save_hi();
