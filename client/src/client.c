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

#include "send_thread_functions.h"
#include "send_thread.h"

///////////////////////////////////////////////////////////////////////////////
// Global variables

// Flag indicating whether we are connected to a server or not.
BOOL g_bConnected = FALSE;

// Client socket for connecting to the server.
// This was turned into a file-scope global so
// that all the functions in this module can
// access it.
int g_nClientSocket = INVALID_SOCKET_VALUE;

// Buffer to hold the client's nickname
char g_szNickname[MAX_NICKNAME_LEN + 1] = {0};

///////////////////////////////////////////////////////////////////////////////
// main application function
//

int main(int argc, char *argv[]) {
	if (!InitializeApplication())
		return -1;

	char* pszHostNameOrIP = NULL;
	int nPort = -1;

    PrintSoftwareTitleAndCopyright();

	// Check the arguments.  If there is less than 3 arguments, then
	// we should print a message to stderr telling the user what to
	// pass on the command line and then quit
	if (!IsCommandLineArgumentCountValid(argc)) {
        LogError(CHATTR_FAILED_TO_VALIDATE_ARGUMENTS);

		fprintf(stderr, USAGE_STRING);

		exit(ERROR);    /* we can just exit here, no spiffy cleanup needed. */
	}

	ParseCommandLine(argv, &pszHostNameOrIP, &nPort);

	g_nClientSocket = CreateSocket();

	if (!IsSocketValid(g_nClientSocket)) {
		fprintf(stderr,
				COULD_NOT_CREATE_CLIENT_TCP_ENDPOINT);

		exit(ERROR);    /* we can just exit here, no spiffy cleanup needed. */
	}

	LPCONNECTIONINFO lpCI = CreateConnectionInfo(
	        pszHostNameOrIP, nPort);
	if (lpCI == NULL){
	    CleanupClient(ERROR);
	}

	ConnectToChatServer(lpCI);

	FreeConnectionInfo(lpCI);

	//SetSocketNonBlocking(g_nClientSocket);

    LogInfo(SET_CLIENT_SOCKET_NON_BLOCKING);

    LogInfo(NOW_CONNECTED_TO_SERVER, pszHostNameOrIP, nPort);

	if (GetLogFileHandle() != stdout) {
		fprintf(stdout,
				NOW_CONNECTED_TO_SERVER,
				pszHostNameOrIP, nPort);
	}

	HandshakeWithServer();

	RegisterEvent(TerminateReceiveThread);

	CreateReceiveThread();

	CreateSendThread();

	WaitThread(g_hReceiveThread);

	WaitThread(g_hSendThread);

	CleanupClient(OK);
}
