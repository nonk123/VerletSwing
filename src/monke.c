#include <SDL3/SDL_video.h>

#include "monke.h"

void init_monke(Monke* this) {
    int w = 0;

    extern SDL_Window* window;
    SDL_GetWindowSize(window, &w, NULL);

    Vec2 pos = {FxFrom(w), 0};
    pos = Vscale(pos, FxFrom(0.5));
    init_verlet(&this->body, pos);
}
