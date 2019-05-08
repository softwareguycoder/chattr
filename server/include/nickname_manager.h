// nickname_manager.h - Defines the interface for a set of functions that
// allow us to manage the nicknames in use by this chat server's various
// clients.

#ifndef __NICKNAME_MANAGER_H__
#define __NICKNAME_MANAGER_H__

/**
 * @brief Parses the user's chosen nickname.  Really just tokenizes src on
 * spaces and returns the second token (ostensibly, the value after the NICK
 * command).
 * @param dest Address of a buffer to receive the result.
 * @param src Address of a buffer containing the source (input).
 * @remarks src may not contain a string literal.  It should be the address
 * of a char buffer that has had the value to be parsed literally copied into
 * it, say, with strcpy().
 */
void GetNicknameFromUser(char* dest, char* src);

/**
 * @brief Registers a nickname (i.e., chat handle) with the server for the
 * particular client sending it.
 * @param lpSendingClient Reference to a CLIENTSTRUCT instance containing data
 * on the client who sent the chat message.
 * @param pszBuffer Address of a buffer containing data received from the
 * client.
 */
BOOL RegisterClientNickname(LPCLIENTSTRUCT lpSendingClient, char* pszBuffer);

#endif /* __NICKNAME_MANAGER_H__ */
