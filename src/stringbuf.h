#pragma once

typedef struct {
    char buf[4096], *ptr;
} StringBuf;

void sb_write(StringBuf*, const char*);
const char* sb_string(const StringBuf*);
