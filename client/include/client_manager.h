/*
 * client_manager.h
 *
 *  Created on: Apr 8, 2019
 *      Author: bhart
 */

#ifndef __CLIENT_MANAGER_H__
#define __CLIENT_MANAGER_H__

#include "client_symbols.h"

BOOL GetNickname(char* pszNickname);
void GreetServer();
void HandshakeWithServer();
void LeaveChatRoom();
void PrintClientUsageDirections();
void ProcessReceivedText(const char* pszReceivedText, int nSize);
int ReceiveFromServer(char** ppszReplyBuffer);
BOOL SetNickname(const char* nickname);
BOOL ShouldStopReceiving(const char* pszReceivedText, int nSize);

#endif /* __CLIENT_MANAGER_H__ */

