// client_list_manager.c - Contains the implementations of callback functoins
// used to manipulate the contents of the list of active clients.
//

#include "stdafx.h"

#include "client_manager.h"
#include "client_struct.h"

///////////////////////////////////////////////////////////////////////////////
// FindClientByID function - Callback that is called repeatedly for each
// member of the client list, and used to determine whether a match to the
// element we are searching for exists within the list.
//

BOOL FindClientByID(void* pClientID, void* pClientStruct) {
	// Check if we were given valid inputs.
	if (pClientID == NULL || pClientStruct == NULL) {
		// Client not found.
		return FALSE;
	}

	// Try to extract the search key (client UUID) from
	// the input.
	if (!IsUUIDValid((UUID*)pClientID)) {
	    return FALSE;
	}

	// Get the current element of the list to match the
	// search key against.
	LPCLIENTSTRUCT lpCS = (LPCLIENTSTRUCT)pClientStruct;

	// If the clientSockFd search key equals the value of the CLIENTSTRUCT
	// instance's nSocket field, then we are golden
	 return AreEqual(*(UUID*)pClientID, lpCS->clientID);
}

///////////////////////////////////////////////////////////////////////////////
// ForceDisconnectionOfClient function - A callback that is called for every
// currently-connected client in the client list, to disconnect them when the
// server is exited by the server console's user.
//

void ForceDisconnectionOfClient(void* pClientStruct) {
    //fprintf(stdout, "In ForceDisconnectionOfClient...\n");

    if (pClientStruct == NULL) {
        // Null value for the pClientStruct parameter; nothing to do.
        return;
    }

    // Forcibly disconnect this client
    ForciblyDisconnectClient((LPCLIENTSTRUCT) pClientStruct);
}

