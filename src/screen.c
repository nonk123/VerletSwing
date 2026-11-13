#include "chat.h"
#include "draw.h"
#include "fmt.h"
#include "game.h"
#include "net.h"
#include "screen.h"
#include "sdl.h"

static ScreenIndex cur_screen = SCR_NONE;

double timestep() {
    return cur_screen == SCR_PLAY ? 1.0 / TICKRATE : 0.0;
}

typedef struct {
    void (*init)(), (*update)(), (*deinit)();
} Screen;

static void on_ready() {
    goto_screen(SCR_PLAY);
}

static void init_scr_join() {
    NutBlast_OnReady(on_ready);

    extern void on_player_joined(NutBlast_PlayerID);
    NutBlast_OnPlayerJoined(on_player_joined);

    extern void on_player_left(NutBlast_PlayerID, NutBlast_Reason);
    NutBlast_OnPlayerLeft(on_player_left);
}

static void update_scr_join() {
    clear_screen(BACKGROUND_BASE);

    const double fs = 48.0;
    const char* s = NULL;

    if (NutBlast_IsConnecting()) {
        s = "Joining...";
    } else if (tap) {
        NutBlast_SetLobbyField("seed", fmt("%u", SDL_rand_bits()));
        NutBlast_IDGAF((NutBlast_HostOptions){.max_players = 4});
    } else {
        s = "Tap to join a lobby";
    }

    if (s)
        draw_text(XY(0.5 * (w_width() - text_width(fs, s)), 0.5 * (w_height() - fs)), fs, s);
}

static void init_scr_play() {
    restart();
    clear_chat();
}

static void update_scr_play() {
    if (NutBlast_IsOnline())
        update_game();
    else
        goto_screen(SCR_JOIN);
}

static void deinit_scr_play() {
    extern void deinit_game();
    deinit_game();
}

static Screen table[SCR_MAX] = {
    [SCR_NONE] = {},
    [SCR_JOIN] = {.init = init_scr_join, .update = update_scr_join},
    [SCR_PLAY] = {.init = init_scr_play, .update = update_scr_play, .deinit = deinit_scr_play},
};

void goto_screen(ScreenIndex idx) {
    if (cur_screen != SCR_NONE && table[cur_screen].deinit)
        table[cur_screen].deinit();

    cur_screen = idx;

    if (cur_screen == SCR_NONE) {
        extern bool PERMADEATH;
        PERMADEATH = true;
    } else if (table[cur_screen].init) {
        table[cur_screen].init();
    }
}

void update_screen() {
    if (cur_screen != SCR_NONE && table[cur_screen].update)
        table[cur_screen].update();

    tap = false;
}
