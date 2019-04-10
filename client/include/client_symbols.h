/*
 * client_symbols.h
 *
 *  Created on: Apr 9, 2019
 *      Author: bhart
 */

#ifndef __CLIENT_SYMBOLS_H__
#define __CLIENT_SYMBOLS_H__

#ifndef BUFLEN
#define BUFLEN					1024
#endif //BUFLEN

#ifndef COPYRIGHT_MESSAGE
#define COPYRIGHT_MESSAGE		"Copyright (c) 2019 by Brian Hart.\n\n"
#endif //COPYRIGHT_MESSAGE

#ifndef ERROR
#define ERROR           		-1		// The server encountered an error
#endif //ERROR

#ifndef FALSE
#define FALSE           		0
#endif //FALSE

#ifndef IPADDRLEN
#define IPADDRLEN   			20
#endif //IPADDRLEN

#ifndef INPUT_PROMPT
#define	INPUT_PROMPT			"> "
#endif //INPUT_PROMPT

#ifndef MAX_LINE_LENGTH
#define MAX_LINE_LENGTH 		255     // The maximum length of a line
#endif //MAX_LINE_LENGTH

#ifndef MAX_MESSAGE_LEN
#define MAX_MESSAGE_LEN			255
#endif //MAX_MESSAGE_LEN

#ifndef MIN_NUM_ARGS
#define MIN_NUM_ARGS			3		// The minimum # of cmd line args to pass
#endif //MIN_NUM_ARGS

#ifndef MSG_TERMINATOR
#define	MSG_TERMINATOR			".\n"
#endif //MSG_TERMINATOR

#ifndef NICKNAME_PROMPT
#define NICKNAME_PROMPT			"> Please type a nickname (15 chars max): > "
#endif //NICKNAME_PROMPT

#ifndef OK
#define OK              		0		// The server completed successfully
#endif //OK

#ifndef PROTOCOL_HELO_COMMAND
#define PROTOCOL_HELO_COMMAND	"HELO\n"		// Protocol command that gets this client marked as a member of the chat room
#endif //PROTOCOL_HELO_COMMAND

#ifndef PROTOCOL_NICK_COMMAND
#define PROTOCOL_NICK_COMMAND	"NICK %s\n"		// Protocol command that registers this user's chat handle with the server
#endif //PROTOCOL_NICK_COMMAND

#ifndef PROTOCOL_QUIT_COMMAND
#define PROTOCOL_QUIT_COMMAND	"QUIT\n"		// Protocol command that 'logs the client off' from the chat server.
#endif //PROTOCOL_QUIT_COMMAND

#ifndef RECV_BLOCK_SIZE
#define RECV_BLOCK_SIZE			1
#endif //RECV_BLOCK_SIZE

#ifndef RECV_FLAGS
#define RECV_FLAGS				0
#endif //RECV_FLAGS

#ifndef SOFTWARE_TITLE
#define SOFTWARE_TITLE  		"chattr v1.0 Chat Client\n"
#endif //SOFTWARE_TITLE

#ifndef TRUE
#define TRUE            		1
#endif //TRUE

#ifndef USAGE_MESSAGE
#define USAGE_MESSAGE			"Type a line and press ENTER to send it to the chat room.\n\n"
#endif //USAGE_MESSAGE

#ifndef USAGE_STRING
#define USAGE_STRING			"Usage: client <host name or IP> <port_num>\n" 	// Usage string
#endif //USAGE_STRING

#endif /* __CLIENT_SYMBOLS_H__ */
