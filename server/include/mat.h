/*
 * mat.h
 *
 *  Created on: Feb 3, 2019
 *      Author: bhart
 */

#ifndef __MAT_H__
#define __MAT_H__

extern HTHREAD g_hMasterThread;

// Count of how many currently-connected clients there are
extern int nClientCount;

/* This is the 'big daddy' thread that accepts all new client connections
 * and then passes each client connection off to its own little 'sub-thread' */
void* MasterAcceptorThread(void* pThreadData);

#endif /* __MAT_H__ */
