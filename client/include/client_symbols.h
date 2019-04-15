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

#ifndef CHATTR_FAILED_TO_VALIDATE_ARGUMENTS
#define CHATTR_FAILED_TO_VALIDATE_ARGUMENTS \
        "chattr: Failed to validate arguments."
#endif //CHATTR_FAILED_TO_VALIDATE_ARGUMENTS

#ifndef COPYRIGHT_MESSAGE
#define COPYRIGHT_MESSAGE			"Copyright (c) 2019 by Brian Hart.\n\n"
#endif //COPYRIGHT_MESSAGE

#ifndef COULD_NOT_CREATE_CLIENT_TCP_ENDPOINT
#define COULD_NOT_CREATE_CLIENT_TCP_ENDPOINT \
        "chattr: Could not create TCP endpoint for connecting to the server.\n"
#endif

#ifndef CLIENT_DATA_FORMAT
#define CLIENT_DATA_FORMAT          "C: %s"
#endif //CLIENT_DATA_FORMAT

#ifndef CLIENT_DISCONNECTED
#define CLIENT_DISCONNECTED         "C: <disconnected>\n"
#endif //CLIENT_DISCONNECTED

#ifndef DATE_BUFFER_SIZE
#define DATE_BUFFER_SIZE			32
#endif //DATE_BUFFER_SIZE

#ifndef DATETIME_FORMAT
#define DATETIME_FORMAT				"%Y-%m-%d_%H%M%S"
#endif //DATETIME_FORMAT

#ifndef DONE_CHATTING
#define DONE_CHATTING               "chattr: Done chatting!\n"
#endif //DONE_CHATTING

#ifndef DISCONNECTED_FROM_CHAT_SERVER
#define DISCONNECTED_FROM_CHAT_SERVER \
    "chattr: Disconnected from the chat server.\n"
#endif //DISCONNECTED_FROM_CHAT_SERVER

#ifndef ERROR_FORCED_DISCONNECT
#define ERROR_FORCED_DISCONNECT     "503 Server forcibly shut down by its " \
                                    "operator.\n"
#endif //ERROR_FORCED_DISCONNECT

#ifndef FAIL_PARSE_PORTNUM
#define FAIL_PARSE_PORTNUM			"chattr: Failed to determine what port " \
									"number you want to use.\n"
#endif //FAIL_PARSE_PORTNUM

#ifndef FAILED_TO_CONNECT_TO_SERVER
#define FAILED_TO_CONNECT_TO_SERVER \
        "chattr: Failed to connect to server '%s' on port %d.\n"
#endif //FAILED_TO_CONNECT_TO_SERVER

#ifndef FAILED_INSTALL_SIGINT_HANDLER
#define FAILED_INSTALL_SIGINT_HANDLER \
    "chattr: Unable to install CTRL+C handler.\n"
#endif //FAILED_INSTALL_SIGINT_HANDLER

#ifndef FAILED_OPEN_LOG
#define FAILED_OPEN_LOG				"ERROR: Failed to open log file '%s' " \
									"for writing.\n"
#endif //FAILED_OPEN_LOG

#ifndef FAILED_SPAWN_RECEIVE_THREAD
#define FAILED_SPAWN_RECEIVE_THREAD \
        "chattr: Failed to spawn the receive thread.  Quitting.\n"
#endif //FAILED_SPAWN_RECEIVE_THREAD

#ifndef FAILED_SPAWN_SEND_THREAD
#define FAILED_SPAWN_SEND_THREAD \
        "chattr: Failed to spawn the send thread.  Quitting.\n"
#endif //FAILED_SPAWN_SEND_THREAD

#ifndef FALSE
#define FALSE           		0
#endif //FALSE

#ifndef IPADDRLEN
#define IPADDRLEN   			20
#endif //IPADDRLEN

#ifndef INPUT_PROMPT
#define	INPUT_PROMPT			"> "
#endif //INPUT_PROMPT

#ifndef INPUT_SIZE
#define INPUT_SIZE				255		// Max chars that can be input
											// at a prompt
#endif //INPUT_SIZE

#ifndef INVALID_PARAMETERS
#define INVALID_PARAMETERS      "chattr: ParseCommandLine: Invalid parameter.\n"
#endif //INVALID_PARAMETERS

#ifndef LOG_FILE_OPEN_MODE
#define LOG_FILE_OPEN_MODE		"a+"	// Mode for opening the log file
											//(appending)
#endif //LOG_FILE_OPEN_MODE

// Path to the log file -- labeled with a date/time format.
#ifndef LOG_FILE_PATH
#define LOG_FILE_PATH			"/home/bhart/logs/chattr/client_%s.log"
#endif //LOG_FILE_PATH

#ifndef MAX_PATH
#define MAX_PATH				255
#endif //MAX_PATH

