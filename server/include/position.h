// position.h - Defines the interface to the POSITION data structure.
//

#ifndef __POSITION_H__
#define __POSITION_H__

/**
 * @brief Defines a node of the linked list and also serves to locate the
 * node.
 */
typedef struct _tagPOSITION {
    ROOT* pListRoot;

    struct _tagPOSITION* pPrev;
    struct _tagPOSITION* pNext;

    void* pvData;
} POSITION;

#endif /* __POSITION_H__ */
