#pragma once

#include <SDL3/SDL_pixels.h>

double lerp(double a, double b, double t);
double lerpin(double a, double b, double t);
double lerpout(double a, double b, double t);

SDL_Color lerp_color(SDL_Color a, SDL_Color b, double t);
