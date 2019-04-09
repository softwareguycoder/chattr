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

int client_socket = -1;		  // Client socket for connecting to the server.

// Max chars that can be input at a prompt
#define INPUT_SIZE			255

// Prompting the user for their chat nickname
#define NICKNAME_PROMPT		"> Please type a nickname (15 chars max): > "

// Mode for opening the log file (appending)
#define LOG_FILE_OPEN_MODE	"a+"

// Path to the log file
#define LOG_FILE_PATH	"/home/bhart/logs/chattr/client.log"

void CleanupClient(int exitCode) {
	log_debug("In CleanupClient");

	log_debug("CleanupClient: Freeing resources for the client socket mutex...");

	FreeSocketMutex();

	log_debug("CleanupClient: Client socket mutex freed.");

	log_debug("CleanupClient: Attempting to close the client socket...");

	CloseSocket(client_socket);

	log_debug("CleanupClient: Client socket closed.");

	log_debug("CleanupClient: Closing the log file handles and exiting with exit code %d.", exitCode);

	close_log_file_handles();

	exit(exitCode);
}

void GetNickname(char* nickname, int size) {
	log_debug("In GetNickname");

	log_info(
			"GetNickname: Checking whether a valid address was supplied for the 'nickname' parameter...");

	if (nickname == NULL) {
		log_error(
				"GetNickname: NULL value supplied for the nickname value. Stopping.");

		log_debug("GetNickname: Done.");

		exit(ERROR);
	}

	log_info("GetNickname: The nickname parameter has a valid memory address.");

	log_info("GetNickname: Checking whether size is a positive value...");

	if (size <= 0) {
		log_error("GetNickname: size is a non-positive value.  Stopping.");

		log_debug("GetNickname: Done.");

		exit(ERROR);
	}

	log_info("GetNickname: size is a positive value.");

	log_info("GetNickname: Prompting the user for the user's chat nickname...");

	if (OK != get_line(NICKNAME_PROMPT, nickname, size)) {
		log_error("GetNickname: Failed to get user nickname.");

		log_debug("GetNickname: Done.");

		exit(ERROR);
	}

	log_debug("GetNickname: result = '%s'", nickname);

	log_debug("GetNickname: Done.");

	return;
}

void GreetServer(){
	log_debug("In GreetServer");

	log_info("GreetServer: Per protocol, sending HELO command...");

	if (0 >= Send(client_socket, "HELO\n")) {
		log_error("GreetServer: Error sending data.  Stopping.");

		log_debug("GreetServer: Done.");

		CleanupClient(ERROR);
	}

	log_info("GreetServer: HELO command sent successfully.");

	log_debug("GreetServer: Done.");
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

	log_info(
			"InitializeApplication: Allocating resources for the socket mutex...");

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

void LeaveChatRoom() {
	if (0 >= Send(client_socket, "QUIT\n")) {
		CleanupClient(ERROR);
	}

	/* mock up a receive operation on the socket by
	 * just sleeping */
	sleep(1);
}

int ParsePortNumber(const char* pszPort) {
	log_debug("In ParsePortNumber");

	log_info(
			"ParsePortNumber: Checking whether the pszPort parameter has a value...");

	log_debug("ParsePortNumber: pszPort = '%s'", pszPort);

	if (pszPort == NULL || pszPort[0] == '\0' || strlen(pszPort) == 0) {
		log_error(
				"ParsePortNumber: The pszPort parameter is required to have a value.");

		if (get_error_log_file_handle() != stderr) {
			fprintf(stderr,
					"chattr: Failed to determine what port number you want to use.\n");
		}

		log_debug("ParsePortNumber: Done.");

		close_log_file_handles();

		exit(ERROR);
	}

	log_info("ParsePortNumber: The pszPort parameter has a value.");

	log_info(
			"ParsePortNumber: Attempting to parse the pszPort parameter's value into a number...");

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

void SetNickname(const char* nickname) {
	char szNicknameCommand[512];

	sprintf(szNicknameCommand, "NICK %s\n", nickname);

	if (0 >= Send(client_socket, szNicknameCommand)) {
		CleanupClient(ERROR);
	}
}

void HandshakeWithServer() {
	PrintClientUsageDirections();

	GreetServer();

	char szNickname[255];

	GetNickname(szNickname, 255);

	SetNickname(szNickname);
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

		CleanupClient(ERROR);
	}

	log_info(
			"chattr: Successfully ascertained that a valid number of arguments has been passed.");

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

		CleanupClient(ERROR);
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
		log_error("chattr: Failed to connect to server '%s' on port %d.",
				hostnameOrIp, port);

		if (stdout != get_log_file_handle()) {
			fprintf(stdout,
					"chattr: Failed to connect to server '%s' on port %d.",
					hostnameOrIp, port);
		}

		CleanupClient(ERROR);
	}

	log_info("chattr: Now connected to server '%s' on port %d.", hostnameOrIp,
			port);

	if (get_log_file_handle() != stdout) {
		fprintf(stdout,
				"chattr: Now connected to the chat server '%s' on port %d.\n",
				hostnameOrIp, port);
	}

	HandshakeWithServer();

	// TODO: Create threads here for sending and receiving

	prompt_for_key_press();

	// log off of the chat server
	LeaveChatRoom();

	if (get_log_file_handle() != stdout) {
		fprintf(stdout, "chattr: Done chatting!\n");
	}

	CleanupClient(OK);
}
