#pragma once

#include "net.h"

void clear_chat(void);
void update_chat(void);

void display_chat_message(NutBlast_PlayerID, const char*);
void display_announcement(const char*);
