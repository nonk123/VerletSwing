#pragma once

#include <stdint.h>

#include <NutBlast.h> // IWYU pragma: export

typedef uint8_t NetChan;
typedef NutBlast_SendOptions SendOptions;

enum {
    CHAN_GAME,
    CHAN_CHAT,
    CHAN_ANNOUNCEMENTS,
    CHAN_MAX,
};

void send_str_to(SendOptions);
void spread_str(SendOptions);
