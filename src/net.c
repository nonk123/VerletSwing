#include "chat.h"
#include "net.h"

void send_str_to(SendOptions opts) {
    NutBlast_Send(opts);
}

void spread_str(SendOptions opts) {
    for (const NutBlast_PlayerID* ids = NutBlast_ListPlayers(); *ids; ids++) {
        opts.to = *ids;
        send_str_to(opts);
    }
}

void send_chat_message(const char* s) {
    display_chat_message(NutBlast_GetPlayerID(), s);

    spread_str((SendOptions){
        .data = s,
        .channel = CHAN_CHAT,
        .reliable = true,
    });
}

void send_announcement(const char* s) {
    display_announcement(s);

    spread_str((SendOptions){
        .data = s,
        .channel = CHAN_ANNOUNCEMENTS,
        .reliable = true,
    });
}

const char* get_player_name(NutBlast_PlayerID id) {
    if (!id)
        id = NutBlast_GetPlayerID();
    return NutBlast_GetPlayerField(id, NUTBLAST_FIELD_PLAYER_NAME);
}
