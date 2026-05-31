#include <S_tructures.h>

#include "assets.h"
#include "fmt.h"

static TinyMap assets = {0};
extern SDL_Renderer* renderer;

static void* get_asset(const char* name, void* (*const load)(const char* path)) {
    void* ptr = TinyDictGet(&assets, name);

    if (ptr)
        return *(void**)ptr;

    ptr = load(fmt("%s%s", ASSETS, name));

    if (!ptr) {
        // FIXME: UHHHH WE SHIT THE BED!!!
    }

    TinyDictPut(&assets, name, (void*)&ptr, sizeof(void*));
    // TODO: attach proper nukes.

    return ptr;
}

static void* load_texture(const char* path) {
    SDL_Surface* surf = SDL_LoadPNG(path);

    if (!surf)
        return NULL;

    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);

    if (!tex) {
        SDL_DestroySurface(surf);
        return NULL;
    }

    SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
    SDL_DestroySurface(surf);

    return tex;
}

SDL_Texture* gettex(const char* name) {
    return get_asset(name, load_texture);
}
