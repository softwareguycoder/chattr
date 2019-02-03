#ifndef LIST_H_
#define LIST_H_
    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <errno.h>
    #include <string.h>
    #include <sys/types.h>
    #include <time.h>

    #define IPADDRLEN   20

    typedef struct _root {


        struct _list * head;
        struct _list * tail;  // tail is only valid for the head node
    } root;

    typedef struct _list {


        //struct _list * head;
        //struct _list * tail;//tail is only valid for the head node
        root * listRoot;
        struct _list * prev;
        struct _list * next;


        void * data;


    } list;

    typedef struct _clientStruct{


        char ipAddr[IPADDRLEN];
        char * userName;
        int sockFD;


    }clientStruct;


    typedef int (* compare_func)(void*, void*);
    typedef void(* doFunction)(void*);
    list * initializeList(void * data);
    int addMember(list ** listHead, void * data);
    list * find_member(list ** listHead, void * value, compare_func cmp_func);
    list * getHead(list ** listMember);
    list * getTail(list ** listMember);
    int removeMember(list ** listHead, void * value, compare_func cmp_func);
    int removeHead(list ** listHead);
    int removeTail(list ** listHead);
    void destroyList(list ** listHead, doFunction func);
    clientStruct * createClientStruct(int client_sock, char * clientIPAddr);
    int cmpFunc(void * client_socket, void * client_Structure);
    int min(int a, int b);


#endif /* LIST_H_ */
