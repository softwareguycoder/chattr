/*
 * client_manager.h
 *
 *  Created on: Apr 8, 2019
 *      Author: bhart
 */

#ifndef __CLIENT_MANAGER_H__
#define __CLIENT_MANAGER_H__

#include "client_symbols.h"

void GetNickname(char* pszNickname, int nSize);
void GreetServer();
void HandshakeWithServer();
void LeaveChatRoom();
void PrintClientUsageDirections();
void ReceiveFromServer(char* pszReplyBuffer);
void SetNickname(const char* nickname);

#endif /* __CLIENT_MANAGER_H__ */

