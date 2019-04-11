/*
 * client_list_manager.c
 *
 *  Created on: Apr 3, 2019
 *      Author: bhart
 */

#include "stdafx.h"

#include "client_struct.h"

///////////////////////////////////////////////////////////////////////////////
// FindClientBySocket function - Callback that is called repeatedly for each
// member of the client list, and used to determine whether a match to the
// element we are searching for exists within the list.
//

BOOL FindClientBySocket(void* pClientSocketFd, void* pClientStruct) {
	// Check if we were given valid inputs.
	if (pClientSocketFd == NULL || pClientStruct == NULL) {
		// Client not found.
		return FALSE;
	}

	// Try to extract the search key (client socket file descriptor) from
	// the input.
	int clientSockFd = *((int*) pClientSocketFd);

	// Get the current element of the list to match the
	// search key against.
	LPCLIENTSTRUCT lpCS = (LPCLIENTSTRUCT)pClientStruct;

	// If the clientSockFd search key equals the value of the CLIENTSTRUCT
	// instance's nSocket field, then we are golden
	if (clientSockFd == lpCS->nSocket) {
		// Found successfully.
		return TRUE;
	}

	// If we are here, then obviously a match was not found.
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
