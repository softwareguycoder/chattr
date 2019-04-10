/*
 * client_manager.h
 *
 *  Created on: Apr 8, 2019
 *      Author: bhart
 */

#ifndef ___CLIENT_MANAGER_H__
#define ___CLIENT_MANAGER_H__

#include "client_struct.h"

extern BOOL g_bShouldTerminateClientThread;

int BroadcastAll(const char* pszMessage);
void DisconnectClient(void* pClientStruct);
void ForciblyDisconnectClient(LPCLIENTSTRUCT lpCurrentClientStruct);
void ReplyToClient(LPCLIENTSTRUCT lpClientStruct, const char* pszBuffer);

#endif /* ___CLIENT_MANAGER_H__ */
