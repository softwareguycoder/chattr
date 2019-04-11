///////////////////////////////////////////////////////////////////////////////
// server.c - TCP chat server in C
// Rudimentary chat server supporting multiple, concurrently-connected clients
// that can chat with each other.
//
// AUTHOR: Brian Hart
// LAST UPDATE: 11 Apr 2019
//
// Shout-out to <https://gist.githubusercontent.com/suyash/2488ff6996c98a8ee3a8
// 4fe3198a6f85/raw/ba06e891060b277867a6f9c7c2afa20da431ec91/server.c> and
// <http://www.linuxhowtos.org/C_C++/socket.htm> for 
// inspiration
//

#include "stdafx.h"
#include "server.h"

#include "mat.h"
#include "server_functions.h"

///////////////////////////////////////////////////////////////////////////////
// Main application code

int main(int argc, char *argv[]) {
	if (!InitializeApplication()) {
		CleanupServer(ERROR);
	}

	PrintSoftwareTitleAndCopyright();

	// Check the arguments.  If the checks fail, then
	// we should print a message to stderr telling the user what to
	// pass on the command line and then quit
	if (!CheckCommandLineArgs(argc, argv)) {
		fprintf(stderr, USAGE_STRING);

		CleanupServer(ERROR);
	}

	char *pszPortNum = argv[1];

	g_nServerSocket = CreateSocket();

	struct sockaddr_in* pServerAddrInfo = SetUpServerOnPort(pszPortNum);

	// Bind the server socket to associate it with this host as a server
	if (BindSocket(g_nServerSocket, pServerAddrInfo) < 0) {
		fprintf(stderr, "server: Could not bind server's TCP endpoint.");

		free_buffer((void**)&pServerAddrInfo);

		CleanupServer(ERROR);
	}

	if (ListenSocket(g_nServerSocket) < 0) {
		fprintf(stderr, "server: Could not open server endpoint for listening.");

		free_buffer((void**)&pServerAddrInfo);

		CleanupServer(ERROR);
	}

	fprintf(stdout, "server: Now listening on port %s\n", argv[1]);

	g_hMasterThread = CreateThreadEx(MasterAcceptorThread, &g_nServerSocket);
	if (INVALID_HANDLE_VALUE == g_hMasterThread) {
		fprintf(stderr, "server: Failed to initialize master acceptor "
				"thread.\n");

		free_buffer((void**)&pServerAddrInfo);

		CleanupServer(ERROR);
	}

	/* Wait until the master acceptor thread terminates.  This thread
	 * is in charge of accepting new client connections and then spinning
	 * off new threads to handle the new connections. */
	WaitThread(g_hMasterThread);

	/* We might never get here; but if we do, we arrive here when the master
	 * acceptor thread has terminated. */
	QuitServer();

	return OK;
}
