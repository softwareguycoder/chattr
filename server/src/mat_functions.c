// mat_functions.c - Implementations of functionality utilized by the Master
// Acceptor Thread (MAT)
//

#include <client_thread_functions.h>
#include "stdafx.h"
#include "server.h"
#include "server_functions.h"

#include "mat.h"
#include "mat_functions.h"

///////////////////////////////////////////////////////////////////////////////
// Internal-use-only functions

///////////////////////////////////////////////////////////////////////////////
// AddConnectedClientToList function

void AddClientToList(LPCLIENTSTRUCT lpCS) {
	if (lpCS == NULL || !IsSocketValid(lpCS->nSocket)) {
		return;
	}

	// ALWAYS Use a mutex to touch the linked list of clients!
	// Also, we are guaranteed (by a null-reference check in the only code
	// that calls this function) to have lpCS be a non-NULL value.
	LockMutex(GetClientListMutex());
	{
		AddElementToTail(&g_pClientList, lpCS);
	}
	UnlockMutex(GetClientListMutex());
}
///////////////////////////////////////////////////////////////////////////////
// GetClientCount function

int GetClientCount() {
	int nCount = 0;
	LockMutex(GetClientListMutex());
	{
		nCount = GetElementCount(g_pClientList);
	}
	UnlockMutex(GetClientListMutex());
	return nCount;
}

///////////////////////////////////////////////////////////////////////////////
// AddNewlyConnectedClientToList function

/**
 * @brief Adds a newly-connected client to the list of connected clients.
 * @param lpCS Reference to an instance of a CLIENTSTRUCT contianing the data
 * for the client.
 */
void AddNewlyConnectedClientToList(LPCLIENTSTRUCT lpCS) {
	if (lpCS == NULL || !IsSocketValid(lpCS->nSocket)) {
		fprintf(stderr, ERROR_CANT_ADD_NULL_CLIENT);

		CleanupServer(ERROR);
		return;
	}

	if (GetClientCount() == MAX_CLIENT_LIST_ENTRIES) {
		/* can't add to the list if the max number of records is
		 * already present. */
		LogError(ERROR_CLIENT_ENTRY_COUNT_EXCEEDED);

		if (GetErrorLogFileHandle() != stderr) {
			fprintf(stderr, ERROR_CLIENT_ENTRY_COUNT_EXCEEDED);
		}

		/* make the server tell everyone to go away and die */
		CleanupServer(ERROR);
	}

	AddClientToList(lpCS);
}

///////////////////////////////////////////////////////////////////////////////
// GetServerSocketFileDescriptor function

/**
 * @brief Given a void pointer to some user state, attempts to get the server
 * socket's file descriptor.
 * @param pThreadData User state that had been passed to the Master Acceptor
 * Thread when it was created.
 * @returns Integer value representing the server socket's file descriptor
 * value as assigned by the operating system.  -1 if an error occurred, such as
 * invalid user state data passed.
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

	/* if we are here, then we have successfully obtained a valid socket
	 * file descriptor from the user state passed to the master acceptor
	 * thread (and this function). */
	return serverSocketFD;
}

///////////////////////////////////////////////////////////////////////////////
// IsClientCountZero function - Returns a value indicating whether the count
// of connected clients has dropped to zero

BOOL IsClientCountZero() {
	// Check for whether the count of connected clients is zero. If so, then
	// we can shut down.
	LockMutex(GetClientListMutex());
	{
		if (GetElementCount(g_pClientList) == 0) {
			if (GetLogFileHandle() != stdout) {
				LogInfo("Master Acceptor Thread: Client count is zero.");
			}

			UnlockMutex(GetClientListMutex());

			return TRUE;  // stop this loop when there are no more
			// connected clients
		}
	}
	UnlockMutex(GetClientListMutex());

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// MakeServerEndpointReusable function
/**
 * @brief Marks a server socket file descriptor as reusable.
 * @param nServerSocket Socket file descriptor for the server's listening
 * socket.
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
	CloseSocket(GetServerSocket());

	LockMutex(GetClientListMutex());
	{
		// If there are no clients connected, then we're done
		if (0 == GetElementCount(g_pClientList)) {
			// Re-register this semaphore
			RegisterEvent(TerminateMasterThread);
			UnlockMutex(GetClientListMutex());
			return;
		}

		// Go through the list of connected clients, one by one, and
		// send signals to each client's thread to die
		DoForEach(g_pClientList, KillClientThread);
		sleep(1);
	}
	UnlockMutex(GetClientListMutex());

	// Re-register this semaphore
	RegisterEvent(TerminateMasterThread);
}

///////////////////////////////////////////////////////////////////////////////
// WaitForNewClientConnection function

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
			//fprintf(stdout, "errno = %d\n", errno);

			//fprintf(stderr, INVALID_CLIENT_SOCKET_HANDLE);

			return NULL;
		} else {
			// Getting EBADF from doing an accept() on the server's socket
			// means it's time to quit
			return NULL;
		}
	}

	char* pszClientIPAddress = inet_ntoa(clientAddress.sin_addr);

	/* Echo a message to the screen that a client connected. */
	fprintf(stdout, NEW_CLIENT_CONN, pszClientIPAddress);

	if (GetLogFileHandle() != stdout) {
		LogInfo(NEW_CLIENT_CONN, pszClientIPAddress);
	}

	// if we are here then we have a brand-new client connection
	LPCLIENTSTRUCT lpCS = CreateClientStruct(nClientSocket, pszClientIPAddress);
	if (NULL == lpCS) {
		fprintf(stderr, FAILED_CREATE_NEW_CLIENT);

		CleanupServer(ERROR);
	}

	// Set the new client endpoint to be non-blocking so that we can
	// poll it continuously for new data in its own thread.
	//SetSocketNonBlocking(lpCS->nSocket);
	return lpCS;
}
