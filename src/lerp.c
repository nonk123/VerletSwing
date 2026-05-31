#include "lerp.h"

double lerp(double a, double b, double t) {
    return a + (b - a) * t;
}

double lerpin(double a, double b, double t) {
    return lerp(a, b, t * t);
}

double lerpin3(double a, double b, double t) {
    return lerp(a, b, t * t * t);
}

double lerpout(double a, double b, double t) {
    return lerp(a, b, t * (2.0 - t));
}

SDL_Color lerp_color(SDL_Color from, SDL_Color to, double f) {
    from.r = (uint8_t)((double)from.r + (double)(to.r - from.r) * f);
    from.g = (uint8_t)((double)from.g + (double)(to.g - from.g) * f);
    from.b = (uint8_t)((double)from.b + (double)(to.b - from.b) * f);
    return from;
}
