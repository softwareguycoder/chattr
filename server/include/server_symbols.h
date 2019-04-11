// server_symbols.h - Unified, single include file for #define'd constants
// that are utilized throughout the server application
//

#ifndef __SERVER_SYMBOLS_H__
#define __SERVER_SYMBOLS_H__

/**
 * @brief Standardized size for buffers.
 */
#ifndef BUFLEN
#define BUFLEN					1024
#endif //BUFLEN

#ifndef CLIENT_IP_ADDR_UNK
#define CLIENT_IP_ADDR_UNK		"server: Client IP address not known.\n"
#endif //CLIENT_IP_ADDR_UNK

/**
 * @brief Copyright message to display on the server's console.
 */
#ifndef COPYRIGHT_MESSAGE
#define COPYRIGHT_MESSAGE	"Copyright (c) 2018-19 by Brian Hart.\n\n"
#endif //COPYRIGHT_MESSAGE

/**
 * @brief Message to reply with saying no nickname registered.
 */
#ifndef ERROR_NO_NICK_RECEIVED
#define ERROR_NO_NICK_RECEIVED		"501 No nickname value specified after " \
									"NICK command.\n"
#endif //ERROR_NO_NICK_RECEIVED

/**
 * @brief Message to send to clients indicating that the server application
 * has been forcibly terminated by its console interactive user.
 */
#ifndef ERROR_FORCED_DISCONNECT
#define ERROR_FORCED_DISCONNECT		"503 Server forcibly shut down by its " \
									"operator.\n"
#endif //ERROR_FORCED_DISCONNECT

#ifndef FAILED_CREATE_NEW_CLIENT
#define FAILED_CREATE_NEW_CLIENT	"server: Failed to create new client " \
									"list entry.\n"
#endif //FAILED_CREATE_NEW_CLIENT

#ifndef FAILED_LAUNCH_CLIENT_THREAD
#define FAILED_LAUNCH_CLIENT_THREAD	"server: Failed to launch client comm " \
									"channel.\n"
#endif //FAILED_LAUNCH_CLIENT_THREAD
/**
 * @brief Maximum length of a string containing a valid IPv4 IP address.
 */
#ifndef IPADDRLEN
#define IPADDRLEN   				20
#endif //IPADDRLEN

/**
 * @brief fopen() mode for opening the log file.
 */
#ifndef LOG_FILE_OPEN_MODE
#define LOG_FILE_OPEN_MODE			"a+"
#endif //LOG_FILE_OPEN_MODE

/**
 * @brief Path to the log file.
 */
#ifndef LOG_FILE_PATH
#define LOG_FILE_PATH				"/home/bhart/logs/chattr/server.log"
#endif //LOG_FILE_PATH

/**
 * @brief The minimum # of cmd line args to pass
 */
#ifndef MIN_NUM_ARGS
#define MIN_NUM_ARGS				2
#endif //MIN_NUM_ARGS

/**
 * @brief Server's administrative message saying a new chatter joined.
 */
#ifndef NEW_CHATTER_JOINED
#define NEW_CHATTER_JOINED			"!@%s joined the chat room.\n"
#endif //NEW_CHATTER_JOINED

/**
 * @brief Server's administrative message saying a chatter left.
 */
#ifndef NEW_CHATTER_LEFT
#define NEW_CHATTER_LEFT			"!@%s left the chat room.\n"
#endif //NEW_CHATTER_LEFT

/**
 * @brief Message to print or log indicating new client connection detected.
 */
#ifndef NEW_CLIENT_CONN
#define NEW_CLIENT_CONN				"S: <New client connection detected " \
									"from %s.>\n"
#endif //NEW_CLIENT_CONN

/**
 * @brief Response to the HELO command indicating operation succeeded.
 * @remarks The HELO command is issued by clients right after they establish
 * the TCP connection to the server.  This command lets the server know that a
 * new user wants to join the chat room.  Until this command is issued, followed
 * by a proper NICK command, no chat messages will be echoed to that particular
 * client if other users chat.
 */
#ifndef OK_FOLLOW_WITH_NICK_REPLY
#define OK_FOLLOW_WITH_NICK_REPLY	"200 Welcome!  Now use the NICK command" \
									" to tell me your nickname.\n"
#endif //OK_FOLLOW_WITH_NICK_REPLY

/**
 * @brief Response to the QUIT command indicating operation succeeded.
 * @remarks THe QUIT command is issued by chat clients who want to tell the
 * server that their user is done chatting.
 */
#ifndef OK_GOODBYE
#define OK_GOODBYE					"200 Goodbye.\n"
#endif //OK_GOODBYE

/**
 * @brief Response to the NICK command signifying operation succeeded.
 * @remarks The NICK command is issued by clients to register a "chat handle"
 * for their user.
 */
#ifndef OK_NICK_REGISTERED
#define OK_NICK_REGISTERED			"201 OK your nickname is %s.\n"
#endif //OK_NICK_REGISTERED

/**
 * @brief Title of this software for displaying on the console.
 */
#ifndef SOFTWARE_TITLE
#define SOFTWARE_TITLE			  	"Chattr TCP chat server v1.0\n"
#endif //SOFTWARE_TITLE

/**
 * @brief Error message to be displayed when we can't bind the server's socket.
 */
#ifndef SERVER_ERROR_FAILED_BIND
#define SERVER_ERROR_FAILED_BIND	"server: Could not bind server's TCP" \
									"endpoint.\n"
#endif //SERVER_ERROR_FAILED_BIND

/**
 * @brief Error message to display when the socket handle for a client is
 * invalid.
 */
#ifndef SERVER_CLIENT_SOCKET_INVALID
#define SERVER_CLIENT_SOCKET_INVALID "server: Invalid value for client's TCP " \
									 "endpoint.\n"
#endif //SERVER_CLIENT_SOCKET_INVALID

/**
 * @brief Error message to be displayed when we can't set up a listening socket.
 */
#ifndef SERVER_ERROR_FAILED_LISTEN
#define SERVER_ERROR_FAILED_LISTEN	"server: Could not open server's TCP" \
									"endpoint for listening.\n"
#endif //SERVER_ERROR_FAILED_LISTEN

#ifndef SERVER_FAILED_START_MAT
#define SERVER_FAILED_START_MAT		"server: Failed to initialize master " \
									"acceptor thread.\n"
#endif //SERVER_FAILED_START_MAT
/**
 * @brief Message to tell the user the server is now listening on the port.
 */
#ifndef SERVER_LISTENING_ON_PORT
#define SERVER_LISTENING_ON_PORT	"server: Now listening on port %s\n"
#endif //SERVER_LISTENING_ON_PORT

/**
 * @brief Usage message to be displayed if the user has not specified correct
 * command-line paramters on startup.
 */
#ifndef USAGE_STRING
#define USAGE_STRING				"Usage: server <port_num>\n"
#endif //USAGE_STRING

#endif /* __SERVER_SYMBOLS_H__ */
