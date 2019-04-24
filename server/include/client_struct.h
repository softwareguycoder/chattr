/*
 * clientStruct.h
 *
 *  Created on: Feb 3, 2019
 *      Author: bhart
 */

#ifndef __CLIENT_STRUCT_H__
#define __CLIENT_STRUCT_H__

#include "server_symbols.h"

#include <../../../threading_core/threading_core/include/threading_core.h>
#include <../../common_core/common_core/include/common_core.h>

/**
 * @brief Structure that contains information about connected clients.
 */
typedef struct _tagCLIENTSTRUCT {
    UUID clientID;
	char szIPAddress[IPADDRLEN];
	char* pszNickname;
	int nSocket;
	HTHREAD hClientThread; /* handle to the thread this client is chatting on */
	long nBytesReceived;
	long nBytesSent;
	BOOL bConnected; /* is this client connected? */
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
LPCLIENTSTRUCT CreateClientStruct(int nClientSocket,
		const char* pszClientIPAddress);

/**
 * @brief Releases the memory allocated for a client structure pointer back to the system.
 * @param pClientStruct Pointer to a CLIENTSTRUCT instance whose memory is to be freed.
 */
void FreeClient(void* pClientStruct);

#endif /* __CLIENT_STRUCT_H__ */
