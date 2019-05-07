// server_globals.h - Header file to contain all the declarations of global
// variables in use by the server application in one place
//

#ifndef __SERVER_GLOBALS_H__
#define __SERVER_GLOBALS_H__

/**
 * @brief Gets a value that specifies the port number on which this server
 * has been configured to listen.
 */
int GetServerPort();

/**
 * @brief Gets a value that indicates whether this server is currently in
 * diagnostic mode.
 * @remarks Putting the server in diagnostic mode makes it more verbose in its
 * reports to the log and to the console.
 */
BOOL IsDiagnosticMode();

/**
 * @brief Sets the current value of the diagnostic mode flag.
 * @param value The new value for the flag.
 */
void SetDiagnosticMode(BOOL value);

/**
 * @brief Sets the port number on which this server is configured to listen.
 * @param value New value for the port number.
 */
void SetServerPort(int value);

/**
 * @brief Port number on which this server is listening.
 */
extern int g_nServerPort;

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
