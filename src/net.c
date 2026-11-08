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
