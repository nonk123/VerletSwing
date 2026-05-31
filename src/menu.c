#include "S_tructures.h"

#include "draw.h"
#include "fmt.h"
#include "game.h"
#include "menu.h"
#include "sdl.h"
#include "vec2.h"

#define TICKRATE (60)
#define CENTER(fs, s) (0.5 * (w_width() - text_width((fs), (s))))

bool chicken_dinner = false;

static void tick_main() {
    static bool tainted = false;

    static const double fs = 48.0, hfs = 0.5 * fs;
    const double y = 0.5 * (w_height() - fs);

    draw_game();

    const char* s = fmt("Tap to %s", tainted ? "Restart" : "Start");
    draw_text(XY(CENTER(fs, s), y - 0.5 * fs), fs, s);

    s = fmt("High-score: %llu", hiscore);
    draw_text(XY(CENTER(fs, s), y + 1.0 * fs), fs, s);

    if (chicken_dinner) {
        s = "NEW RECORD!";
        draw_text(XY(CENTER(hfs, s), y + 2.0 * fs), hfs, s);
    }

    if (tap) {
        tainted = true;
        restart();
        push_menu(MEN_GAME);
    }
}

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

typedef struct {
    void (*tick)();
    Menu self_id;
} MenuStack;

static MenuStack* stack = NULL;
static MenuStack templates[MEN_MAX] = {
    [MEN_MAIN] = {tick_main},
    [MEN_GAME] = {tick_game},
};

static void fucking_die() {
    extern bool PERMADEATH;
    PERMADEATH = true;
}

double timestep() {
    return cur_menu() == MEN_GAME ? 1.0 / TICKRATE : 0.0;
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
}
