/*
 * client_manager.h
 *
 *  Created on: Apr 8, 2019
 *      Author: bhart
 */

#ifndef ___CLIENT_MANAGER_H__
#define ___CLIENT_MANAGER_H__

#include "client_struct.h"

int BroadcastToAllClients(const char* pszMessage);
int BroadcastToAllClientsExceptSender(const char* pszMessage,
		LPCLIENTSTRUCT lpSendingClient);
void ForciblyDisconnectClient(LPCLIENTSTRUCT lpCS);
int ReplyToClient(LPCLIENTSTRUCT lpCS, const char* pszBuffer);

#endif /* ___CLIENT_MANAGER_H__ */
