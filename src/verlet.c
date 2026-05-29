#include "verlet.h"

static const Vec2 gravity = {.x = 0.0, .y = 512.0};

void push(VerletBody* this, Vec2 amount) {
    this->instant_impulse = Vadd(this->instant_impulse, amount);
}

void verlet(VerletBody* this) {
    Vec2 sumoffucks = {0.0, 0.0};

    sumoffucks = Vadd(sumoffucks, this->instant_impulse);
    this->instant_impulse = XY(0.0, 0.0);

    if (this->f_gravity)
        sumoffucks = Vadd(sumoffucks, gravity);

    const Vec2 temp = this->pos, fuck = Vscale(sumoffucks, timestep() * timestep());
    this->pos = Vadd(Vsub(Vscale(temp, 2.0), this->old_pos), fuck);
    this->old_pos = temp;
}

void init_verlet(VerletBody* this, Vec2 pos) {
    this->pos = this->old_pos = pos;
    this->instant_impulse = XY(0.0, 0.0);
}
