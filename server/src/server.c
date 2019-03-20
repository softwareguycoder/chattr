///////////////////////////////////////////////////////////////////////////////
// server.c - Echo server in C
// The server receives text a line at a time and echoes the text back to its
// client only AFTER an entire line has been received.
//
// AUTHOR: Brian Hart
// DATE: 20 Sep 2018
//
// Shout-out to <https://gist.githubusercontent.com/suyash/2488ff6996c98a8ee3a8
// 4fe3198a6f85/raw/ba06e891060b277867a6f9c7c2afa20da431ec91/server.c> and
// <http://www.linuxhowtos.org/C_C++/socket.htm> for 
// inspiration
//

/*
 TCP uses 2 types of sockets: the connection socket and the listen socket.
 The goal is to separate the connection phase from the data exchange phase.
 */

#include "stdafx.h"
#include "utils.h"

#include "server.h"
#include "list.h"
#include "mat.h"
#include "clientThread.h"
#include "clientStruct.h"

POSITION* clientList = NULL;

// Let us create a global for the mutex lock object
HMUTEX hClientListMutex; // global mutex handle

// Handle for the master thread that accepts all incoming connections
HTHREAD hMasterThread;

int server_socket = 0;
int is_execution_over = 0;

BOOL FindClientBySocket(void* pClientSocketFd, void* pClientStruct) {
	if (pClientSocketFd == NULL || pClientStruct == NULL)
		return FALSE;

	int clientSockFd = *((int*) pClientSocketFd);
	CLIENTSTRUCT* client_Struct = (CLIENTSTRUCT*) pClientStruct;

	if (clientSockFd == client_Struct->sockFD) {
		return TRUE;
	}

	return FALSE;
}

void FreeClient(void* pClientStruct) {
	if (pClientStruct == NULL)
		return;

	free(pClientStruct);
	pClientStruct = NULL;
}

void QuitServer() {
	fprintf(stdout, "In quit_server\n");

	// If the socket file descriptor in the global variable server_socket
	// is less than or equal zero, then there is nothing to do here.
	if (server_socket <= 0) {
		fprintf(stdout,
				"quit_server: The server_socket variable has a negative value.\n");

		fprintf(stdout, "quit_server: Done.\n");
		return;
	}

	fprintf(stdout, "quit_server: Closing the server's TCP endpoint...\n");

	if (server_socket > 0) {
		close(server_socket);
		server_socket = -1;
	}

	fprintf(stdout, "S: <disconnected>\n");

	fprintf(stdout, "quit_server: Server endpoint closed.\n");
	fprintf(stdout, "quit_server: execution finished with no errors.\n");

	DestroyList(&clientList, FreeClient);

	is_execution_over = 1;

	fprintf(stdout, "quit_server: Done.\n");
}

void CleanupServer(int exitCode){
	// Handle the case where the user presses CTRL+C in the terminal
	// by performing an orderly shut down of the server and freeing
	// operating system resources.

	QuitServer();
	exit(exitCode);
}

// Functionality to handle the case where the user has pressed CTRL+C
// in this process' terminal window
void ServerCleanupHandler(int s) {
	printf("\n");

	CleanupServer(OK);
}

// Installs a sigint handler to handle the case where the user
// presses CTRL+C in this process' terminal window.  This allows 
// us to clean up the main while loop and free operating system
// resources gracefully.
//
// Shout-out to <https://stackoverflow.com/questions/1641182/
// how-can-i-catch-a-ctrl-c-event-c> for this code.
void install_sigint_handler() {
	struct sigaction sigIntHandler;

	sigIntHandler.sa_handler = ServerCleanupHandler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;

	sigaction(SIGINT, &sigIntHandler, NULL);
}

int main(int argc, char *argv[]) {
	printf(SOFTWARE_TITLE);
	printf(COPYRIGHT_MESSAGE);

	//int bytesReceived = 0, bytesSent = 0;

	// Since the usual way to exit this program is for the user to
	// press CTRL+C to forcibly terminate it, install a Linux SIGINT
	// handler here so that when the user does this, we may still
	// get a chance to run the proper cleanup code.
	install_sigint_handler();

	log_info("server: Checking arguments...");

	// Check the arguments.  If there is less than 2 arguments, then 
	// we should print a message to stderr telling the user what to 
	// pass on the command line and then quit
	if (argc < MIN_NUM_ARGS) {
		fprintf(stderr, USAGE_STRING);
		exit(ERROR);
	}

	if (argc >= MIN_NUM_ARGS)
		log_info("server: Port number configured as %s.", argv[1]);

	log_info("server: Initializing client tracking module...");

	hClientListMutex = CreateMutex();
	if (INVALID_HANDLE_VALUE == hClientListMutex)
	{
		error("Failed to initialize the client tracking module.");
	}

	log_info("server: The client tracking module has been initialized.");

	log_info("server: Creating server TCP endpoint...");

	server_socket = SocketDemoUtils_createTcpSocket();

	log_info("server: The TCP endpoint for the server has been created.");

	// Assume that the first argument (argv[1]) is the port number 
	// that the user wants us to listen on 
	struct sockaddr_in server_address;      // socket address for the server
	memset(&server_address, 0, sizeof(server_address));

	log_info("server: Initializing server binding information...");

	SocketDemoUtils_populateServerAddrInfo(argv[1], &server_address);

	// Bind the server socket to associate it with this host as a server
	if (SocketDemoUtils_bind(server_socket, &server_address) < 0) {
		log_error("server: Could not bind endpoint.");
		exit(ERROR);
	}

	log_info("server: Endpoint bound to localhost on port %s.", argv[1]);

	if (SocketDemoUtils_listen(server_socket) < 0) {
		log_error("server: Could not open server endpoint for listening.");
	}

	log_info("server: Now listening on port %s", argv[1]);

	log_info("server: Started master accepter thread.");

	hMasterThread = CreateThread(MasterAcceptorThread);

	/* Wait until the master thread terminates */
	WaitThread(hMasterThread);

	return OK;
}
