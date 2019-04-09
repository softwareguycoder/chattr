///////////////////////////////////////////////////////////////////////////////
// client.c - Echo client in C
// This program allows the user to connect to an ECHO server residing on a
// IP address and port as supplied on the command line.  The user interface
// of this program allows the user to type lines of text to be sent to the
// server.
//
// AUTHOR: Brian Hart
// DATE: 21 Sep 2018
//
// Shout out to https://gist.github.com/DnaBoss/a5e1ea07de5ef3525937 for 
// code that provided inspiration
//

#include "stdafx.h"
#include "client.h"

#include "client_manager.h"

// Max chars that can be input at a prompt
#define INPUT_SIZE			255

// Prompting the user for their chat nickname
#define NICKNAME_PROMPT		"> Please type a nickname (15 chars max): > "

// Mode for opening the log file (appending)
#define LOG_FILE_OPEN_MODE	"a+"

// Path to the log file
#define LOG_FILE_PATH	"/home/bhart/logs/chattr/client.log"

void GetNickname(char* nickname, int size) {
	if (nickname == NULL) {
		exit(ERROR);
	}

	if (size <= 0) {
		exit(ERROR);
	}

	if (OK != get_line(NICKNAME_PROMPT, nickname, size)) {
		exit(ERROR);
	}

	return;
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

	set_log_file(fpLogFile);
	set_error_log_file(fpLogFile);

	log_debug("In InitializeApplication");

	log_info("InitializeApplication: Allocating resources for the socket mutex...");

	CreateSocketMutex();

	log_info("InitializeApplication: Resources allocated for socket mutex.");

	/*set_log_file(stdout);
	 set_error_log_file(stderr);*/

	log_debug("InitializeApplication: Done.");

	return TRUE;
}

BOOL IsCommandLineArgumentCountValid(int argc) {
	log_debug("In IsCommandLineArgumentCountValid");

	log_info(
			"IsCommandLineArgumentCountValid: Checking the count of command-line arguments...");

	log_debug("IsCommandLineArgumentCountValid: argc = %d", argc);

	BOOL result = argc >= MIN_NUM_ARGS;

	if (!result) {
		log_info(
				"IsCommandLineArgumentCountValid: The count of command-line arguments must be at least %d.",
				MIN_NUM_ARGS);
	} else {
		log_info(
				"IsCommandLineArgumentCountValid: The count of command-line arguments is valid.");
	}

	log_debug("IsCommandLineArgumentCountValid: result = %d", result);

	log_debug("IsCommandLineArgumentCountValid: Done.");

	return result;
}

int ParsePortNumber(const char* pszPort) {
	log_debug("In ParsePortNumber");

	log_info(
			"ParsePortNumber: Checking whether the pszPort parameter has a value...");

	log_debug("ParsePortNumber: pszPort = '%s'", pszPort);

	if (pszPort == NULL || pszPort[0] == '\0' || strlen(pszPort) == 0) {
		log_error("ParsePortNumber: The pszPort parameter is required to have a value.");

		if (get_error_log_file_handle() != stderr) {
			fprintf(stderr,
					"chattr: Failed to determine what port number you want to use.\n");
		}

		log_debug("ParsePortNumber: Done.");

		close_log_file_handles();

		exit(ERROR);
	}

	log_info("ParsePortNumber: The pszPort parameter has a value.");

	log_info("ParsePortNumber: Attempting to parse the pszPort parameter's value into a number...");

	int result = -1;

	int retcode = char_to_long(pszPort, (long*) &result);

	if (retcode < 0) {
		log_error("ParsePortNumber: Could not read port number of server.");

		if (get_error_log_file_handle() != stderr) {
			fprintf(stderr,
					"chattr: Failed to determine what port number you want to use.\n");
		}

		log_debug("ParsePortNumber: Done.");

		close_log_file_handles();

		exit(ERROR);
	}

	log_info(
			"ParsePortNumber: Successfully obtained a value for the port number.");

	log_debug("ParsePortNumber: result = %d", result);

	log_debug("ParsePortNumber: Done.");

	return result;
}

int main(int argc, char *argv[]) {
	if (!InitializeApplication())
		return -1;

	log_debug("In main");

	printf(SOFTWARE_TITLE);
	printf(COPYRIGHT_MESSAGE);

	log_info("chattr: Checking arguments...");

	log_debug("chattr: argc = %d", argc);

	// Check the arguments.  If there is less than 3 arguments, then 
	// we should print a message to stderr telling the user what to 
	// pass on the command line and then quit
	if (!IsCommandLineArgumentCountValid(argc)) {
		log_error("chattr: Failed to validate arguments.");

		fprintf(stderr, USAGE_STRING);

		log_debug("chattr: Done.");

		close_log_file_handles();

		exit(ERROR);
	}

	log_info(
			"chattr: Successfully ascertained that a valid number of arguments has been passed.");

	int client_socket = -1;		  // Client socket for connecting to the server.

	log_debug("chattr: argv[1] = '%s'", argv[1]);

	log_debug("chattr: argv[2] = '%s'", argv[2]);

	const char* hostnameOrIp = argv[1]; // address or host name of the remote server

	int port = ParsePortNumber(argv[2]);

	log_debug("chattr: port = %d", port);

	log_info("chattr: Attempting to allocate new connection endpoint...");

	client_socket = CreateSocket();

	if (!IsSocketValid(client_socket)) {
		log_error(
				"chattr: Could not create endpoint for connecting to the server.");

		close_log_file_handles();

		FreeSocketMutex();

		exit(ERROR);
	}

	log_info("chattr: Created new TCP connection endpoint successfully.");

	log_info("chattr: Configured to connect to server at address '%s'.",
			hostnameOrIp);

	log_info("chattr: Configured to connect to server listening on port %d.",
			port);

	log_info("chattr: Now attempting to connect to the server...");

	// Attempt to connect to the server.  The function below is guaranteed to close the socket
	// and forcibly terminate this program in the event of a network error, so we do not need
	// to check the result.
	if (OK != ConnectSocket(client_socket, hostnameOrIp, port)) {
		log_error("chattr: Failed to connect to server '%s' on port %d.", hostnameOrIp, port);

		if (stdout != get_log_file_handle()) {
			fprintf(stdout, "chattr: Failed to connect to server '%s' on port %d.", hostnameOrIp, port);
		}

		log_debug("chattr: Now attempting to release resources for the socket mutex...");

		FreeSocketMutex();

		log_debug("chattr: Resources for socket mutex have been freed.");

		log_debug("chattr: Done.");

		close_log_file_handles();

		exit(ERROR);
	}

	log_info("chattr: Now connected to server '%s' on port %d.", hostnameOrIp,
			port);

	if (get_log_file_handle() != stdout) {
		fprintf(stdout,
				"chattr: Now connected to the chat server '%s' on port %d.\n",
				hostnameOrIp, port);
	}

	PrintClientUsageDirections();

	prompt_for_key_press();

	// log off of the chat server
	Send(client_socket, "QUIT\n");

	// TODO: Create threads here for sending and receiving

	if (get_log_file_handle() != stdout) {
			fprintf(stdout, "chattr: Done chatting!\n");
		}

	log_debug("chattr: Now attempting to release resources for the socket mutex...");

	FreeSocketMutex();

	log_debug("chattr: Resources for socket mutex have been freed.");

	log_debug("chattr: Closing the client socket...");

	CloseSocket(client_socket);

	log_debug("chattr: Client socket closed.");

	close_log_file_handles();

	return OK;
}
