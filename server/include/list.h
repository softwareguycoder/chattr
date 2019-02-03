#ifndef LIST_H_
#define LIST_H_

typedef struct _root {

	struct _list* head;
	struct _list* tail;  // tail is only valid for the head node
} root;

typedef struct _list {

	//struct _list* head;
	//struct _list* tail;//tail is only valid for the head node
	root* listRoot;
	struct _list* prev;
	struct _list* next;

	void* data;

} POSITION;

typedef int (*LPCOMPARE_ROUTINE)(void*, void*);
typedef void (*doFunction)(void*);
POSITION* initializeList(void* data);
int addMember(POSITION** listHead, void* data);
POSITION* FindMember(POSITION** listHead, void* value,
		LPCOMPARE_ROUTINE lpfnCompare);
POSITION* GetHeadPosition(POSITION** listMember);
POSITION* GetTailPosition(POSITION** listMember);
int removeMember(POSITION** listHead, void* value,
		LPCOMPARE_ROUTINE lpfnCompare);
int removeHead(POSITION** listHead);
int removeTail(POSITION** listHead);
void destroyList(POSITION** listHead, doFunction func);

#endif /* LIST_H_*/
