#include "Queue.h"
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

//Establishes two Queues; Two needed to ensure simultaneous message relay
std::queue<std::string> discordMessageQueue;
std::mutex discordMessageMutex;
std::condition_variable discordMessageCV;

std::queue<std::string> twitchMessageQueue;
std::mutex twitchMessageMutex;
std::condition_variable twitchMessageCV;


//Handles sending messages to the discord queue
std::string sendToDiscordQueue(const std::string& msg) {
	std::lock_guard<std::mutex> lock(discordMessageMutex);
	discordMessageQueue.push(msg);
	discordMessageCV.notify_one();
	return msg;
}

//Handles retrieving messages from the discord queue
std::string retrieveFromDiscordQueue() {
	std::unique_lock<std::mutex> lock(discordMessageMutex);
	discordMessageCV.wait(lock, [] { return !discordMessageQueue.empty(); });

	std::string msg = std::move(discordMessageQueue.front());
	discordMessageQueue.pop();
	return msg;
}

//Handles sending messages to the twitch queue
std::string sendToTwitchQueue(const std::string& msg) {
	std::lock_guard<std::mutex> lock(twitchMessageMutex);
	twitchMessageQueue.push(msg);
	twitchMessageCV.notify_one();
	return msg;
}

//Handles retrieving messages from the twitch queue
std::string retrieveFromTwitchQueue() {
	std::unique_lock<std::mutex> lock(twitchMessageMutex);
	twitchMessageCV.wait(lock, [] {return !twitchMessageQueue.empty(); });
	

	std::string msg = std::move(twitchMessageQueue.front());
	twitchMessageQueue.pop();
	return msg;
}
