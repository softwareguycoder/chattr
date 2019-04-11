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
#include "mat.h"
#include "client_list.h"
#include "client_manager.h"
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

/*BOOL g_bKeepAlive = FALSE;*/

int server_socket = 0;
int is_execution_over = 0;

void DestroyClientListMutex() {
	LogDebug("In DestroyClientListMutex");

	LogInfo(
			"DestroyClientListMutex: Checking whether the client list mutex has already been freed...");

	if (INVALID_HANDLE_VALUE == hClientListMutex) {
		LogInfo(
				"DestroyClientListMutex: The client list mutex handle has already been freed.  Nothing to do.");

		LogDebug("DestroyClientListMutex: Done.");

		return;
	}

	LogInfo(
			"DestroyClientListMutex: The client list mutex handle has not been freed yet.  Doing so...");

	DestroyMutex(hClientListMutex);

	LogInfo("DestroyClientListMutex: Client list mutex handle freed.");

	LogDebug("DestroyClientListMutex: Done.");
}

void QuitServer() {
	LogDebug("In QuitServer");

	fprintf(stdout, "server: Shutting down...\n");

	LogInfo(
			"QuitServer: Attempting to kill the Master Acceptor Thread (MAT)...");

	KillThread(hMasterThread);

	LogInfo("QuitServer: MAT killed.");

	sleep(1); /* induce a context switch */

	LogInfo("QuitServer: Releasing system resources consumed by interlock infrastructure...");

	DestroyInterlock();

	LogInfo("QuitServer: Atomic opreation interlock resources released.");

	fprintf(stdout, "S: <disconnected>\n");

	LogInfo("QuitServer: Freeing socket mutex...");

	FreeSocketMutex();

	LogInfo("QuitServer: Socket mutex freed.");

	LogInfo("QuitServer: execution finished with no errors.");

	LogInfo(
			"QuitServer: Releasing resources associated with the list of clients...");

	DestroyList(&clientList, FreeClient);

	LogInfo("QuitServer: Client list resources freed.");

	LogInfo(
			"QuitServer: Releasing resources consumed by the client list mutex...");

	DestroyClientListMutex();

	LogInfo("QuitServer: Client list mutex resources freed.");

	is_execution_over = 1;

	LogDebug("QuitServer: Done.");
}

void CleanupServer(int exitCode) {
	LogDebug("In CleanupServer");

	LogDebug("CleanupServer: exitCode = %d", exitCode);

	// Handle the case where the user presses CTRL+C in the terminal
	// by performing an orderly shut down of the server and freeing
	// operating system resources.

	LogInfo("CleanupServer: Obtaining a lock on the client list mutex...");

	LockMutex(hClientListMutex);
	{
		LogInfo("CleanupServer: Lock established.");

		LogInfo(
				"CleanupServer: Checking whether the count of connected clients is greater than zero...");

		if (nClientCount > 0) {
			LogInfo(
					"CleanupServer: The count of connected clients is greater than zero.");

			LogInfo("CleanupServer: Forcibly disconnecting each client...");

			ForEach(&clientList, DisconnectClient);

			LogInfo("CleanupServer: Disconnection operation completed.");
		} else {
			LogInfo(
					"CleanupServer: Zero clients are currently connected.  Nothing to do.");
		}

		LogInfo("CleanupServer: Releasing the client list lock...");
	}
	UnlockMutex(hClientListMutex);

	LogInfo("CleanupServer: Client list lock released.");

	LogInfo("CleanupServer: Calling QuitServer...");

	QuitServer();

	LogInfo("CleanupServer: Finished calling QuitServer.");

	LogInfo("CleanupServer: Closing the log file...");

	CloseLogFileHandles();

	/* beyond this point, we cannot utlize the log_* functions */

	exit(exitCode);	// terminate program
}

void CreateClientListMutex() {
	LogDebug("In CreateClientListMutex");

	LogInfo(
			"CreateClientListMutex: Checking whether the client list mutex handle has been created...");

	if (INVALID_HANDLE_VALUE != hClientListMutex) {
		LogInfo(
				"CreateClientListMutex: Client list mutex already initialized.  Nothing to do.");

		LogDebug("CreateClientListMutex: Done.");

		return;
	}

	LogInfo(
			"CreateClientListMutex: Client list mutex handle needs to be initialized.  Doing so...");

	hClientListMutex = CreateMutex();
	if (INVALID_HANDLE_VALUE == hClientListMutex) {
		LogError(
				"CreateClientListMutex: Failed to initialize the client tracking module.");

		LogDebug("CreateClientListMutex: Done.");

		CleanupServer(ERROR);
	}

	LogInfo(
			"CreateClientListMutex: Client mutex has been initialized successfully.");

	LogDebug("CreateClientListMutex: Done.");
}

