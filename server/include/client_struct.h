/*
 * clientStruct.h
 *
 *  Created on: Feb 3, 2019
 *      Author: bhart
 */

#ifndef __CLIENT_STRUCT_H__
#define __CLIENT_STRUCT_H__

#include "stdafx.h"
#include "server_symbols.h"

/**
 * @brief Structure that contains information about connected clients.
 */
typedef struct _tagCLIENTSTRUCT {
	/**
	 * @name clientID
	 * @brief Holds a UUID value that uniquely identifies this client.
	 */
    UUID clientID;

    /**
     * @name szIPAddress
     * @brief Buffer to hold a string that identifies which IP address the
     * client is coming from.
     */
	char szIPAddress[IPADDRLEN];

	/**
	 * @name pszNickname
	 * @brief Address of a buffer that tracks the current chat nickname
	 * the client's user wants.
	 */
	char* pszNickname;

	/**
	 * @name nSocket
	 * @brief Value of the socket file descriptor to use when communicating
	 * with this client.
	 */
	int nSocket;

	/**
	 * @name hClientThread
	 * @brief HTHREAD handle to the thread over which communications with
	 * this client take place.
	 */
	HTHREAD hClientThread; /* handle to the thread this client is chatting on */

	/**
	 * @name nBytesReceived
	 * @brief Set this member to the total number of bytes received from the
	 * client.
	 */
	long nBytesReceived;

	/**
	 * @name nBytesSent
	 * @brief Set this member to the total number of bytes sent to the client.
	 */
	long nBytesSent;

	/**
	 * @name bConnected
	 * @brief Flag that indicates whether this client is in the connected
	 * state.
	 */
	BOOL bConnected;
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
 * @brief Releases the memory allocated for a client structure pointer back
 * to the system.
 * @param pClientStruct Pointer to a CLIENTSTRUCT instance whose memory is to
 * be freed.
 */
void FreeClient(void* pClientStruct);

/**
 * @brief Determines whether the client referenced is in the connnected state.
 * @remarks Connected state is defined as (a) being connected over TCP and
 * (b) having successfully issued the HELO protocol command.
 */
BOOL IsClientConnected(void* pvClientStruct);

#endif /* __CLIENT_STRUCT_H__ */
