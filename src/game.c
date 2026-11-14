#include <SDL3/SDL_time.h>

#include <S_tructures.h>

#include "chat.h"
#include "draw.h"
#include "fmt.h"
#include "game.h"
#include "lerp.h"
#include "net.h"
#include "screen.h"
#include "sdl.h"
#include "stringbuf.h"
#include "verlet.h"

#define SLIVER (4.0)
#define MAX_HOOK_DISTANCE (360.0)

#define STRAT_THRESHOLD (8.0)

#define ANCHOR_STEP (120.0)
#define ANCHOR_PROB (0.2)
#define MAX_CONSECUTIVE_SKIPS (4)

#define CULL_RADIUS (2.0 * w_width())
#define FIRST_ANCHOR_X (240.0)

#define DASH_COOLDOWN (5.5)
#define DASH_BOOST (512.0)

#define DEATH_SECS (1.4)

uint64_t score = 0;
int64_t last_scored_idx = -1;

typedef struct {
    VerletBody* segs;
    Vec2 end;
} Rope;

typedef struct {
    VerletBody body;
    Rope rope;
    double dash_cooldown;
} Monke;

typedef struct {
    Vec2 pos;
    bool scored;
} Anchor;

static TinyMap monkes = {0};
static Anchor* anchors = NULL;

static double death_timer = DEATH_SECS;

static void tick_timer(double* rem) {
    *rem -= timestep();

    if (*rem <= 1e-3)
        *rem = 0.0;
}

static void nuke_rope(Rope* this) {
    FreeTinyD(this->segs);
    this->segs = NULL;
}

static void init_monke(Monke* this) {
    Vec2 pos = Vscale(XY(0.0, -STRAT_THRESHOLD), 0.5);
    init_verlet(&this->body, pos);
    nuke_rope(&this->rope);

    this->dash_cooldown = 0.0;
}

static int64_t anchor_idx(const double x) {
    return (int64_t)(SDL_round((x - FIRST_ANCHOR_X) / ANCHOR_STEP));
}

static void place_anchor(const double x, const double y) {
    for (size_t i = 0; i < TinyDLength(anchors); i++)
        if (anchors[i].pos.x == x)
            return;

    const bool scored = anchor_idx(x) < last_scored_idx;
    anchors = TinyDPush(anchors, &(Anchor){x, y, scored});
}

static Monke* our_monke() {
    return (Monke*)TinyMapGet(&monkes, NutBlast_GetPlayerID());
}

static void cull_anchors() {
    const Monke* monke = our_monke();

    if (!monke)
        return;

    const double center = monke->body.pos.x;

    for (size_t i = 0; i < TinyDLength(anchors);) {
        const double cur = anchors[i].pos.x;

        if (cur < center - 2.0 * CULL_RADIUS || cur > center + 2.0 * CULL_RADIUS)
            anchors = TinyDErase(anchors, i);
        else
            i += 1;
    }
}

static void generate_anchors() {
    const Monke* monke = our_monke();

    if (!monke)
        return;

    const double pad = 16.0, range = w_height() * 0.66;

    if (TinyDLength(anchors))
        cull_anchors();
    else
        place_anchor(FIRST_ANCHOR_X, pad + 0.5 * range);

    uint32_t seed = 0;
    SDL_sscanf(NutBlast_GetLobbyField("seed"), "%u", &seed);
    SDL_srand(seed);

    const double center = monke->body.pos.x;
    const double start = center - 0.98 * CULL_RADIUS;
    const double end = center + 0.98 * CULL_RADIUS;
    size_t skips = 0;

    for (double x = FIRST_ANCHOR_X; x < end; x += ANCHOR_STEP) {
        // NOTE: making sure we run both `SDL_randf`s in the same iteration for the LCG to produce a deterministic
        // result.
        const double y = pad + SDL_randf() * range;
        const double prob = SDL_randf();

        if (prob < ANCHOR_PROB || skips >= MAX_CONSECUTIVE_SKIPS) {
            if (x >= start)
                place_anchor(x, y);
            skips = 0;
        } else {
            skips += 1;
        }
    }
}

static void sync_camera_with_monke() {
    const Monke* monke = our_monke();

    if (monke)
        set_camera_target(XY(monke->body.pos.x - w_width() / 4, 0.0));
}

static void free_monke(void* self) {
    Monke* monke = self;
    nuke_rope(&monke->rope);
}

static void insert_monke(NutBlast_PlayerID id) {
    Monke monke = {0};
    init_monke(&monke);
    monke.body.f_gravity = true;

    TinyBucket* buck = TinyMapPut(&monkes, id, &monke, sizeof(monke));

    if (buck)
        buck->data.cleanup = free_monke;
}

void on_player_joined(NutBlast_PlayerID id) {
    insert_monke(id);
    display_announcement(fmt("%s joined", get_player_name(id)));
}

