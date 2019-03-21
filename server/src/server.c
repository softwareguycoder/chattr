///////////////////////////////////////////////////////////////////////////////
// server.c - TCP chat server in C
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

// Mode for opening the log file (appending)
#define LOG_FILE_OPEN_MODE	"a+"

// Path to the log file
#define LOG_FILE_PATH	"/home/bhart/logs/chattr/server.log"

POSITION* clientList = NULL;

// Let us create a global for the mutex lock object
HMUTEX hClientListMutex; // global mutex handle

// Handle for the master thread that accepts all incoming connections
HTHREAD hMasterThread;

int server_socket = 0;
int is_execution_over = 0;

BOOL FindClientBySocket(void* pClientSocketFd, void* pClientStruct) {
	log_debug("In FindClientBySocket");

	log_info("FindClientBySocket: Checking whether both parameters are filled in...");

	if (pClientSocketFd == NULL || pClientStruct == NULL){
		log_warning("FindClientBySocket: One or both parameters not specified.");

		log_debug("FindClientBySocket: Returning FALSE.");

		log_debug("FindClientBySocket: Done.");

		return FALSE;
	}

	log_info("FindClientBySocket: Attempting to retrieve client socket file descriptor...");

	int clientSockFd = *((int*) pClientSocketFd);

	log_debug("FindClientBySocket: clientSockFd = %d", clientSockFd);

	log_info("FindClientBySocket: Attempting to retrieve client structure pointer...");

	CLIENTSTRUCT* client_Struct = (CLIENTSTRUCT*) pClientStruct;

	log_info("FindClientBySocket: Checking whether the client socket is associated with the client structure...");

	if (clientSockFd == client_Struct->sockFD) {
		log_info("FindClientBySocket: Client structure matching the supplied socket value found.");

		log_debug("FindClientBySocket: Returning TRUE.");

		log_debug("FindClientBySocket: Done.");

		return TRUE;
	}

	log_debug("FindClientBySocket: Returning FALSE.");

	log_debug("FindClientBySocket: Done.");

	return FALSE;
}

void FreeClient(void* pClientStruct) {
	log_debug("In FindClientBySocket");

	if (pClientStruct == NULL)
		return;

	free(pClientStruct);
	pClientStruct = NULL;
}

void QuitServer() {
	log_debug("In QuitServer");

	// If the socket file descriptor in the global variable server_socket
	// is less than or equal zero, then there is nothing to do here.
	if (server_socket <= 0) {
		fprintf(stdout,
				"QuitServer: The server_socket variable has a negative value.");

		log_debug("QuitServer: Done.");
		return;
	}

	log_info("QuitServer: Closing the server's TCP endpoint...");

	if (server_socket > 0) {
		close(server_socket);
		server_socket = -1;
	}

	fprintf(stdout, "S: <disconnected>\n");

	log_info("QuitServer: Server endpoint closed.");
	log_info("QuitServer: execution finished with no errors.");

	DestroyList(&clientList, FreeClient);

	is_execution_over = 1;

	log_debug("QuitServer: Done.");
}

void CleanupServer(int exitCode){
	// Handle the case where the user presses CTRL+C in the terminal
	// by performing an orderly shut down of the server and freeing
	// operating system resources.

	QuitServer();

	close_log_file();

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

int main(int argc, char *argv[])
{
	/*remove(LOG_FILE_PATH);
	set_log_file(fopen(LOG_FILE_PATH, LOG_FILE_OPEN_MODE));
	set_error_log_file(get_log_file_handle());
	*/

	set_log_file(stdout);
	set_error_log_file(stderr);

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

		close_log_file();

		exit(ERROR);
	}

	if (argc >= MIN_NUM_ARGS)
		log_info("server: Port number configured as %s.", argv[1]);

	log_info("server: Initializing client tracking module...");

	hClientListMutex = CreateMutex();
	if (INVALID_HANDLE_VALUE == hClientListMutex)
	{
		log_error("Failed to initialize the client tracking module.");

		close_log_file();

		exit(ERROR);
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

		close_log_file();

		exit(ERROR);
	}

	log_info("server: Endpoint bound to localhost on port %s.", argv[1]);

	if (SocketDemoUtils_listen(server_socket) < 0) {
		log_error("server: Could not open server endpoint for listening.");

		close_log_file();

		exit(ERROR);
	}

	log_info("server: Now listening on port %s", argv[1]);

	log_info("server: Started master accepter thread.");

	hMasterThread = CreateThreadEx(MasterAcceptorThread, &server_socket);

	/* Wait until the master thread terminates */
	WaitThread(hMasterThread);

	close_log_file();

	return OK;
}
