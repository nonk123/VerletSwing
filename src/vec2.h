#pragma once

#include <S_fixed.h>

typedef struct {
    Fixed x, y;
} Vec2;

#define XY(_x, _y) ((Vec2){.x = (_x), .y = (_y)})

Vec2 Vadd(Vec2, Vec2), Vsub(Vec2, Vec2);
Vec2 Vscale(Vec2, Fixed);

Fixed Vlen(Vec2), Vdist(Vec2, Vec2);
