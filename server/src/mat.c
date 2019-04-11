///////////////////////////////////////////////////////////////////////////////
// mat.c - Master Acceptor Thread for the chat server
//
// AUTHOR: Brian Hart
// CREATED DATE: 3 Feb 2019
//
// Shout-out to <https://gist.githubusercontent.com/suyash/2488ff6996c98a8ee3a8
// 4fe3198a6f85/raw/ba06e891060b277867a6f9c7c2afa20da431ec91/server.c> and
// <http://www.linuxhowtos.org/C_C++/socket.htm> for
// inspiration
//

#include "stdafx.h"
#include "server.h"

#include "mat.h"
#include "client_list.h"
#include "client_struct.h"
#include "client_thread.h"
#include "client_thread_manager.h"

#include "server_functions.h"

BOOL g_bShouldTerminateMasterThread = FALSE;

#define INVALID_SERVER_SOCKET_HANDLE	"Invalid server socket file " \
										"descriptor passed.\n"
#define SERVER_SOCKET_REQUIRED 			"You should have passed the server " \
										"socket file descriptor."

void KillClientThread(void* pClientStruct) {
	if (pClientStruct == NULL) {
		return;
	}

	LPCLIENTSTRUCT lpCS = (LPCLIENTSTRUCT)pClientStruct;

	if (lpCS->hClientThread == INVALID_HANDLE_VALUE) {
		return;
	}

	KillThread(lpCS->hClientThread);

	sleep(1); /* force a CPU context switch so the semaphore can work */
}

void TerminateMasterThread(int signum) {
	if (SIGSEGV != signum) {
		return;
	}

	/* Mark the master thread for termination so it will shut down
	 * the next time it loops */
	g_bShouldTerminateMasterThread = TRUE;

	/* Close the server socket handle to release operating system
	 * resources. */
	CloseSocket(g_nServerSocket);

	// If there are no clients connected, then we're done
	if (0 == g_nClientCount) {
		// Re-register this semaphore
		RegisterEvent(TerminateMasterThread);
		return;
	}

	// Go through the list of connected clients, one by one, and
	// send signals to each client's thread to die
	LockMutex(g_hClientListMutex);
	{
		ForEach(&g_pClientList, KillClientThread);
	}
	UnlockMutex(g_hClientListMutex);

	// Re-register this semaphore
	RegisterEvent(TerminateMasterThread);
}

/**
 * @brief Adds a newly-connected client to the list of connected clients.
 * @param lpCS Reference to an instance of a CLIENTSTRUCT contianing the data for the client.
 */
void AddNewlyConnectedClientToList(LPCLIENTSTRUCT lpCS) {

	// ALWAYS Use a mutex to touch the linked list of clients!
	// Also, we are guaranteed (by a null-reference check in the only code
	// that calls this function) to have lpCS be a non-NULL value.
	LockMutex(g_hClientListMutex);
	{
		if (g_pClientList == NULL) {
			g_pClientList = AddHead(lpCS);
		} else {
			AddMember(&g_pClientList, lpCS);
		}
	}
	UnlockMutex(g_hClientListMutex);
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
		LogError(SERVER_SOCKET_REQUIRED);

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
		LogError(SERVER_SOCKET_REQUIRED);

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
		LogError(INVALID_SERVER_SOCKET_HANDLE);

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

	SetSocketNonBlocking(lpResult->nSocket);

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
		LPCLIENTSTRUCT lpCS = WaitForNewClientConnection(server_socket);
		if (NULL == lpCS) {
			LogError(
					"MasterAcceptorThread: New client connection structure instance is NULL.");

			LogDebug("MasterAcceptorThread: Done.");

			break;
		}

		LogInfo(
				"MasterAcceptorThread: Adding the client to our list of connected clients...");

		AddNewlyConnectedClientToList(lpCS);

		LogInfo(
				"MasterAcceptorThread: Finished adding the client to the list of connected clients.");

		LogInfo(
				"MasterAcceptorThread: Creating client thread to handle communications with that client...");

		LaunchNewClientThread(lpCS);

		LogInfo("MasterAcceptorThread: New client thread created.");

		LogInfo(
				"MasterAcceptorThread: Attempting to increment the count of connected clients...");

		// Increment the count of connected clients
		InterlockedIncrement(&g_nClientCount);

		LogInfo(
				"MasterAcceptorThread: The count of connected clients has been incremented successfully.");

		LogInfo(
				"MasterAcceptorThread: Checking whether the count of connected clients has dropped to zero...");

		// Check for whether the count of connected clients is zero. If so, then we can shut down.
		LockMutex(g_hClientListMutex);
		{
			LogDebug("MasterAcceptorThread: Connected clients: %d.",
					g_nClientCount);

			if (g_nClientCount == 0) {
				LogInfo(
						"MasterAcceptorThread: Connected client count is zero.");

				break;	// stop this loop when there are no more connected clients
			}
		}
		UnlockMutex(g_hClientListMutex);

		LogInfo(
				"MasterAcceptorThread: The count of connected clients is greater than zero.");
	}

	LogInfo("MasterAcceptorThread: Thread is cleaning up.");

	LogInfo(
			"MasterAcceptorThread: Cleaning up the interlocking increment/decrement mutex...");

	LogDebug("MasterAcceptorThread: Done.");

	return NULL;
}
