/*
 * clientStruct.c
 *
 *  Created on: Feb 3, 2019
 *      Author: bhart
 */

#include "stdafx.h"
#include "server.h"

#include "client_struct.h"

LPCLIENTSTRUCT CreateClientStruct(int nClientSocket,
		const char* pszClientIPAddress) {

	LogDebug("In CreateClientStruct");

	LogDebug("CreateClientStruct: nClientSocket = %d", nClientSocket);

	LogInfo(
			"CreateClientStruct: Checking whether the client socket file descriptor is valid...");

	if (!IsSocketValid(nClientSocket)) {
		LogError(
				"CreateClientStruct: The client socket file descriptor passed is not valid.");

		LogDebug("CreateClientStruct: Done.");

		CleanupServer(ERROR);
	}

	LogInfo(
			"CreateClientStruct: The client socket file descriptor passed is valid.");

	LogDebug("CreateClientStruct: pszClientIPAddress = '%s'",
			pszClientIPAddress);

	LogInfo("Checking whether the pszClientIPAddress is a blank value...");

	if (pszClientIPAddress == NULL || strlen(pszClientIPAddress) == 0) {
		LogError(
				"CreateClientStruct: The client's IP address is blank.  This value is required.");

		LogDebug("CreateClientStruct: Done.");

		CleanupServer(ERROR);
	}

	LogInfo(
			"CreateClientStruct: The client's IP address is filled in.  Allocating memory...");

	LPCLIENTSTRUCT lpClientStruct = (LPCLIENTSTRUCT) calloc(1,
			sizeof(CLIENTSTRUCT));

	LogInfo(
			"CreateClientStruct: %d B of memory allocated for new CLIENTSTRUCT instance.",
			sizeof(CLIENTSTRUCT));

	LogInfo(
			"CreateClientStruct: Initializing the new memory location to have all zeros...");

	// Set the memory occupied by the CLIENTSTRUCT structure to contain all zeroes
	memset(lpClientStruct, 0, sizeof(CLIENTSTRUCT));

	LogInfo("CreateClientStruct: Memory initialized.");

	LogInfo("CreateClientStruct: Initializing the client structure...");

	lpClientStruct->sockFD = nClientSocket;

	memcpy(lpClientStruct->ipAddr, pszClientIPAddress,
			min(strlen(pszClientIPAddress), IPADDRLEN));

	/* A client isn't 'connected' until the HELO protocol command is issued by the client.
	 * This is to allow clients to 'get ready' before they start being sent other chatters'
	 * messages. */
	lpClientStruct->bConnected = FALSE;

	LogInfo(
			"CreateClientStruct: New client data structure instance has been created and initialized.");

	LogInfo("CreateClientStruct: Done.");

	return lpClientStruct;

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
