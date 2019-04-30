// client_functions.c - File containing application functions for this program
//

#include "stdafx.h"
#include "client.h"

#include "client_functions.h"

#include "client_manager.h"
#include "receive_thread.h"

#include "send_thread_functions.h"
#include "send_thread.h"

///////////////////////////////////////////////////////////////////////////////
// ConfigureLogFile function - Configures settings for the log file and format
// the filename to match the current system date and time.  Since, in principle
// many clients may be running, we want to have the ability to name their log
// files after something that varies with each client.
//

void ConfigureLogFile() {
    char szLogFileName[MAX_PATH + 1];
    FormatLogFileName(szLogFileName);

    /* Overwrite any existing log file */
    remove(szLogFileName);

    FILE* fpLogFile = fopen(szLogFileName, LOG_FILE_OPEN_MODE);
    if (fpLogFile == NULL) {
        fprintf(stderr, FAILED_OPEN_LOG, szLogFileName);
        CleanupClient(ERROR); /* Terminate program if we can't open the log file */
    }

    SetLogFileHandle(fpLogFile);
    SetErrorLogFileHandle(fpLogFile);
}

///////////////////////////////////////////////////////////////////////////////
// ConnectToChatServer function

void ConnectToChatServer(LPCONNECTIONINFO lpConnectionInfo) {
    if (lpConnectionInfo == NULL
            || !IsConnectionInfoValid(lpConnectionInfo->szHostname,
                    lpConnectionInfo->nPort)) {
        CleanupClient(ERROR);
    }

    // Attempt to connect to the server at the specified hostname and on the
    // specified port.
    if (OK
            != ConnectSocket(g_nClientSocket, lpConnectionInfo->szHostname,
                    lpConnectionInfo->nPort)) {
        fprintf(stderr, FAILED_TO_CONNECT_TO_SERVER,
                lpConnectionInfo->szHostname, lpConnectionInfo->nPort);

        CleanupClient(ERROR);
    }

    g_bConnected = TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// CleanupClient function - Releases operating system resources consumed by the
// client and exits the process
//

void CleanupClient(int nExitCode) {
    if (INVALID_HANDLE_VALUE != g_hReceiveThread) {
        DestroyThread(g_hReceiveThread);
    }

    if (INVALID_HANDLE_VALUE != g_hSendThread) {
        DestroyThread(g_hSendThread);
    }

    if (GetLogFileHandle() != stdout) {
        fprintf(stdout, DONE_CHATTING);
    }

    FreeSocketMutex();

    if (IsSocketValid(g_nClientSocket)) {
        CloseSocket(g_nClientSocket);
        g_nClientSocket = INVALID_SOCKET_HANDLE;
    }

    g_bConnected = FALSE;
    ClearNickname();

    LogInfo(CLIENT_DISCONNECTED);

    if (GetLogFileHandle() != stdout) {
        fprintf(stdout, DISCONNECTED_FROM_CHAT_SERVER);
    }

    CloseLogFileHandles();

    exit(nExitCode);
}

void ClearNickname() {
    memset(g_szNickname, 0, MAX_NICKNAME_LEN + 1);
}

///////////////////////////////////////////////////////////////////////////////
// ClientCleanupHandler function - A callback that provides functionality to
// handle the case where the user has pressed CTRL+C in this process' Terminal
// window in a graceful manner
//

void ClientCleanupHandler(int signum) {
    printf("\n");

    LeaveChatRoom();

    if (!HasChatSessionBegun()) {
        FreeSocketMutex();
        exit(OK);
    }

    CleanupClient(OK);
}

///////////////////////////////////////////////////////////////////////////////
// CreateReceiveThread function

void CreateReceiveThread() {
    g_hReceiveThread = CreateThread(ReceiveThread);

    // Verify that the Receive Thread was started successfully.
    if (INVALID_HANDLE_VALUE == g_hReceiveThread) {
        fprintf(stderr,
        FAILED_SPAWN_RECEIVE_THREAD);

        CleanupClient(ERROR);
    }
}

///////////////////////////////////////////////////////////////////////////////
// CreateSendThread function

void CreateSendThread() {
    g_hSendThread = CreateThread(SendThread);

    // Verify that the Send Thread was started successfully.
    if (INVALID_HANDLE_VALUE == g_hSendThread) {
        fprintf(stderr,
        FAILED_SPAWN_SEND_THREAD);

        CleanupClient(ERROR);
    }
}

///////////////////////////////////////////////////////////////////////////////
// FormatLogFileName function - Fills a buffer with the fully-qualified path
// to utilize for a log file.  We format the filename with the current system
// date and time.
//

void FormatLogFileName(char* pszBuffer) {
    if (pszBuffer == NULL) {
        fprintf(stderr, PARSE_COMMAND_LINE_INVALID_PARAMETERS);

        CleanupClient(ERROR);
    }

    char szDateBuffer[DATE_BUFFER_SIZE + 1];
    FormatDate(szDateBuffer, DATE_BUFFER_SIZE + 1, DATETIME_FORMAT);

    sprintf(pszBuffer, LOG_FILE_PATH, szDateBuffer);
}

BOOL HasChatSessionBegun() {
    return g_bConnected && !IsNullOrWhiteSpace(g_szNickname);
}

///////////////////////////////////////////////////////////////////////////////
// InitializeApplication function - Does one-time startup initialization of
// the client
//

/**
 * @brief Runs code that is meant to only be run once on startup.
 * @return TRUE if successful, FALSE if an error occurred.
 * @remarks The application should be terminated immediately if this
 * function returns FALSE.
 */
BOOL InitializeApplication() {
    ConfigureLogFile();

    CreateSocketMutex();

    DisableBufferedConsoleIO();

    InstallSigintHandler();

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// InstallSigintHandler function - Installs a SIGINT handler to handle the case
// where the user presses CTRL+C in this process' Terminal window.  This allows
// us to clean up the main while loop and free operating system resources
//gracefully.
//
// Shout-out to <https://stackoverflow.com/questions/1641182/
// how-can-i-catch-a-ctrl-c-event-c> for this code.
//

void InstallSigintHandler() {
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = ClientCleanupHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    if (OK != sigaction(SIGINT, &sigIntHandler, NULL)) {
        fprintf(stderr, FAILED_INSTALL_SIGINT_HANDLER);

        perror("chattr[sigaction]");

        CleanupClient(ERROR);
    }
}

///////////////////////////////////////////////////////////////////////////////
// IsCommandLineArgumentCountValid function - Determines whether the user has
// supplied enough information on the command line in order for this program
// to execute successfully.
//

BOOL IsCommandLineArgumentCountValid(int argc) {
    return argc >= MIN_NUM_ARGS;
}

///////////////////////////////////////////////////////////////////////////////
// ParseCommandLine function

void ParseCommandLine(char* argv[], char** ppszHostname, int* pnPort) {
    if (ppszHostname == NULL) {
        LogError(PARSE_COMMAND_LINE_INVALID_PARAMETERS);
        fprintf(stderr, USAGE_STRING);

        exit(ERROR); /* we can just exit here, no spiffy cleanup needed. */
    }

    if (pnPort == NULL) {
        LogError(PARSE_COMMAND_LINE_INVALID_PARAMETERS);
        fprintf(stderr, USAGE_STRING);

        exit(ERROR); /* we can just exit here, no spiffy cleanup needed. */
    }

    *ppszHostname = argv[1];

    *pnPort = ParsePortNumber(argv[2]);
}

///////////////////////////////////////////////////////////////////////////////
// ParsePortNumber function - Tries to turn the string containing the port
// number the user typed as a command-line argument into an integer literal for
// usage in connecting to the server.
//

int ParsePortNumber(const char* pszPort) {
    // If the port number is blank, fail.
    if (IsNullOrWhiteSpace(pszPort)) {
        fprintf(stderr, FAIL_PARSE_PORTNUM);

        CleanupClient(ERROR);
    }

    // Try to parse the string containing the port number into an integer
    // value.
    long nResult = 0L;

    if (StringToLong(pszPort, (long*) &nResult) < 0) {
        fprintf(stderr, FAIL_PARSE_PORTNUM);

        CleanupClient(ERROR);
    }

    //return nResult;
    return (int) nResult;
}

///////////////////////////////////////////////////////////////////////////////
// PrintSoftwareTitleAndCopyright function - Prints the software's title and
// the copyright message.
//

void PrintSoftwareTitleAndCopyright() {
    ClearScreen();

    printf(SOFTWARE_TITLE);
    printf(COPYRIGHT_MESSAGE);
}

///////////////////////////////////////////////////////////////////////////////

