// client_list_manager.c - Contains the implementations of callback functoins
// used to manipulate the contents of the list of active clients.
//

#include "stdafx.h"

#include "client_manager.h"
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

	if (!IsSocketValid(clientSockFd)) {
	    return FALSE;
	}

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

///////////////////////////////////////////////////////////////////////////////
// ForceDisconnectionOfClient function - A callback that is called for every
// currently-connected client in the client list, to disconnect them when the
// server is exited by the server console's user.
//

void ForceDisconnectionOfClient(void* pClientStruct) {
    if (pClientStruct == NULL) {
        // Null value for the pClientStruct parameter; nothing to do.
        return;
    }

    // Forcibly disconnect this client
    ForciblyDisconnectClient((LPCLIENTSTRUCT) pClientStruct);
}

