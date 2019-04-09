///////////////////////////////////////////////////////////////////////////////
// client_manager.c - Main client functionality manager functions
//
// This file contains all the routines that are centeral to the user interface
// and operation of the chat client program.
//
// AUTHOR: Brian Hart
// DATE: 8 Apr 2019
//
// Shout out to https://gist.github.com/DnaBoss/a5e1ea07de5ef3525937 for
// code that provided inspiration
//

#include "stdafx.h"
#include "client_manager.h"

#define USAGE_MESSAGE	"\nNow connected to the chat server.  Lines will appear on the screen\n" \
						"when other chatters type messages or if the host has administrative messages\n" \
						"for everyone in the chat room.  You can use @ mentions and #hashtags just like\n" \
						"on other popular services.\n\nThe first step is to tell us the nickname,\n" \
						"or chat handle, you want to use.  To leave the chat room, type QUIT in all-caps\n" \
						"on a line by itself.  Thanks for using chattr!"

///////////////////////////////////////////////////////////////////////////////
// PrintClientUsageDirections function

void PrintClientUsageDirections() {
	log_debug("In PrintClientUsageDirections");

	log_info(
			"PrintClientUsageDirections: Printing the usage directions for the user...");

	/* Print some usage directions */
	fprintf(stdout, USAGE_MESSAGE);

	log_info("PrintClientUsageDirections: Usage directions printed.");

	log_debug("PrintClientUsageDirections: Done.");
}

///////////////////////////////////////////////////////////////////////////////
// ReceiveFromServer function

void ReceiveFromServer(int client_socket, char* reply_buffer) {
	log_debug("In ReceiveFromServer");

	log_info(
			"ReceiveFromServer: Checking whether the client_socket value passed refers to a valid socket...");

	log_debug("ReceiveFromServer: client_socket = %d", client_socket);

	if (!IsSocketValid(client_socket)) {
		fprintf(stderr,
				"chattr: Failed to receive the line of text back from the server.");

		log_error(
				"ReceiveFromServer: The client_socket value passed is not a valid socket file descriptor.");

		log_debug(
				"ReceiveFromServer: Releasing the memory of the socket mutex...");

		FreeSocketMutex();

		log_debug("ReceiveFromServer: Memory consumed by socket mutex freed.");

		log_debug("ReceiveFromServer: Done.");

		exit(ERROR);
	}

	/* Wipe away any existing reply buffer */

	if (reply_buffer != NULL) {
		free_buffer((void**) &reply_buffer);
	}

	/* Do a receive. Cleanup if the operation was not successful. */

	if (0 > Receive(client_socket, &reply_buffer)) {
		free_buffer((void**) &reply_buffer);

		fprintf(stderr,
				"chattr: Failed to receive the line of text back from the server.");

		log_error(
				"ReceiveFromServer: The client_socket value passed is not a valid socket file descriptor.");

		log_debug(
				"ReceiveFromServer: Releasing the memory of the socket mutex...");

		FreeSocketMutex();

		log_debug("ReceiveFromServer: Memory consumed by socket mutex freed.");

		log_debug("ReceiveFromServer: Done.");

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
}
