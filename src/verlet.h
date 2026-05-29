#pragma once

#include <SDL3/SDL_stdinc.h>

#include "vec2.h"

double timestep();

typedef struct {
    Vec2 pos, old_pos, instant_impulse;
    uint8_t f_gravity : 1;
} VerletBody;

void init_verlet(VerletBody*, Vec2 pos);
void push(VerletBody*, Vec2 amount);

void verlet(VerletBody*);
