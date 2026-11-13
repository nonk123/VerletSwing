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

void send_chat_message(const char*);
void send_announcement(const char*);

const char* get_player_name(NutBlast_PlayerID);
