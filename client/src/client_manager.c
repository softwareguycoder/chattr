///////////////////////////////////////////////////////////////////////////////
// client_manager.c - Main client functionality manager functions
//
// This file contains all the routines that are centeral to the user interface
// and operation of the chat client program.
//
// AUTHOR: Brian Hart
// DATE: 8 Apr 2019
//
// Shout out to https://gist.github.com/DnaBoss/a5e1ea07de5ef3525937 for
// code that provided inspiration
//

#include "stdafx.h"
#include "client_manager.h"

///////////////////////////////////////////////////////////////////////////////
// PrintClientUsageDirections function

void PrintClientUsageDirections() {
	log_debug("In PrintClientUsageDirections");

	log_info("PrintClientUsageDirections: Printing the usage directions for the user...");

	/* Print some usage directions */
	fprintf(stdout,
			"\nType the message to send to the server at the '>' prompt, and then press ENTER.\n");
	fprintf(stdout,
			"The server's reply, if any, will be shown with a 'S:' prefix.\n");
	fprintf(stdout,
			"When you have nothing more to say, type a dot ('.') on a line by itself.\n");
	fprintf(stdout, "To exit, type 'exit' or 'quit' and then press ENTER.\n");

	log_info("PrintClientUsageDirections: Usage directions printed.");

	log_debug("PrintClientUsageDirections: Done.");
}
