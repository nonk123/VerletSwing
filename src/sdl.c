#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>

#define S_TRUCTURES_IMPLEMENTATION
#define S_Alloc SDL_malloc
#define S_Free SDL_free
#define S_Memset SDL_memset
#define S_Memcpy SDL_memcpy
#include <S_tructures.h>

#include <NutBlast.h>

#include "cmake.h"
#include "fmt.h"
#include "net.h"
#include "screen.h"
#include "sdl.h"

#define SWIPE_THRESHOLD (0.08)
#define SWIPE_TIMEOUT (0.25)

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

bool left_press = false, right_press = false, tap = false;

static SDL_AppResult SDL_Fail() {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "%s", SDL_GetError());
    return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppInit(void** ctx, int argc, char* argv[]) {
    (void)ctx, (void)argc, (void)argv;

    NutBlast_Init((NutBlast_InitOptions){
        .game_id = GAME_TITLE " " GAME_VERSION,
        .max_channels = CHAN_MAX,
    });

    SDL_SetAppMetadata(GAME_TITLE, GAME_VERSION, GAME_PACKAGE);
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
        return SDL_Fail();

    if (!SDL_CreateWindowAndRenderer(GAME_TITLE, 800, 600, 0, &window, &renderer))
        return SDL_Fail();

    const char* name = fmt("%04d", SDL_rand(10000));
    NutBlast_SetPlayerField(NUTBLAST_FIELD_PLAYER_NAME, name);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "We are %s", name);

    SDL_SetRenderVSync(renderer, SDL_RENDERER_VSYNC_ADAPTIVE);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    goto_screen(SCR_TAP_TO_JOIN);

    return SDL_APP_CONTINUE;
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
        tap = true;

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
        tap = true;

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

#define NANOSEC (1000000000.0)
static uint64_t then = 0;
static double dt = 0.0;

double delta() {
    return dt;
}

bool PERMADEATH = false;

SDL_AppResult SDL_AppIterate(void* ctx) {
    (void)ctx;

    NutBlast_Update();

    if (PERMADEATH)
        return SDL_APP_SUCCESS;

    const uint64_t now = SDL_GetTicksNS();
    dt = (double)(now - then) / NANOSEC;
    then = now;

    update_screen();
    NutBlast_Flush();

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

    NutBlast_Cleanup();
}

int w_width() {
    int w = 0;
    SDL_GetWindowSizeInPixels(window, &w, NULL);
    return w;
}

int w_height() {
    int h = 0;
    SDL_GetWindowSizeInPixels(window, NULL, &h);
    return h;
}
