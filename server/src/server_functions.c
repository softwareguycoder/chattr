// server_functions.c - File to contain the various application-specific
// functions used by the server's main function
//

#include "stdafx.h"
#include "server.h"

#include "client_manager.h"
#include "server_functions.h"

///////////////////////////////////////////////////////////////////////////////
// CheckCommandLineArgs function - Checks the command-line args passed (and the
// count thereof) to ensure that the arguments will be usable

BOOL CheckCommandLineArgs(int argc, char *argv[]) {
	return argc >= MIN_NUM_ARGS && argv != NULL
			&& argv[1] != NULL && argv[1][0] != '\0';
}

///////////////////////////////////////////////////////////////////////////////
// CleanupServer function - Called by routines all over the application to end
// the program gracefully, making sure to release resources and terminate all
// threads in an orderly way

void CleanupServer(int nExitCode) {
	// Handle the case where the user presses CTRL+C in the terminal
	// by performing an orderly shut down of the server and freeing
	// operating system resources.

	LockMutex(g_hClientListMutex);
	{
		if (g_nClientCount > 0) {
			ForEach(&g_pClientList, DisconnectClient);
		}
	}
	UnlockMutex(g_hClientListMutex);

	QuitServer();

	CloseLogFileHandles();

	/* beyond this point, we cannot utlize the log_* functions */

	exit(nExitCode);	// terminate program
}

///////////////////////////////////////////////////////////////////////////////
// ConfigureLogFile function - Removes the old log file from disk (if
// applicable) and sets up the file pointers and handles for the new one

void ConfigureLogFile() {
	remove(LOG_FILE_PATH);
	SetLogFileHandle(fopen(LOG_FILE_PATH, LOG_FILE_OPEN_MODE));
	SetErrorLogFileHandle(GetLogFileHandle());

	/*set_log_file(stdout);
	 set_error_log_file(stderr);*/
}

///////////////////////////////////////////////////////////////////////////////
// CreateClientListMutex function - Sets up operating system resources for the
// mutex handle which controls threads' access to the list of clients.  Since
// this is only called exactly once during the lifetime of the application, it's
// place is in this file

void CreateClientListMutex() {
	LogDebug("In CreateClientListMutex");

	LogInfo(
			"CreateClientListMutex: Checking whether the client list mutex handle has been created...");

	if (INVALID_HANDLE_VALUE != g_hClientListMutex) {
		LogInfo(
				"CreateClientListMutex: Client list mutex already initialized.  Nothing to do.");

		LogDebug("CreateClientListMutex: Done.");

		return;
	}

	LogInfo(
			"CreateClientListMutex: Client list mutex handle needs to be initialized.  Doing so...");

	g_hClientListMutex = CreateMutex();
	if (INVALID_HANDLE_VALUE == g_hClientListMutex) {
		LogError(
				"CreateClientListMutex: Failed to initialize the client tracking module.");

		LogDebug("CreateClientListMutex: Done.");

		CleanupServer(ERROR);
	}

	LogInfo(
			"CreateClientListMutex: Client mutex has been initialized successfully.");

	LogDebug("CreateClientListMutex: Done.");
}

///////////////////////////////////////////////////////////////////////////////
// DestroyClientListMutex function - Releases the system resources occupied
// by the client list mutex handle.  It's here because this function just
// needs to be called exactly once during the excecution of the server.

void DestroyClientListMutex() {
	LogDebug("In DestroyClientListMutex");

	LogInfo(
			"DestroyClientListMutex: Checking whether the client list mutex has already been freed...");

	if (INVALID_HANDLE_VALUE == g_hClientListMutex) {
		LogInfo(
				"DestroyClientListMutex: The client list mutex handle has already been freed.  Nothing to do.");

		LogDebug("DestroyClientListMutex: Done.");

		return;
	}

	LogInfo(
			"DestroyClientListMutex: The client list mutex handle has not been freed yet.  Doing so...");

	DestroyMutex(g_hClientListMutex);

	LogInfo("DestroyClientListMutex: Client list mutex handle freed.");

	LogDebug("DestroyClientListMutex: Done.");
}

///////////////////////////////////////////////////////////////////////////////
// PrintSoftwareTitleAndCopyright function - Does exactly what it says on the
// tin.

void PrintSoftwareTitleAndCopyright() {
	printf(SOFTWARE_TITLE);
	printf(COPYRIGHT_MESSAGE);
}

///////////////////////////////////////////////////////////////////////////////
// InitializeApplication function - Runs functionality that should be executed
// exactly once during the lifetime of the application, at application startup.

BOOL InitializeApplication() {
	/* Configure settings for the log file */
	ConfigureLogFile();

	InitializeInterlock();

	/* Initialize the socket mutex object in the inetsock_core library */
	CreateSocketMutex();

	CreateClientListMutex();

	// Since the usual way to exit this program is for the user to
	// press CTRL+C to forcibly terminate it, install a Linux SIGINT
	// handler here so that when the user does this, we may still
	// get a chance to run the proper cleanup code.
	InstallSigintHandler();

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// InstallSigintHandler function - Registers a function to be called when the
// user presses CTRL+C, in order to perform an orderly shutdown

void InstallSigintHandler() {
	struct sigaction sigIntHandler;

	sigIntHandler.sa_handler = ServerCleanupHandler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;

	if (OK != sigaction(SIGINT, &sigIntHandler, NULL)) {
		fprintf(stderr, "server: Unable to install CTRL+C handler.");

		perror("server[sigaction]");

		FreeSocketMutex();

		exit(ERROR);
	}
}

///////////////////////////////////////////////////////////////////////////////
// QuitServer function - Called to conduct an orderly shutdown of the server,
// stopping all threads and releasing operating system resources in an orderly
// fashion.

void QuitServer() {
	fprintf(stdout, "server: Shutting down...\n");

	KillThread(g_hMasterThread);

	sleep(1); /* induce a context switch */

	DestroyInterlock();

	fprintf(stdout, "S: <disconnected>\n");

	FreeSocketMutex();

	DestroyList(&g_pClientList, FreeClient);

	DestroyClientListMutex();
}

///////////////////////////////////////////////////////////////////////////////
// ServerCleanupHandler function - Called when the user presses CTRL+C.  This
// function initiates an orderly shut down of the server application.

void ServerCleanupHandler(int signum) {
	printf("\n");

	CleanupServer(OK);
}

///////////////////////////////////////////////////////////////////////////////
// SetUpServerOnPort function - Sets up the server to be bound to the specified
// port

struct sockaddr_in* SetUpServerOnPort(const char* pszPortNum) {
	struct sockaddr_in* pResult = NULL;

	if (pszPortNum == NULL || pszPortNum[0] == '\0') {
		// Blank port number, nothing to do.
		fprintf(stderr, "server: No port number specified on the "
				"command-line.\n");

		CleanupServer(ERROR);
	}

	pResult = (struct sockaddr_in*)malloc(1*sizeof(struct sockaddr_in));
	if (pResult == NULL) {
		fprintf(stderr, "server: Insufficient operating system memory.\n");

		// Failed to allocate memory
		CleanupServer(ERROR);
	}

	// Zero out the memory occupied by the structure
	memset(pResult, 0, sizeof(struct sockaddr_in));

	// Intialize the structure with server address and port information
	GetServerAddrInfo(pszPortNum, pResult);

	return pResult;
}
