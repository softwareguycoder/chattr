// client_struct.c - Provides implementations of functions that create and/or
// manipulate a CLIENTSTRUCT instance (CLIENTSTRUCT is a structure that
// provides information about an individual client connection).
//

#include "stdafx.h"
#include "server.h"

#include "client_struct.h"
#include "server_functions.h"

///////////////////////////////////////////////////////////////////////////////
// CreateClientStruct - Allocates memory for, and initializes, a new instance
// of a CLIENTSTRUCT structure with the socket handle and IP address provided.
//

LPCLIENTSTRUCT CreateClientStruct(int nClientSocket,
		const char* pszClientIPAddress) {

	if (!IsSocketValid(nClientSocket)) {
		// The client socket handle passed is not valid; nothing to do.
		fprintf(stderr, SERVER_CLIENT_SOCKET_INVALID);

		CleanupServer(ERROR);
	}

	if (IsNullOrWhiteSpace(pszClientIPAddress)) {
		// The client IP address needs to be filled in; nothing to do.
		fprintf(stderr, CLIENT_IP_ADDR_UNK);

		CleanupServer(ERROR);
	}

	// Allocate memory for a new CLIENTSTRUCT instance
	LPCLIENTSTRUCT lpClientStruct =
	        (LPCLIENTSTRUCT) calloc(1, sizeof(CLIENTSTRUCT));
	if (lpClientStruct == NULL) {
	    fprintf(stderr, FAILED_ALLOC_CLIENT_STRUCT);

	    CleanupServer(ERROR);
	}

	// Set the memory occupied by the CLIENTSTRUCT structure to contain all zeroes
	memset(lpClientStruct, 0, sizeof(CLIENTSTRUCT));

	/* Tag each client with a universally-unique identifier */
	GenerateNewUUID(&(lpClientStruct->clientID));

	// Save the client socket handle into the nSocket field of the structure
	lpClientStruct->nSocket = nClientSocket;

	// Initialize the pszIPAddress string field of the client structure with the
	// IP address passed to us.
	memcpy(lpClientStruct->szIPAddress, pszClientIPAddress,
			MinimumOf(strlen(pszClientIPAddress), IPADDRLEN));

	/* A client isn't 'connected' until the HELO protocol command is issued by the client.
	 * This is to allow clients to 'get ready' before they start being sent other chatters'
	 * messages. */
	lpClientStruct->bConnected = FALSE;

	return lpClientStruct;
}

///////////////////////////////////////////////////////////////////////////////
// FreeClient function - Releases operating system resources consumed by the
// client information structure.
//

void FreeClient(void* pClientStruct) {
	if (pClientStruct == NULL) {
		// Null pointer passed for the thing to be freed; nothing to do.
		return;
	}

	free(pClientStruct);
	pClientStruct = NULL;
}
