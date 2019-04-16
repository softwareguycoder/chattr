// mat.h - Defines functions and symbols for the Master Acceptor Thread (MAT).
// The MAT is in charge of continuously waiting for new client conncetions,
// and then, when a new connection comes in, the MAT is to spin off a new
// thread just for that connection.
//

#ifndef __MAT_H__
#define __MAT_H__

// Count of how many currently-connected clients there are
extern int g_nClientCount;

extern BOOL g_bShouldTerminateMasterThread;

/* This is the 'big daddy' thread that accepts all new client connections
 * and then passes each client connection off to its own little 'sub-thread' */
void* MasterAcceptorThread(void* pThreadData);

#endif /* __MAT_H__ */
