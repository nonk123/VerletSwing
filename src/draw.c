#include <SDL3/SDL_render.h>

#include "draw.h"
#include "vec2.h"

#define FONT_WIDTH 96
#define FONT_HEIGHT 96

#define GLYPH_WIDTH 6
#define GLYPH_HEIGHT 12

#define GLYPH_COUNT ((FONT_WIDTH / GLYPH_WIDTH) * (FONT_HEIGHT / GLYPH_HEIGHT))

static Vec2 cam_offset = XY(0.0, 0.0);

Vec2 camera_pos() {
    return cam_offset;
}

void set_camera_pos(Vec2 value) {
    cam_offset = value;
}

static SDL_Texture* fontsheet = NULL;
extern SDL_Renderer* renderer;

bool sdl_load_font() {
    SDL_Surface* surf = SDL_LoadPNG(ASSETS "monogram-bitmap.png");

    if (!surf)
        return false;

    fontsheet = SDL_CreateTextureFromSurface(renderer, surf);

    if (!fontsheet) {
        SDL_DestroySurface(surf);
        return false;
    }

    SDL_SetTextureScaleMode(fontsheet, SDL_SCALEMODE_NEAREST);
    SDL_DestroySurface(surf);

    return true;
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

static const uint8_t fontmap[GLYPH_COUNT]
    = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_abcdefghijklmnopqrstuvwxyz{|}~";

double text_width(double fs, const char* text) {
    double res = 0.0;

    while (SDL_StepUTF8(&text, NULL))
        res += ((float)GLYPH_WIDTH / (float)GLYPH_HEIGHT) * (float)fs;

    return res;
}

void draw_text(Vec2 pos, double fs, const char* text) {
    uint32_t c = 0;
    float x = 0;

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
            .x = (float)pos.x + x * (float)text_width(fs, " "),
            .y = (float)pos.y,
            .w = (float)text_width(fs, " "),
            .h = (float)fs,
        };

        SDL_RenderTexture(renderer, fontsheet, &src, &dst);
        x += 1.f;
    }
}
