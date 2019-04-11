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

#include "stdafx.h"
#include "server.h"

#include "mat.h"
#include "client_list.h"
#include "client_struct.h"
#include "client_thread.h"
#include "client_thread_manager.h"

HTHREAD g_hMasterThread;

BOOL g_bShouldTerminateMasterThread = FALSE;

#define GSSFD_INVALID_SERVER_SOCKET_DESCRIPTOR	"GetServerSocketFileDescriptor: Invalid server socket file descriptor passed."
#define GSSFD_MUST_PASS_SERVER_SOCKET_DESCRIPTOR "GetServerSocketFileDescriptor: You should have passed the server socket file descriptor."

void TerminateMasterThread(int s) {
	LogDebug("In TerminateMasterThread");

	LogInfo(
			"TerminateMasterThread: Checking whether we've received the SIGSEGV signal...");

	LogDebug("TerminateMasterThread: s = %d", s);

	if (SIGSEGV != s) {
		LogError(
				"TerminateMasterThread: The signal received is not SIGSEGV.  Nothing to do.");

		LogDebug("TerminateMasterThread: Done.");

		return;
	}

	LogInfo("TerminateMasterThread: SIGSEGV signal code detected.");

	LogInfo("TerminateMasterThread: Setitng the termination flag...");

	g_bShouldTerminateMasterThread = TRUE;

	LogInfo("TerminateMasterThread: The termination flag has been set.");

	LogInfo(
			"TerminateMasterThread: Signaling the MAT to terminate by closing the server socket...");

	CloseSocket(server_socket);

	LogInfo("TerminateMasterThread: Closed the server TCP endpoint.");

	LogInfo(
			"TerminateMasterThread: Checking whether there are any connected clients...");

	LogDebug("TerminateMasterThread: client_count = %d", nClientCount);

	if (0 == nClientCount) {
		LogInfo(
				"TerminateMasterThread: There aren't any clients connected.  Nothing to do.");

		LogDebug("TerminateMasterThread: Done.");
		return;
	}

	LogInfo(
			"TerminateMasterThread: Attempting to signal all client threads to terminate...");

	LogInfo("TerminateMasterThread: Requesting lock on client list mutex...");

	LockMutex(hClientListMutex);
	{
		LogInfo("TerminateMasterThread: Client list mutex lock obtained.");

		POSITION* pos = GetHeadPosition(&clientList);
		if (pos == NULL) {
			LogError(
					"TerminateMasterThread: Failed to get the starting location of the client list.");

			LogInfo(
					"TerminateMasterThread: Releasing client list mutex lock...");

			UnlockMutex(hClientListMutex);

			LogInfo("TerminateMasterThread: Client list mutex lock released.");

			LogDebug("TerminateMasterThread: Done.");

			return;
		}

		do {
			LogInfo(
					"TerminateMasterThread: Attempting to get the current client's information...");

			LPCLIENTSTRUCT lpCurrentClientStruct = (LPCLIENTSTRUCT) pos->data;
			if (lpCurrentClientStruct == NULL) {
				LogWarning(
						"TerminateMasterThread: Failed to get information for the current client.  Skipping it...");
				continue;
			}

			LogInfo(
					"TerminateMasterThread: Information for current client obtained.  Signaling it to die...");

			KillThread(lpCurrentClientStruct->hClientThread);

			sleep(1); /* force a CPU context switch */

			LogInfo(
					"TerminateMasterThread: Killed client thread for connection from %s.",
					lpCurrentClientStruct->ipAddr);

		} while ((pos = GetNext(pos)) != NULL);

		LogInfo("TerminateMasterThread: Releasing client list mutex lock...");
	}
	UnlockMutex(hClientListMutex);

	LogInfo("TerminateMasterThread: Client list mutex lock released.");

	LogInfo(
			"TerminateMasterThread: Re-registering the TerminateMasterThread event...");

	RegisterEvent(TerminateMasterThread);

	LogInfo(
			"TerminateMasterThread: The TerminateMasterThread event has been re-registered.");

	LogDebug("TerminateMasterThread: Done.");
}

/**
 * @brief Adds a newly-connected client to the list of connected clients.
 * @param lpClientData Reference to an instance of a CLIENTSTRUCT contianing the data for the client.
 */
