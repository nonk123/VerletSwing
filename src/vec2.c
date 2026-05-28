#include "vec2.h"

Vec2 Vadd(Vec2 a, Vec2 b) {
    a.x = Fadd(a.x, b.x);
    a.y = Fadd(a.y, b.y);
    return a;
}

Vec2 Vsub(Vec2 a, Vec2 b) {
    a.x = Fsub(a.x, b.x);
    a.y = Fsub(a.y, b.y);
    return a;
}

Vec2 Vscale(Vec2 a, Fixed s) {
    a.x = Fmul(a.x, s);
    a.y = Fmul(a.y, s);
    return a;
}
