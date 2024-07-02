#include "MirrorBotTwitch.h"
#include "Queue.h"
#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <wininet.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "Ws2_32.lib")

//Encodes the messages to be sent over the IRC
std::string encode(const std::string& STR)
{
    std::string result;
    for (char c : STR) {
        switch (c) {
        case '\x10': result += "\x10\x10"; break;
        case '\0': result += "\x10""0"; break;
        case '\n': result += "\x10n"; break;
        case '\r':result += "\x10r"; break;
        default: result += c;
        }
    }
    return result;
}

//Decodes the messages
std::string uncode(const std::string& STR)
{
    std::string result = STR;
    std::string::size_type index = 0;
    while ((index = result.find("\x10", index)) != std::string::npos)
    {
        result.erase(index, 1);

        if (index < result.length()) {
            switch (result[index]) {
            case '0': result[index] = '\0'; break;
            case 'n': result[index] = '\n'; break;
            case 'r': result[index] = '\r'; break;
            }
        }
        ++index;
    }
    return result;
}

//Delims the messages
std::string parse(std::string& STR, const std::string& delim)
{
    std::string result;
    auto pos = STR.find(delim);
    if (pos == std::string::npos)
    {
        result = std::move(STR);
        STR.clear();
    }
    else
    {
        result = STR.substr(0, pos);
        STR.erase(0, pos + delim.length());
    }
    return result;
}

//handles the sending length and cut off of sending
bool sendStr(SOCKET ket, const std::string& STR)
{
    int totalSent = 0;
    int len = STR.length();

    while (totalSent < len)
    {
        int ret = send(ket, STR.c_str() + totalSent, len - totalSent, 0);
        if (ret == SOCKET_ERROR)
        {
            std::cout << "send() error: " << WSAGetLastError() << std::endl;
            return false;
        }
        totalSent += ret;
    }

    return true;
}

//Sends the messages
bool sendCmd(SOCKET& ket, const std::string& cmd)
{
    std::cout << "Sending: " << cmd << std::endl;
    return sendStr(ket, encode(cmd)) && sendStr(ket, "\r\n");

}

//Handles errors in the server; Handles Ping/Pong server rquest; Sends to Discord Queue
void toQueue(SOCKET& ket) {
    //Creates Buffer
    char buf[1024];
    std::string LineBuffer;
    std::string::size_type StartIdx = 0;

    do
    {
        int ret = recv(ket, buf, sizeof(buf), 0);

        //Server Error
        if (ret == SOCKET_ERROR)
        {
            std::cout << "recv() error: " << WSAGetLastError() << std::endl;
            closesocket(ket);
        }

        if (ret == 0)
        {
            std::cout << "Server disconnected" << std::endl;
            break;
        }

        //Gets message from Twitch
        LineBuffer.append(buf, ret);

        do
        {
            //Finds start of message + user pair
            std::string::size_type pos = LineBuffer.find('\n', StartIdx);
            if (pos == std::string::npos)
                break;

            //Finds length of pos
            std::string::size_type len = pos;
            if ((pos > 0) && (LineBuffer[pos - 1] == '\r'))
                --len;

            //Decodes the Message
            std::string msg = uncode(LineBuffer.substr(0, len));
            LineBuffer.erase(0, pos + 1);
            StartIdx = 0;

            //Declares the user name std::string
            std::string senderNick;

            //Removes the : and finds the user name
            if (!msg.empty() && (msg[0] == ':'))
            {
                std::string tmp = parse(msg, " ");
                tmp.erase(0, 1); // remove ':'
                senderNick = parse(tmp, "!");
            }

            //Sends message to Discord if there is !chat is in the message or Mirror mode is true and it is a user generated message
            while (!msg.empty() && (MirrorT == true || msg.find("!chat") != std::string::npos) && msg.find("PRIVMSG") != std::string::npos) {
                sendToDiscordQueue(msg);
                break;
            }

            //Handles the ping/pong request from the IRC server
            if (msg == "PING") {
                sendCmd(ket, "PONG :" + TWITCH_SERVER);
            }
            msg.clear();

            //Handles closing the thread if End command is called
            if (ENDST == true) {
                goto end;
            }
        } while (true);
    } while (true);
    end:
        std::cout << "Hope you had a great workout. From Sending" << std::endl;
}

//Handles messages coming from Discord
void fromQueue(SOCKET& ket) {
    //Creates buffer
    char buf[1024];
    std::string LineBuffer;
    std::string::size_type StartIdx = 0;

    //Main while loop
    do
    {
        int ret = recv(ket, buf, sizeof(buf), 0);
        //Socket error handeling
        if (ret == SOCKET_ERROR)
        {
            std::cout << "recv() error: " << WSAGetLastError() << std::endl;
            closesocket(ket);
        }

        if (ret == 0)
        {
            std::cout << "Server disconnected" << std::endl;
            break;
        }

        //While loop to get messages
        do
        {
            if (!twitchMessageQueue.empty()) {
                std::string msg = retrieveFromTwitchQueue();
                msg = "PRIVMSG #" + TWITCH_NICK + " :" + msg; 
                sendCmd(ket, msg); 
            }

            //Handles the End command
            if (ENDRT == true) {
                goto end;
            }
            
        } while (true);
    } while (true);
   end:
        std::cout << "Hope you had a great workout. From Receving." << std::endl;
}