void AddNewlyConnectedClientToList(LPCLIENTSTRUCT lpClientData) {
	LogDebug("In AddNewlyConnectedClientToList");

	LogInfo(
			"AddNewlyConnectedClientToList: Checking whether the 'lpClientData' has a NULL reference...");

	if (lpClientData == NULL) {

		LogError(
				"AddNewlyConnectedClientToList: Required parameter 'lpClientData' has a NULL reference.  Stopping.");

		LogDebug("AddNewlyConnectedClientToList: Done.");

		CleanupServer(ERROR);
	}

	LogInfo(
			"AddNewlyConnectedClientToList: The 'lpClientData' parameter has a valid reference.");

	LogInfo(
			"AddNewlyConnectedClientToList: Obtaining mutually-exclusive lock on client list...");

	// ALWAYS Use a mutex to touch the linked list of clients!
	LockMutex(hClientListMutex);
	{
		LogInfo("AddNewlyConnectedClientToList: Lock obtained.");

		LogInfo(
				"AddNewlyConnectedClientToList: Registering client in client list...");

		LogDebug(
				"AddNewlyConnectedClientToList: Count of registered clients is currently %d.",
				nClientCount);

		if (nClientCount == 0) {
			LogDebug(
					"AddNewlyConnectedClientToList: Adding client info to head of internal client list...");

			clientList = AddHead(lpClientData);
			if (clientList == NULL)
				LogError(
						"AddNewlyConnectedClientToList: Failed to initialize the master list of clients.");
		} else if (clientList != NULL) {
			LogDebug(
					"AddNewlyConnectedClientToList: Adding client info to internal client list...");

			AddMember(&clientList, lpClientData);
		}

		LogInfo(
				"AddNewlyConnectedClientToList: Releasing lock on client list...");
	}
	UnlockMutex(hClientListMutex);

	LogInfo("AddNewlyConnectedClientToList: Lock released.");

	LogDebug("AddNewlyConnectedClientToList: Done.");
}

/**
 * @brief Given a void pointer to some user state, attempts to get the server socket's file descriptor.
 * @param pThreadData User state that had been passed to the Master Acceptor Thread when it was
 * created.
 * @returns Integer value representing the server socket's file descriptor value as assigned
 * by the operating system.  -1 if an error occurred, such as invalid user state data passed.
 */
int GetServerSocketFileDescriptor(void* pThreadData) {
	LogDebug("In GetServerSocketFileDescriptor");

	// Validate the input. pThreadData must have a value (i.e., not be NULL),
	// be castable to int*, and then be dereferenced to an int value (the
	// server socket's file descriptor (FD)).  The int value so obtained
	// must then meet further criteria in that it must be strictly greater
	// than zero.

	int result = ERROR; /* If a validation fails, then return ERROR */

	LogInfo(
			"GetServerSocketFileDescriptor: Checking the pThreadData parameter for valid user state...");

	if (pThreadData == NULL) {
		LogError(GSSFD_MUST_PASS_SERVER_SOCKET_DESCRIPTOR);

		LogDebug("GetServerSocketFileDescriptor: Result = %d", result);

		LogDebug("GetServerSocketFileDescriptor: Done.");

		CleanupServer(ERROR);
	}

	LogInfo(
			"GetServerSocketFileDescriptor: The pThreadData contains a valid memory address.");

	LogDebug(
			"GetServerSocketFileDescriptor: Attempting to cast pThreadData to int*...");

	int* pServerSocketFD = (int*) pThreadData;
	if (pServerSocketFD == NULL) {
		LogError(GSSFD_MUST_PASS_SERVER_SOCKET_DESCRIPTOR);

		LogDebug("GetServerSocketFileDescriptor: Result = %d", result);

		LogDebug("GetServerSocketFileDescriptor: Done.");

		CleanupServer(ERROR);
	}

	LogDebug(
			"GetServerSocketFileDescriptor: Successfully cast pThreadData to int*.");

	LogDebug(
			"GetServerSocketFileDescriptor: Attempting to dereference the pServerSocketFD pointer...");

	result = *pServerSocketFD;
	if (result <= 0) {
		LogError(GSSFD_INVALID_SERVER_SOCKET_DESCRIPTOR);

		LogDebug("GetServerSocketFileDescriptor: Result = %d", result);

		LogDebug("GetServerSocketFileDescriptor: Done.");

		CleanupServer(ERROR);
	}

	/* if we are here, then we have successfully obtained a valid socket file descriptor from the
	 * user state passed to the master acceptor thread (and this function). */

	LogDebug("GetServerSocketFileDescriptor: Result = %d", result);

	LogDebug("GetServerSocketFileDescriptor: Done.");

	return result;
}

