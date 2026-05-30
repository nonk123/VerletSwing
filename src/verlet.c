#include "verlet.h"

static const Vec2 gravity = {.x = 0.0, .y = 768.0};

void push(VerletBody* this, Vec2 amount) {
    this->pos = Vadd(this->pos, Vscale(amount, timestep()));
}

void verlet(VerletBody* this) {
    Vec2 sumoffucks = XY(0.0, 0.0);

    if (this->f_gravity)
        sumoffucks = Vadd(sumoffucks, gravity);

    const Vec2 temp = this->pos, fuck = Vscale(sumoffucks, timestep() * timestep());
    this->pos = Vadd(Vsub(Vscale(temp, 2.0), this->old_pos), fuck);
    this->old_pos = temp;
}

void init_verlet(VerletBody* this, Vec2 pos) {
    this->pos = this->old_pos = pos;
}
