///////////////////////////////////////////////////////////////////////////////
// server.c - Echo server in C
// The server receives text a line at a time and echoes the text back to its
// client only AFTER an entire line has been received.
//
// AUTHOR: Brian Hart
// DATE: 20 Sep 2018
//
// Shout-out to <https://gist.githubusercontent.com/suyash/2488ff6996c98a8ee3a8
// 4fe3198a6f85/raw/ba06e891060b277867a6f9c7c2afa20da431ec91/server.c> and
// <http://www.linuxhowtos.org/C_C++/socket.htm> for 
// inspiration
//

/*
	TCP uses 2 types of sockets: the connection socket and the listen socket.
	The goal is to separate the connection phase from the data exchange phase.
*/

#include "stdafx.h"
#include "server.h"

int server_socket = 0;
int is_execution_over = 0;

void quit_server()
{
    // If the socket file descriptor in the global variable server_socket
    // is less than or equal zero, then there is nothing to do here.
    if (server_socket <= 0)
    {
        return;
    }

    close(server_socket);
    server_socket = 0;

    is_execution_over = 1;

    log_info("server: Server endpoint closed.");
	log_info("server: execution finished with no errors.");
}

// Functionality to handle the case where the user has pressed CTRL+C
// in this process' terminal window
void cleanup_handler(int s)
{
	printf("\n");

    // Handle the case where the user presses CTRL+C in the terminal
    // by performing an orderly shut down of the server and freeing
    // operating system resources.

	quit_server();

    exit(OK);
}

// Installs a sigint handler to handle the case where the user
// presses CTRL+C in this process' terminal window.  This allows 
// us to clean up the main while loop and free operating system
// resources gracefully.
//
// Shout-out to <https://stackoverflow.com/questions/1641182/
// how-can-i-catch-a-ctrl-c-event-c> for this code.
void install_sigint_handler()
{
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = cleanup_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);
}

int main(int argc, char *argv[]) 
{
    printf(SOFTWARE_TITLE);
    printf(COPYRIGHT_MESSAGE);

    int bytesReceived = 0, bytesSent = 0;
    
    // Since the usual way to exit this program is for the user to
    // press CTRL+C to forcibly terminate it, install a Linux SIGINT
    // handler here so that when the user does this, we may still 
    // get a chance to run the proper cleanup code.
    install_sigint_handler();
  
    log_info("server: Checking arguments...");

	// Check the arguments.  If there is less than 2 arguments, then 
	// we should print a message to stderr telling the user what to 
	// pass on the command line and then quit
	if (argc < MIN_NUM_ARGS) 
	{
		fprintf(stderr, USAGE_STRING);		
		exit(ERROR);
	}

    if (argc >= MIN_NUM_ARGS)
    	log_info("server: Port number configured as %s.", argv[1]);

    server_socket = SocketDemoUtils_createTcpSocket();

	log_info("server: new TCP endpoint created.");
        
	// Assume that the first argument (argv[1]) is the port number 
	// that the user wants us to listen on 
	struct sockaddr_in server_address;      // socket address for the server
	memset(&server_address, 0, sizeof(server_address));

    SocketDemoUtils_populateServerAddrInfo(argv[1], &server_address);
		
	// Bind the server socket to associate it with this host as a server
	if (SocketDemoUtils_bind(server_socket, &server_address) < 0) 
	{
		log_error("server: Could not bind endpoint.");
		exit(ERROR);
	}

	log_info("server: Endpoint bound to localhost on port %s.", argv[1]);

	if (SocketDemoUtils_listen(server_socket) < 0)
	{
		log_error("server: Could not open server endpoint for listening.");
	}

	log_info("server: Now listening on port %s", argv[1]);

	// socket address used to store client address
	struct sockaddr_in client_address;

	int client_socket = -1;

	// run indefinitely
	while(1) 
	{
		//log_info("server: Waiting for client connection...\n");

		// We now call the accept function.  This function holds us up
		// until a new client connection comes in, whereupon it returns
		// a file descriptor that represents the socket on our side that
		// is connected to the client.
		if ((client_socket = SocketDemoUtils_accept(server_socket, 
                &client_address)) < 0) 
		{
            close(client_socket);
            client_socket = -1;

            continue;
			//error("server: Could not open an endpoint to accept data\n");
		}

		int wait_for_new_connection = 0;

        while(1)
        {          
		    // receive all the lines of text that the client wants to send.
		    // put them all in a buffer.
		    char* buf = NULL;
		    int bytes = 0;		

		    // just call getline (above) over and over again until 
		    // all the data has been read that the client wants to send.
		    // Clients should figure out a way to determine when to stop	
		    // sending input.
		    if ((bytes = SocketDemoUtils_recv(client_socket, &buf)) > 0) 
            {           
                bytesReceived += bytes;

                fprintf(stdout, "C: %s", buf);

                if (strcmp(buf, ".\n") == 0
                		|| strcmp(buf, "QUIT\n") == 0)
                {               
                    // disconnect from the client
                    close(client_socket);   
                    client_socket = -1;

                    // alert the server console that the client has disconnected
                    fprintf(stdout, "C: <disconnected>\n");

                    // throw away the buffer since we just need it to hold
                    // one line at a time.
                    free_buffer((void**)&buf);

                    wait_for_new_connection = 1;

                    if (strcmp(buf, "QUIT\n") != 0)
                    	break;	// if the client just sent a dot, wait for new connections.
                    else
                    {
                    	// if client sent the word QUIT all-caps with a newline, this is
                    	// to be treated like a protocol command.  Dispose of the server's
                    	// socket entirely and then exit this process.
                    	quit_server();
                    	exit(OK);
                    }
                }   

                // echo received content back
                bytes = SocketDemoUtils_send(client_socket, buf);
                if (bytes < 0)
                {
                    log_error("server: Send failed.");
                    exit(ERROR);
                }

                bytesSent += bytes;

                fprintf(stdout, "S: %s", buf);

                // throw away the buffer since we just need it to hold
                // one line at a time.
                free_buffer((void**)&buf);
            }
        }

        if (wait_for_new_connection)
        {
        	continue;
        }
		// Let the 'outer' while loop start over again, waiting to accept new
		// client connections.  User must close the server 'by hand'
		// but we want the server to remain 'up' as long as possible,
		// just in case that more clients want to connect
	}

	log_info("server: Execution finished with no errors.");

	return OK;
}
