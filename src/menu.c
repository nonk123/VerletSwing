#include "S_tructures.h"

#include "draw.h"
#include "fmt.h"
#include "game.h"
#include "menu.h"
#include "sdl.h"
#include "vec2.h"

#define TICKRATE (60)

typedef struct {
    void (*tick)();
    Menu self_id;
} MenuStack;

static void tick_main() {
    static bool tainted = false;

    draw_game();

    const double fs = 48.0;
    const char* s = fmt("Tap to %s", tainted ? "Restart" : "Start");
    draw_text(XY(0.5 * (w_width() - text_width(fs, s)), 0.5 * (w_height() - fs)), fs, s);

    if (just_tapped()) {
        tainted = true;
        restart();
        push_menu(MEN_GAME);
    }
}

double timestep() {
    return cur_menu() == MEN_GAME ? 1.0 / TICKRATE : 0.0;
}

extern bool left_press, right_press;

static void tick_game() {
    static double ticks = 0.0;
    ticks += delta() * TICKRATE;

    if (cur_menu() != MEN_GAME)
        ticks = 0.0;

    for (; ticks >= 1.0; ticks -= 1.0) {
        update();
        right_press = false;
    }

    draw_game();
}

static MenuStack* stack = NULL;
static MenuStack templates[MEN_MAX] = {
    [MEN_MAIN] = {tick_main},
    [MEN_GAME] = {tick_game},
};

static void fucking_die() {
    extern bool PERMADEATH;
    PERMADEATH = true;
}

Menu cur_menu() {
    return stack[TinyDLength(stack) - 1].self_id;
}

void after_menu_changed() {
    left_press = right_press = false;
}

void pop_menu() {
    if (TinyDLength(stack))
        stack = TinyDPopBack(stack);

    after_menu_changed();

    if (!TinyDLength(stack))
        fucking_die();
}

void push_menu(Menu next) {
    if (next == MEN_NULL) {
        fucking_die();
        return;
    }

    if (!stack)
        stack = MakeTinyD(MenuStack);

    stack = TinyDAppendPro(stack, &templates[next]);
    stack[TinyDLength(stack) - 1].self_id = next;

    after_menu_changed();
}

void tick_menu() {
    MenuStack* last = &stack[TinyDLength(stack) - 1];

    if (last->tick)
        last->tick();

    update_camera();
}

extern uint64_t score, hiscore;

void load_hi() {
    SDL_Storage* store = open_user_storage();

    if (!store)
        return;

    // TODO: implement.

    SDL_CloseStorage(store);
}

void maybe_save_hi() {
    if (score < hiscore)
        return;

    SDL_Storage* store = open_user_storage();

    if (!store)
        return;

    // TODO: implement.

    SDL_CloseStorage(store);
}
