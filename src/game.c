#include <SDL3/SDL_time.h>

#include <S_tructures.h>

#include "draw.h"
#include "fmt.h"
#include "game.h"
#include "sdl.h"
#include "verlet.h"

#define SLIVER (4.0)
#define MAX_HOOK_DISTANCE (360.0)

#define ANCHOR_STEP (0.7 * MAX_HOOK_DISTANCE)
#define CULL_RADIUS (2.0 * w_width())

#define RELEASE_BOOST_COOLDOWN (1.5)
#define RELEASE_BOOST (256.0)

#define DASH_COOLDOWN (1.5)
#define DASH_BOOST (256.0)

#define DEATH_SECS (1.4)

#define BACKGROUND_BASE RGB(0, 0, 0)
#define BACKGROUND_DEAD RGB(127, 15, 15)

static uint64_t score = 0;

typedef struct {
    VerletBody *segs, *start;
    Vec2 end;
} Rope;

typedef struct {
    VerletBody body;
    Rope rope;
    double boost_cooldown, dash_cooldown;
} Monke;

typedef struct {
    Vec2 pos;
    bool scored;
} Anchor;

static Monke monke = {0};
static Anchor* anchors = NULL;

static double death_timer = 0.0;

static void tick_timer(double* rem) {
    *rem -= timestep();

    if (*rem <= 1e-3)
        *rem = 0.0;
}

static void nuke_rope(Rope* this) {
    FreeTinyD(this->segs);
    this->start = this->segs = NULL;
}

static void init_monke(Monke* this) {
    Vec2 pos = Vscale(XY(0.0, -24.0), 0.5);
    init_verlet(&this->body, pos);

    nuke_rope(&monke.rope);

    this->boost_cooldown = 0.0;
    this->dash_cooldown = 0.0;
}

static void place_anchor(double x) {
    const double y = 16.0 + SDL_randf() * (w_height() * 0.66);
    anchors = TinyDAppendPro(anchors, &(Anchor){x, y});
}

static double step() {
    return ANCHOR_STEP * (0.4 + SDL_randf());
}

static void cull_anchors() {
    const double center = monke.body.pos.x;

    for (size_t i = 0; i < TinyDLength(anchors);) {
        const double cur = anchors[i].pos.x;

        if (cur < center - 2.0 * CULL_RADIUS || cur > center + 2.0 * CULL_RADIUS)
            anchors = TinyDErase(anchors, i);
        else
            i += 1;
    }
}

static void generate_anchors() {
    if (TinyDLength(anchors))
        cull_anchors();
    else
        place_anchor(ANCHOR_STEP);

    double min = anchors[0].pos.x, max = min;

    for (size_t i = 1; i < TinyDLength(anchors); i++) {
        const double cur = anchors[i].pos.x;

        if (cur < min)
            min = cur;

        if (cur > max)
            max = cur;
    }

    const double center = monke.body.pos.x;

    while (min > ANCHOR_STEP && min > center - CULL_RADIUS) {
        min -= step();
        place_anchor(min);
    }

    while (max < center + CULL_RADIUS) {
        max += step();
        place_anchor(max);
    }
}

void restart() {
    score = 0;

    init_monke(&monke);
    monke.body.f_gravity = true;

    FreeTinyD(anchors);
    anchors = MakeTinyD(Anchor);

    generate_anchors();
}

static void score_anchors() {
    for (size_t i = 0; i < TinyDLength(anchors); i++) {
        const double x = anchors[i].pos.x;

        if (!anchors[i].scored && x >= 0.0 && monke.body.pos.x >= x)
            score += 1, anchors[i].scored = true;
    }
}

static int closest_anchor() {
    if (!TinyDLength(anchors))
        return -1;

    int closest = 0;

    for (int i = 1; i < TinyDLength(anchors); i++)
        if (Vdist(monke.body.pos, anchors[i].pos) <= Vdist(monke.body.pos, anchors[closest].pos))
            closest = i;

    if (Vdist(monke.body.pos, anchors[closest].pos) <= MAX_HOOK_DISTANCE)
        return closest;

    return -1;
}

