// client_struct.c - Provides implementations of functions that create and/or
// manipulate a CLIENTSTRUCT instance (CLIENTSTRUCT is a structure that
// provides information about an individual client connection).
//

#include "stdafx.h"
#include "server.h"

#include "client_struct.h"
#include "server_functions.h"

LPCLIENTSTRUCT CreateClientStruct(int nClientSocket,
		const char* pszClientIPAddress) {

	if (!IsSocketValid(nClientSocket)) {
		// The client socket handle passed is not valid; nothing to do.
		fprintf(stderr, SERVER_CLIENT_SOCKET_INVALID);

		CleanupServer(ERROR);
	}

	if (pszClientIPAddress == NULL || strlen(pszClientIPAddress) == 0) {
		// The client IP address needs to be filled in; nothing to do.
		fprintf(stderr, CLIENT_IP_ADDR_UNK);

		CleanupServer(ERROR);
	}

	// Allocate memory for a new CLIENTSTRUCT instance
	LPCLIENTSTRUCT lpCS = (LPCLIENTSTRUCT) calloc(1,
			sizeof(CLIENTSTRUCT));

	// Set the memory occupied by the CLIENTSTRUCT structure to contain all zeroes
	memset(lpCS, 0, sizeof(CLIENTSTRUCT));

	// Save the client socket handle into the sockFD field of the structure
	lpCS->sockFD = nClientSocket;

	// Initialize the ipAddr string field of the client structure with the
	// IP address passed to us.
	memcpy(lpCS->ipAddr, pszClientIPAddress,
			min(strlen(pszClientIPAddress), IPADDRLEN));

	/* A client isn't 'connected' until the HELO protocol command is issued by the client.
	 * This is to allow clients to 'get ready' before they start being sent other chatters'
	 * messages. */
	lpCS->bConnected = FALSE;

	return lpCS;
}

void FreeClient(void* pClientStruct) {
	LogDebug("In FreeClient");

	LogInfo(
			"FreeClient: Checking whether supplied CLIENTSTRUCT pointer is NULL...");

	if (pClientStruct == NULL) {
		LogWarning(
				"FreeClient: The client structure has already been freed.  Nothing to do.");

		LogDebug("FreeClient: Done.");

		return;
	}

	LogInfo("FreeClient: The pClientStruct pointer references a valid memory address.");

	LogInfo("FreeClient: Freeing the CLIENTSTRUCT pointer...");

	free(pClientStruct);
	pClientStruct = NULL;

	LogInfo("FreeClient: The memory has been released back to the system.");

	LogDebug("FreeClient: Done.");
}
