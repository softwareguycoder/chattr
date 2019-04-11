// client_functions.c - File containing application functions for this program
//

#include "stdafx.h"
#include "client.h"
#include "client_functions.h"

#include "client_manager.h"
#include "receive_thread.h"
#include "send_thread.h"

///////////////////////////////////////////////////////////////////////////////
// CleanupClient function - Releases operating system resources consumed by the
// client and exits the process
//

void CleanupClient(int nExitCode) {
	FreeSocketMutex();

	CloseSocket(nClientSocket);

	CloseLogFileHandles();

	exit(nExitCode);
}

///////////////////////////////////////////////////////////////////////////////
// ClientCleanupHandler function - A callback that provides functionality to
// handle the case where the user has pressed CTRL+C in this process' Terminal
// window in a graceful manner
//

void ClientCleanupHandler(int signum) {
	printf("\n");

	LeaveChatRoom();

	CleanupClient(OK);
}

///////////////////////////////////////////////////////////////////////////////
// InitializeApplication function - Does one-time startup initialization of
// the client
//

/**
 * @brief Runs code that is meant to only be run once on startup.
 * @return TRUE if successful, FALSE if an error occurred.
 * @remarks The application should be terminated immediately if this
 * function returns FALSE.
 */
BOOL InitializeApplication() {
	remove(LOG_FILE_PATH);

	FILE* fpLogFile = fopen(LOG_FILE_PATH, LOG_FILE_OPEN_MODE);
	if (fpLogFile == NULL) {
		fprintf(stderr, "Failed to open log file '%s' for writing.\n",
		LOG_FILE_PATH);
		exit(ERROR); /* Terminate program if we can't open the log file */
	}

	SetLogFileHandle(fpLogFile);
	SetErrorLogFileHandle(fpLogFile);

	InstallSigintHandler();

	CreateSocketMutex();

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// InstallSigintHandler function - Installs a SIGINT handler to handle the case
// where the user presses CTRL+C in this process' Terminal window.  This allows
// us to clean up the main while loop and free operating system resources
//gracefully.
//
// Shout-out to <https://stackoverflow.com/questions/1641182/
// how-can-i-catch-a-ctrl-c-event-c> for this code.
//

void InstallSigintHandler() {
	struct sigaction sigIntHandler;

	sigIntHandler.sa_handler = ClientCleanupHandler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;

	if (OK != sigaction(SIGINT, &sigIntHandler, NULL)) {
		fprintf(stderr, "chattr: Unable to install CTRL+C handler.\n");

		perror("chattr[sigaction]");

		CleanupClient(ERROR);
	}
}

///////////////////////////////////////////////////////////////////////////////
// IsCommandLineArgumentCountValid function - Determines whether the user has
// supplied enough information on the command line in order for this program
// to execute successfully.
//

BOOL IsCommandLineArgumentCountValid(int argc) {
	return argc >= MIN_NUM_ARGS;
}

///////////////////////////////////////////////////////////////////////////////
// ParsePortNumber function - Tries to turn the string containing the port
// number the user typed as a command-line argument into an integer literal for
// usage in connecting to the server.
//

int ParsePortNumber(const char* pszPort) {
	// If the port number is blank, fail.
	if (pszPort == NULL || pszPort[0] == '\0' || strlen(pszPort) == 0) {
		fprintf(stderr, FAIL_PARSE_PORTNUM);

		CleanupClient(ERROR);
	}

	// Try to parse the string containing the port number into an integer
	// value.
	long nResult = 0L;

	if (StringToLong(pszPort, (long*) &nResult) < 0) {
		fprintf(stderr, FAIL_PARSE_PORTNUM);

		CleanupClient(ERROR);
	}

	//return nResult;
	return (int)nResult;
}

///////////////////////////////////////////////////////////////////////////////

