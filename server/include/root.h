/*
 * root.h
 *
 *  Created on: Feb 3, 2019
 *      Author: bhart
 */

#ifndef __ROOT_H__
#define __ROOT_H__

/* Forward declaration of POSITION structure */
typedef struct _tagPOSITION POSITION;

typedef struct _tagROOT {

	POSITION* head;
	POSITION* tail;  // tail is only valid for the head node
} ROOT;

#endif /* __ROOT_H__ */
