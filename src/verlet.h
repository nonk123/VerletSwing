#pragma once

#include "vec2.h"

Fixed timestep();

typedef struct {
    Vec2 pos, old_pos, accel;
} VerletBody;

void init_verlet(VerletBody*, Vec2 pos);

void verlet(VerletBody*);
