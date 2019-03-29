///////////////////////////////////////////////////////////////////////////////
// mat.c - Master Acceptor Thread for the chat server
// The server receives text a line at a time and echoes the text back to its
// client only AFTER an entire line has been received.
//
// AUTHOR: Brian Hart
// DATE: 20 Mar 2019
//
// Shout-out to <https://gist.githubusercontent.com/suyash/2488ff6996c98a8ee3a8
// 4fe3198a6f85/raw/ba06e891060b277867a6f9c7c2afa20da431ec91/server.c> and
// <http://www.linuxhowtos.org/C_C++/socket.htm> for
// inspiration
//

/*
 * mat.c
 *
 *  Created on: Feb 3, 2019
 *      Author: bhart
 */

#include <client_struct.h>
#include <client_thread.h>
#include <client_thread_manager.h>
#include "stdafx.h"
#include "server.h"

#include "mat.h"
#include "list.h"
#include "utils.h"

HTHREAD hMasterThread;

int client_count = 0;

#define GSSFD_INVALID_SERVER_SOCKET_DESCRIPTOR	"GetServerSocketFileDescriptor: Invalid server socket file descriptor passed."
#define GSSFD_MUST_PASS_SERVER_SOCKET_DESCRIPTOR "GetServerSocketFileDescriptor: You should have passed the server socket file descriptor."

/**
 * @brief Adds a newly-connected client to the list of connected clients.
 * @param lpClientData Reference to an instance of a CLIENTSTRUCT contianing the data for the client.
 */
void AddNewlyConnectedClientToList(LPCLIENTSTRUCT lpClientData) {
	log_debug("In AddNewlyConnectedClientToList");

	log_info(
			"AddNewlyConnectedClientToList: Checking whether the 'lpClientData' has a NULL reference...");

	if (lpClientData == NULL) {

		log_error(
				"AddNewlyConnectedClientToList: Required parameter 'lpClientData' has a NULL reference.  Stopping.");

		log_debug("AddNewlyConnectedClientToList: Done.");

		exit(ERROR);
	}

	log_info(
			"AddNewlyConnectedClientToList: The 'lpClientData' parameter has a valid reference.");

	log_info(
			"AddNewlyConnectedClientToList: Obtaining mutually-exclusive lock on client list...");

	// ALWAYS Use a mutex to touch the linked list of clients!
	LockMutex(hClientListMutex);
	{
		log_info("AddNewlyConnectedClientToList: Lock obtained.");

		log_info(
				"AddNewlyConnectedClientToList: Registering client in client list...");

		log_debug(
				"AddNewlyConnectedClientToList: Count of registered clients is currently %d.",
				client_count);

		if (client_count == 0) {
			log_debug(
					"AddNewlyConnectedClientToList: Adding client info to head of internal client list...");

			clientList = AddHead(lpClientData);
			if (clientList == NULL)
				log_error(
						"AddNewlyConnectedClientToList: Failed to initialize the master list of clients.");
		} else if (clientList != NULL) {
			log_debug(
					"AddNewlyConnectedClientToList: Adding client info to internal client list...");

			AddMember(&clientList, lpClientData);
		}

		log_info(
				"AddNewlyConnectedClientToList: Releasing lock on client list...");
	}
	UnlockMutex(hClientListMutex);

	log_info("AddNewlyConnectedClientToList: Lock released.");

	log_debug("AddNewlyConnectedClientToList: Done.");
}

/**
 * @brief Given a void pointer to some user state, attempts to get the server socket's file descriptor.
 * @param pThreadData User state that had been passed to the Master Acceptor Thread when it was
 * created.
 * @returns Integer value representing the server socket's file descriptor value as assigned
 * by the operating system.  -1 if an error occurred, such as invalid user state data passed.
 */
int GetServerSocketFileDescriptor(void* pThreadData) {
	log_debug("In GetServerSocketFileDescriptor");

	// Validate the input. pThreadData must have a value (i.e., not be NULL),
	// be castable to int*, and then be dereferenced to an int value (the
	// server socket's file descriptor (FD)).  The int value so obtained
	// must then meet further criteria in that it must be strictly greater
	// than zero.

	int result = ERROR; /* If a validation fails, then return ERROR */

	log_info(
			"GetServerSocketFileDescriptor: Checking the pThreadData parameter for valid user state...");

	if (pThreadData == NULL) {
		log_error(GSSFD_MUST_PASS_SERVER_SOCKET_DESCRIPTOR);

		log_debug("GetServerSocketFileDescriptor: Result = %d", result);

		log_debug("GetServerSocketFileDescriptor: Done.");

		exit(ERROR);
	}

	log_info(
			"GetServerSocketFileDescriptor: The pThreadData contains a valid memory address.");

	log_debug(
			"GetServerSocketFileDescriptor: Attempting to cast pThreadData to int*...");

	int* pServerSocketFD = (int*) pThreadData;
	if (pServerSocketFD == NULL) {
		log_error(GSSFD_MUST_PASS_SERVER_SOCKET_DESCRIPTOR);

		log_debug("GetServerSocketFileDescriptor: Result = %d", result);

		log_debug("GetServerSocketFileDescriptor: Done.");

		exit(ERROR);
	}

	log_debug(
			"GetServerSocketFileDescriptor: Successfully cast pThreadData to int*.");

	log_debug(
			"GetServerSocketFileDescriptor: Attempting to dereference the pServerSocketFD pointer...");

	result = *pServerSocketFD;
	if (result <= 0) {
		log_error(GSSFD_INVALID_SERVER_SOCKET_DESCRIPTOR);

		log_debug("GetServerSocketFileDescriptor: Result = %d", result);

		log_debug("GetServerSocketFileDescriptor: Done.");

		exit(ERROR);
	}

	/* if we are here, then we have successfully obtained a valid socket file descriptor from the
	 * user state passed to the master acceptor thread (and this function). */

	log_debug("GetServerSocketFileDescriptor: Result = %d", result);

	log_debug("GetServerSocketFileDescriptor: Done.");

	return result;
}

