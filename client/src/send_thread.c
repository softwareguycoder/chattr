// send_thread.c - Contains the implementation of the client's sending thread
//

#include "stdafx.h"
#include "client.h"

#include "send_thread_functions.h"
#include "send_thread.h"

///////////////////////////////////////////////////////////////////////////////
// SendThread function

void *SendThread(void *pvData) {
    SetThreadCancelState(PTHREAD_CANCEL_ENABLE);
    SetThreadCancelType(PTHREAD_CANCEL_DEFERRED);

    // Double check to ensure we have a valid socket file descriptor for
    // communications.  If not, then stop.
    if (!IsSocketValid(g_nClientSocket)) {
        return NULL;
    }

    if (IsNullOrWhiteSpace(g_szNickname)) {
        return NULL;
    }

    // Poll stdin for user input.  Once we get some, send it off to the server
    // and then poll again.  Keep going until the ShouldStopSending() function
    // tells us to stop.

    // Buffer for the current line inputted by the user
    char szCurLine[MAX_LINE_LENGTH + 1];
    memset(szCurLine, 0, MAX_LINE_LENGTH + 1);

    char szPrompt[MAX_NICKNAME_LEN + 4];
    memset(szPrompt, 0, MAX_NICKNAME_LEN + 4);

    sprintf(szPrompt, CHAT_PROMPT_FORMAT, g_szNickname);

    // Continuously run a GetLineFromUser.  Since the GetLineFromUser call
    // blocks the calling thread and not the entire program, we can call
    // GetLineFromUser here and lines sent from the chat server's other clients
    // will still be received by the other thread we have spun up for
    // receiving text and written to stdout even whilst GetLineFromUser is
    // blocking.
    while (0 <= GetLineFromUser("", szCurLine,
        MAX_LINE_LENGTH - 1)) {

        // Get everything off the stdin
        FlushStdin();

        if (g_bShouldTerminateSendThread) {
            g_bShouldTerminateSendThread = FALSE;
            break;
        }

        if (IsNullOrWhiteSpace(szCurLine)) {
            continue;		// skip instances where the user just presses ENTER
                            // or just types spaces
        }

        // NOTE: The GetLineFromUser function does not preserve the newline
        // from user input!  so we need to add it back in, since the server
        // protocol specifies that all chat messages need to terminate with
        // a newline character ( ASCII 10 ).

        sprintf(szCurLine, "%s\n", szCurLine);

        if (strcasecmp(szCurLine, "quit\n") == 0) {
            if (!IsUppercase(szCurLine)) {
                // blank out the current line
                memset(szCurLine, 0, MAX_LINE_LENGTH + 1);

                fprintf(stderr, "ERROR: To finish your chat session, please "
                        "type QUIT in all upper-case.\n");
                continue;
            }
        }

        // If we are here, then there is something to be sent.  Go ahead and
        // send it to the socket.  Just skip the current input if an error
        // occurs.
        if (0 > Send(g_nClientSocket, szCurLine)) {
            // If we are here, then an error occurred with sending.
            continue;
        }

        // If we are here, then the send operation occurred successfully.
        // Log the communications with the server in the log file
        if (GetLogFileHandle() != stdout) {
            LogInfo(CLIENT_DATA_FORMAT, szCurLine);
        }

        sleep(1);	// Force a context switch to allow the receive thread to
                    // detect any server replies

        // Ask if we should keep sending, or whether it's time to
        // stop waiting for input.
        if (!ShouldKeepSending(szCurLine)) {
            break;
        }

        if (g_bShouldTerminateSendThread) {
            g_bShouldTerminateSendThread = FALSE;
            break;
        }
    }

    if (GetLogFileHandle() != stdout) {
        LogInfo("Send thread shutting down.");
    }

    // Done with send thread
    return NULL;
}
