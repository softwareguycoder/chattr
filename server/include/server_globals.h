// server_globals.h - Header file to contain all the declarations of global
// variables in use by the server application in one place
//

#ifndef __SERVER_GLOBALS_H__
#define __SERVER_GLOBALS_H__

/**
 * @brief Indicates whether the server is in diagnostic mode.
 * @remarks Putting the server into diagnostic mode is done by supplying the
 * '-v' switch on the command line as the second argument.  This mode causes
 * the server to be very verbose to the console.
 */
extern BOOL g_bDiagnosticMode;

/**
 * @brief Reference to the linked list of clients.
 */
extern POSITION* g_pClientList;

/**
 * @brief Lock object for client list access.
 */
extern HMUTEX g_hClientListMutex;

/**
 * @brief Thread handle for the Master Acceptor Thread (MAT).
 */
extern HTHREAD g_hMasterThread;

/**
 * @brief Socket file descriptor for the server's TCP endpoint.
 */
extern int g_nServerSocket;

#endif /* __SERVER_GLOBALS_H__ */
