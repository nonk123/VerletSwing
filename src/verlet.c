#include "verlet.h"

static const Vec2 gravity = {.x = Fx0, .y = FxFrom(178)};

void verlet(VerletBody* this) {
    Vec2 temp = this->pos;
    this->pos = Vadd(temp, Vadd(Vsub(temp, this->old_pos), Vscale(gravity, Fmul(timestep(), timestep()))));
    this->old_pos = temp;
}

void init_verlet(VerletBody* this, Vec2 pos) {
    this->pos = this->old_pos = pos;
    this->accel = (Vec2){Fx0, Fx0};
}
