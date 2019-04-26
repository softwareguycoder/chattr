// client_list_manager.h - Interface of callback functions utilized by the
// functions that search and access the list of clients.
//

#ifndef __CLIENT_LIST_MANAGER_H__
#define __CLIENT_LIST_MANAGER_H__

/**
 * @brief Callback used to search the list of clients for a particular client.
 * @param pvClientId UUID that refers to a specific client.
 * @param pvClientStruct Address of an instance of CLIENTSTRUCT referring to the
 * current client in the list being searched.
 * @returns TRUE if the client referenced by pClientStruct has a value for its
 * clientID member that is equal to that referenced by pvClientId; FALSE
 * otherwise.
 */
BOOL FindClientByID(void* pvClientId, void* pvClientStruct);

/**
 * @brief Callback used to search the list of clients for a particular client.
 * @param pvNickname String containing the chat nickname of the client.
 * @param pvClientStruct Address of an instance of CLIENTSTRUCT referring to the
 * current client in the list being searched.
 * @returns TRUE if the nickname provided matches that (case-sensitive) of a
 * particular client.
 * @remarks Per our server's chat protocol, all client nicknames should be
 * unique (in terms of case).
 */
BOOL FindClientByNickname(void* pvNickname, void* pvClientStruct);

/**
 * @brief Callback that is called for each client in the list of clients to
 * forcibly terminate the link with that client.
 * @param pvClientStruct Address of a CLIENTSTRUCT instance that refers to the
 * specific client whose session should be killed.
 * @remarks This is really just an alias for the ForciblyDisconnectClient
 * function; however, since it's a callback, its prototype must match the
 * LPACTION_ROUTINE signature.
 *
 */
void ForceDisconnectionOfClient(void* pvClientStruct);

#endif /* __CLIENT_LIST_MANAGER_H__ */
