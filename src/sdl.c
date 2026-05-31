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
#include "fmt.h"
#include "game.h"
#include "menu.h"
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

    SDL_SetAppMetadata(GAME_TITLE, GAME_VERSION, GAME_PACKAGE);

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
        return SDL_Fail();

    static const uint32_t wf = SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED;
    if (!SDL_CreateWindowAndRenderer(GAME_TITLE, 800, 600, wf, &window, &renderer))
        return SDL_Fail();

    if (!sdl_load_font())
        return SDL_Fail();

    SDL_SetRenderVSync(renderer, SDL_RENDERER_VSYNC_ADAPTIVE);

    load_hi();
    push_menu(MEN_MAIN);

    SDL_ShowWindow(window);

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

#define NANOSEC (1000000000.0)
static uint64_t then = 0;
static double dt = 0.0;

double delta() {
    return dt;
}

bool PERMADEATH = false;

SDL_AppResult SDL_AppIterate(void* ctx) {
    (void)ctx;

    if (PERMADEATH)
        return SDL_APP_SUCCESS;

    const uint64_t now = SDL_GetTicksNS();
    dt = (double)(now - then) / NANOSEC;
    then = now;

    tick_menu();
    tap = false;

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

SDL_Storage* open_user_storage() {
    return SDL_OpenUserStorage("nonk", GAME_CODENAME, 0);
}

void load_hi() {
    SDL_Storage* store = open_user_storage();

    if (!store)
        return;

    static char buf[32] = {0};

    uint64_t len = 0;
    const bool ok = SDL_GetStorageFileSize(store, "hi", &len);

    if (ok && len && len < sizeof(buf)) {
        SDL_ReadStorageFile(store, "hi", buf, len);
        hiscore = SDL_strtoull(buf, NULL, 10);
    }

    SDL_CloseStorage(store);
}

void maybe_save_hi() {
    if (score < hiscore)
        return;

    hiscore = score;
    chicken_dinner = true;

    SDL_Storage* store = open_user_storage();

    if (!store)
        return;

    const char* s = fmt("%llu", hiscore);
    SDL_WriteStorageFile(store, "hi", s, SDL_strlen(s));

    SDL_CloseStorage(store);
}
