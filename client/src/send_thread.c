/*
 * send_thread.c
 *
 *  Created on: Apr 8, 2019
 *      Author: bhart
 */

#include "stdafx.h"
#include "client.h"

#include "send_thread.h"

HTHREAD hSendThread;

#define PROMPT			"> "

#define HELO_COMMAND	"HELO\n"
#define NICK_COMMAND	"NICK "
#define QUIT_COMMAND	"QUIT\n"

#define MSG_TERMINATOR	".\n"
#define NEWLINE			"\n"

///////////////////////////////////////////////////////////////////////////////
// ShouldKeepSending function

BOOL ShouldKeepSending(const char* cur_line) {
	if (cur_line == NULL
			|| cur_line[0] == '\0'
			|| strcasecmp(cur_line, QUIT_COMMAND) == 0
			|| strcmp(cur_line, MSG_TERMINATOR) == 0) {
		return FALSE;
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// ShowPrompt function

void ShowPrompt() {
	fprintf(stdout, PROMPT);
}

///////////////////////////////////////////////////////////////////////////////
// SendThread function

void *SendThread(void *pData) {
	if (pData == NULL){
		return NULL;
	}

	int *pClientSockFd = (int*)pData;
	int client_socket = *pClientSockFd;

	if (!IsSocketValid(client_socket)){
		return NULL;
	}

	char cur_line[MAX_LINE_LENGTH + 1]; // Buffer for the current line inputted by the user

	//ShowPrompt();

	while (NULL != fgets(cur_line, MAX_LINE_LENGTH, stdin)) {
		if (0 > Send(client_socket, cur_line)) {
			break;
		}

		if (!ShouldKeepSending(cur_line))
			break;

		//ShowPrompt();
	}
	return NULL;
}
