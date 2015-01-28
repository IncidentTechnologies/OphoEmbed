#ifndef LIST_H_
#define LIST_H_

#include "..\Common\RESULT.h"
#include "..\OS\console.h"

extern Console *g_pConsole;

typedef struct LIST_NODE{
	int m_id;
	void *m_pItem;
	struct LIST_NODE *m_next;
	struct LIST_NODE *m_prev;
} listNode;

listNode *CreateListNode(int id, void *pItem);
RESULT DeleteListNode(listNode *node);

typedef struct {
	listNode *m_first;
	listNode *m_last;
	int m_count;
	int m_idCount;
} list;

list *CreateList();
RESULT DeleteList(list *list);
RESULT PushItem(list *list, void *pItem);
RESULT AppendItem(list *list, void *pItem);
void *PopItem(list *list);
void *PopFrontItem(list *list);

// Test List
RESULT TestList(Console *pc, char *pszCount_n);

#endif // LIST_H_
