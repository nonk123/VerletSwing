#include "verlet.h"

static const Vec2 gravity = {.x = Fx0, .y = FxFrom(256)};

void verlet(VerletBody* this) {
    Vec2 temp = this->pos, grave = Vscale(gravity, Fmul(timestep(), timestep()));
    this->pos = Vadd(this->pos, Vadd(Vsub(this->pos, this->old_pos), grave));
    this->old_pos = temp;
}

void init_verlet(VerletBody* this, Vec2 pos) {
    this->pos = this->old_pos = pos;
    this->accel = (Vec2){Fx0, Fx0};
}
