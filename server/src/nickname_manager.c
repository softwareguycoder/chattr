// nickname_manager.c - Implementations of a group of functions that, collect-
// ively, define an interface for dealing with chat nickname(s) in use by
// this server's various clients.

#include "stdafx.h"
#include "server.h"

#include "server_functions.h"
#include "nickname_manager.h"

///////////////////////////////////////////////////////////////////////////////
// Externally-exposed functions

///////////////////////////////////////////////////////////////////////////////
// GetNicknameFromClient function

void GetNicknameFromClient(char* dest, char* src) {
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
