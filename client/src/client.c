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

// Mode for opening the log file (appending)
#define LOG_FILE_OPEN_MODE	"a+"

// Path to the log file
#define LOG_FILE_PATH	"/home/bhart/logs/chattr/client.log"

/**
 * @brief Runs code that is meant to only be run once on startup.
 * @return TRUE if successful, FALSE if an error occurred.
 * @remarks The application should be terminated immediately if this
 * function returns FALSE.
 */
int initialize_application() {
	remove(LOG_FILE_PATH);
	set_log_file(fopen(LOG_FILE_PATH, LOG_FILE_OPEN_MODE));
	set_error_log_file(get_log_file_handle());

	/*set_log_file(stdout);
	set_error_log_file(stderr);*/

	return TRUE;
}

int main(int argc, char *argv[]) {
	if (!initialize_application())
		return -1;

	printf(SOFTWARE_TITLE);
	printf(COPYRIGHT_MESSAGE);

	log_info("client: Checking arguments...");

	// Check the arguments.  If there is less than 3 arguments, then 
	// we should print a message to stderr telling the user what to 
	// pass on the command line and then quit
	if (argc < MIN_NUM_ARGS) {
		fprintf(stderr, USAGE_STRING);

		close_log_file();

		exit(ERROR);
	}

	int client_socket = 0;        // Client socket for connecting to the server.
	char cur_line[MAX_LINE_LENGTH + 1]; // Buffer for the current line inputted by the user

	const char* hostnameOrIp = argv[1]; // address or host name of the remote server

	int port = 0;
	int retcode = char_to_long(argv[2], (long*) &port); // port number that server is listening on
	if (retcode < 0) {
		log_error("client: Could not read port number of server.");

		close_log_file();

		exit(ERROR);
	}

	log_info("client: Configured to connect to server at address '%s'.",
			hostnameOrIp);
	log_info("client: Configured to connect to server listening on port %d.",
			port);
	log_info("client: Attempting to allocate new connection endpoint...");

	client_socket = SocketDemoUtils_createTcpSocket();
	if (client_socket <= 0) {
		log_error(
				"client: Could not create endpoint for connecting to the server.");

		close_log_file();

		FreeSocketMutex();

		exit(ERROR);
	}

	log_info("client: Created connection endpoint successfully.");

	// Attempt to connect to the server.  The function below is guaranteed to close the socket
	// and forcibly terminate this program in the event of a network error, so we do not need
	// to check the result.
	SocketDemoUtils_connect(client_socket, hostnameOrIp, port);

	/* Print some usage directions */
	fprintf(stdout,
			"\nType the message to send to the server at the '>' prompt, and then press ENTER.\n");
	fprintf(stdout,
			"The server's reply, if any, will be shown with a 'S:' prefix.\n");
	fprintf(stdout,
			"When you have nothing more to say, type a dot ('.') on a line by itself.\n");
	fprintf(stdout, "To exit, type 'exit' or 'quit' and then press ENTER.\n");

	/* Show a '>' prompt to the user.  If the user just presses ENTER at a
	 prompt, then just give the user a new prompt.  If the user enters the
	 words 'exit' or 'quit' at the '>' prompt, then exit this program.  Otherwise,
	 send whatever string(s) the user types at the '>' prompt to the server,
	 and then display the server's response, if any.
	 */

	//int total_read = 0;             // total reply bytes read from the server
	int total_entered = 0; // total bytes typed by the user for the current message

	fprintf(stdout, "> ");

	while (NULL != fgets(cur_line, MAX_LINE_LENGTH, stdin)) {
		if (strcasecmp(cur_line, ".\n") == 0
				|| strcasecmp(cur_line, "exit\n") == 0
				|| strcasecmp(cur_line, "quit\n") == 0) {
			SocketDemoUtils_send(client_socket, cur_line);
			break;
		}

		if (strcasecmp(cur_line, "\n") == 0) {
			fprintf(stdout, "> ");
			continue;
		}

		// Keep a running total of the total bytes entered
		total_entered += strlen(cur_line);

		// send the text just now entered by the user to the server
		if (SocketDemoUtils_send(client_socket, cur_line) < 0) {
			error_and_close(client_socket, "client: Failed to send the data.");

			exit(ERROR);
		}

		// If a period '.' has been sent to the server, this is the way the user
		// says they are done using the server, so stop here before trying to receive
		// a reply from the server.
		if (strcasecmp(cur_line, ".\n") == 0)
			break;

		// Now, assume the server has sent a reply, and call the recv() function
		// to attempt to pull the text sent back by the server off of the data
		// stream.  Assume that the server just sends back one line at a time.
		char *reply_buffer = NULL;

		if (0 > SocketDemoUtils_recv(client_socket, &reply_buffer)) {
			free_buffer((void**) &reply_buffer);
			error_and_close(client_socket,
					"client: Failed to receive the line of text back from the server.");
			FreeSocketMutex();
			exit(ERROR);
		} else {
			// Print the line received from the server to the console with a
			// 'S: ' prefix in front of it.  We assume that the reply_buffer
			// contains the newline character.  Free the memory allocated for
			// the server reply.  Do not use the log_info routine here since we
			// want a more protocol-formmatted message to appear on screen.
			fprintf(stdout, "S: %s", reply_buffer);

			free_buffer((void**) &reply_buffer);
		}

		fprintf(stdout, "> ");
	}

	SocketDemoUtils_close(client_socket);

	fprintf(stdout, "S: <disconnected>\n");

	log_info("client: Exited normally with error code %d.", OK);

	close_log_file();

	return OK;
}
