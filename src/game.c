#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>

#include <S_tructures.h>

#include "game.h"
#include "verlet.h"

typedef struct {
    VerletBody body;
} Monke;

typedef struct {
    Vec2 pos;
} Anchor;

static Monke monke = {0};
static Anchor* anchors = NULL;

static void init_monke(Monke* this) {
    int w = 0;

    extern SDL_Window* window;
    SDL_GetWindowSize(window, &w, NULL);

    Vec2 pos = {FxFrom(w), Fx0};
    pos = Vscale(pos, FxFrom(0.5));
    init_verlet(&this->body, pos);
}

void restart() {
    int h = 0;

    extern SDL_Window* window;
    SDL_GetWindowSize(window, NULL, &h);

    init_monke(&monke);
    monke.body.f_gravity = true;

    FreeTinyD(anchors);
    anchors = MakeTinyD(Anchor);

    const Fixed step = FxFrom(512);
    Fixed x = Fx0;

    for (int i = 0; i < 10; i++) {
        x = Fadd(x, step);
        const Fixed y = FxFrom(h / 4) + FxFrom(SDL_rand(h / 2));

        Anchor anch = {x, y};
        anchors = TinyDAppend(anchors, anch);
    }
}

void update() {
    verlet(&monke.body);
}

static void fill_square(Vec2 pos, float radius, SDL_Color color) {
    extern SDL_Renderer* renderer;

    const SDL_FRect rect = {
        .x = Fx2Float(pos.x) - radius,
        .y = Fx2Float(pos.y) - radius,
        .w = radius,
        .h = radius,
    };

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}

#define RGB(_r, _g, _b) ((SDL_Color){.r = (_r), .g = (_g), .b = (_b), .a = SDL_ALPHA_OPAQUE})

void draw() {
    for (size_t i = 0; i < TinyDLength(anchors); i++)
        fill_square(anchors[i].pos, 32.f, RGB(127, 127, 0));

    fill_square(monke.body.pos, 32.f, RGB(255, 0, 0));
}
