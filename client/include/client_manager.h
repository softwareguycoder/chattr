// client_manager.h - Interface to the functions clients need to carry out
// protocol-specific communications with the server
//

#ifndef __CLIENT_MANAGER_H__
#define __CLIENT_MANAGER_H__

#include "client_symbols.h"

extern POSITION* g_pChatterList; /* list of chatters */

BOOL GetNicknameFromUser(char* pszNickname);
void GreetServer();
void HandleAdminOrChatMessage(const char* pszReceivedText);
void HandleIncorrectNicknameSubmitted(char* pszNickname, int nNicknameSize,
		char* pszReplyBuffer);
void HandleProtocolReply(const char* pszReplyMessage);
void HandshakeWithServer();
BOOL IsAdminOrChatMessage(const char* pszReceivedText);
BOOL IsMultilineResponseTerminator(const char* pszMessage);
void LeaveChatRoom();
void PrintChatterName(void* pvChatterName);
void PrintChattersInRoom();
void PrintClientUsageDirections();
void ProcessMultilineResponse();
void ProcessReceivedText(const char* pszReceivedText, int nSize);
void PromptUserForNickname(char* pszNicknameBuffer);
int ReceiveFromServer(char** ppszReplyBuffer);
BOOL SetNickname(const char* nickname);
BOOL ShouldStopReceiving(const char* pszReceivedText, int nSize);

#endif /* __CLIENT_MANAGER_H__ */

