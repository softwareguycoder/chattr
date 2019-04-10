/*
 * client_manager.h
 *
 *  Created on: Apr 8, 2019
 *      Author: bhart
 */

#ifndef __CLIENT_MANAGER_H__
#define __CLIENT_MANAGER_H__

#include "client_symbols.h"

void GetNickname(char* nickname, int size);
void GreetServer();
void HandshakeWithServer();
void LeaveChatRoom();
void PrintClientUsageDirections();
void ReceiveFromServer(int client_socket, char* reply_buffer);
void SetNickname(const char* nickname);

#endif /* __CLIENT_MANAGER_H__ */

