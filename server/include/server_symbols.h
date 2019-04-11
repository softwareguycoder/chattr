/*
 * symbols.h
 *
 *  Created on: Mar 29, 2019
 *      Author: bhart
 */

#ifndef __SERVER_SYMBOLS_H__
#define __SERVER_SYMBOLS_H__

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

#ifndef ERROR_FORCED_DISCONNECT
#define ERROR_FORCED_DISCONNECT		"503 Server forcibly shut down by its operator.\n"
#endif //ERROR_FORCED_DISCONNECT

#ifndef LOG_FILE_OPEN_MODE
#define LOG_FILE_OPEN_MODE			"a+"	// Mode for opening the log file
#endif //LOG_FILE_OPEN_MODE

// Path to the log file
#ifndef LOG_FILE_PATH
#define LOG_FILE_PATH				"/home/bhart/logs/chattr/server.log"
#endif //LOG_FILE_PATH

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
#define NEW_CHATTER_JOINED			"!@%s joined the chat room.\n"
#endif //NEW_CHATTER_JOINED

#ifndef NEW_CHATTER_LEFT
#define NEW_CHATTER_LEFT			"!@%s left the chat room.\n"
#endif //NEW_CHATTER_LEFT

#endif /* __SERVER_SYMBOLS_H__ */
