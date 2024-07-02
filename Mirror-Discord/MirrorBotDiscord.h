#pragma once
#ifndef MIRRORBOTDISCORD_H
#define MIRRORBOTDISCORD_H

#include <dpp/dpp.h>

extern volatile bool MirrorD;
extern volatile bool MirrorT;
extern volatile bool END;
extern volatile bool ENDST;
extern volatile bool ENDRT;
extern dpp::cluster& bot;

void message_create_event(dpp::cluster& bot);
void handle_SlashCommands(dpp::cluster& bot);
void runDiscordBot(dpp::cluster& bot);

#endif
