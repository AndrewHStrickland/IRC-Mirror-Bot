#include "MirrorBotDiscord.h"
#include "MirrorBotTwitch.h"
#include <dpp/dpp.h>
#include <iostream>


//Token
const std::string BOT_TOKEN = "<Discord-Token>";
const std::string TWITCH_SERVER = "<Twitch-Server>";
const int TWITCH_PORT = 6667;
const std::string TWITCH_NICK = "<Username-Of-Bot-Account>";
std::string TWITCH_OAUTH;
const std::string TWITCH_CHANNEL = "<Twitch-Channel-To-Join>";
volatile bool MirrorD = false;
volatile bool MirrorT = false;
volatile bool END = false;
volatile bool ENDRT = false;
volatile bool ENDST = false;

int main() {
    //Starts the bot cluster
    dpp::cluster bot(BOT_TOKEN, dpp::i_default_intents | dpp::i_message_content);

    //Tells bot to log
    bot.on_log(dpp::utility::cout_logger());


    /*Initialize IRC libraries*/
    WSADATA wsaData;
    HINTERNET hInternet = InternetOpen(L"TwitchBot", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        std::cerr << "Failed to initialize WinINet." << std::endl;
        return 0;
    }

    int ref = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (ref != 0) {
        std::cout << "Winsock Error: " << ref << std::endl;
        return 0;
    }

    //Channels called and errors handled
    SOCKET ket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ket == INVALID_SOCKET)
    {
        std::cout << "Ket Error: " << WSAGetLastError() << std::endl;
        return 0;
    }

    //Set up address resolution hints
    struct addrinfo hints, * servinfo;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;       // Use AF_INET for IPv4 addresses
    hints.ai_socktype = SOCK_STREAM; // Use SOCK_STREAM for TCP

    //Resolve the server address and port
    int getaddrResult = getaddrinfo(TWITCH_SERVER.c_str(), std::to_string(TWITCH_PORT).c_str(), &hints, &servinfo);
    if (getaddrResult != 0) {
        std::cout << "getaddrinfo failed: " << getaddrResult << std::endl;
        WSACleanup();
        return 0;
    }

    //Connection and error handling
    if (connect(ket, servinfo->ai_addr, (int)servinfo->ai_addrlen) != 0)
    {
        std::cout << "connect() error: " << WSAGetLastError() << std::endl;
        freeaddrinfo(servinfo);
        WSACleanup();
        return 0;
    }
    std::cout << "connected" << std::endl;

    //Clean up the address info structure
    freeaddrinfo(servinfo);
   
    //Request Oauth code for login
    HINTERNET hUrl = InternetConnectA(hInternet, "id.twitch.tv", INTERNET_DEFAULT_HTTPS_PORT,
        NULL, NULL, INTERNET_SERVICE_HTTP, 0, 1);
    if (hUrl) {
        HINTERNET hRequest = HttpOpenRequestA(hUrl, "POST", "/oauth2/token", NULL, NULL,
            NULL, INTERNET_FLAG_SECURE | INTERNET_FLAG_RELOAD, 1);
        if (hRequest) {
            std::string postData = "grant_type=refresh_token";
            postData += "&refresh_token=<Refresh-Token-Here>";
            postData += "&client_id=<Client-Id-Here>";
            postData += "&client_secret=<Secret-Here>";

            std::string headers = "Content-Type: application/x-www-form-urlencoded";

            bool sent = HttpSendRequestA(hRequest, headers.c_str(), headers.length(),
                (LPVOID)postData.c_str(), postData.length());
                
            char buffer[1024];
            DWORD bytesRead = 0;
            while (InternetReadFile(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
                TWITCH_OAUTH.append(buffer, bytesRead);
                }
            TWITCH_OAUTH = TWITCH_OAUTH.substr(17, 30);
            TWITCH_OAUTH = "oauth:" + TWITCH_OAUTH;
            std::cout << TWITCH_OAUTH << std::endl;
        }
    }

    //Sends loging information to Twitch
    sendCmd(ket, "PASS " + TWITCH_OAUTH);
    sendCmd(ket, "NICK " + TWITCH_NICK);
    sendCmd(ket, "JOIN " + TWITCH_CHANNEL);

    InternetCloseHandle(hInternet);
    
    //Discord Bot
    
     message_create_event(bot);
     handle_SlashCommands(bot);
    
    std::thread discordThread([&bot] {
        runDiscordBot(bot);
        });

    //Twitch Bot Threads
    std::thread toQueue([&ket] {
        toQueue(ket);
        });
    std::thread fromQueue([&ket] { 
        fromQueue(ket); 
        });
    

    //starts the cluster and returns to code
	bot.start(dpp::st_return);
   
    //waits for three threads to return
    fromQueue.join();
    toQueue.join();
    discordThread.join();


    //cleanup and shutdown
    closesocket(ket); 
    bot.shutdown();

	return true;
}
