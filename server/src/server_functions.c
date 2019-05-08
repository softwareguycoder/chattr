// server_functions.c - File to contain the various application-specific
// functions used by the server's main function
//

#include "stdafx.h"
#include "server.h"

#include "client_manager.h"
#include "client_list_manager.h"
#include "mat.h"
#include "server_functions.h"

BOOL g_bHasServerQuit = FALSE;

///////////////////////////////////////////////////////////////////////////////
// CheckCommandLineArgs function - Checks the command-line args passed (and the
// count thereof) to ensure that the arguments will be usable

BOOL CheckCommandLineArgs(int argc, char *argv[]) {
    // We expect one non-blank command-line argument, which is the port number
    // that the user wants the server to listen on.  The string should actually
    // be the ASCII representation of a positive integer.

    return argc >= MIN_NUM_ARGS
    		&& argv != NULL && !IsNullOrWhiteSpace(argv[1])
            && IsNumeric(argv[1]);
}

///////////////////////////////////////////////////////////////////////////////
// CleanupServer function - Called by routines all over the application to end
// the program gracefully, making sure to release resources and terminate all
// threads in an orderly way

void CleanupServer(int nExitCode) {
    // Handle the case where the user presses CTRL+C in the terminal
    // by performing an orderly shut down of the server and freeing
    // operating system resources.

    //fprintf(stdout, "server: Waiting on the client list mutex...\n");

    LockMutex(GetClientListMutex());
    {
        //fprintf(stdout, "server: Got client list mutex...\n");

        if (GetElementCount(g_pClientList) > 0) {
            DoForEach(g_pClientList, ForceDisconnectionOfClient);
        }

        //fprintf(stdout, "Releasing client list mutex...\n");
    }
    UnlockMutex(GetClientListMutex());

    //fprintf(stdout, "Client list mutex released.\n");

    QuitServer();

    fprintf(stdout, "server: Executing final cleanup actions...\n");

    CloseLogFileHandles();

    /* beyond this point, we cannot utlize the log_* functions */

    fprintf(stdout, "<terminated program>\n");

    exit(nExitCode);	// terminate program
}

///////////////////////////////////////////////////////////////////////////////
// ConfigureLogFile function - Removes the old log file from disk (if
// applicable) and sets up the file pointers and handles for the new one

void ConfigureLogFile() {
    remove(LOG_FILE_PATH);
    SetLogFileHandle(fopen(LOG_FILE_PATH, LOG_FILE_OPEN_MODE));
    SetErrorLogFileHandle(GetLogFileHandle());

    /*set_log_file(stdout);
     set_error_log_file(stderr);*/
}

///////////////////////////////////////////////////////////////////////////////
// CreateClientListMutex function - Sets up operating system resources for the
// mutex handle which controls threads' access to the list of clients.  Since
// this is only called exactly once during the lifetime of the application, it's
// place is in this file

void CreateClientListMutex() {
    if (INVALID_HANDLE_VALUE != GetClientListMutex()) {
        return;
    }

    SetClientListMutex(CreateMutex());
    if (INVALID_HANDLE_VALUE == GetClientListMutex()) {
        CleanupServer(ERROR);
    }
}

///////////////////////////////////////////////////////////////////////////////
// CreateMasterAcceptorThread function - Creates the master acceptor thread
// (MAT) that accepts incoming client connections and spins off a new thread
// to handle each separate connection

void CreateMasterAcceptorThread() {

	int nServerSocket = GetServerSocket();
    SetMasterThreadHandle(CreateThreadEx(MasterAcceptorThread,
    		&nServerSocket));

    if (INVALID_HANDLE_VALUE == GetMasterThreadHandle()) {
        fprintf(stderr, SERVER_FAILED_START_MAT);

        CleanupServer(ERROR);
    }
}

///////////////////////////////////////////////////////////////////////////////
// CreateSockAddr function - Allocates storage for a new insance of sockaddr_in
//

struct sockaddr_in* CreateSockAddr() {
    struct sockaddr_in* pResult = (struct sockaddr_in*) malloc(
            1 * sizeof(struct sockaddr_in));
    if (pResult == NULL) {
        fprintf(stderr, OUT_OF_MEMORY);

        // Failed to allocate memory
        CleanupServer(ERROR);
    }

    // Zero out the memory occupied by the structure
    memset(pResult, 0, sizeof(struct sockaddr_in));

    return pResult;
}

///////////////////////////////////////////////////////////////////////////////
// DestroyClientListMutex function - Releases the system resources occupied
// by the client list mutex handle.  It's here because this function just
// needs to be called exactly once during the excecution of the server.

void DestroyClientListMutex() {
    if (INVALID_HANDLE_VALUE == GetClientListMutex()) {
        return;
    }

    DestroyMutex(GetClientListMutex());
    SetClientListMutex(INVALID_HANDLE_VALUE);
}

///////////////////////////////////////////////////////////////////////////////
// InitializeApplication function - Runs functionality that should be executed
// exactly once during the lifetime of the application, at application startup.

