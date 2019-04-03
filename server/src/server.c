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

#include "server.h"
#include "mat.h"
#include "utils.h"
#include "client_list.h"
#include "client_struct.h"
#include "client_thread.h"

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

void DestroyClientListMutex(){
	log_debug("In DestroyClientListMutex");

	log_info("DestroyClientListMutex: Checking whether the client list mutex has already been freed...");

	if (INVALID_HANDLE_VALUE == hClientListMutex) {
		log_info("DestroyClientListMutex: The client list mutex handle has already been freed.  Nothing to do.");

		log_debug("DestroyClientListMutex: Done.");

		return;
	}

	log_info("DestroyClientListMutex: The client list mutex handle has not been freed yet.  Doing so...");

	DestroyMutex(hClientListMutex);

	log_info("DestroyClientListMutex: Client list mutex handle freed.");

	log_debug("DestroyClientListMutex: Done.");
}

void QuitServer() {
	log_debug("In QuitServer");

	KillThread(hMasterThread);

	// If the socket file desClientcriptor in the global variable server_socket
	// is less than or equal zero, then there is nothing to do here.
	if (!isValidSocket(server_socket)) {
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

	log_info("QuitServer: Freeing socket mutex...");

	FreeSocketMutex();

	log_info("QuitServer: Socket mutex freed.");

	log_info("QuitServer: execution finished with no errors.");

	log_info("QuitServer: Releasing resources associated with the list of clients...");

	DestroyList(&clientList, FreeClient);

	log_info("QuitServer: Client list resources freed.");

	log_info("QuitServer: Releasing resources consumed by the client list mutex...");

	DestroyClientListMutex();

	log_info("QuitServer: Client list mutex resources freed.");

	is_execution_over = 1;

	log_debug("QuitServer: Done.");
}

void CleanupServer(int exitCode) {
	log_debug("In CleanupServer");

	log_debug("CleanupServer: exitCode = %d", exitCode);

	// Handle the case where the user presses CTRL+C in the terminal
	// by performing an orderly shut down of the server and freeing
	// operating system resources.

	log_info("CleanupServer: Calling QuitServer...");

	QuitServer();

	log_info("CleanupServer: Finished calling QuitServer.");

	log_info("CleanupServer: Closing the log file...");

	close_log_file_handles();

	/* beyond this point, we cannot utlize the log_* functions */

	exit(exitCode);	// terminate program
}

void CreateClientListMutex() {
	log_debug("In CreateClientListMutex");

	log_info("CreateClientListMutex: Checking whether the client list mutex handle has been created...");

	if (INVALID_HANDLE_VALUE != hClientListMutex) {
		log_info("CreateClientListMutex: Client list mutex already initialized.  Nothing to do.");

		log_debug("CreateClientListMutex: Done.");

		return;
	}

	log_info("CreateClientListMutex: Client list mutex handle needs to be initialized.  Doing so...");

	hClientListMutex = CreateMutex();
	if (INVALID_HANDLE_VALUE == hClientListMutex) {
		log_error("CreateClientListMutex: Failed to initialize the client tracking module.");

		log_debug("CreateClientListMutex: Done.");

		CleanupServer(ERROR);
	}

	log_info("CreateClientListMutex: Client mutex has been initialized successfully.");

	log_debug("CreateClientListMutex: Done.");
}

// Functionality to handle the case where the user has pressed CTRL+C
// in this process' terminal window
void ServerCleanupHandler(int s) {
	log_debug("In ServerCleanupHandler");

	log_info(
			"ServerCleanupHandler: Since we're here, user has pressed CTRL+C.");

	printf("\n");

	log_info(
			"ServerCleanupHandler: Calling CleanupServer with OK exit code...");

	CleanupServer(OK);

	log_info("ServerCleanupHandler: CleanupServer called.");

	log_debug("ServerCleanupHandler: Done.");
}

// Installs a sigint handler to handle the case where the user
// presses CTRL+C in this process' terminal window.  This allows 
// us to clean up the main while loop and free operating system
// resources gracefully.
//
// Shout-out to <https://stackoverflow.com/questions/1641182/
// how-can-i-catch-a-ctrl-c-event-c> for this code.
void InstallSigintHandler() {
	log_debug("In InstallSigintHandler");

	log_debug(
			"InstallSigintHandler: Configuring operating system structure...");

	struct sigaction sigIntHandler;

	sigIntHandler.sa_handler = ServerCleanupHandler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;

	log_debug(
			"InstallSigintHandler: Structure configured.  Calling sigaction function...");

	if (OK != sigaction(SIGINT, &sigIntHandler, NULL)) {
		fprintf(stderr, "server: Unable to install CTRL+C handler.");

		log_error("server: Unable to install CTRL+C handler.");

		perror("server[sigaction]");

		log_debug("server: Freeing the socket mutex object...");

		FreeSocketMutex();

		log_debug("server: Socket mutex object freed.");

		log_debug("server: Done.");

		exit(ERROR);
	}

	log_debug("InstallSigintHandler: SIGINT handler (for CTRL+C) installed.");

	log_debug("InstallSigintHandler: Done.");
}

void ConfigureLogFile() {
	remove(LOG_FILE_PATH);
	set_log_file(fopen(LOG_FILE_PATH, LOG_FILE_OPEN_MODE));
	set_error_log_file(get_log_file_handle());

	/*set_log_file(stdout);
	 set_error_log_file(stderr);*/
}

BOOL InitializeApplication() {
	/* Configure settings for the log file */
	ConfigureLogFile();

	log_info("Welcome to the log for the server application");

	log_debug("In InitializeApplication");

	log_info("InitializeApplication: Creating socket mutex object...");

	/* Initialize the socket mutex object in the inetsock_core library */
	CreateSocketMutex();

	log_info("InitializeApplication: Socket mutex has been created successfully.");

	log_info("InitializeApplication: Initializing client list mutex...");

	CreateClientListMutex();

	log_info("InitializeApplication: Client list mutex has been initialized.");

	log_info("InitializeApplication: Installing a SIGINT handler to perform cleanup when CTRL+C is pressed...");

	// Since the usual way to exit this program is for the user to
	// press CTRL+C to forcibly terminate it, install a Linux SIGINT
	// handler here so that when the user does this, we may still
	// get a chance to run the proper cleanup code.
	InstallSigintHandler();

	log_info("InitializeApplication: SIGINT CTRL+C cleanup handler now installed.");

	log_debug("InitializeApplication: Done.");

	return TRUE;
}

int main(int argc, char *argv[]) {
	if (!InitializeApplication())
		return -1;

	printf(SOFTWARE_TITLE);
	printf(COPYRIGHT_MESSAGE);

	//int bytesReceived = 0, bytesSent = 0;

	log_info("server: argc = %d", argc);

	log_info("server: Checking arguments...");

	// Check the arguments.  If there is less than 2 arguments, then 
	// we should print a message to stderr telling the user what to 
	// pass on the command line and then quit
	if (argc < MIN_NUM_ARGS) {
		fprintf(stderr, USAGE_STRING);

		CleanupServer(ERROR);
	}

	if (argc >= MIN_NUM_ARGS)
		log_info("server: Port number configured as %s.", argv[1]);

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

		CleanupServer(ERROR);
	}

	log_info("server: Endpoint bound to localhost on port %s.", argv[1]);

	log_info("server: Attempting to listen on port %s...", argv[1]);

	if (SocketDemoUtils_listen(server_socket) < 0) {
		log_error("server: Could not open server endpoint for listening.");

		CleanupServer(ERROR);
	}

	log_info("server: Now listening on port %s", argv[1]);

	if (get_log_file_handle() != stdout) {
		fprintf(stdout, "server: Now listening on port %s\n", argv[1]);
	}

	log_info("server: Starting Master Acceptor Thread (MAT)...");

	hMasterThread = CreateThreadEx(MasterAcceptorThread, &server_socket);

	log_info("server: Started MAT.");

	log_info("server: Waiting until the MAT terminates...");

	/* Wait until the master thread terminates */
	WaitThread(hMasterThread);

	log_debug("server: Done.");

	QuitServer();

	close_log_file_handles();

	return OK;
}