void on_player_left(NutBlast_PlayerID id, NutBlast_Reason r) {
    display_announcement(fmt("%s left: %s", get_player_name(id), r.msg));
    TinyMapErase(&monkes, id);
}

void deinit_game() {
    FreeTinyMap(&monkes);
    NutBlast_OnPlayerJoined(NULL);
    NutBlast_OnPlayerLeft(NULL);
}

void restart() {
    score = 0;
    last_scored_idx = -1;

    insert_monke(NutBlast_GetPlayerID());

    FreeTinyD(anchors);
    anchors = MakeTinyD(Anchor);

    generate_anchors();
    sync_camera_with_monke();
}

static void score_anchors() {
    const Monke* monke = our_monke();

    if (!monke)
        return;

    for (size_t i = 0; i < TinyDLength(anchors); i++) {
        const double x = anchors[i].pos.x;

        if (!anchors[i].scored && monke->body.pos.x >= x) {
            score += 1;
            last_scored_idx = anchor_idx(x);
            anchors[i].scored = true;
        }
    }
}

static int closest_anchor() {
    if (!TinyDLength(anchors))
        return -1;

    Monke* monke = (Monke*)TinyMapGet(&monkes, NutBlast_GetPlayerID());

    if (!monke)
        return -1;

    int closest = 0;
    const double ahead = 2.0 * monke->body.pos.x - monke->body.old_pos.x;

    for (int i = 1; i < TinyDLength(anchors); i++)
        if (SDL_fabs(ahead - anchors[i].pos.x) <= SDL_fabs(ahead - anchors[closest].pos.x))
            closest = i;

    if (Vdist(monke->body.pos, anchors[closest].pos) <= MAX_HOOK_DISTANCE)
        return closest;

    closest = 0;

    for (int i = 1; i < TinyDLength(anchors); i++)
        if (Vdist(monke->body.pos, anchors[i].pos) <= Vdist(monke->body.pos, anchors[closest].pos))
            closest = i;

    if (Vdist(monke->body.pos, anchors[closest].pos) <= MAX_HOOK_DISTANCE)
        return closest;

    return -1;
}

static void maybe_manifest_rope() {
    Monke* monke = our_monke();

    if (!left_press)
        nuke_rope(&monke->rope);

    if (!left_press || monke->rope.segs)
        return;

    const int closest = closest_anchor();

    if (closest < 0)
        return;

    const Vec2 dir = Vsub(anchors[closest].pos, monke->body.pos);
    const size_t segs = (size_t)(Vlen(dir) / SLIVER);

    if (segs < 2)
        return;

    const Vec2 initial_velocity = Vsub(monke->body.pos, monke->body.old_pos);

    monke->rope.segs = MakeTinyD(VerletBody);
    monke->rope.end = anchors[closest].pos;

    for (size_t i = 0; i <= segs; i++) {
        const Vec2 pos = Vscale(dir, (double)i / (double)segs);

        VerletBody seg = {0};
        init_verlet(&seg, Vadd(pos, monke->body.pos));
        seg.old_pos = Vsub(seg.old_pos, initial_velocity);
        seg.f_gravity = true;

        monke->rope.segs = TinyDPush(monke->rope.segs, &seg);
    }
}

