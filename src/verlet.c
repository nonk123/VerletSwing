#include "verlet.h"

#define MAX_VEL (176.0)
#define GRAVITY XY(0.0, 768.0)

void push(VerletBody* this, Vec2 amount) {
    this->pos = Vadd(this->pos, Vscale(amount, timestep()));
}

void verlet(VerletBody* this) {
    Vec2 sumoffucks = XY(0.0, 0.0);

    if (this->f_gravity)
        sumoffucks = Vadd(sumoffucks, GRAVITY);

    const Vec2 temp = this->pos;
    const Vec2 fuck = Vscale(sumoffucks, timestep() * timestep());

    Vec2 vel = Vsub(temp, this->old_pos);

    if (Vlen(vel) > MAX_VEL)
        vel = Vscale(Vnorm(vel), MAX_VEL);

    this->pos = Vadd(temp, Vadd(vel, fuck));
    this->old_pos = temp;
}

void init_verlet(VerletBody* this, Vec2 pos) {
    this->pos = this->old_pos = pos;
}