// Functionality to handle the case where the user has pressed CTRL+C
// in this process' terminal window
void ServerCleanupHandler(int s) {
	LogDebug("In ServerCleanupHandler");

	LogInfo("ServerCleanupHandler: Since we're here, user has pressed CTRL+C.");

	printf("\n");

	LogInfo("ServerCleanupHandler: Calling CleanupServer with OK exit code...");

	CleanupServer(OK);

	LogInfo("ServerCleanupHandler: CleanupServer called.");

	LogDebug("ServerCleanupHandler: Done.");
}

// Installs a sigint handler to handle the case where the user
// presses CTRL+C in this process' terminal window.  This allows 
// us to clean up the main while loop and free operating system
// resources gracefully.
//
// Shout-out to <https://stackoverflow.com/questions/1641182/
// how-can-i-catch-a-ctrl-c-event-c> for this code.
void InstallSigintHandler() {
	LogDebug("In InstallSigintHandler");

	LogDebug("InstallSigintHandler: Configuring operating system structure...");

	struct sigaction sigIntHandler;

	sigIntHandler.sa_handler = ServerCleanupHandler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;

	LogDebug(
			"InstallSigintHandler: Structure configured.  Calling sigaction function...");

	if (OK != sigaction(SIGINT, &sigIntHandler, NULL)) {
		fprintf(stderr, "server: Unable to install CTRL+C handler.");

		LogError("server: Unable to install CTRL+C handler.");

		perror("server[sigaction]");

		LogDebug("server: Freeing the socket mutex object...");

		FreeSocketMutex();

		LogDebug("server: Socket mutex object freed.");

		LogDebug("server: Done.");

		exit(ERROR);
	}

	LogDebug("InstallSigintHandler: SIGINT handler (for CTRL+C) installed.");

	LogDebug("InstallSigintHandler: Done.");
}

void ConfigureLogFile() {
	remove(LOG_FILE_PATH);
	SetLogFileHandle(fopen(LOG_FILE_PATH, LOG_FILE_OPEN_MODE));
	SetErrorLogFileHandle(GetLogFileHandle());

	/*set_log_file(stdout);
	 set_error_log_file(stderr);*/
}

BOOL InitializeApplication() {
	/* Configure settings for the log file */
	ConfigureLogFile();

	LogInfo("Welcome to the log for the server application");

	LogDebug("In InitializeApplication");

	LogInfo("InitializeApplication: Initializing interlock for atomic operations...");

	InitializeInterlock();

	LogInfo("IntiailizeApplcation: Initialized atomic operation interlocks.");

	LogInfo("InitializeApplication: Creating socket mutex object...");

	/* Initialize the socket mutex object in the inetsock_core library */
	CreateSocketMutex();

	LogInfo(
			"InitializeApplication: Socket mutex has been created successfully.");

	LogInfo("InitializeApplication: Initializing client list mutex...");

	CreateClientListMutex();

	LogInfo("InitializeApplication: Client list mutex has been initialized.");

	LogInfo(
			"InitializeApplication: Installing a SIGINT handler to perform cleanup when CTRL+C is pressed...");

	// Since the usual way to exit this program is for the user to
	// press CTRL+C to forcibly terminate it, install a Linux SIGINT
	// handler here so that when the user does this, we may still
	// get a chance to run the proper cleanup code.
	InstallSigintHandler();

	LogInfo(
			"InitializeApplication: SIGINT CTRL+C cleanup handler now installed.");

	LogDebug("InitializeApplication: Done.");

	return TRUE;
}

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

	server_socket = CreateSocket();

	LogInfo("server: The TCP endpoint for the server has been created.");

	// Assume that the first argument (argv[1]) is the port number 
	// that the user wants us to listen on 
	struct sockaddr_in server_address;      // socket address for the server
	memset(&server_address, 0, sizeof(server_address));

	LogInfo("server: Initializing server binding information...");

	GetServerAddrInfo(argv[1], &server_address);

	// Bind the server socket to associate it with this host as a server
	if (BindSocket(server_socket, &server_address) < 0) {
		LogError("server: Could not bind endpoint.");

		CleanupServer(ERROR);
	}

	LogInfo("server: Endpoint bound to localhost on port %s.", argv[1]);

	LogInfo("server: Attempting to listen on port %s...", argv[1]);

	if (ListenSocket(server_socket) < 0) {
		LogError("server: Could not open server endpoint for listening.");

		CleanupServer(ERROR);
	}

	LogInfo("server: Now listening on port %s", argv[1]);

	if (GetLogFileHandle() != stdout) {
		fprintf(stdout, "server: Now listening on port %s\n", argv[1]);
	}

	LogInfo("server: Starting Master Acceptor Thread (MAT)...");

	hMasterThread = CreateThreadEx(MasterAcceptorThread, &server_socket);

	LogInfo("server: Started MAT.");

	LogInfo("server: Waiting until the MAT terminates...");

	/* Wait until the master thread terminates */
	WaitThread(hMasterThread);

	LogDebug("server: Done.");

	QuitServer();

	CloseLogFileHandles();

	return OK;
}
