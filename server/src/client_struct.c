/*
 * clientStruct.c
 *
 *  Created on: Feb 3, 2019
 *      Author: bhart
 */

#include <client_struct.h>
#include "stdafx.h"
#include "utils.h"

LPCLIENTSTRUCT CreateClientStruct(int nClientSocket,
		const char* pszClientIPAddress) {

	log_debug("In CreateClientStruct");

	log_debug("CreateClientStruct: nClientSocket = %d", nClientSocket);

	log_info(
			"CreateClientStruct: Checking whether the client socket file descriptor is valid...");

	if (!isValidSocket(nClientSocket)) {
		log_error(
				"CreateClientStruct: The client socket file descriptor passed is not valid.");

		log_debug("CreateClientStruct: Done.");

		return NULL;
	}

	log_info(
			"CreateClientStruct: The client socket file descriptor passed is valid.");

	log_debug("CreateClientStruct: pszClientIPAddress = '%s'",
			pszClientIPAddress);

	log_info("Checking whether the pszClientIPAddress is a blank value...");

	if (pszClientIPAddress == NULL || strlen(pszClientIPAddress) == 0) {
		log_error(
				"CreateClientStruct: The client's IP address is blank.  This value is required.");

		log_debug("CreateClientStruct: Done.");

		return NULL;
	}

	log_info(
			"CreateClientStruct: The client's IP address is filled in.  Allocating memory...");

	LPCLIENTSTRUCT lpClientStruct = (LPCLIENTSTRUCT) calloc(1,
			sizeof(CLIENTSTRUCT));

	log_info(
			"CreateClientStruct: %d B of memory allocated for new CLIENTSTRUCT instance.",
			sizeof(CLIENTSTRUCT));

	log_info(
			"CreateClientStruct: Initializing the new memory location to have all zeros...");

	// Set the memory occupied by the CLIENTSTRUCT structure to contain all zeroes
	memset(lpClientStruct, 0, sizeof(CLIENTSTRUCT));

	log_info("CreateClientStruct: Memory initialized.");

	log_info("CreateClientStruct: Initializing the client structure...");

	lpClientStruct->sockFD = nClientSocket;

	memcpy(lpClientStruct->ipAddr, pszClientIPAddress,
			min(strlen(pszClientIPAddress), IPADDRLEN));

	/* A client isn't 'connected' until the HELO protocol command is issued by the client.
	 * This is to allow clients to 'get ready' before they start being sent other chatters'
	 * messages. */
	lpClientStruct->bConnected = FALSE;

	log_info(
			"CreateClientStruct: New client data structure instance has been created and initialized.");

	log_info("CreateClientStruct: Done.");

	return lpClientStruct;

}

void FreeClient(void* pClientStruct) {
	log_debug("In FreeClient");

	log_info(
			"FreeClient: Checking whether supplied CLIENTSTRUCT pointer is NULL...");

	if (pClientStruct == NULL) {
		log_warning(
				"FreeClient: The client structure has already been freed.  Nothing to do.");

		log_debug("FreeClient: Done.");

		return;
	}

	log_info("FreeClient: Freeing the CLIENTSTRUCT pointer...");

	free(pClientStruct);
	pClientStruct = NULL;

	log_info("FreeClient: The memory has been released back to the system.");

	log_debug("FreeClient: Done.");
}