static void constrain_rope(Monke* monke) {
    for (int i = 0; i < (int)TinyDLength(monke->rope.segs); i++) {
        VerletBody* const a = i ? &monke->rope.segs[i - 1] : &monke->body;
        VerletBody* const b = &monke->rope.segs[i];

        if (i == TinyDLength(monke->rope.segs) - 1)
            b->pos = monke->rope.end; // !! IMPORTANT
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

static void verlet_rope(Monke* monke) {
    for (size_t i = 0; i < TinyDLength(monke->rope.segs); i++)
        verlet(monke->rope.segs + i);

    for (size_t i = 0; i < 120; i++)
        constrain_rope(monke);
}

static void draw_rope(Rope rope) {
    for (size_t i = 0; i < TinyDLength(rope.segs); i++)
        fill_square(rope.segs[i].pos, 3.0, RGB(255, 255, 255));
}

static void draw_game() {
    clear_screen(lerp_color(BACKGROUND_BASE, BACKGROUND_DEAD, 1.0 - death_timer / DEATH_SECS));

    sync_camera_with_monke();
    update_camera();

    const double fs = 48.0, pad = 8.0;
    const char* s = NULL;

    TINY_MAP_FOREACH(&monkes, it) {
        Monke* monke = it.data;
        draw_rope(monke->rope);
    }

    const int closest = closest_anchor();

    for (int i = 0; i < TinyDLength(anchors); i++) {
        const SDL_Color col = i == closest ? RGB(255, 255, 0) : RGB(127, 127, 0);
        fill_square(anchors[i].pos, 16.0, col);
    }

    TINY_MAP_FOREACH(&monkes, it) {
        Monke* monke = it.data;
        const bool is_us = it.bucket->hash == NutBlast_GetPlayerID();

        SDL_Color player_color = is_us ? RGB(255, 0, 0) : RGB(127, 64, 0);

        if (is_us) {
            const Vec2 vel = Vsub(monke->body.pos, monke->body.old_pos);
            const Vec2 scale = XY(1.0 - monke->dash_cooldown / DASH_COOLDOWN, 1.0);

            player_color.a = (uint8_t)lerpin3(0.0, 255.0, 1.0 - monke->dash_cooldown / DASH_COOLDOWN);
            draw_texture("velvec.png", monke->body.pos, scale, SDL_atan2(vel.y, vel.x), player_color);
            player_color.a = 255;
        }

        fill_square(monke->body.pos, 16.0, player_color);

        if (is_us && monke->body.pos.y < -STRAT_THRESHOLD) {
            s = fmt("%d", (int)-monke->body.pos.y);
            draw_text(XY(monke->body.pos.x - camera_pos().x - 0.5 * text_width(fs, s), pad), fs, s);
        }
    }

    s = fmt("SCORE: %llu", score);
    draw_text(XY(0.5 * (w_width() - text_width(fs, s)), w_height() - fs - pad), fs, s);
}

static void pull_updates() {
    NutBlast_Message msg = {0};

    while (NutBlast_NextMessage(CHAN_POS, &msg)) {
        Monke* monke = (Monke*)TinyMapGet(&monkes, msg.from);

        if (monke)
            SDL_sscanf(msg.data, "%lf:%lf", &monke->body.pos.x, &monke->body.pos.y);
    }

    while (NutBlast_NextMessage(CHAN_ROPE, &msg)) {
        Monke* monke = (Monke*)TinyMapGet(&monkes, msg.from);

        if (!monke)
            continue;

        if (SDL_strlen(msg.data)) {
            SDL_sscanf(msg.data, "%lf:%lf", &monke->rope.end.x, &monke->rope.end.y);
            while (*msg.data++ != ';') {}
        } else {
            FreeTinyD(monke->rope.segs);
            monke->rope.segs = NULL;
            continue;
        }

        FreeTinyD(monke->rope.segs);
        monke->rope.segs = MakeTinyD(VerletBody);

        while (*msg.data) {
            Vec2 pos = {0};
            SDL_sscanf(msg.data, "%lf:%lf", &pos.x, &pos.y);
            while (*msg.data++ != ';') {}

            VerletBody seg = {0};
            init_verlet(&seg, pos);
            monke->rope.segs = TinyDPush(monke->rope.segs, &seg);
        }
    }
}

static void run_updates() {
    TINY_MAP_FOREACH(&monkes, it) {
        Monke* monke = it.data;
        const NutBlast_PlayerID id = it.bucket->hash;
        const bool is_us = id == NutBlast_GetPlayerID();

        if (is_us) {
            tick_timer(&monke->dash_cooldown);

            if (right_press && monke->dash_cooldown == 0.0) {
                const Vec2 dir = Vnorm(Vsub(monke->body.pos, monke->body.old_pos)), dash = Vscale(dir, DASH_BOOST);

                for (size_t i = 0; i <= TinyDLength(monke->rope.segs); i++)
                    push(i ? &monke->rope.segs[i - 1] : &monke->body, dash);

                monke->dash_cooldown = DASH_COOLDOWN;
            }
        }

        verlet(&monke->body);
        verlet_rope(monke);

        if (is_us) {
            if (monke->body.pos.y > w_height())
                tick_timer(&death_timer);
            else
                death_timer = DEATH_SECS;
        }
    }

    if (death_timer == 0.0) {
        restart();
        send_announcement(fmt("%s died!", get_player_name(0)));
    }
}

static void tick() {
    generate_anchors();
    score_anchors();
    maybe_manifest_rope();
    run_updates();
}

static void push_updates() {
    const Monke* monke = our_monke();

    if (!monke)
        return;

    spread_str((SendOptions){
        .data = fmt("%lf:%lf", monke->body.pos.x, monke->body.pos.y),
        .channel = CHAN_POS,
    });

    StringBuf sb = {0};

    if (monke->rope.segs) {
        sb_write(&sb, fmt("%lf:%lf;", monke->rope.end.x, monke->rope.end.y));

        for (size_t i = 0; i < TinyDLength(monke->rope.segs); i++)
            sb_write(&sb, fmt("%lf:%lf;", monke->rope.segs[i].pos.x, monke->rope.segs[i].pos.y));
    }

    spread_str((SendOptions){
        .data = sb_string(&sb),
        .channel = CHAN_ROPE,
    });
}

void update_game() {
    pull_updates();

    static double ticks = 0.0;
    ticks += delta() * TICKRATE;

    for (; ticks >= 1.0; ticks -= 1.0) {
        tick();
        right_press = false;
    }

    push_updates();
    draw_game();
    update_chat();
}
