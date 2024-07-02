#pragma once
#ifndef MIRRORBOTTWITCH_H
#define MIRRORBOTTWITCH_H

#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <wininet.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>


extern volatile bool MirrorT;
extern volatile bool ENDRT;
extern volatile bool ENDST;
extern std::string TWITCH_OAUTH;
extern const std::string TWITCH_SERVER;
extern const std::string TWITCH_NICK;

std::string encode(const std::string& STR); //encoder
std::string uncode(const std::string& STR); //decoder
std::string parse(std::string& STR, const std::string& delim); //parser
bool sendStr(SOCKET ket, const std::string& STR); //controls the send
bool sendCmd(SOCKET& ket, const std::string& cmd); //sends to twitch
void toQueue(SOCKET& ket); //sends to Queue and handles decoding
void fromQueue(SOCKET& ket); //Retrieves from Queue
#endif 
