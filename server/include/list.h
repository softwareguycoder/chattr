#ifndef INCLUDE_LIST_H_
#define INCLUDE_LIST_H_

/* Forward declaration of POSITION node structure */
typedef struct _tagPOSITION POSITION;

typedef struct _tagROOT {

	POSITION* head;
	POSITION* tail;  // tail is only valid for the head node
} ROOT;

typedef BOOL (*LPCOMPARE_ROUTINE)(void*, void*);
typedef void (*LPACTION_ROUTINE)(void*);
POSITION* initializeList(void* data);
int addMember(POSITION** listHead, void* data);
POSITION* FindMember(POSITION** listHead, void* value,
		LPCOMPARE_ROUTINE lpfnCompare);
POSITION* GetHeadPosition(POSITION** listMember);
POSITION* GetTailPosition(POSITION** listMember);
int removeMember(POSITION** listHead, void* value,
		LPCOMPARE_ROUTINE lpfnCompare);
int removeHead(POSITION** listHead);
BOOL RemoveTail(POSITION** listHead);
void destroyList(POSITION** listHead, LPACTION_ROUTINE func);

#endif /* INCLUDE_LIST_H_*/
