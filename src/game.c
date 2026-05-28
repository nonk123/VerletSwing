#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>

#include "game.h"
#include "verlet.h"

typedef struct {
    VerletBody body;
} Monke;

static Monke monke = {0};

static void init_monke(Monke* this) {
    int w = 0;

    extern SDL_Window* window;
    SDL_GetWindowSize(window, &w, NULL);

    Vec2 pos = {FxFrom(w), Fx0};
    pos = Vscale(pos, FxFrom(0.5));
    init_verlet(&this->body, pos);
}

void restart() {
    init_monke(&monke);
}

void update() {
    verlet(&monke.body);
}

void draw() {
    extern SDL_Renderer* renderer;

    const float w = 64.f, h = 64.f;
    const SDL_FRect rect = {
        .x = Fx2Float(monke.body.pos.x) - w / 2.f,
        .y = Fx2Float(monke.body.pos.y) - h / 2.f,
        .w = w / 2.f,
        .h = h / 2.f,
    };

    SDL_SetRenderDrawColor(renderer, 255, 32, 32, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &rect);
}
