/*
 * root.h
 *
 *  Created on: Feb 3, 2019
 *      Author: bhart
 */

#ifndef INCLUDE_ROOT_H_
#define INCLUDE_ROOT_H_

typedef struct _tagROOT {

	POSITION* head;
	POSITION* tail;  // tail is only valid for the head node
} ROOT;

#endif /* INCLUDE_ROOT_H_ */
