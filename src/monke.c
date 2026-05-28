#include <SDL3/SDL_video.h>

#include "monke.h"

void init_monke(Monke* this) {
    int w = 0, h = 0;

    extern SDL_Window* window;
    SDL_GetWindowSize(window, &w, &h);

    Vec2 pos = {FxFrom(w), FxFrom(h)};
    pos = Vscale(pos, FxFrom(0.5));
    init_verlet(&this->body, pos);
}
