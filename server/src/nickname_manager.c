// nickname_manager.c - Implementations of a group of functions that, collect-
// ively, define an interface for dealing with chat nickname(s) in use by
// this server's various clients.

#include "stdafx.h"
#include "server.h"

#include "client_list_manager.h"
#include "client_manager.h"
#include "client_struct.h"
#include "server_functions.h"
#include "nickname_manager.h"

///////////////////////////////////////////////////////////////////////////////
// Internally-used functions

///////////////////////////////////////////////////////////////////////////////
// Externally-exposed functions

///////////////////////////////////////////////////////////////////////////////
// GetNicknameFromClient function

void GetNicknameFromUser(char* dest, char* src) {
    if (IsNullOrWhiteSpace(src)) {
        ThrowNullReferenceException();
    }

    if (dest == NULL) {
        ThrowNullReferenceException();
    }

    const int BUFFER_SIZE = strlen(src);

    char szNickname[BUFFER_SIZE + 1];
    memset(szNickname, 0, BUFFER_SIZE);

    char *pszBuffer = strtok(src, " ");
    if (pszBuffer != NULL) {
        pszBuffer = strtok(NULL, " ");
    }

    if (!IsNullOrWhiteSpace(pszBuffer)) {
        /* we expect that the buffer now contains a nickname
         * such as brian\n and we expect the contents to end
         * in a newline.  Since we parsed on whitespace as a
         * separator, make sure to do a Trim() on the string
         * so as to remove any other remaining whitespace
         * characters. */

        Trim(szNickname, BUFFER_SIZE + 1, pszBuffer);

        strcpy(dest, szNickname);
    }
}

///////////////////////////////////////////////////////////////////////////////
// RegisterClientNickname function

BOOL RegisterClientNickname(LPCLIENTSTRUCT lpSendingClient, char* pszBuffer) {
    if (lpSendingClient == NULL) {
        ThrowNullReferenceException();
    }

    if (IsNullOrWhiteSpace(pszBuffer)) {
        ThrowNullReferenceException();
    }

    if (!IsNullOrWhiteSpace(lpSendingClient->pszNickname)) {
    	// Nickname has already been registered
    	lpSendingClient->nBytesSent	+=
    			ReplyToClient(lpSendingClient, NICK_ALREADY_REGISTERED);
    	return TRUE;	// message handled
    }

    /* buffer to store the nickname parsed from the string */
    char szNickname[MAX_NICKNAME_LEN + 1];
    memset(szNickname, 0, MAX_NICKNAME_LEN + 1);

    /* per protocol, the NICK command establishes the user's chat nickname */
    char szReplyBuffer[MAX_LINE_LENGTH + 1];
    memset(szReplyBuffer, 0, MAX_LINE_LENGTH + 1);

    // let's parse this command with lpClientStructstrtok.
    // Protocol spec says this command is NICK <chat-nickname>\n with no spaces
    // allowed in the <chat-nickname>.  The nickname can only be 15 or less
    // characters long.  Nicknames can only be alphanumeric.
    GetNicknameFromUser(szNickname, pszBuffer);

    if (IsNullOrWhiteSpace(szNickname)) {
        // Tell the client they are wrong for sending a blank
        // value for the nickname
    	lpSendingClient->nBytesSent +=
    			ReplyToClient(lpSendingClient, ERROR_NO_NICK_RECEIVED);
        return TRUE;   // command handled but error occurred
    }

    const int NICKNAME_LENGTH = strlen(szNickname);

    if (NICKNAME_LENGTH > MAX_NICKNAME_LEN) {
    	lpSendingClient->nBytesSent +=
    			ReplyToClient(lpSendingClient, ERROR_NICK_TOO_LONG);
        return TRUE;   // command handled but error occurred
    }

    // Check to ensure the requested nickname isn't already taken
    if (NULL !=
            FindElement(g_pClientList, szNickname, FindClientByNickname)) {
    	lpSendingClient->nBytesSent +=
    			ReplyToClient(lpSendingClient, ERROR_NICKNAME_IN_USE);
        return TRUE; // command handled but error occurred
    }

    // Allocate a buffer to hold the nickname and make sure to leave
    // room for the null terminator
    lpSendingClient->pszNickname = (char*) malloc(
            (NICKNAME_LENGTH + 1)* sizeof(char));
    memset(lpSendingClient->pszNickname, 0,
            (NICKNAME_LENGTH + 1)*sizeof(char));

    // Copy the contents of the buffer referenced by pszNickname to that
    // referenced by lpClientStruct->pszNickname
    strcpy(lpSendingClient->pszNickname, szNickname);

    // Now send the user a reply telling them OK your nickname is <bla>
    sprintf(szReplyBuffer, OK_NICK_REGISTERED,
            lpSendingClient->pszNickname);

    lpSendingClient->nBytesSent +=
    		ReplyToClient(lpSendingClient, szReplyBuffer);

    /* Now, tell everyone (except the new guy)
     * that a new chatter has joined! Yay!! */

    sprintf(szReplyBuffer, NEW_CHATTER_JOINED, lpSendingClient->pszNickname);

    /** Tell ALL connected clients (except the one that just
     * joined) that there's a new connected client. */
    BroadcastToAllClientsExceptSender(szReplyBuffer, lpSendingClient);

    return TRUE;    // command handled successfully
}

///////////////////////////////////////////////////////////////////////////////
