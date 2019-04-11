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
	LogDebug("In CleanupServer");

	LogDebug("CleanupServer: exitCode = %d", nExitCode);

	// Handle the case where the user presses CTRL+C in the terminal
	// by performing an orderly shut down of the server and freeing
	// operating system resources.

	LogInfo("CleanupServer: Obtaining a lock on the client list mutex...");

	LockMutex(g_hClientListMutex);
	{
		LogInfo("CleanupServer: Lock established.");

		LogInfo(
				"CleanupServer: Checking whether the count of connected clients is greater than zero...");

		if (g_nClientCount > 0) {
			LogInfo(
					"CleanupServer: The count of connected clients is greater than zero.");

			LogInfo("CleanupServer: Forcibly disconnecting each client...");

			ForEach(&g_pClientList, DisconnectClient);

			LogInfo("CleanupServer: Disconnection operation completed.");
		} else {
			LogInfo(
					"CleanupServer: Zero clients are currently connected.  Nothing to do.");
		}

		LogInfo("CleanupServer: Releasing the client list lock...");
	}
	UnlockMutex(g_hClientListMutex);

	LogInfo("CleanupServer: Client list lock released.");

	LogInfo("CleanupServer: Calling QuitServer...");

	QuitServer();

	LogInfo("CleanupServer: Finished calling QuitServer.");

	LogInfo("CleanupServer: Closing the log file...");

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

///////////////////////////////////////////////////////////////////////////////
// InstallSigintHandler function - Registers a function to be called when the
// user presses CTRL+C, in order to perform an orderly shutdown

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

///////////////////////////////////////////////////////////////////////////////
// QuitServer function - Called to conduct an orderly shutdown of the server,
// stopping all threads and releasing operating system resources in an orderly
// fashion.

void QuitServer() {
	LogDebug("In QuitServer");

	fprintf(stdout, "server: Shutting down...\n");

	LogInfo(
			"QuitServer: Attempting to kill the Master Acceptor Thread (MAT)...");

	KillThread(g_hMasterThread);

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

	DestroyList(&g_pClientList, FreeClient);

	LogInfo("QuitServer: Client list resources freed.");

	LogInfo(
			"QuitServer: Releasing resources consumed by the client list mutex...");

	DestroyClientListMutex();

	LogInfo("QuitServer: Client list mutex resources freed.");

	LogDebug("QuitServer: Done.");
}

///////////////////////////////////////////////////////////////////////////////
// ServerCleanupHandler function - Called when the user presses CTRL+C.  This
// function initiates an orderly shut down of the server application.

void ServerCleanupHandler(int signum) {
	LogDebug("In ServerCleanupHandler");

	LogInfo("ServerCleanupHandler: Since we're here, user has pressed CTRL+C.");

	printf("\n");

	LogInfo("ServerCleanupHandler: Calling CleanupServer with OK exit code...");

	CleanupServer(OK);

	LogInfo("ServerCleanupHandler: CleanupServer called.");

	LogDebug("ServerCleanupHandler: Done.");
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
