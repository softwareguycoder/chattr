/*
 * send_thread_functions.h
 *
 *  Created on: Apr 14, 2019
 *      Author: bhart
 */

#ifndef __SEND_THREAD_FUNCTIONS_H__
#define __SEND_THREAD_FUNCTIONS_H__

#include <../../../threading_core/threading_core/include/threading_core.h>

extern BOOL g_bShouldTerminateSendThread;

/* global handle to the send thread */
extern HTHREAD g_hSendThread;

/**
 * @brief Returns a value that indicates whether the chat program should keep
 * waiting for the user to type lines to send to the server.
 * @param pszCurLine Address of a buffer containing the text that was just
 * sent to the server.
 * @return TRUE if the sending thread should be stopped; FALSE otherwise.
 */
BOOL ShouldKeepSending(const char* pszCurLine);

/**
 * @brief Handles the SIGSEGV singal and determines whether to terminate the
 * sending thread.
 * @param signum Signal code from the operating system.
 */
void TerminateSendThread(int signum);

#endif /* __SEND_THREAD_FUNCTIONS_H__ */
