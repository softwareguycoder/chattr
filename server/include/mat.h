/*
 * mat.h
 *
 *  Created on: Feb 3, 2019
 *      Author: bhart
 */

#ifndef INCLUDE_MAT_H_
#define INCLUDE_MAT_H_

/* This is the 'big daddy' thread that accepts all new client connections
 * and then passes each client connection off to its own little 'sub-thread' */
void* MasterAcceptorThread(void* pThreadData);

#endif /* INCLUDE_MAT_H_ */
