///////////////////////////////////////////////////////////////////////////////
// server.c - TCP chat server in C
// Rudimentary chat server supporting multiple, concurrently-connected clients
// that can chat with each other.
//
// AUTHOR: Brian Hart
// DATE: 19 Feb 2019
//
// Shout-out to <https://gist.githubusercontent.com/suyash/2488ff6996c98a8ee3a8
// 4fe3198a6f85/raw/ba06e891060b277867a6f9c7c2afa20da431ec91/server.c> and
// <http://www.linuxhowtos.org/C_C++/socket.htm> for 
// inspiration
//

#include "stdafx.h"
#include "server.h"

#include "client_list.h"
#include "client_manager.h"
#include "client_struct.h"
#include "client_thread.h"
#include "mat.h"
#include "server_functions.h"

POSITION* g_pClientList = NULL;

// Let us create a global for the mutex lock object
HMUTEX g_hClientListMutex; // global mutex handle

// Handle for the master thread that accepts all incoming connections
HTHREAD g_hMasterThread;

int g_nServerSocket = 0;

// Functionality to handle the case where the user has pressed CTRL+C
// in this process' terminal window
// Installs a sigint handler to handle the case where the user
// presses CTRL+C in this process' terminal window.  This allows 
// us to clean up the main while loop and free operating system
// resources gracefully.
//
// Shout-out to <https://stackoverflow.com/questions/1641182/
// how-can-i-catch-a-ctrl-c-event-c> for this code.
int main(int argc, char *argv[]) {
	if (!InitializeApplication())
		return -1;

	printf(SOFTWARE_TITLE);
	printf(COPYRIGHT_MESSAGE);

	//int bytesReceived = 0, bytesSent = 0;

	LogInfo("server: argc = %d", argc);

	LogInfo("server: Checking arguments...");

	// Check the arguments.  If there is less than 2 arguments, then 
	// we should print a message to stderr telling the user what to 
	// pass on the command line and then quit
	if (argc < MIN_NUM_ARGS) {
		fprintf(stderr, USAGE_STRING);

		CleanupServer(ERROR);
	}

	if (argc >= MIN_NUM_ARGS)
		LogInfo("server: Port number configured as %s.", argv[1]);

	LogInfo("server: Creating server TCP endpoint...");

	g_nServerSocket = CreateSocket();

	LogInfo("server: The TCP endpoint for the server has been created.");

	// Assume that the first argument (argv[1]) is the port number 
	// that the user wants us to listen on 
	struct sockaddr_in server_address;      // socket address for the server
	memset(&server_address, 0, sizeof(server_address));

	LogInfo("server: Initializing server binding information...");

	GetServerAddrInfo(argv[1], &server_address);

	// Bind the server socket to associate it with this host as a server
	if (BindSocket(g_nServerSocket, &server_address) < 0) {
		LogError("server: Could not bind endpoint.");

		CleanupServer(ERROR);
	}

	LogInfo("server: Endpoint bound to localhost on port %s.", argv[1]);

	LogInfo("server: Attempting to listen on port %s...", argv[1]);

	if (ListenSocket(g_nServerSocket) < 0) {
		LogError("server: Could not open server endpoint for listening.");

		CleanupServer(ERROR);
	}

	LogInfo("server: Now listening on port %s", argv[1]);

	if (GetLogFileHandle() != stdout) {
		fprintf(stdout, "server: Now listening on port %s\n", argv[1]);
	}

	LogInfo("server: Starting Master Acceptor Thread (MAT)...");

	g_hMasterThread = CreateThreadEx(MasterAcceptorThread, &g_nServerSocket);

	LogInfo("server: Started MAT.");

	LogInfo("server: Waiting until the MAT terminates...");

	/* Wait until the master thread terminates */
	WaitThread(g_hMasterThread);

	LogDebug("server: Done.");

	QuitServer();

	CloseLogFileHandles();

	return OK;
}
