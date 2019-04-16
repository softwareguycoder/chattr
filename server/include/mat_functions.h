// mat_functions.h - Provides declarations of functions and symbols utilized by
// the Master Acceptor Thread (MAT).
//

#ifndef __MAT_FUNCTIONS_H__
#define __MAT_FUNCTIONS_H__

#define INVALID_SERVER_SOCKET_HANDLE    "Invalid server socket file " \
                                        "descriptor passed.\n"
#define INVALID_CLIENT_SOCKET_HANDLE    "Invalid client socket file " \
                                        "descriptor obtained from OS.\n"
#define SERVER_SOCKET_REQUIRED          "You should have passed the server " \
                                        "socket file descriptor to the MAT.\n"

void AddNewlyConnectedClientToList(LPCLIENTSTRUCT lpCS);

int GetServerSocketFileDescriptor(void* pThreadData);

BOOL IsClientCountZero();

void MakeServerEndpointReusable(int nServerSocket);

void TerminateMasterThread(int signum);

LPCLIENTSTRUCT WaitForNewClientConnection(int nServerSocket);

#endif /* __MAT_FUNCTIONS_H__ */
