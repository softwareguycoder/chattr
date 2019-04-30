// send_functions.c -Functions that are used by the sending thread.
//

#include "stdafx.h"
#include "client.h"

#include "client_functions.h"
#include "send_thread_functions.h"

///////////////////////////////////////////////////////////////////////////////
// g_bShouldTerminateSendThread Global variable

BOOL g_bShouldTerminateSendThread = FALSE;

///////////////////////////////////////////////////////////////////////////////
// g_hSendThread Global variable

HTHREAD g_hSendThread;

///////////////////////////////////////////////////////////////////////////////
// ShouldKeepSending function - Examines the current line that is supposed to
// contain the data that was just sent, and determines if it was, basically,
// the QUIT command that is supposed to terminate communications.
//

BOOL ShouldKeepSending(const char* pszCurLine) {
    if (IsNullOrWhiteSpace(pszCurLine)
            || strcasecmp(pszCurLine, PROTOCOL_QUIT_COMMAND) == 0) {
        /* clear out the current nickname value */
        ClearNickname();

        // The QUIT command has been issued, so we should stop sending.
        return FALSE;
    }

    // If we are here, then we can keep the sending thread alive.
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// TerminateSendThread function

void TerminateSendThread(int signum) {
    // Double-check that the semaphore signal is SIGSEGV; otherwise, ignore
    // it.
    if (SIGSEGV != signum) {
        return;
    }

    // Mark the receive thread terminate flag
    g_bShouldTerminateSendThread = TRUE;

    // Re-register this semaphore
    RegisterEvent(TerminateSendThread);
}
