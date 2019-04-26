// client_list_manager.c - Contains the implementations of callback functoins
// used to manipulate the contents of the list of active clients.
//

#include "stdafx.h"

#include "client_manager.h"
#include "client_list_manager.h"
#include "client_struct.h"

///////////////////////////////////////////////////////////////////////////////
// FindClientByID function - Callback that is called repeatedly for each
// member of the client list, and used to determine whether a match to the
// element we are searching for exists within the list.
//

BOOL FindClientByID(void* pvClientId, void* pvClientStruct) {
    // Check if we were given valid inputs.
    if (pvClientId == NULL || pvClientStruct == NULL) {
        ThrowNullReferenceException();
    }

    // Try to extract the search key (client UUID) from
    // the input.
    if (!IsUUIDValid((UUID*) pvClientId)) {
        return FALSE;
    }

    // Get the current element of the list to match the
    // search key against.
    LPCLIENTSTRUCT lpCS = (LPCLIENTSTRUCT) pvClientStruct;

    // If the clientSockFd search key equals the value of the CLIENTSTRUCT
    // instance's nSocket field, then we are golden
    return AreEqual(*(UUID*) pvClientId, lpCS->clientID);
}

///////////////////////////////////////////////////////////////////////////////
// FindClientByNickname function - Searches the list for a client with the
// specified nickname.  Does a case-sensitive comparison.  This is because, in
// the realm of chat software, bob and BoB can be recognizable as two different
// chatters

BOOL FindClientByNickname(void* pvNickname, void* pvClientStruct) {
    if (pvNickname == NULL) {
        ThrowNullReferenceException();
    }

    if (pvClientStruct == NULL) {
        ThrowNullReferenceException();
    }

    const char* pszNickname = (char*) pvNickname;
    if (IsNullOrWhiteSpace(pszNickname)) {
        ThrowNullReferenceException();
    }

    LPCLIENTSTRUCT lpCS = (LPCLIENTSTRUCT) pvClientStruct;
    if (lpCS == NULL) {
        ThrowNullReferenceException();
    }

    if (IsNullOrWhiteSpace(lpCS->pszNickname)) {
        ThrowNullReferenceException();
    }

    return strcmp(pszNickname, lpCS->pszNickname) == 0;
}

///////////////////////////////////////////////////////////////////////////////
// ForceDisconnectionOfClient function - A callback that is called for every
// currently-connected client in the client list, to disconnect them when the
// server is exited by the server console's user.
//

void ForceDisconnectionOfClient(void* pvClientStruct) {
    //fprintf(stdout, "In ForceDisconnectionOfClient...\n");

    if (pvClientStruct == NULL) {
        // Null value for the pClientStruct parameter; nothing to do.
        return;
    }

    // Forcibly disconnect this client
    ForciblyDisconnectClient((LPCLIENTSTRUCT) pvClientStruct);
}