#ifndef MAX_LINE_LENGTH
#define MAX_LINE_LENGTH 		255     // The maximum length of a line
#endif //MAX_LINE_LENGTH

#ifndef MAX_MESSAGE_LEN
#define MAX_MESSAGE_LEN			255
#endif //MAX_MESSAGE_LEN

#ifndef MAX_NICKNAME_LEN
#define MAX_NICKNAME_LEN		15		// Maximum length of a chat handle
											// is 15 chars
#endif //MAX_NICKNAME_LEN

#ifndef MIN_NUM_ARGS
#define MIN_NUM_ARGS			3		// The minimum # of cmd line args to
											//pass
#endif //MIN_NUM_ARGS

#ifndef MIN_SIZE
#define MIN_SIZE				1		// The smallest value of a size
#endif //MIN_SIZE

#ifndef MSG_TERMINATOR
#define	MSG_TERMINATOR			".\n"
#endif //MSG_TERMINATOR

#ifndef NICKNAME_NOTALPHA
#define NICKNAME_NOTALPHA		"ERROR: Nickname can only contain letters " \
								"and/or numbers.\n"
#endif //NICKNAME_NOTALPHA

#ifndef NICKNAME_PROMPT
#define NICKNAME_PROMPT			"> Please type a nickname (15 chars max): > "
#endif //NICKNAME_PROMPT

#ifndef NICKNAME_REQUIRED
#define NICKNAME_REQUIRED		"ERROR: A value for the nickname is required.\n"
#endif //NICKNAME_REQUIRED

#ifndef NICKNAME_TOOLONG
#define NICKNAME_TOOLONG		"ERROR: Nickname can only be %d chars in " \
								"length.\n"
#endif	//NICKNAME_TOOLONG

#ifndef NICKNAME_UNKERROR
#define NICKNAME_UNKERROR		"We didn't quite catch the value you typed. " \
								"Please try again.\n"
#endif //NICKNAME_UNKERROR

#ifndef NOW_CONNECTED_TO_SERVER
#define NOW_CONNECTED_TO_SERVER \
        "chattr: Now connected to the chat server '%s' on port %d.\n"
#endif //NOW_CONNECTED_TO_SERVER

#ifndef OK_GOODBYE
#define OK_GOODBYE				"200 Goodbye.\n"
#endif //OK_GOODBYE

#ifndef PORT_NUMBER_NOT_VALID
#define PORT_NUMBER_NOT_VALID \
        "chattr: Port number must be in the range 1024-49151 inclusive.\n"
#endif //PORT_NUMBER_NOT_VALID

#ifndef PROTOCOL_HELO_COMMAND
#define PROTOCOL_HELO_COMMAND	"HELO\n"		// Protocol command that
													// gets this client marked
													// as a member of the
													// chat room
#endif //PROTOCOL_HELO_COMMAND

#ifndef PROTOCOL_NICK_COMMAND
#define PROTOCOL_NICK_COMMAND	"NICK %s\n"		// Protocol command that
													// registers this user's
													// chat handle with the
													// server
#endif //PROTOCOL_NICK_COMMAND

#ifndef PROTOCOL_QUIT_COMMAND
#define PROTOCOL_QUIT_COMMAND	"QUIT\n"		// Protocol command that
													// 'logs the client off'
													// from the chat server.
#endif //PROTOCOL_QUIT_COMMAND

#ifndef SERVER_DATA_FORMAT
#define SERVER_DATA_FORMAT      "S: %s"
#endif //SERVER_DATA_FORMAT

#ifndef SERVER_DISCONNECTED
#define SERVER_DISCONNECTED     "S: <disconnected>\n"
#endif //SERVER_DISCONNECTED

#ifndef SET_CLIENT_SOCKET_NON_BLOCKING
#define SET_CLIENT_SOCKET_NON_BLOCKING \
        "chattr: Client socket has been set to non-blocking.\n"
#endif //SET_CLIENT_SOCKET_NON_BLOCKING

#ifndef SOFTWARE_TITLE
#define SOFTWARE_TITLE  		"chattr v1.0 Chat Client\n"
#endif //SOFTWARE_TITLE

#ifndef TRUE
#define TRUE            		1
#endif //TRUE

#ifndef USAGE_MESSAGE
#define USAGE_MESSAGE			"Type a line and press ENTER to send it " \
								"to the chat room.\n" \
								"Type QUIT (must be all caps) on its own " \
								"line to exit.\n\n"
#endif //USAGE_MESSAGE

// Usage string
#ifndef USAGE_STRING
#define USAGE_STRING			"Usage: client <host name or IP> <port_num>\n"
#endif //USAGE_STRING

#endif /* __CLIENT_SYMBOLS_H__ */
