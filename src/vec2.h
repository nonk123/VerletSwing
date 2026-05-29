#pragma once

typedef struct {
    double x, y;
} Vec2;

#define XY(_x, _y) ((Vec2){.x = (_x), .y = (_y)})

Vec2 Vadd(Vec2, Vec2), Vsub(Vec2, Vec2);
Vec2 Vscale(Vec2, double);

double Vlen(Vec2), Vdist(Vec2, Vec2);
Vec2 Vnorm(Vec2);
