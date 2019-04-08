/*
 * client_manager.h
 *
 *  Created on: Apr 8, 2019
 *      Author: bhart
 */

#ifndef ___CLIENT_MANAGER_H__
#define ___CLIENT_MANAGER_H__

extern BOOL g_bShouldTerminateClientThread;

void ForciblyDisconnectClient(LPCLIENTSTRUCT lpCurrentClientStruct);

#endif /* ___CLIENT_MANAGER_H__ */
