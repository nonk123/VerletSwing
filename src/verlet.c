#include "verlet.h"

static const Vec2 gravity = {.x = Fx0, .y = FxFrom(512)};

void verlet(VerletBody* this) {
    Vec2 sumoffucks = {Fx0, Fx0};

    if (this->f_gravity)
        sumoffucks = Vadd(sumoffucks, gravity);

    const Vec2 temp = this->pos, fuck = Vscale(sumoffucks, Fmul(timestep(), timestep()));
    this->pos = Vadd(Vsub(Vscale(temp, FxFrom(2)), this->old_pos), fuck);
    this->old_pos = temp;
}

void init_verlet(VerletBody* this, Vec2 pos) {
    this->pos = this->old_pos = pos;
}
