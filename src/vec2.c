#include <SDL3/SDL_stdinc.h>

#include "vec2.h"

Vec2 Vadd(Vec2 a, Vec2 b) {
    a.x += b.x, a.y += b.y;
    return a;
}

Vec2 Vsub(Vec2 a, Vec2 b) {
    a.x -= b.x, a.y -= b.y;
    return a;
}

Vec2 Vscale(Vec2 a, double s) {
    a.x *= s, a.y *= s;
    return a;
}

double Vlen(Vec2 v) {
    v.x = SDL_fabs(v.x), v.y = SDL_fabs(v.y);
    const double min = SDL_min(v.x, v.y), max = SDL_max(v.x, v.y);

    if (max <= 1e-3)
        return 0.0;

    const double r = min / max;
    return max * SDL_sqrt(1.0 + r * r);
}

double Vdist(Vec2 a, Vec2 b) {
    return Vlen(Vsub(b, a));
}
