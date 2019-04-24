// client_list_manager.h - Interface of callback functions utilized by the
// functions that search and access the list of clients.
//

#ifndef __CLIENT_LIST_MANAGER_H__
#define __CLIENT_LIST_MANAGER_H__

/**
 * @brief Callback used to search the list of clients for a particular client.
 * @param pClientSockFd Address of an integer variable storing the socket
 * file descriptor for a given client.
 * @param pClientStruct Address of an instance of CLIENTSTRUCT referring to the
 * current client in the list being searched.
 * @returns TRUE if the client referenced by pClientStruct has a socket file
 * descriptor matching that pointed to by pClientSocketFd; FALSE otherwise.
 */
BOOL FindClientByID(void* pClientSocketFd, void* pClientStruct);

/**
 * @brief Callback that is called for each client in the list of clients to
 * forcibly terminate the link with that client.
 * @param pClientStruct Address of a CLIENTSTRUCT instance that refers to the
 * specific client whose session should be killed.
 * @remarks This is really just an alias for the ForciblyDisconnectClient
 * function; however, since it's a callback, its prototype must match the
 * LPACTION_ROUTINE signature.
 *
 */
void ForceDisconnectionOfClient(void* pClientStruct);

#endif /* __CLIENT_LIST_MANAGER_H__ */
