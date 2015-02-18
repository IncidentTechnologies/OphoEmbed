#include "list.h"

// List Node
listNode *CreateListNode(int id, void *pItem) {
	listNode *newNode = (listNode*)calloc(1, sizeof(listNode));
	newNode->m_id = id;
	newNode->m_pItem = pItem;

	return newNode;
}

RESULT DeleteListNode(listNode *node) {
	if(node != NULL) {
		if(node->m_pItem != NULL) {
			free(node->m_pItem);
			node->m_pItem = NULL;
		}

		free(node);
		node = NULL;
	}

	return R_OK;
}

// List Functions
list *CreateList() {
	list *newList = (list*)calloc(1, sizeof(list));
	return newList;
}

RESULT DeleteList(list *list) {
	RESULT r = R_OK;

	if(list != NULL) {
		listNode *tempNode = list->m_first;
		while(tempNode != NULL) {
			listNode *tempDel = tempNode;
			tempNode = tempNode->m_next;
			CRM_NA(DeleteListNode(tempDel), "DeleteList: Failed to delete node");
		}

		free(list);
		list = NULL;
	}

Error:
	return r;
}


// Append item to start of list
RESULT AppendItem(list *list, void *pItem) {
	RESULT r = R_OK;

	listNode *newNode = CreateListNode(list->m_idCount++, pItem);

	if(list->m_count == 0) {
		list->m_first = newNode;
		list->m_last = newNode;
	}
	else {
		newNode->m_next = list->m_first;
		list->m_first->m_prev = newNode;
		list->m_first = newNode;
	}

	list->m_count++;

Error:
	return r;
}

// Push item to end of list
RESULT PushItem(list *list, void *pItem) {
	RESULT r = R_OK;

	listNode *newNode = CreateListNode(list->m_idCount++, pItem);

	if(list->m_count == 0) {
		list->m_first = newNode;
		list->m_last = newNode;
	}
	else {
		newNode->m_prev = list->m_last;
		list->m_last->m_next = newNode;
		list->m_last = newNode;
	}

	list->m_count++;

Error:
	return r;
}

// Pop item off end
void *PopItem(list *list) {
	listNode *popNode = list->m_last;
	list->m_last = popNode->m_prev;
	list->m_last->m_next = NULL;

	if(list->m_last == NULL)
		list->m_first = NULL;

	// Retain item, delete node
	void *pItem = popNode->m_pItem;
	free(popNode);
	popNode = NULL;

	list->m_count--;

	// Free the node
	if(popNode != NULL) {
		free(popNode);
		popNode = NULL;
	}

	return pItem;
}

// Pop the front item
void *PopFrontItem(list *list) {
	listNode *popNode = list->m_first;
	list->m_first = popNode->m_next;
	list->m_first->m_prev = NULL;

	if(list->m_first == NULL)
		list->m_last = NULL;

	// Retain item, delete node
	void *pItem = popNode->m_pItem;
	free(popNode);
	popNode = NULL;

	list->m_count--;

	// Free the node
	if(popNode != NULL) {
		free(popNode);
		popNode = NULL;
	}

	return pItem;
}

RESULT TestList(Console *pc, char *pszCount_n) {
	RESULT r = R_OK;
	int i = 0;
	int popped_n = 0;

	int count_n = atoi(pszCount_n);	// TODO: Fix the arguments

	PrintToOutput(pc, "Testing Linked List");

	list *newList = CreateList();
	CNRM_NA(newList, "TestList: Failed to create list");

	for(i = 0; i < count_n; i++) {
		char *pszTemp = (char *)calloc(25, sizeof(char));
		sprintf(pszTemp, "%d", i);
		CRM(PushItem(newList, (void*)pszTemp), "Failed to push %s", pszTemp);
		//PrintToOutput(pc, "Pushed: %s new count: %d", pszTemp, newList->m_count);
	}

	PrintToOutput(pc, "Pushed: %d items new count: %d", i, newList->m_count);

	for(i = newList->m_count; i > 0; i--) {
		char* temp = (char*)PopItem(newList);
		//PrintToOutput(pc, "Popped: %s new count: %d", temp, newList->m_count);
		free(temp);
		popped_n++;
	}

	PrintToOutput(pc, "Popped: %d items new count: %d", popped_n, newList->m_count);

	CRM_NA(DeleteList(newList), "TestList: Failed to delete list");
Error:
	return r;
}


