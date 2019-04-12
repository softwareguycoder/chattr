///////////////////////////////////////////////////////////////////////////////
// client.c - Chat client in C
// This program allows the user to connect to a Chat server residing on a
// IP address and port as supplied on the command line.  The user interface
// of this program allows the user to type lines of text to be sent to the
// server.
//
// AUTHOR: Brian Hart
// DATE: 13 Nov 2018
//
// Shout out to https://gist.github.com/DnaBoss/a5e1ea07de5ef3525937 for 
// code that provided inspiration
//

#include "stdafx.h"
#include "client.h"
#include "client_functions.h"

#include "client_manager.h"
#include "receive_thread.h"
#include "send_thread.h"

// Client socket for connecting to the server.
// This was turned into a file-scope global so
// that all the functions in this module can
// access it.
int nClientSocket = -1;

///////////////////////////////////////////////////////////////////////////////
// main application function
//

int main(int argc, char *argv[]) {
	if (!InitializeApplication())
		return -1;

	printf(SOFTWARE_TITLE);
	printf(COPYRIGHT_MESSAGE);

	// Check the arguments.  If there is less than 3 arguments, then 
	// we should print a message to stderr telling the user what to 
	// pass on the command line and then quit
	if (!IsCommandLineArgumentCountValid(argc)) {
		LogError("chattr: Failed to validate arguments.");

		fprintf(stderr, USAGE_STRING);

		CleanupClient(ERROR);
	}

	const char* pszHostNameOrIP = argv[1]; // address or host name of the remote server

	int nPort = ParsePortNumber(argv[2]);

	nClientSocket = CreateSocket();

	if (!IsSocketValid(nClientSocket)) {
		fprintf(stderr,
				"chattr: Could not create TCP endpoint for connecting to the"
				" server.\n");

		CleanupClient(ERROR);
	}

	// Attempt to connect to the server.  The function below is guaranteed to close the socket
	// and forcibly terminate this program in the event of a network error, so we do not need
	// to check the result.
	if (OK != ConnectSocket(nClientSocket, pszHostNameOrIP, nPort)) {
		fprintf(stderr,
				"chattr: Failed to connect to server '%s' on port %d.\n",
				pszHostNameOrIP, nPort);

		CleanupClient(ERROR);
	}

	SetSocketNonBlocking(nClientSocket);

	LogInfo("chattr: Client socket has been set to non-blocking.");

	LogInfo("chattr: Now connected to the chat server '%s' on port %d.",
				pszHostNameOrIP, nPort);

	if (GetLogFileHandle() != stdout) {
		fprintf(stdout,
				"chattr: Now connected to the chat server '%s' on port %d.\n",
				pszHostNameOrIP, nPort);
	}

	HandshakeWithServer();

	RegisterEvent(TerminateReceiveThread);

	g_hReceiveThread = CreateThread(ReceiveThread);

	if (INVALID_HANDLE_VALUE == g_hReceiveThread) {
		fprintf(stderr,
				"chattr: Failed to spawn the receive thread.  Quitting.\n");

		CleanupClient(ERROR);
	}

	g_hSendThread = CreateThread(SendThread);

	WaitThread(g_hReceiveThread);

	WaitThread(g_hSendThread);

	DestroyThread(g_hReceiveThread);

	DestroyThread(g_hSendThread);

	if (GetLogFileHandle() != stdout) {
		fprintf(stdout, "chattr: Done chatting!\n");
	}

	CleanupClient(OK);
}
