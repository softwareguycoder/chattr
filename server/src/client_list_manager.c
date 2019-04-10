/*
 * client_list_manager.c
 *
 *  Created on: Apr 3, 2019
 *      Author: bhart
 */

#include "stdafx.h"

#include "client_struct.h"

BOOL FindClientBySocket(void* pClientSocketFd, void* pClientStruct) {
	LogDebug("In FindClientBySocket");

	LogInfo(
			"FindClientBySocket: Checking whether both parameters are filled in...");

	if (pClientSocketFd == NULL || pClientStruct == NULL) {
		LogWarning(
				"FindClientBySocket: One or both parameters not specified.");

		LogDebug("FindClientBySocket: Returning FALSE.");

		LogDebug("FindClientBySocket: Done.");

		return FALSE;
	}

	LogInfo(
			"FindClientBySocket: Attempting to retrieve client socket file descriptor...");

	int clientSockFd = *((int*) pClientSocketFd);

	LogDebug("FindClientBySocket: clientSockFd = %d", clientSockFd);

	LogInfo(
			"FindClientBySocket: Attempting to retrieve client structure pointer...");

	CLIENTSTRUCT* client_Struct = (CLIENTSTRUCT*) pClientStruct;

	LogInfo(
			"FindClientBySocket: Checking whether the client socket is associated with the client structure...");

	if (clientSockFd == client_Struct->sockFD) {
		LogInfo(
				"FindClientBySocket: Client structure matching the supplied socket value found.");

		LogDebug("FindClientBySocket: Returning TRUE.");

		LogDebug("FindClientBySocket: Done.");

		return TRUE;
	}

	LogDebug("FindClientBySocket: Returning FALSE.");

	LogDebug("FindClientBySocket: Done.");

	return FALSE;
}

