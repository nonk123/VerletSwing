#pragma once

#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_storage.h>

int w_width(), w_height();
bool is_left_pressed(), is_right_pressed(), just_tapped();

double timestep(), delta();

SDL_Storage* open_user_storage();
