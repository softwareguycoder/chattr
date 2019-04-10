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

#include "client_manager.h"

// Client socket for connecting to the server.
// This was turned into a file-scope global so
// that all the functions in this module can
// access it.
int nClientSocket = -1;

void CleanupClient(int nExitCode) {
	LogDebug("In CleanupClient");

	LogDebug("CleanupClient: nExitCode = %d", nExitCode);

	LogDebug(
			"CleanupClient: Freeing resources for the client socket mutex...");

	FreeSocketMutex();

	LogDebug("CleanupClient: Client socket mutex freed.");

	LogDebug("CleanupClient: Attempting to close the client socket...");

	CloseSocket(nClientSocket);

	LogDebug("CleanupClient: Client socket closed.");

	LogDebug(
			"CleanupClient: Closing the log file handles and exiting with exit code %d.",
			nExitCode);

	CloseLogFileHandles();

	exit(nExitCode);
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

	LogInfo(
			"InitializeApplication: Allocating resources for the socket mutex...");

	CreateSocketMutex();

	LogInfo("InitializeApplication: Resources allocated for socket mutex.");

	/*set_log_file(stdout);
	 set_error_log_file(stderr);*/

	LogDebug("InitializeApplication: Done.");

	return TRUE;
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

	int nResult = -1;

	int nReturnCode = char_to_long(pszPort, (long*) &nResult);

	if (nReturnCode < 0) {
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

	LogDebug("ParsePortNumber: result = %d", nResult);

	LogDebug("ParsePortNumber: Done.");

	return nResult;
}

int main(int argc, char *argv[]) {
	if (!InitializeApplication())
		return -1;

	LogDebug("In main");

	printf(SOFTWARE_TITLE);
	printf(COPYRIGHT_MESSAGE);

	LogInfo("chattr: Checking arguments...");

	LogDebug("chattr: argc = %d", argc);

	// Check the arguments.  If there is less than 3 arguments, then 
	// we should print a message to stderr telling the user what to 
	// pass on the command line and then quit
	if (!IsCommandLineArgumentCountValid(argc)) {
		LogError("chattr: Failed to validate arguments.");

		fprintf(stderr, USAGE_STRING);

		CleanupClient(ERROR);
	}

	LogInfo(
			"chattr: Successfully ascertained that a valid number of arguments has been passed.");

	LogDebug("chattr: argv[1] = '%s'", argv[1]);

	LogDebug("chattr: argv[2] = '%s'", argv[2]);

	const char* pszHostNameOrIP = argv[1]; // address or host name of the remote server

	int nPort = ParsePortNumber(argv[2]);

	LogDebug("chattr: port = %d", nPort);

	LogInfo("chattr: Attempting to allocate new connection endpoint...");

	nClientSocket = CreateSocket();

	if (!IsSocketValid(nClientSocket)) {
		LogError(
				"chattr: Could not create endpoint for connecting to the server.");

		CleanupClient(ERROR);
	}

	LogInfo("chattr: Created new TCP connection endpoint successfully.");

	LogInfo("chattr: Configured to connect to server at address '%s'.",
			pszHostNameOrIP);

	LogInfo("chattr: Configured to connect to server listening on port %d.",
			nPort);

	LogInfo("chattr: Now attempting to connect to the server...");

	// Attempt to connect to the server.  The function below is guaranteed to close the socket
	// and forcibly terminate this program in the event of a network error, so we do not need
	// to check the result.
	if (OK != ConnectSocket(nClientSocket, pszHostNameOrIP, nPort)) {
		LogError("chattr: Failed to connect to server '%s' on port %d.",
				pszHostNameOrIP, nPort);

		if (stdout != GetLogFileHandle()) {
			fprintf(stdout,
					"chattr: Failed to connect to server '%s' on port %d.",
					pszHostNameOrIP, nPort);
		}

		CleanupClient(ERROR);
	}

	LogInfo("chattr: Now connected to server '%s' on port %d.", pszHostNameOrIP,
			nPort);

	if (GetLogFileHandle() != stdout) {
		fprintf(stdout,
				"chattr: Now connected to the chat server '%s' on port %d.\n",
				pszHostNameOrIP, nPort);
	}

	HandshakeWithServer();

	// TODO: Create threads here for sending and receiving

	PromptForKeyPress();

	// log off of the chat server
	LeaveChatRoom();

	if (GetLogFileHandle() != stdout) {
		fprintf(stdout, "chattr: Done chatting!\n");
	}

	CleanupClient(OK);
}
