#include <SDL3/SDL_render.h>

#include "assets.h"
#include "draw.h"
#include "sdl.h"
#include "vec2.h"

#define FONT_WIDTH (96)
#define FONT_HEIGHT (96)

#define GLYPH_WIDTH (6)
#define GLYPH_HEIGHT (12)

#define GLYPH_COUNT ((FONT_WIDTH / GLYPH_WIDTH) * (FONT_HEIGHT / GLYPH_HEIGHT))

#define CAM_SPEED (10240.0)
#define CAM_MAX_DELAY (0.5)

#define WHITE RGB(255, 255, 255)

SDL_Color lerp_color(SDL_Color from, SDL_Color to, double f) {
    from.r = (uint8_t)((double)from.r + (double)(to.r - from.r) * f);
    from.g = (uint8_t)((double)from.g + (double)(to.g - from.g) * f);
    from.b = (uint8_t)((double)from.b + (double)(to.b - from.b) * f);
    return from;
}

static Vec2 cam_offset = XY(0.0, 0.0), cam_target = XY(0.0, 0.0);

Vec2 camera_pos() {
    return cam_offset;
}

void set_camera_target(Vec2 value) {
    cam_target = value;
}

void update_camera() {
    const Vec2 dir = Vnorm(Vsub(cam_target, cam_offset));
    const double dist = Vlen(Vsub(cam_target, cam_offset)), delayed = CAM_SPEED * CAM_MAX_DELAY;
    const double vel = dist > delayed ? dist - delayed : SDL_min(CAM_SPEED * delta(), dist);
    cam_offset = Vadd(cam_offset, Vscale(dir, vel));
}

extern SDL_Renderer* renderer;

void clear_screen(SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
}

void fill_square(Vec2 pos, double ext, SDL_Color color) {
    const SDL_FRect rect = {
        .x = (float)pos.x - (float)cam_offset.x - (float)ext,
        .y = (float)pos.y - (float)cam_offset.y - (float)ext,
        .w = (float)ext * 2.f,
        .h = (float)ext * 2.f,
    };

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}

void draw_texture(const char* name, Vec2 pos, Vec2 scale, double angle, SDL_Color tint) {
    SDL_Texture* tex = gettex(name);

    const SDL_FRect dst = {
        .x = (float)pos.x - (float)cam_offset.x - 0.5f * (float)tex->w * (float)scale.x,
        .y = (float)pos.y - (float)cam_offset.y - 0.5f * (float)tex->h * (float)scale.y,
        .w = (float)tex->w * (float)scale.x,
        .h = (float)tex->h * (float)scale.y,
    };

    SDL_SetTextureColorMod(tex, tint.r, tint.g, tint.b);
    SDL_SetTextureAlphaMod(tex, tint.a);

    SDL_RenderTextureRotated(renderer, tex, NULL, &dst, angle * 180.0 / SDL_PI_D, NULL, SDL_FLIP_NONE);

    SDL_SetTextureColorMod(tex, 255, 255, 255);
    SDL_SetTextureAlphaMod(tex, 255);
}

static const uint8_t fontmap[GLYPH_COUNT]
    = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

double text_width(double fs, const char* text) {
    double res = 0.0;

    while (SDL_StepUTF8(&text, NULL))
        res += ((float)GLYPH_WIDTH / (float)GLYPH_HEIGHT) * (float)fs;

    return res;
}

void draw_text(Vec2 pos, double fs, const char* text) {
    uint32_t c = 0;
    SDL_Texture* tex = gettex("monogram-bitmap.png");

    while ((c = SDL_StepUTF8(&text, NULL))) {
        if (c < 32 || c >= GLYPH_COUNT)
            continue;

        uint32_t idx = 0;

        for (; idx < sizeof(fontmap); idx++)
            if (fontmap[idx] == c)
                break;

        const SDL_FRect src = {
            .x = GLYPH_WIDTH * (idx % (FONT_WIDTH / GLYPH_WIDTH)),
            .y = GLYPH_HEIGHT * (idx / (FONT_WIDTH / GLYPH_WIDTH)),
            .w = GLYPH_WIDTH,
            .h = GLYPH_HEIGHT,
        };

        const SDL_FRect dst = {
            .x = (float)pos.x,
            .y = (float)pos.y,
            .w = (float)text_width(fs, " "),
            .h = (float)fs,
        };

        SDL_SetTextureColorMod(tex, 255, 255, 255);
        SDL_SetTextureAlphaMod(tex, 255);

        SDL_RenderTexture(renderer, tex, &src, &dst);

        pos.x += text_width(fs, " ");
    }
}
