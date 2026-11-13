#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>

#include "draw.h"
#include "fmt.h"
#include "net.h"
#include "sdl.h"

#define POP_INTERVAL (5.0)
static char LINES[8][256] = {0};

static double pop_timer = POP_INTERVAL;

void clear_chat() {
    for (size_t i = 0; i < SDL_arraysize(LINES); i++)
        SDL_memset(LINES[i], 0, sizeof(LINES[i]));
}

static void set_line(size_t i, const char* s) {
    SDL_strlcpy(LINES[i], s, sizeof(LINES[i]));
}

static void pop_front() {
    for (size_t i = 0; i < SDL_arraysize(LINES) - 1; i++)
        set_line(i, LINES[i + 1]);
    set_line(SDL_arraysize(LINES) - 1, "");
}

static void push_line(const char* s) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", s);
    pop_timer = POP_INTERVAL;

    for (size_t i = 0; i < SDL_arraysize(LINES) - 1; i++) {
        if (!LINES[i][0]) {
            set_line(i, s);
            return;
        }
    }

    pop_front();
    set_line(SDL_arraysize(LINES) - 1, s);
}

void display_chat_message(NutBlast_PlayerID from, const char* s) {
    push_line(fmt("%s: %s", get_player_name(from), s));
}

void display_announcement(const char* s) {
    push_line(fmt("ANAL: %s", s));
}

void update_chat() {
    NutBlast_Message msg = {0};

    while (NutBlast_NextMessage(CHAN_CHAT, &msg))
        display_chat_message(msg.from, msg.data);

    while (NutBlast_NextMessage(CHAN_ANNOUNCEMENTS, &msg))
        display_announcement(msg.data);

    pop_timer -= delta();

    if (pop_timer <= 1e-3) {
        pop_timer = POP_INTERVAL;
        pop_front();
    }

    const double fs = 32.0, pad = 8.0;

    for (int i = 0; i < SDL_arraysize(LINES); i++)
        draw_text(XY(pad, w_height() - SDL_arraysize(LINES) * fs + i * fs - pad), fs, LINES[i]);
}
