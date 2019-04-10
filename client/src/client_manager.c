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
#include "client.h"

#include "client_manager.h"

///////////////////////////////////////////////////////////////////////////////
// GetNickname function: Prompts the user for a nickname, and places the value
// typed into the buffer pointed to by the nickname parameter.
//

void GetNickname(char* pszNickname, int nSize) {
	log_debug("In GetNickname");

	log_info(
			"GetNickname: Checking whether a valid address was supplied for the 'pszNickname' parameter...");

	if (pszNickname == NULL) {
		log_error(
				"GetNickname: NULL value supplied for the pszNickname value. Stopping.");

		log_debug("GetNickname: Done.");

		exit(ERROR);
	}

	log_info("GetNickname: The nickname parameter has a valid memory address.");

	log_info("GetNickname: Checking whether size is a positive value...");

	if (nSize < MIN_SIZE) {
		log_error("GetNickname: size is a non-positive value.  Stopping.");

		log_debug("GetNickname: Done.");

		exit(ERROR);
	}

	log_info("GetNickname: size is a positive value.");

	log_info("GetNickname: Prompting the user for the user's chat nickname...");

	if (OK != get_line(NICKNAME_PROMPT, pszNickname, nSize)) {
		log_error("GetNickname: Failed to get user nickname.");

		log_debug("GetNickname: Done.");

		exit(ERROR);
	}

	log_debug("GetNickname: result = '%s'", pszNickname);

	log_debug("GetNickname: Done.");

	return;
}

///////////////////////////////////////////////////////////////////////////////
// GreetServer function: carries out the first step of the chat protocol, which
// consists of isusing the HELO command to the server
//

void GreetServer() {
	log_debug("In GreetServer");

	log_info("GreetServer: Greeting the server...");

	if (0 >= Send(nClientSocket, PROTOCOL_HELO_COMMAND)) {
		log_error("GreetServer: Error sending data.  Stopping.");

		log_debug("GreetServer: Done.");

		CleanupClient(ERROR);
	}

	log_info("GreetServer: Server greeted successfully.");

	log_debug("GreetServer: Done.");
}

///////////////////////////////////////////////////////////////////////////////
// HandshakeWithServer function: Carries out the handshaking with the chat
// server, per protocol.  This consists of (1) sending the HELO command and
// getting an "OK" response; (2) if that succeeds, then prompts the user
// to type a nickname or 'chat handle' which is then sent to the server using
// the NICK command of the protocol.  For more details, see protocol_spec.txt
//

void HandshakeWithServer() {
	PrintClientUsageDirections();

	GreetServer();

	char szNickname[255];

	GetNickname(szNickname, 255);

	SetNickname(szNickname);
}

///////////////////////////////////////////////////////////////////////////////
// LeaveChatRoom function

void LeaveChatRoom() {
	if (0 >= Send(nClientSocket, PROTOCOL_QUIT_COMMAND)) {
		CleanupClient(ERROR);
	}

	/* mock up a receive operation on the socket by
	 * just sleeping */
	sleep(1);
}

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

void ReceiveFromServer(char* pszReplyBuffer) {
	log_debug("In ReceiveFromServer");

	log_info(
			"ReceiveFromServer: Checking whether the nClientSocket value passed refers to a valid socket...");

	log_debug("ReceiveFromServer: nClientSocket = %d", nClientSocket);

	if (!IsSocketValid(nClientSocket)) {
		fprintf(stderr,
				"chattr: Failed to receive the line of text back from the server.");

		log_error(
				"ReceiveFromServer: The nClientSocket value passed is not a valid socket file descriptor.");

		log_debug(
				"ReceiveFromServer: Releasing the memory of the socket mutex...");

		FreeSocketMutex();

		log_debug("ReceiveFromServer: Memory consumed by socket mutex freed.");

		log_debug("ReceiveFromServer: Done.");

		exit(ERROR);
	}

	/* Wipe away any existing reply buffer */

	if (pszReplyBuffer != NULL) {
		free_buffer((void**) &pszReplyBuffer);
	}

	/* Do a receive. Cleanup if the operation was not successful. */

	if (0 > Receive(nClientSocket, &pszReplyBuffer)
			&& errno != EBADF && errno != EWOULDBLOCK) {
		free_buffer((void**) &pszReplyBuffer);

		fprintf(stderr,
				"chattr: Failed to receive the line of text back from the server.");

		log_debug(
				"ReceiveFromServer: Releasing the memory of the socket mutex...");

		FreeSocketMutex();

		log_debug("ReceiveFromServer: Memory consumed by socket mutex freed.");

		log_debug("ReceiveFromServer: Done.");

		exit(ERROR);
	} else {
		// Print the line received from the server to the console with a
		// 'S: ' prefix in front of it.  We assume that the pszReplyBuffer
		// contains the newline character.  Free the memory allocated for
		// the server reply.  Do not use the log_info routine here since we
		// want a more protocol-formmatted message to appear on screen.
		fprintf(stdout, "S: %s", pszReplyBuffer);

		free_buffer((void**) &pszReplyBuffer);
	}
}

///////////////////////////////////////////////////////////////////////////////
// SetNickname function: Sets the user's chat handle or nickname to the desired
// value

void SetNickname(const char* pszNickname) {
	log_debug("In SetNickname");

	// TODO: Add logging to SetNickname

	char szNicknameCommand[512];

	sprintf(szNicknameCommand, PROTOCOL_NICK_COMMAND, pszNickname);

	if (0 >= Send(nClientSocket, szNicknameCommand)) {
		CleanupClient(ERROR);
	}
}