/**
 * @brief Marks a server socket file descriptor as reusable.
 * @param server_socket Socket file descriptor for the server's listening socket.
 * @remarks Sets TCP settings on the socket to mark it as reusable, so that
 * multiple connections can be accepted.
 */
void MakeServerEndpointReusable(int server_socket) {
	LogDebug("In MakeServerEndpointReusable");

	LogDebug("MakeServerEndpointReusable: server_socket = %d", server_socket);

	LogInfo(
			"MakeServerEndpointReusable: Checking whether the server socket file descriptor is valid...");

	if (server_socket <= 0) {
		LogError(
				"MakeServerEndpointReusable: The server socket file descriptor has an invalid value.");

		LogDebug("MakeServerEndpointReusable: Done.");

		CleanupServer(ERROR);
	}

	LogInfo(
			"MakeServerEndpointReusable: Attempting to mark server TCP endpoint as reusable...");

	if (OK != SetSocketReusable(server_socket)) {
		LogError(
				"MakeServerEndpointReusable: Unable to configure the server's TCP endpoint.");

		perror("MakeServerEndpointReusable");

		LogDebug("MakeServerEndpointReusable: Done.");

		CleanupServer(ERROR);
	}

	LogInfo(
			"MakeServerEndpointReusable: The server's TCP endpoint has been configured to be reusable.");

	LogDebug("MakeServerEndpointReusable: Done.");
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
	LogDebug("In WaitForNewClientConnection");

	// Each time a client connection comes in, its IP address where it's coming from is
	// read, and its IP address, file descriptor, and individual thread handle
	// are all bundled up into the CLIENTSTRUCT structure which then is passed to a
	// new 'client thread.'

	LogInfo(
			"WaitForNewClientConnection: Checking whether a valid socket file descriptor was passed for the server...");

	LogDebug("WaitForNewClientConnection: server_socket = %d", server_socket);

	if (!IsSocketValid(server_socket)) {
		LogError(
				"WaitForNewClientConnection: Server socket file descriptor does not have a valid value.");

		LogDebug("WaitForNewClientConnection: Done.");

		CleanupServer(ERROR);
	}

	LogInfo(
			"WaitForNewClientConnection: The server socket file descriptor has a valid value.");

	struct sockaddr_in client_address;

	LogInfo(
			"WaitForNewClientConnection: Waiting for new client connection...");

	int client_socket = AcceptSocket(server_socket, &client_address);

	LogInfo(
			"WaitForNewClientConnection: Checking whether a valid socket descriptor was obtained...");

	LogDebug("WaitForNewClientConnection: client_socket = %d", client_socket);

	if (!IsSocketValid(client_socket)) {
		if (EBADF != errno) {
			LogError(
					"WaitForNewClientConnection: Client socket file descriptor does not have a valid value.");

			LogDebug("WaitForNewClientConnection: Done.");

			CleanupServer(ERROR);
		} else {
			LogWarning(
					"WaitForNewClientConnection: Accept returned with EBADF, possible thread termination.");

			LogDebug("WaitForNewClientConnection: Done.");

			return NULL;
		}
	}

	LogInfo(
			"WaitForNewClientConnection: Client socket file descriptor is valid.");

	LogInfo("WaitForNewClientConnection: New client connection detected.");

	char* client_ip_address = inet_ntoa(client_address.sin_addr);

	/* Echo a message to the screen that a client connected. */
	if (GetLogFileHandle() != stdout) {
		fprintf(stdout, "S: <new connection from %s>\n", client_ip_address);
	}

	LogDebug("WaitForNewClientConnection: client_ip_address = '%s'",
			client_ip_address);

	if (GetLogFileHandle() != stdout) {
		fprintf(stdout, "S: <New client connection detected from %s.>\n",
				client_ip_address);
	}

	LogInfo(
			"WaitForNewClientConnection: Attempting to create new client list entry...");

	// if we are here then we have a brand-new client connection
	LPCLIENTSTRUCT lpResult = CreateClientStruct(client_socket,
			client_ip_address);

	if (NULL == lpResult) {
		LogError(
				"WaitForNewClientConnection: Failed to create new client list entry.");

		LogDebug("WaitForNewClientConnection: Done.");

		CleanupServer(ERROR);
	}

	LogInfo(
			"WaitForNewClientConnection: New client list entry initialized successfully.");

	LogInfo("WaitForNewClientConnection: Setting new client endpoint to be nonblocking...");

	SetSocketNonBlocking(lpResult->sockFD);

	LogInfo("WaitForNewClientConnection: New client endpoint made nonblocking.");

	LogDebug("WaitForNewClientConnection: Done.");

	return lpResult;
}

