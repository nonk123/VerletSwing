#pragma once

#include "vec2.h"

Fixed timestep();

typedef struct {
    Vec2 pos, old_pos;
    uint8_t f_gravity : 1;
} VerletBody;

void init_verlet(VerletBody*, Vec2 pos);

void verlet(VerletBody*);
