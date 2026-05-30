#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>

#define S_TRUCTURES_IMPLEMENTATION
#include <S_tructures.h>

#include "cmake.h"
#include "draw.h"
#include "game.h"
#include "sdl.h"

#define SWIPE_THRESHOLD (0.08)
#define SWIPE_TIMEOUT (0.25)

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

static SDL_AppResult SDL_Fail() {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "%s", SDL_GetError());
    return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppInit(void** ctx, int argc, char* argv[]) {
    (void)ctx, (void)argc, (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
        return SDL_Fail();

    if (!SDL_CreateWindowAndRenderer(
            GAME_NAME, 800, 600, SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED, &window, &renderer))
    {
        return SDL_Fail();
    }

    if (!sdl_load_font())
        return SDL_Fail();

    SDL_SetRenderVSync(renderer, SDL_RENDERER_VSYNC_ADAPTIVE);
    SDL_ShowWindow(window);

    restart();

    return SDL_APP_CONTINUE;
}

static bool left_press = false, right_press = false;

bool is_left_pressed() {
    return left_press;
}

bool is_right_pressed() {
    return right_press;
}

SDL_AppResult SDL_AppEvent(void* ctx, SDL_Event* event) {
    (void)ctx;

    static SDL_FingerID grapple_finger = 0;

    switch (event->type) {
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        return SDL_APP_SUCCESS;

    case SDL_EVENT_FINGER_DOWN:
        if (event->tfinger.x <= 0.5) {
            left_press = true;
            grapple_finger = event->tfinger.fingerID;
        }

        if (event->tfinger.x > 0.5)
            right_press = true;

        break;

    case SDL_EVENT_FINGER_UP:
        if (event->tfinger.fingerID == grapple_finger)
            left_press = false;

        break;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (event->button.which == SDL_TOUCH_MOUSEID)
            break;

        if (event->button.button == SDL_BUTTON_LEFT)
            left_press = true;

        if (event->button.button == SDL_BUTTON_RIGHT)
            right_press = true;

        break;

    case SDL_EVENT_MOUSE_BUTTON_UP:
        if (event->button.which == SDL_TOUCH_MOUSEID)
            break;

        if (event->button.button == SDL_BUTTON_LEFT)
            left_press = false;

        // right_press gets reset after `update()` either way

        break;

    case SDL_EVENT_KEY_DOWN:
#ifndef __EMSCRIPTEN__
        if (event->key.key == SDLK_ESCAPE)
            return SDL_APP_SUCCESS;
#endif

        if (event->key.key == SDLK_SPACE)
            left_press = true;

        if (event->key.key == SDLK_LSHIFT)
            right_press = true;

        if (event->key.key == SDLK_R)
            restart();

        break;

    case SDL_EVENT_KEY_UP:
        if (event->key.key == SDLK_SPACE)
            left_press = false;

        break;

    default:
        break;
    }

    return SDL_APP_CONTINUE;
}

#define TICKRATE (60)
#define NANOSEC (1000000000.0)

static uint64_t then = 0, now = 0;

double timestep() {
    return 1.0 / TICKRATE;
}

SDL_AppResult SDL_AppIterate(void* ctx) {
    (void)ctx;

    static double ticks = 0.0;
    now = SDL_GetTicksNS();

    const double dt = (double)(now - then) / NANOSEC;

    if (then)
        ticks += (double)(now - then) / (NANOSEC / TICKRATE);

    then = now;

    for (; ticks >= 1.0; ticks -= 1.0) {
        update();
        right_press = false;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    draw(dt);

    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* ctx, SDL_AppResult result) {
    (void)ctx, (void)result;

    if (renderer)
        SDL_DestroyRenderer(renderer);

    if (window)
        SDL_DestroyWindow(window);

    SDL_Quit();
}

int w_width() {
    int w = 0;
    SDL_GetWindowSize(window, &w, NULL);
    return w;
}

int w_height() {
    int h = 0;
    SDL_GetWindowSize(window, NULL, &h);
    return h;
}
