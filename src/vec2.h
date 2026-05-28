#pragma once

#include <S_fixed.h>

typedef struct {
    Fixed x, y;
} Vec2;

Vec2 Vadd(Vec2, Vec2), Vsub(Vec2, Vec2);
Vec2 Vscale(Vec2, Fixed);
