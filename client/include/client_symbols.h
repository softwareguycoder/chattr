/*
 * client_symbols.h
 *
 *  Created on: Apr 9, 2019
 *      Author: bhart
 */

#ifndef __CLIENT_SYMBOLS_H__
#define __CLIENT_SYMBOLS_H__

#ifndef MAX_MESSAGE_LEN
#define MAX_MESSAGE_LEN				255
#endif

#ifndef IPADDRLEN
#define IPADDRLEN   20
#endif //IPADDRLEN

// Message to reply with saying no nickname registered.
#ifndef BUFLEN
#define BUFLEN						1024
#endif //BUFLEN

#ifndef ERROR_NO_NICK_RECEIVED
#define ERROR_NO_NICK_RECEIVED		"501 No nickname value specified after NICK command.\n"
#endif //ERROR_NO_NICK_RECEIVED

#ifndef OK_FOLLOW_WITH_NICK_REPLY
#define OK_FOLLOW_WITH_NICK_REPLY	"200 Welcome!  Now use the NICK command to tell me your nickname.\n"
#endif //OK_FOLLOW_WITH_NICK_REPLY

#ifndef OK_GOODBYE
#define OK_GOODBYE					"200 Goodbye.\n"
#endif //OK_GOODBYE

#ifndef OK_NICK_REGISTERED
#define OK_NICK_REGISTERED			"201 OK your nickname is %s.\n"
#endif //OK_NICK_REGISTERED

#ifndef NEW_CHATTER_JOINED
#define NEW_CHATTER_JOINED			"!Hey everyone, @%s joined the chat room.\n"
#endif //NEW_CHATTER_JOINED

#endif /* __CLIENT_SYMBOLS_H__ */
