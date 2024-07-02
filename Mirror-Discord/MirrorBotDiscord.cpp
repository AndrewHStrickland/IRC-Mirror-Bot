#include "MirrorBotDiscord.h"
#include "Queue.h"
#include <dpp/dpp.h>
#include <iostream>
#include <chrono>
#include <thread>

/*IDs go here*/
const std::string CHANNEL_ID = "<ID-Of-Channel-Goes-Here>";
const std::string BOT_ID = "<ID-of-Bot-Goes-Here>"

//Finds command and sends messages to queue
void message_create_event(dpp::cluster& bot) {
    //When bot finds messages
    bot.on_message_create([&bot](const dpp::message_create_t& event) {

        //If mirror mode is on and in channel and not from bot id; Send message to twitch queue
        if (event.msg.channel_id == CHANNEL_ID && MirrorD == true && event.msg.author.id != BOT_ID) {
            std::string msg = event.msg.author.username + ": " + event.msg.content;
            if (twitchMessageQueue.empty()) {
                if (msg.at(0)=='.') {
                    msg = msg.erase(0, 1);
                }
                sendToTwitchQueue(msg);
            }
        }

        //If Mirror mode is off and !chat command is called and not from bot id; Send message to twitch queue
        else if (event.msg.channel_id == CHANNEL_ID && event.msg.content.find("!chat") != std::string::npos && event.msg.author.id != BOT_ID) {
            std::string content = event.msg.content;
            std::string::size_type n = 0;
            n = content.find_first_not_of("\t", n);
            n = content.find_first_of(" \t", n);
            content.erase(0, content.find_first_not_of(" \t", n));
            std::string msg = event.msg.author.username + ": " + content;
            if (msg.at(0) == '.') {
                msg = msg.erase(0, 1);
            }
            sendToTwitchQueue(msg);
        }
        });

}

//Handles slash commands
void handle_SlashCommands(dpp::cluster& bot) {
    //Runs all slash commands
    bot.on_slashcommand([&bot](const dpp::slashcommand_t& event) {

        //Finds permissions for user sending commands
        dpp::permission perms = event.command.get_resolved_permission(event.command.usr.id);

        //If user has correct permissions and uses a slash command
        if (perms.has(dpp::p_kick_members)) {

            //Turns on Mirror mode for both sides
            if (event.command.get_command_name() == "mirror") {
                MirrorT = true;
                MirrorD = true;
                event.reply(dpp::message("Mirror mode has been Activated. Enter /manual to turn off.").set_flags(dpp::m_ephemeral));
                return;
            }
            //Turns off Mirror for both sides
            else if (event.command.get_command_name() == "manual") {
                MirrorT = false;
                MirrorD = false;
                event.reply(dpp::message("Mirror mode has been Deactivated. Enter /mirror to turn on.").set_flags(dpp::m_ephemeral));
                return;
            }
            //Ends the programs
            else if (event.command.get_command_name() == "end") {
                ENDRT = true;
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                ENDST = true;
                sendToTwitchQueue("End");
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                END = true;
                event.reply(dpp::message("Bot is being turned off. Hope the workout went well").set_flags(dpp::m_ephemeral));
                return;
            }
        }
        
        //If user tries to use slash commands and doesn't have correct permissions; Send rejection message
        else {
            event.reply(dpp::message("My apologizes, you don't got the needed promissions for this action. Try !chat").set_flags(dpp::m_ephemeral));
            return;
        }
        });

    //Establishes new commands on startup
    bot.on_ready([&bot](const dpp::ready_t& event) {
        if (dpp::run_once<struct register_bot_commands>()) {
            dpp::slashcommand mirrorcommand("mirror", "Set mirror mode to true", bot.me.id);
            dpp::slashcommand manualcommand("manual", "Set mirror mode to true", bot.me.id);
            dpp::slashcommand endcommand("end", "Set END to true", bot.me.id);

            bot.global_bulk_command_create({ mirrorcommand, manualcommand, endcommand});
        }
        });
}

//Pulls user generated messages from queue
void runDiscordBot(dpp::cluster& bot) {

    //Establishes std::strings for the while loop
    const std::string prefix = "PRIVMSG #";
    const std::string proclimation = ": ";
    do {
            //Only run if there is a message in queue
            if (!discordMessageQueue.empty()) { 
                //find message
                std::string content = retrieveFromDiscordQueue(); 

                //removes "PRIVMSG #"
                if (content.compare(0, prefix.length(), prefix) == 0) { 
                    content = content.substr(prefix.length()); 
                }

                //splits name and message
                size_t pos = content.find(':'); 
                if (pos != std::string::npos) { 
                    std::string name = content.substr(0, pos); 
                    std::string message = content.substr(pos + 1); 
                    name.pop_back(); 

                    //removes the !chat command prompt
                    size_t pos2 = message.find("!chat"); 
                    if (pos2 != std::string::npos) { 
                        message.erase(pos2, 5); 
                        message.erase(0, 1); 
                    }

                    //Constructs final message
                    std::string doth_proclaim = name + proclimation + message; 
                    if (message.find("!manual") == std::string::npos && message.find("!mirror") == std::string::npos && content.find(prefix) != std::string::npos) {
                        //send new message
                        bot.message_create(dpp::message(CHANNEL_ID, doth_proclaim)); 
                    }

                }
            }
            
            //If end command is called; End Thread
            if (END == true) {
                goto end;
            }
        } while (END != true);
    end:
       std::cout << "Hope you had a great workout. From Discord" << std::endl;

}