static void maybe_manifest_rope() {
    tick_timer(&monke.boost_cooldown);

    if (!is_left_pressed()) {
        if (monke.rope.segs && monke.boost_cooldown == 0.0) {
            push(&monke.body, XY(0.0, -RELEASE_BOOST));
            monke.boost_cooldown = RELEASE_BOOST_COOLDOWN;
        }

        nuke_rope(&monke.rope);
    }

    if (!is_left_pressed() || monke.rope.segs)
        return;

    const int closest = closest_anchor();

    if (closest < 0)
        return;

    const Vec2 dir = Vsub(anchors[closest].pos, monke.body.pos);
    const size_t segs = (size_t)(Vlen(dir) / SLIVER);

    if (segs < 2)
        return;

    const Vec2 initial_velocity = Vsub(monke.body.pos, monke.body.old_pos);

    monke.rope.segs = MakeTinyD(VerletBody);
    monke.rope.end = anchors[closest].pos;
    monke.rope.start = &monke.body;

    for (size_t i = 0; i <= segs; i++) {
        const Vec2 pos = Vscale(dir, (double)i / (double)segs);

        VerletBody seg = {0};
        init_verlet(&seg, Vadd(pos, monke.body.pos));
        seg.old_pos = Vsub(seg.old_pos, initial_velocity);
        seg.f_gravity = true;

        monke.rope.segs = TinyDAppendPro(monke.rope.segs, &seg);
    }
}

static void constrain_rope(Rope* rope) {
    for (int i = 0; i < (int)TinyDLength(rope->segs); i++) {
        VerletBody* const a = i ? &rope->segs[i - 1] : rope->start;
        VerletBody* const b = &rope->segs[i];

        if (i == TinyDLength(rope->segs) - 1)
            b->pos = rope->end; // !! IMPORTANT
        // rope doesn't rope ^ without this

        const double dist = Vdist(a->pos, b->pos);

        if (dist > 1e-2) {
            const double diff = (SLIVER - dist) / dist;
            const Vec2 bounce = Vscale(Vsub(a->pos, b->pos), 0.5 * diff);

            a->pos = Vadd(a->pos, bounce);
            b->pos = Vsub(b->pos, bounce);
        }
    }
}

static void verlet_rope(Rope* rope) {
    for (size_t i = 0; i < TinyDLength(rope->segs); i++)
        verlet(rope->segs + i);

    for (size_t i = 0; i < 120; i++)
        constrain_rope(rope);
}

void update() {
    generate_anchors();
    score_anchors();
    maybe_manifest_rope();

    tick_timer(&monke.dash_cooldown);

    if (is_right_pressed() && monke.dash_cooldown == 0.0) {
        const Vec2 dash = XY(DASH_BOOST, 0.0);

        for (size_t i = 0; i <= TinyDLength(monke.rope.segs); i++)
            push(i ? &monke.rope.segs[i - 1] : &monke.body, dash);

        monke.dash_cooldown = DASH_COOLDOWN;
    }

    verlet(&monke.body);
    verlet_rope(&monke.rope);

    if (monke.body.pos.y > w_height())
        tick_timer(&death_timer);
    else
        death_timer = DEATH_SECS;

    if (death_timer == 0.0)
        restart();
}

static void draw_rope(Rope rope) {
    for (size_t i = 0; i < TinyDLength(rope.segs); i++)
        fill_square(rope.segs[i].pos, 3.0, RGB(255, 255, 255));
}

void draw() {
    clear_screen(lerp_color(BACKGROUND_BASE, BACKGROUND_DEAD, 1.0 - death_timer / DEATH_SECS));

    set_camera_target(XY(monke.body.pos.x - w_width() / 4, 0.0));
    update_camera();

    draw_rope(monke.rope);

    const int closest = closest_anchor();

    for (int i = 0; i < TinyDLength(anchors); i++) {
        const SDL_Color col = i == closest ? RGB(255, 255, 0) : RGB(127, 127, 0);
        fill_square(anchors[i].pos, 16.0, col);
    }

    fill_square(monke.body.pos, 16.0, RGB(255, 0, 0));

    const double fs = 48.0, pad = 8.0;

    const char* s = fmt("A%zu", TinyDLength(anchors));
    draw_text(XY(w_width() - text_width(fs, s) - pad, pad), fs, s);

    s = fmt("SCORE: %llu", score);
    draw_text(XY(0.5 * (w_width() - text_width(fs, s)), pad), fs, s);
}
