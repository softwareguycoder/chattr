/*
 * client_list_manager.c
 *
 *  Created on: Apr 3, 2019
 *      Author: bhart
 */

#include "stdafx.h"

#include "client_struct.h"

BOOL FindClientBySocket(void* pClientSocketFd, void* pClientStruct) {
	log_debug("In FindClientBySocket");

	log_info(
			"FindClientBySocket: Checking whether both parameters are filled in...");

	if (pClientSocketFd == NULL || pClientStruct == NULL) {
		log_warning(
				"FindClientBySocket: One or both parameters not specified.");

		log_debug("FindClientBySocket: Returning FALSE.");

		log_debug("FindClientBySocket: Done.");

		return FALSE;
	}

	log_info(
			"FindClientBySocket: Attempting to retrieve client socket file descriptor...");

	int clientSockFd = *((int*) pClientSocketFd);

	log_debug("FindClientBySocket: clientSockFd = %d", clientSockFd);

	log_info(
			"FindClientBySocket: Attempting to retrieve client structure pointer...");

	CLIENTSTRUCT* client_Struct = (CLIENTSTRUCT*) pClientStruct;

	log_info(
			"FindClientBySocket: Checking whether the client socket is associated with the client structure...");

	if (clientSockFd == client_Struct->sockFD) {
		log_info(
				"FindClientBySocket: Client structure matching the supplied socket value found.");

		log_debug("FindClientBySocket: Returning TRUE.");

		log_debug("FindClientBySocket: Done.");

		return TRUE;
	}

	log_debug("FindClientBySocket: Returning FALSE.");

	log_debug("FindClientBySocket: Done.");

	return FALSE;
}

