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
#include "client_thread_manager.h"

#include "server_functions.h"

BOOL g_bShouldTerminateMasterThread = FALSE;

#define INVALID_SERVER_SOCKET_HANDLE	"Invalid server socket file " \
										"descriptor passed.\n"
#define INVALID_CLIENT_SOCKET_HANDLE	"Invalid client socket file " \
										"descriptor obtained from OS.\n"
#define SERVER_SOCKET_REQUIRED 			"You should have passed the server " \
										"socket file descriptor to the MAT.\n"

///////////////////////////////////////////////////////////////////////////////
// KillClientThread function - A callback that is run for each element in the
// client list in order to kill each client's thread.
//

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

///////////////////////////////////////////////////////////////////////////////
// TerminateMasterThread function - Semaphore callback that is signaled when the
// server is shutting down, in order to make the MAT shut down in an orderly
// fashion.
//

void TerminateMasterThread(int signum) {
	if (g_bShouldTerminateMasterThread) {
		return;
	}

	// If signum is not equal to SIGSEGV, then ignore this semaphore
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
 * @param lpCS Reference to an instance of a CLIENTSTRUCT contianing the data
 * for the client.
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
	// Validate the input. pThreadData must have a value (i.e., not be NULL),
	// is castable to int*, and then can be dereferenced to an int value (the
	// server socket's file descriptor (FD)).  The int value so obtained
	// must then meet further criteria in that it must be strictly greater
	// than zero.

	int serverSocketFD = ERROR; /* If a validation fails, then return ERROR */

	if (pThreadData == NULL) {
		fprintf(stderr, SERVER_SOCKET_REQUIRED);

		CleanupServer(ERROR);
	}

	int* pServerSocketFD = (int*) pThreadData;
	if (pServerSocketFD == NULL) {
		fprintf(stderr, SERVER_SOCKET_REQUIRED);

		CleanupServer(ERROR);
	}

	serverSocketFD = *pServerSocketFD;
	if (!IsSocketValid(serverSocketFD)) {
		fprintf(stderr, INVALID_SERVER_SOCKET_HANDLE);

		CleanupServer(ERROR);
	}

	/* if we are here, then we have successfully obtained a valid socket file descriptor from the
	 * user state passed to the master acceptor thread (and this function). */
	return serverSocketFD;
}

/**
 * @brief Marks a server socket file descriptor as reusable.
 * @param nServerSocket Socket file descriptor for the server's listening socket.
 * @remarks Sets TCP settings on the socket to mark it as reusable, so that
 * multiple connections can be accepted.
 */
void MakeServerEndpointReusable(int nServerSocket) {
	if (!IsSocketValid(nServerSocket)) {
		fprintf(stderr, INVALID_SERVER_SOCKET_HANDLE);

		CleanupServer(ERROR);
	}

	if (OK != SetSocketReusable(nServerSocket)) {
		perror("MakeServerEndpointReusable");

		CleanupServer(ERROR);
	}

	// If we are here, we've successfully set preferences on the server
	// socket to make it reusable -- i.e., that it can be connected to
	// again and again by multiple clients
}

/**
 * @brief Waits until a client connects, and then provides information about
 * the connection.
 * @param nServerSocket Socket file descriptor of the listening server endpoint.
 * @remarks Blocks the calling thread until a new client connects. When a new
 * client connection is received and is represented by a valid socket file
 * descriptor, a CLIENTSTRUCT structure instance is filled with the client's
 * socket file descriptor and the client's IP address, and the address of this
 * structure is returned.  Be sure to free the structure instance when you're
 * done with it.
 */
LPCLIENTSTRUCT WaitForNewClientConnection(int nServerSocket) {
	// Each time a client connection comes in, its IP address where it's coming
	// from is read, and its IP address, file descriptor, and individual thread
	// handle are all bundled up into the CLIENTSTRUCT structure which then is
	// passed to a new 'client thread.'

	if (!IsSocketValid(nServerSocket)) {
		fprintf(stderr, INVALID_SERVER_SOCKET_HANDLE);

		CleanupServer(ERROR);
	}

	struct sockaddr_in clientAddress;

	// Wait for a new client to connect
	int nClientSocket = AcceptSocket(nServerSocket, &clientAddress);

	if (!IsSocketValid(nClientSocket)) {
		if (EBADF != errno) {
			fprintf(stderr, INVALID_CLIENT_SOCKET_HANDLE);

			CleanupServer(ERROR);
		} else {
			// Getting EBADF from doing an accept() on the server's socket
			// means it's time to quit
			return NULL;
		}
	}

	char* pszClientIPAddress = inet_ntoa(clientAddress.sin_addr);

	/* Echo a message to the screen that a client connected. */
	fprintf(stdout, NEW_CLIENT_CONN, pszClientIPAddress);

	// if we are here then we have a brand-new client connection
	LPCLIENTSTRUCT lpCS = CreateClientStruct(nClientSocket,
			pszClientIPAddress);
	if (NULL == lpCS) {
		fprintf(stderr, FAILED_CREATE_NEW_CLIENT);

		CleanupServer(ERROR);
	}

	// Set the new client endpoint to be non-blocking so that we can
	// poll it continuously for new data in its own thread.
	SetSocketNonBlocking(lpCS->nSocket);
	return lpCS;
}

void* MasterAcceptorThread(void* pThreadData) {
	RegisterEvent(TerminateMasterThread);

	// Extract the file descriptor of the server's TCP endpoint from
	// the user state passed to this thread.  The GetServerSocketFileDescriptor
	// function's return value is guaranteed to be valid.
	int nServerSocket = GetServerSocketFileDescriptor(pThreadData);

	// This thread procedure runs an infinite loop which runs while the server
	// socket is listening for new connections.  This thread's sole mission in
	// life is to wait for incoming client connections, accept them as they come
	// in, and then go back to waiting for more incoming client connections.

	while (1) {
		// If we have been signaled to stop, then abort
		if (g_bShouldTerminateMasterThread) {
			return NULL;
		}

		MakeServerEndpointReusable(nServerSocket);

		// We now call the accept function.  This function holds us up
		// until a new client connection comes in, whereupon it returns
		// a file descriptor that represents the socket on our side that
		// is connected to the client.  The output of the function called
		// below is guaranteed to be valid.
		LPCLIENTSTRUCT lpCS = WaitForNewClientConnection(nServerSocket);

		// Add the info for the newly connected client to the list we maintain
		AddNewlyConnectedClientToList(lpCS);

		// Increment the count of connected clients
		InterlockedIncrement(&g_nClientCount);

		// Launch a new thread to handle the communications with this client
		LaunchNewClientThread(lpCS);

		// Check for whether the count of connected clients is zero. If so, then
		// we can shut down.
		LockMutex(g_hClientListMutex);
		{
			if (g_nClientCount == 0) {
				UnlockMutex(g_hClientListMutex);

				break;	// stop this loop when there are no more
							// connected clients
			}
		}
		UnlockMutex(g_hClientListMutex);
	}

	return NULL;
}
