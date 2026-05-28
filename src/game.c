#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>

#include <S_tructures.h>

#include "game.h"
#include "verlet.h"

#define SLIVER FxFrom(1)
#define MAX_HOOK_DISTANCE FxFrom(314)

typedef struct {
    VerletBody *segs, *end;
    Vec2 anchor;
} Rope;

typedef struct {
    VerletBody body;
    Rope rope;
} Monke;

typedef struct {
    Vec2 pos;
} Anchor;

static Monke monke = {0};
static Anchor* anchors = NULL;

static void nuke_rope(Rope* this) {
    FreeTinyD(this->segs);
    this->end = this->segs = NULL;
}

static void init_monke(Monke* this) {
    int w = 0;

    extern SDL_Window* window;
    SDL_GetWindowSize(window, &w, NULL);

    Vec2 pos = {FxFrom(w), Fx0};
    pos = Vscale(pos, FxFrom(0.5));
    init_verlet(&this->body, pos);

    nuke_rope(&monke.rope);
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

    for (int i = 0; i < 10; i++) {
        const Fixed x = Fmul(step, FxFrom(i)), y = FxFrom(h / 4) + FxFrom(SDL_rand(h / 2));
        anchors = TinyDAppendPro(anchors, &(Anchor){x, y});
    }
}

static void maybe_manifest_rope() {
    if (!is_pressed() && monke.rope.segs) {
        nuke_rope(&monke.rope);
        return;
    }

    if (!is_pressed() || monke.rope.segs || !TinyDLength(anchors))
        return;

    size_t closest = 0;

    for (size_t i = 1; i < TinyDLength(anchors); i++)
        if (Vdist(monke.body.pos, anchors[i].pos) <= Vdist(monke.body.pos, anchors[closest].pos))
            closest = i;

    if (Vdist(monke.body.pos, anchors[closest].pos) > MAX_HOOK_DISTANCE)
        return;

    monke.rope.segs = MakeTinyD(VerletBody);
    monke.rope.anchor = anchors[closest].pos;
    monke.rope.end = &monke.body;

    const Vec2 dir = Vsub(anchors[closest].pos, monke.body.pos);
    const size_t segs = FxToInt(Fdiv(Fabs(dir.x), SLIVER));

    for (size_t i = 0; i <= segs; i++) {
        const Vec2 pos = Vscale(dir, FxFrom((double)i / (double)segs));

        VerletBody seg = {0};
        init_verlet(&seg, Vadd(pos, monke.body.pos));
        seg.f_gravity = i < segs;

        monke.rope.segs = TinyDAppend(monke.rope.segs, seg);
    }
}

static void constrain_rope(Rope* rope) {
    for (int i = (int)TinyDLength(rope->segs) - 2; i >= -1; i--) {
        VerletBody* const a = i < 0 ? rope->end : &rope->segs[i];
        VerletBody* const b = &rope->segs[i + 1];

        if (i == TinyDLength(monke.rope.segs) - 2)
            b->pos = rope->anchor; // !! IMPORTANT
        // rope doesn't rope ^ without this

        const Fixed dist = Vdist(a->pos, b->pos);

        if (dist == Fx0)
            continue;

        const Fixed diff = Fdiv(Fsub(SLIVER, dist), dist);
        const Vec2 bounce = Vscale(Vsub(a->pos, b->pos), Fhalf(diff));

        a->pos = Vadd(a->pos, bounce);
        b->pos = Vsub(b->pos, bounce);
    }
}

static void verlet_rope(Rope* rope) {
    for (size_t i = 0; i < TinyDLength(rope->segs); i++)
        verlet(&rope->segs[i]);

    for (size_t i = 0; i < 60; i++)
        constrain_rope(rope);
}

void update() {
    maybe_manifest_rope();

    verlet(&monke.body);
    verlet_rope(&monke.rope);
}

static void fill_square(Vec2 pos, float radius, SDL_Color color) {
    extern SDL_Renderer* renderer;

    const SDL_FRect rect = {
        .x = Fx2Float(pos.x) - radius,
        .y = Fx2Float(pos.y) - radius,
        .w = radius * 2.f,
        .h = radius * 2.f,
    };

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}

#define RGB(_r, _g, _b) ((SDL_Color){.r = (_r), .g = (_g), .b = (_b), .a = SDL_ALPHA_OPAQUE})

static void draw_rope(Rope rope) {
    for (size_t i = 0; i < TinyDLength(rope.segs); i++)
        fill_square(rope.segs[i].pos, 3.f, RGB(255, 255, 255));
}

void draw() {
    for (size_t i = 0; i < TinyDLength(anchors); i++)
        fill_square(anchors[i].pos, 16.f, RGB(127, 127, 0));

    draw_rope(monke.rope);
    fill_square(monke.body.pos, 16.f, RGB(255, 0, 0));
}
