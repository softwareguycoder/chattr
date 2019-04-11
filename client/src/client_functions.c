/*
 * client_functions.c
 *
 *  Created on: Apr 11, 2019
 *      Author: bhart
 */

#include "stdafx.h"
#include "client.h"
#include "client_functions.h"

#include "client_manager.h"
#include "receive_thread.h"
#include "send_thread.h"

void CleanupClient(int nExitCode) {
	FreeSocketMutex();

	CloseSocket(nClientSocket);

	CloseLogFileHandles();

	exit(nExitCode);
}

// Functionality to handle the case where the user has pressed CTRL+C
// in this process' terminal window
void ClientCleanupHandler(int signum) {
	LogDebug("In ClientCleanupHandler");

	LogInfo("ClientCleanupHandler: Since we're here, user has pressed CTRL+C.");

	printf("\n");

	LogInfo("ClientCleanupHandler: Leaving the chat room...");

	LeaveChatRoom();

	LogInfo("ClientCleanupHandler: Left the chat room.");

	LogInfo("ClientCleanupHandler: Calling CleanupServer with OK exit code...");

	CleanupClient(OK);

	LogInfo("ClientCleanupHandler: CleanupServer called.");

	LogDebug("ClientCleanupHandler: Done.");
}

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

	LogDebug("In InitializeApplication");

	LogInfo("InitializeApplication: Installing a SIGINT handler to cleanup when CTRL+C is pressed...");

	InstallSigintHandler();

	LogInfo("InitializeApplication: SIGINT handler installed.");

	LogInfo(
			"InitializeApplication: Allocating resources for the socket mutex...");

	CreateSocketMutex();

	LogInfo("InitializeApplication: Resources allocated for socket mutex.");

	/*set_log_file(stdout);
	 set_error_log_file(stderr);*/

	LogDebug("InitializeApplication: Done.");

	return TRUE;
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

	sigIntHandler.sa_handler = ClientCleanupHandler;
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

BOOL IsCommandLineArgumentCountValid(int argc) {
	LogDebug("In IsCommandLineArgumentCountValid");

	LogInfo(
			"IsCommandLineArgumentCountValid: Checking the count of command-line arguments...");

	LogDebug("IsCommandLineArgumentCountValid: argc = %d", argc);

	BOOL result = argc >= MIN_NUM_ARGS;

	if (!result) {
		LogInfo(
				"IsCommandLineArgumentCountValid: The count of command-line arguments must be at least %d.",
				MIN_NUM_ARGS);
	} else {
		LogInfo(
				"IsCommandLineArgumentCountValid: The count of command-line arguments is valid.");
	}

	LogDebug("IsCommandLineArgumentCountValid: result = %d", result);

	LogDebug("IsCommandLineArgumentCountValid: Done.");

	return result;
}

int ParsePortNumber(const char* pszPort) {
	LogDebug("In ParsePortNumber");

	LogInfo(
			"ParsePortNumber: Checking whether the pszPort parameter has a value...");

	LogDebug("ParsePortNumber: pszPort = '%s'", pszPort);

	if (pszPort == NULL || pszPort[0] == '\0' || strlen(pszPort) == 0) {
		LogError(
				"ParsePortNumber: The pszPort parameter is required to have a value.");

		if (GetErrorLogFileHandle() != stderr) {
			fprintf(stderr,
					"chattr: Failed to determine what port number you want to use.\n");
		}

		LogDebug("ParsePortNumber: Done.");

		CloseLogFileHandles();

		exit(ERROR);
	}

	LogInfo("ParsePortNumber: The pszPort parameter has a value.");

	LogInfo(
			"ParsePortNumber: Attempting to parse the pszPort parameter's value into a number...");

	long nResult = 0L;

	if (StringToLong(pszPort, (long*) &nResult) < 0) {
		LogError("ParsePortNumber: Could not read port number of server.");

		if (GetErrorLogFileHandle() != stderr) {
			fprintf(stderr,
					"chattr: Failed to determine what port number you want to use.\n");
		}

		LogDebug("ParsePortNumber: Done.");

		CloseLogFileHandles();

		exit(ERROR);
	}

	LogInfo(
			"ParsePortNumber: Successfully obtained a value for the port number.");

	LogDebug("ParsePortNumber: result = %d", (int)nResult);

	LogDebug("ParsePortNumber: Done.");

	//return nResult;
	return (int)nResult;
}
