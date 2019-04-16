// root.h - Defines the ROOT structure which comprises part of the doubly-linked
// list of clients.  The POSITION reference is an individual node of the list.
//

#ifndef __ROOT_H__
#define __ROOT_H__

/* Forward declaration of POSITION structure */
typedef struct _tagPOSITION POSITION;

typedef struct _tagROOT {
	POSITION* pHead;
	POSITION* pTail;  // tail is only valid for the head node
} ROOT;

#endif /* __ROOT_H__ */
