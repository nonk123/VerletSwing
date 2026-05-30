#include <SDL3/SDL_stdinc.h>

#include "fmt.h"

#define MAX_BUFS (8)

const char* fmt(const char* message, ...) {
    static char buf[MAX_BUFS * 128] = {0};
    static const size_t linelen = sizeof(buf) / MAX_BUFS;

    static char* cur = buf;
    cur += linelen;

    if (cur >= buf + sizeof(buf))
        cur = buf;

    va_list args = {0};

    va_start(args, message);
    SDL_vsnprintf(cur, linelen, message, args);
    va_end(args);

    return cur;
}
