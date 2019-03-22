/*
 * clientStruct.h
 *
 *  Created on: Feb 3, 2019
 *      Author: bhart
 */

#ifndef INCLUDE_CLIENTSTRUCT_H_
#define INCLUDE_CLIENTSTRUCT_H_

#include "stdafx.h"

/**
 * @brief Structure that contains information about connected clients.
 */
typedef struct _tagCLIENTSTRUCT {

	char ipAddr[IPADDRLEN];
	char* pszNickname;
	int sockFD;
	HTHREAD hClientThread;	/* handle to the thread this client is chatting on */
	int bytesReceived;
	int bytesSent;
	BOOL bConnected;	/* is this client connected? */
} CLIENTSTRUCT, *LPCLIENTSTRUCT;

/**
 * @brief Creates an instance of a CLIENTSTRUCT structure and fills it with info
 * about the client.
 * @param nClientSocket Client's server endpoint socket file descriptor.
 * @param pszClientIPAddress Client's IP address as a string (i.e., 268.7.34.2)
 * @returns LPCLIENTSTRUCT pointing to the newly-created-and-initialized instance
 * of the client structure.
 * @remarks Supplies a reference to an instance of CLIENTSTRUCT filled with the
 * socket for sending data back to clients in reply to protocol commands or
 * chat messages.
 */
LPCLIENTSTRUCT CreateClientStruct(int nClientSocket, const char* pszClientIPAddress);

#endif /* INCLUDE_CLIENTSTRUCT_H_ */
