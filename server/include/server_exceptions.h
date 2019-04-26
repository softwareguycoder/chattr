/*
 * server_exceptions.h
 *
 *  Created on: Apr 26, 2019
 *      Author: bhart
 */

#ifndef __SERVER_EXCEPTIONS_H__
#define __SERVER_EXCEPTIONS_H__

/**
 * @brief Causes the program to report that a NULL pointer was passed to a
 * function that was not expecting it.
 * @remarks This function writes an error message to STDERR and then shuts
 * the software down gracefully.
 */
void ThrowNullReferenceException();

#endif /* INCLUDE_SERVER_EXCEPTIONS_H_ */
