#pragma once
#ifndef QUEUE_H
#define QUEUE_H

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

extern std::queue<std::string> discordMessageQueue;
extern std::mutex discordMessageMutex;
extern std::condition_variable discordMessageCV;

extern std::queue<std::string> twitchMessageQueue;
extern std::mutex twitchMessageMutex;
extern std::condition_variable twitchMessageCV;

extern std::string msg;

extern std::string retrieveFromDiscordQueue();
extern std::string sendToDiscordQueue(const std::string& msg);
extern std::string sendToTwitchQueue(const std::string& msg);
extern std::string retrieveFromTwitchQueue();


#endif