BOOL InitializeApplication() {
    /* Configure settings for the log file */
    ConfigureLogFile();

    // Since the usual way to exit this program is for the user to
    // press CTRL+C to forcibly terminate it, install a Linux SIGINT
    // handler here so that when the user does this, we may still
    // get a chance to run the proper cleanup code.
    InstallSigintHandler();

    InitializeInterlock();

    /* Initialize the socket mutex object in the inetsock_core library */
    CreateSocketMutex();

    CreateClientListMutex();

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// InstallSigintHandler function - Registers a function to be called when the
// user presses CTRL+C, in order to perform an orderly shutdown

void InstallSigintHandler() {
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = ServerCleanupHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    if (OK != sigaction(SIGINT, &sigIntHandler, NULL)) {
        fprintf(stderr, "server: Unable to install CTRL+C handler.");

        perror("server[sigaction]");

        FreeSocketMutex();

        exit(ERROR);
    }
}

///////////////////////////////////////////////////////////////////////////////
// ParseCommandLine function

void ParseCommandLine(int argc,
		char *argv[], int* pnPort, BOOL* pbDiagnosticMode) {
    if (argv == NULL) {
        // Blank port number, nothing to do.
        fprintf(stderr, SERVER_NO_PORT_SPECIFIED);

        exit(ERROR);    /* we can just exit here, no spiffy cleanup needed. */
    }

    if (pnPort == NULL) {
        // Blank port number, nothing to do.
        fprintf(stderr, SERVER_NO_PORT_SPECIFIED);

        exit(ERROR);    /* we can just exit here, no spiffy cleanup needed. */
    }

    if (pbDiagnosticMode == NULL) {
    	// Nothing to do.
    	fprintf(stderr, ERROR_CANT_PARSE_DIAGNOSTIC_MODE);

    	exit(ERROR);
    }

    if (IsNullOrWhiteSpace(argv[1])) {
        // Blank port number, nothing to do.
        fprintf(stderr, SERVER_NO_PORT_SPECIFIED);

        exit(ERROR);    /* we can just exit here, no spiffy cleanup needed. */
    }

    int nResult = StringToLong(argv[1], (long*) pnPort);
    if (nResult != OK && nResult != EXACTLY_CORRECT) {
        fprintf(stderr, SERVER_NO_PORT_SPECIFIED);

        exit(ERROR);    /* we can just exit here, no spiffy cleanup needed. */
    }

    if (argc == DIAGNOSTIC_MODE_PARM_COUNT) {
    	*pbDiagnosticMode = EqualsNoCase(argv[2], "-v");
    }
}

///////////////////////////////////////////////////////////////////////////////
// PrintSoftwareTitleAndCopyright function - Does exactly what it says on the
// tin.

void PrintSoftwareTitleAndCopyright() {
    ClearScreen();

    printf(SOFTWARE_TITLE);
    printf(COPYRIGHT_MESSAGE);
}

///////////////////////////////////////////////////////////////////////////////
// QuitServer function - Called to conduct an orderly shutdown of the server,
// stopping all threads and releasing operating system resources in an orderly
// fashion.

void QuitServer() {

    if (g_bHasServerQuit) {
        return;
    }

    if (!g_bHasServerQuit) {
        g_bHasServerQuit = TRUE;
    }

    LogInfo(SERVER_SHUTTING_DOWN);

    if (GetLogFileHandle() != stdout) {
        fprintf(stdout, SERVER_SHUTTING_DOWN);
    }

    if (INVALID_HANDLE_VALUE != GetMasterThreadHandle()) {
        KillThread(GetMasterThreadHandle());
    }

    sleep(1); /* induce a context switch */

    DestroyInterlock();

    if (IsSocketValid(GetServerSocket())) {
    	fprintf(stdout, "server: Closing TCP endpoint...\n");

        CloseSocket(GetServerSocket());

        fprintf(stdout, SERVER_DISCONNECTED);
    }

    FreeSocketMutex();

    ClearList(&g_pClientList, FreeClient);

    DestroyClientListMutex();
}

///////////////////////////////////////////////////////////////////////////////
// ServerCleanupHandler function - Called when the user presses CTRL+C.  This
// function initiates an orderly shut down of the server application.

void ServerCleanupHandler(int signum) {
    fprintf(stdout, CALLING_CLEANUP_HANDLER);

    CleanupServer(OK);
}

///////////////////////////////////////////////////////////////////////////////
// SetUpServerOnPort function - Sets up the server to be bound to the specified
// port and starts the server listening on it.

void SetUpServerOnPort(int nPort) {
    if (!IsUserPortNumberValid(nPort)) {
        fprintf(stderr, PORT_NUMBER_NOT_VALID);

        FreeSocketMutex();

        exit(ERROR);    /* we can just exit here, no spiffy cleanup needed. */
    }

    struct sockaddr_in* pSockAddr = CreateSockAddr();

    // Intialize the structure with server address and port information
    GetServerAddrInfo(nPort, pSockAddr);

    // Bind the server socket to associate it with this host as a server
    if (BindSocket(GetServerSocket(), pSockAddr) < 0) {
        fprintf(stderr, SERVER_ERROR_FAILED_BIND);

        FreeBuffer((void**) &pSockAddr);

        FreeSocketMutex();

        exit(ERROR);    /* we can just exit here, no spiffy cleanup needed. */
    }

    if (ListenSocket(GetServerSocket()) < 0) {
        fprintf(stderr, SERVER_ERROR_FAILED_LISTEN);

        FreeBuffer((void**) &pSockAddr);

        FreeSocketMutex();

        exit(ERROR);    /* we can just exit here, no spiffy cleanup needed. */
    }

    fprintf(stdout, SERVER_LISTENING_ON_PORT, nPort);

    FreeBuffer((void**) &pSockAddr);
}
