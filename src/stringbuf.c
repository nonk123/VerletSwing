#include <stddef.h>

#include <SDL3/SDL_stdinc.h>

#include "stringbuf.h"

void sb_write(StringBuf* self, const char* s) {
    if (!self->ptr)
        self->ptr = self->buf;

    const size_t capacity = sizeof(self->buf), used = (size_t)(self->ptr - self->buf);

    if (used < capacity)
        self->ptr += SDL_strlcpy(self->ptr, s, capacity - used);
    else
        self->ptr = self->buf + capacity;
}

const char* sb_string(const StringBuf* self) {
    return self ? self->buf : "";
}