void* MasterAcceptorThread(void* pThreadData) {
	LogDebug("In MasterAcceptorThread");

	LogInfo(
			"MasterAcceptorThread: Registering the TerminateMasterThread signal handler...");

	RegisterEvent(TerminateMasterThread);

	LogInfo(
			"MasterAcceptorThread: The TerminateMasterThread signal handler has been registered.");

	LogInfo(
			"MasterAcceptorThread: Attempting to read the server TCP endpoint descriptor from user state...");

	int server_socket = GetServerSocketFileDescriptor(pThreadData);

	LogDebug("MasterAcceptorThread: server_socket = %d", server_socket);

	LogInfo(
			"MasterAcceptorThread: Checking whether the server socket file descriptor is valid...");

	if (!IsSocketValid(server_socket)) {
		LogError(
				"MasterAcceptorThread: Failed to validate server TCP endpoint descriptor value.");

		LogDebug("MasterAcceptorThread: Done.");

		CleanupServer(ERROR);
	}

	LogInfo(
			"MasterAcceptorThread: Server TCP endpoint file descriptor information obtained successfully.");

	LogInfo(
			"MasterAcceptorThread: The server socket file descriptor is valid.");

	LogInfo(
			"MasterAcceptorThread: Beginning client connection monitoring loop...");

	// This thread procedure runs an infinite loop which runs while the server socket
	// is listening for new connections.  This thread's sole mission in life is to
	// wait for incoming client connections, accept them as they come in, and then
	// go back to waiting for more incoming client connections.

	while (1) {
		LogInfo(
				"MasterAcceptorThread: Checking whether the termination flag is set...");

		LogDebug("MasterAcceptorThread: g_bShouldTerminate = %d",
				g_bShouldTerminateMasterThread);

		if (g_bShouldTerminateMasterThread) {
			LogWarning(
					"MasterAcceptorThread: Termination flag has been set.  Aborting...");

			LogDebug("MasterAcceptorThread: Done.");

			return NULL;
		}

		LogInfo("MasterAcceptorThread: The termination flag is not set.");

		LogInfo(
				"MasterAcceptorThread: Attempting to make server TCP endpoint reusable...");

		MakeServerEndpointReusable(server_socket);

		LogInfo(
				"MasterAcceptorThread: Successfully configured server TCP endpoint.");

		LogInfo(
				"MasterAcceptorThread: Waiting for a new client connection...");

		// We now call the accept function.  This function holds us up
		// until a new client connection comes in, whereupon it returns
		// a file descriptor that represents the socket on our side that
		// is connected to the client.
		LPCLIENTSTRUCT lpClientData = WaitForNewClientConnection(server_socket);
		if (NULL == lpClientData) {
			LogError(
					"MasterAcceptorThread: New client connection structure instance is NULL.");

			LogDebug("MasterAcceptorThread: Done.");

			break;
		}

		LogInfo(
				"MasterAcceptorThread: Adding the client to our list of connected clients...");

		AddNewlyConnectedClientToList(lpClientData);

		LogInfo(
				"MasterAcceptorThread: Finished adding the client to the list of connected clients.");

		LogInfo(
				"MasterAcceptorThread: Creating client thread to handle communications with that client...");

		LaunchNewClientThread(lpClientData);

		LogInfo("MasterAcceptorThread: New client thread created.");

		LogInfo(
				"MasterAcceptorThread: Attempting to increment the count of connected clients...");

		// Increment the count of connected clients
		InterlockedIncrement(&nClientCount);

		LogInfo(
				"MasterAcceptorThread: The count of connected clients has been incremented successfully.");

		LogInfo(
				"MasterAcceptorThread: Checking whether the count of connected clients has dropped to zero...");

		// Check for whether the count of connected clients is zero. If so, then we can shut down.
		LockMutex(hClientListMutex);
		{
			LogDebug("MasterAcceptorThread: Connected clients: %d.",
					nClientCount);

			if (nClientCount == 0) {
				LogInfo(
						"MasterAcceptorThread: Connected client count is zero.");

				break;	// stop this loop when there are no more connected clients
			}
		}
		UnlockMutex(hClientListMutex);

		LogInfo(
				"MasterAcceptorThread: The count of connected clients is greater than zero.");
	}

	LogInfo("MasterAcceptorThread: Thread is cleaning up.");

	LogInfo(
			"MasterAcceptorThread: Cleaning up the interlocking increment/decrement mutex...");

	LogDebug("MasterAcceptorThread: Done.");

	return NULL;
}
