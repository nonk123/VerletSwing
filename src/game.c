#include <SDL3/SDL_time.h>

#include <S_tructures.h>

#include "draw.h"
#include "game.h"
#include "sdl.h"
#include "verlet.h"

#define SLIVER (4.0)
#define MAX_HOOK_DISTANCE (460.0)

#define RELEASE_BOOST_COOLDOWN (1.5)
#define RELEASE_BOOST (10240.0)

#define DEATH_SECS (1.4)

static uint64_t score = 0;

typedef struct {
    VerletBody *segs, *start;
    Vec2 end;
} Rope;

typedef struct {
    VerletBody body;
    Rope rope;
    double boost_cooldown;
} Monke;

typedef struct {
    Vec2 pos;
    bool scored;
} Anchor;

static Monke monke = {0};
static Anchor* anchors = NULL;

static double death_timer = 0.0;

static void nuke_rope(Rope* this) {
    FreeTinyD(this->segs);
    this->start = this->segs = NULL;
}

static void init_monke(Monke* this) {
    Vec2 pos = Vscale(XY(0.0, -24.0), 0.5);
    init_verlet(&this->body, pos);

    nuke_rope(&monke.rope);

    this->boost_cooldown = 0.0;
}

static void place_anchor(const double x) {
    const double y = w_height() / 4 + SDL_rand(w_height() / 2);
    anchors = TinyDAppendPro(anchors, &(Anchor){x, y});
}

#define ANCHOR_STEP (0.7 * MAX_HOOK_DISTANCE)

static double step() {
    return ANCHOR_STEP * (0.5 + SDL_randf());
}

static void generate_anchors() {
    const double center = monke.body.pos.x, radius = 2.0 * w_width();

    if (!TinyDLength(anchors))
        place_anchor(center + ANCHOR_STEP);

    for (size_t i = 0; i < TinyDLength(anchors);) {
        const double cur = anchors[i].pos.x;

        if (cur < center - 2.0 * radius || cur > center + 2.0 * radius)
            anchors = TinyDErase(anchors, i);
        else
            i += 1;
    }

    double min = anchors[0].pos.x, max = min;

    for (size_t i = 1; i < TinyDLength(anchors); i++) {
        const double cur = anchors[i].pos.x;

        if (cur < min)
            min = cur;

        if (cur > max)
            max = cur;
    }

    while (min > center - radius) {
        min -= step();
        place_anchor(min);
    }

    while (max < center + radius) {
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
    Vec2 cursor = Vadd(mouse_pos_v(), camera_pos());

    for (int i = 1; i < TinyDLength(anchors); i++)
        if (Vdist(cursor, anchors[i].pos) <= Vdist(cursor, anchors[closest].pos))
            closest = i;

    if (Vdist(monke.body.pos, anchors[closest].pos) <= MAX_HOOK_DISTANCE)
        return closest;

    closest = 0;

    for (int i = 1; i < TinyDLength(anchors); i++)
        if (Vdist(monke.body.pos, anchors[i].pos) <= Vdist(monke.body.pos, anchors[closest].pos))
            closest = i;

    if (Vdist(monke.body.pos, anchors[closest].pos) <= MAX_HOOK_DISTANCE)
        return closest;

    return -1;
}

static void maybe_manifest_rope() {
    monke.boost_cooldown -= timestep();

    if (monke.boost_cooldown < 1e-3)
        monke.boost_cooldown = 0.0;

    if (!is_pressed()) {
        if (monke.rope.segs && monke.boost_cooldown == 0.0) {
            push(&monke.body, XY(0.0, -RELEASE_BOOST));
            monke.boost_cooldown = RELEASE_BOOST_COOLDOWN;
        }

        nuke_rope(&monke.rope);
    }

    if (!is_pressed() || monke.rope.segs)
        return;

    const int closest = closest_anchor();

    if (closest < 0)
        return;

    const Vec2 dir = Vsub(anchors[closest].pos, monke.body.pos);
    const size_t segs = (size_t)(Vlen(dir) / SLIVER);

    if (segs < 2)
        return;

    monke.rope.segs = MakeTinyD(VerletBody);
    monke.rope.end = anchors[closest].pos;
    monke.rope.start = &monke.body;

    for (size_t i = 0; i <= segs; i++) {
        const Vec2 pos = Vscale(dir, (double)i / (double)segs);

        VerletBody seg = {0};
        init_verlet(&seg, Vadd(pos, monke.body.pos));
        seg.f_gravity = i < segs;

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

    verlet(&monke.body);
    verlet_rope(&monke.rope);

    if (monke.body.pos.y > w_height())
        death_timer += timestep();
    else
        death_timer = 0.0;

    if (death_timer >= DEATH_SECS)
        restart();
}

static void draw_rope(Rope rope) {
    for (size_t i = 0; i < TinyDLength(rope.segs); i++)
        fill_square(rope.segs[i].pos, 3.0, RGB(255, 255, 255));
}

void draw(double dt) {
    set_camera_target(XY(monke.body.pos.x - w_width() / 4, 0.0));
    update_camera(dt);

    draw_rope(monke.rope);

    const int closest = closest_anchor();

    for (int i = 0; i < TinyDLength(anchors); i++) {
        const SDL_Color col = i == closest ? RGB(255, 255, 0) : RGB(127, 127, 0);
        fill_square(anchors[i].pos, 16.0, col);
    }

    fill_square(monke.body.pos, 16.0, RGB(255, 0, 0));

    const double fs = 48.0, pad = 8.0;

    if (death_timer > 0.0) {
        const char* txt = "DEATH IMMINENT";
        draw_text(XY(0.5 * (w_width() - text_width(fs, txt)), w_height() - fs - pad), fs, txt);
    }

    static char buf[128] = {0};
    SDL_snprintf(buf, sizeof(buf), "A%zu", TinyDLength(anchors));
    draw_text(XY(w_width() - text_width(fs, buf) - pad, pad), fs, buf);

    SDL_snprintf(buf, sizeof(buf), "SCORE: %llu", score);
    draw_text(XY(0.5 * (w_width() - text_width(fs, buf)), pad), fs, buf);
}
