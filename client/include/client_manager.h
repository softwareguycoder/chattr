/*
 * client_manager.h
 *
 *  Created on: Apr 8, 2019
 *      Author: bhart
 */

#ifndef __CLIENT_MANAGER_H__
#define __CLIENT_MANAGER_H__

#define PROTOCOL_HELO_COMMAND	"HELO\n"		// Protocol command that gets this client marked as a member of the chat room
#define PROTOCOL_NICK_COMMAND	"NICK %s\n"		// Protocol command that registers this user's chat handle with the server
#define PROTOCOL_QUIT_COMMAND	"QUIT\n"		// Protocol command that 'logs the client off' from the chat server.
#define NICKNAME_PROMPT			"> Please type a nickname (15 chars max): > "
#define USAGE_MESSAGE			"Type a line and press ENTER to send it to the chat room.\n\n"

void GetNickname(char* nickname, int size);
void GreetServer();
void HandshakeWithServer();
void LeaveChatRoom();
void PrintClientUsageDirections();
void ReceiveFromServer(int client_socket, char* reply_buffer);
void SetNickname(const char* nickname);

#endif /* __CLIENT_MANAGER_H__ */