/**
 * @brief Marks a server socket file descriptor as reusable.
 * @param server_socket Socket file descriptor for the server's listening socket.
 * @remarks Sets TCP settings on the socket to mark it as reusable, so that
 * multiple connections can be accepted.
 */
void MakeServerEndpointReusable(int server_socket) {
	log_debug("In MakeServerEndpointReusable");

	log_debug("MakeServerEndpointReusable: server_socket = %d", server_socket);

	log_info(
			"MakeServerEndpointReusable: Checking whether the server socket file descriptor is valid...");

	if (server_socket <= 0) {
		log_error(
				"MakeServerEndpointReusable: The server socket file descriptor has an invalid value.");

		log_debug("MakeServerEndpointReusable: Done.");

		exit(ERROR);
	}

	log_info(
			"MakeServerEndpointReusable: Attempting to mark server TCP endpoint as reusable...");

	if (SocketDemoUtils_setSocketReusable(server_socket) < 0) {
		log_error(
				"MakeServerEndpointReusable: Unable to configure the server's TCP endpoint.");

		perror("MakeServerEndpointReusable");

		log_debug("MakeServerEndpointReusable: Done.");

		exit(ERROR);
	}

	log_info(
			"MakeServerEndpointReusable: The server's TCP endpoint has been configured to be reusable.");

	log_debug("MakeServerEndpointReusable: Done.");
}

/**
 * @brief Waits until a client connects, and then provides information about the connection.
 * @param server_socket Socket file descriptor of the listening server endpoint.
 * @remarks Blocks the calling thread until a new client connects. When a new
 * client connection is received and is represented by a valid socket file descriptor,
 * a CLIENTSTRUCT structure instance is filled with the client's socket file descriptor
 * and the client's IP address, and the address of this structure is returned.  Be sure
 * to free the structure instance when you're done with it.
 */
LPCLIENTSTRUCT WaitForNewClientConnection(int server_socket) {
	// Each time a client connection comes in, its IP address where it's coming from is
	// read, and its IP address, file descriptor, and individual thread handle
	// are all bundled up into the CLIENTSTRUCT structure which then is passed to a
	// new 'client thread.'

	struct sockaddr_in client_address;

	if (server_socket <= 0) {
		exit(ERROR);
	}

	int client_socket = SocketDemoUtils_accept(server_socket, &client_address);

	if (client_socket <= 0) {
		exit(ERROR);
	}

	// if we are here then we have a brand-new client connection
	LPCLIENTSTRUCT lpResult = CreateClientStruct(client_socket,
			inet_ntoa(client_address.sin_addr));

	return lpResult;
}

void* MasterAcceptorThread(void* pThreadData) {
	log_debug("In MasterAcceptorThread");

	int server_socket = GetServerSocketFileDescriptor(pThreadData);
	if (server_socket <= 0) {
		return NULL; /* Failed to get server socket file descriptor from pThreadData */
	}

	// This thread procedure runs an infinite loop which runs while the server socket
	// is listening for new connections.  This thread's sole mission in life is to
	// wait for incoming client connections, accept them as they come in, and then
	// go back to waiting for more incoming client connections.

	while (1) {
		MakeServerEndpointReusable(server_socket);

		log_info(
				"MasterAcceptorThread: Waiting for a new client connection...");

		// We now call the accept function.  This function holds us up
		// until a new client connection comes in, whereupon it returns
		// a file descriptor that represents the socket on our side that
		// is connected to the client.
		LPCLIENTSTRUCT lpClientData = WaitForNewClientConnection(server_socket);
		if (lpClientData == NULL) {
			log_error(
					"MasterAcceptorThread: New client connection structure instance is NULL.");

			log_debug("MasterAcceptorThread: Done.");

			break;
		}

		log_info(
				"MasterAcceptorThread: Adding the client to our list of connected clients...");

		AddNewlyConnectedClientToList(lpClientData);

		log_info(
				"MasterAcceptorThread: Finished adding the client to the list of connected clients.");

		log_info(
				"MasterAcceptorThread: Creating client thread to handle communications with that client...");

		LaunchNewClientThread(lpClientData);

		lpClientData->hClientThread = CreateThreadEx(ClientThread,
				lpClientData);

		log_info("MasterAcceptorThread: New client thread created.");

		log_debug(
				"MasterAcceptorThread: Attempting to increment the count of connected clients...");

		// Increment the count of connected clients
		InterlockedIncrement(&client_count);

		// Check for whether the count of connected clients is zero. If so, then we can shut down.
		LockMutex(hClientListMutex);
		{
			log_debug("MasterAcceptorThread: Connected clients: %d.",
					client_count);

			if (client_count == 0)
				break;// stop this loop when there are no more connected clients.
		}
		UnlockMutex(hClientListMutex);
	}

	log_info(
			"MasterAcceptorThread: The count of connected clients has dropped to zero.");

	DestroyMutex(hInterlockMutex);

	log_debug("MasterAcceptorThread: Done.");

	return NULL;
}
