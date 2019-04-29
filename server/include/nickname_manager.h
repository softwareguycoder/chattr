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
void GetNicknameFromClient(char* dest, char* src);

#endif /* __NICKNAME_MANAGER_H__ */
