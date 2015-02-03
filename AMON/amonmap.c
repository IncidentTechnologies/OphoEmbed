#include "amonmap.h"
#include "..\DS\list.h"

AMONNode *CreateAMONNode(int numLinks, int ID) {
	AMONNode *newNode = (AMONNode*)calloc(1, sizeof(AMONNode));
	newNode->m_id = ID;
	newNode->m_links = (AMONNode**)calloc(numLinks, sizeof(AMONNode*));
	newNode->m_links_n = numLinks;
	return newNode;
}

AMONMap *CreateAMONMap(int numLinks, int rootID) {
	AMONMap *newMap = (AMONMap*)calloc(1, sizeof(AMONMap));
	newMap->m_links = numLinks;
	newMap->m_root = CreateAMONNode(numLinks, rootID);
	newMap->m_count = 0;
	newMap->m_mapID = rootID + 1;
	return newMap;
}

RESULT AddAMONNode(AMONMap* map, int destID, int linkID, int ID) {
	RESULT r = R_OK;

	CBRM((linkID < map->m_links), "AddAMONNode: Can't link on %d, max links %d", linkID, map->m_links - 1);

	AMONNode *destNode = FindAMONNode(map, destID);
	CNRM(destNode, "AddAMONNode: Failed to find destination node %d", destID);

	CBRM((destNode->m_links[linkID] == NULL), "AddAMONNode: Failed to add node, link %d not unassigned", linkID);

	// If all tests passed, we can add the link
	destNode->m_links[linkID] = CreateAMONNode(map->m_links, ID);
	map->m_count++;
	map->m_mapID++;

Error:
	return r;
}

AMONNode *FindAMONNode(AMONMap *map, int id) {
	AMONNode *node = NULL;
	int i = 0;

	// Use breadth first search, push all children into a queue, then iterate and repeat
	list *searchQueue = CreateList();
	PushItem(searchQueue, (void*)(map->m_root));

	while(searchQueue->m_count > 0) {
		AMONNode *tempNode = (AMONNode*)(PopFrontItem(searchQueue));
		if(tempNode->m_id == id) {
			node = tempNode;
			break;
		}
		else {
			for(i = 0; i < map->m_links; i++) {
				if(tempNode->m_links[i] != NULL)
					PushItem(searchQueue, (void*)(tempNode->m_links[i]));
			}
		}
	}

	DeleteList(searchQueue);
	return node;
}

int GetNumberOfMapLinks(AMONNode* node, int id, int linkID, int depth) {
	if(node->m_links[linkID]->m_id == id)
		return depth;
	else if(node->m_links[linkID] == NULL)
		return 0;
	else
		return GetNumberOfMapLinks(node->m_links[linkID], id, linkID, depth + 1);
}

int GetNumberOfEastWestMapLinks(AMONMap *map, int id, int westLinkId, int eastLinkId) {
	AMONNode *node = NULL;
	int links = 0;

	// Search along only the provised west and east link ids
	if((links = GetNumberOfMapLinks(map->m_root, id, eastLinkId, 1)) != 0)
		return links;
	else if((links = GetNumberOfMapLinks(map->m_root, id, westLinkId, 1)) != 0)
		return links * -1;
	else
		return 0;
}

AMONNode *FindAMONNodeParent(AMONMap *map, int id) {
	AMONNode *node = NULL;
	int i = 0;

	if(map->m_root->m_id == id)
		return NULL;

	// Use breadth first search, push all children into a queue, then iterate and repeat
	list *searchQueue = CreateList();
	PushItem(searchQueue, (void*)(map->m_root));

	while(searchQueue->m_count > 0) {
		AMONNode *tempNode = (AMONNode*)(PopFrontItem(searchQueue));

		// Check all children for the ID
		for(i = 0; i < map->m_links; i++) {
			if(tempNode->m_links[i] != NULL && tempNode->m_links[i]->m_id == id){
				node = tempNode;
				break;
			}
		}

		// Push the breadth first children to check them
		for(i = 0; i < map->m_links; i++) {
			if(tempNode->m_links[i] != NULL)
				PushItem(searchQueue, (void*)(tempNode->m_links[i]));
		}
	}

	DeleteList(searchQueue);
	return node;
}

RESULT RemoveAMONNodeByID(AMONMap* map, int ID) {
	RESULT r = R_OK;
	int i = 0;

	// First we need to find the node
	AMONNode *destNodeParent = FindAMONNodeParent(map, ID);
	CNRM(destNodeParent, "RemoveAMONNode: Node id %d not found in AMON map", ID);

	// Remove the child, then set the link to NULL
	for(i = 0; i < destNodeParent->m_links_n; i++) {
		if(destNodeParent->m_links[i] != NULL && destNodeParent->m_links[i]->m_id == ID){
			CRM(RemoveAMONNode(destNodeParent->m_links[i]), "RemoveAMONNodeByID: Failed to remove node %d", ID);
			destNodeParent->m_links[i] = NULL;
		}
	}

Error:
	return r;
}

RESULT RemoveAMONNode(AMONNode* node) {
	RESULT r = R_OK;
	int i = 0;

	if(node == NULL)
		return R_OK;

	// delete all children nodes first
	for(i = 0; i < node->m_links_n; i++)
		RemoveAMONNode(node->m_links[i]);

	free(node);
	node = NULL;

Error:
	return r;
}

typedef struct {
	AMONNode *node;
	int linkID;
	int devID;
} AMON_MAP_NODE_INFO;

RESULT PrintAMONMap(Console *pc, AMONMap *map) {
	RESULT r = R_OK;
	int i = 0;

	list *searchQueue = CreateList();
	AMON_MAP_NODE_INFO *nodeInfo = (AMON_MAP_NODE_INFO*)calloc(1, sizeof(AMON_MAP_NODE_INFO));
	nodeInfo->node = map->m_root;
	nodeInfo->linkID = -1;
	nodeInfo->devID = -1;

	PushItem(searchQueue, (void*)(nodeInfo));
	nodeInfo = NULL;

	while(searchQueue->m_count > 0) {
		//AMONNode *tempNode = (AMONNode*)(PopFrontItem(searchQueue));
		nodeInfo = (AMON_MAP_NODE_INFO*)(PopFrontItem(searchQueue));

		if(nodeInfo->devID == -1)
			PrintToOutput(pc, "%d: Master Node id %d", i, nodeInfo->node->m_id);
		else
			PrintToOutput(pc, "%d: Node id %d connected to device %d on link %d", i, nodeInfo->node->m_id, nodeInfo->devID, nodeInfo->linkID);

		for(i = 0; i < map->m_links; i++) {
			if(nodeInfo->node->m_links[i] != NULL) {
				AMON_MAP_NODE_INFO *newNodeInfo = (AMON_MAP_NODE_INFO*)calloc(1, sizeof(AMON_MAP_NODE_INFO));
				newNodeInfo->node = nodeInfo->node->m_links[i];
				newNodeInfo->linkID = i;
				newNodeInfo->devID = nodeInfo->node->m_id;
				PushItem(searchQueue, (void*)(newNodeInfo));
			}
		}

		free(nodeInfo);
		nodeInfo = NULL;
	}

	CRM_NA(DeleteList(searchQueue), "PrintAMONMap: Failed to delete list");

Error:
	return r;
}


// Console test functions
AMONMap *g_amonmap;

RESULT TestCreateAMONMap(Console *pc, char *pszLinks_n) {
	RESULT r = R_OK;

	int links_n = atoi(pszLinks_n);
	g_amonmap = CreateAMONMap(links_n, 0);

Error:
	return r;
}

RESULT TestAddAMONNode(Console *pc, char *pszDestID, char *pszLinkID) {
	RESULT r = R_OK;

	int destID = atoi(pszDestID);
	int linkID = atoi(pszLinkID);

	CRM(AddAMONNode(g_amonmap, destID, linkID, g_amonmap->m_mapID), "TestAddAMONNode: Failed to add node to node %d at link %d", destID, linkID);

Error:
	return r;
}

RESULT TestRemoveAMONNode(Console *pc, char *pszID) {
	RESULT r = R_OK;

	int id = atoi(pszID);
	CRM(RemoveAMONNodeByID(g_amonmap, id), "TestRemoveAMONNode: Failed to remove node %d", id);

Error:
	return r;
}

RESULT TestPrintAMONMap(Console *pc) {
	return PrintAMONMap(pc, g_amonmap);
}

RESULT TestNumberOfLinks(Console *pc, char *pszID, char *pszLinkID) {
	RESULT r = R_OK;

	int id = atoi(pszID);
	int linkID = atoi(pszLinkID);
	int links = GetNumberOfMapLinks(g_amonmap->m_root, id, linkID, 1);

	if(links != 0) {
		DEBUG_LINEOUT("TestNumberOfLinks: Node %d found on link ID %d at depth %d", id, linkID, links);
	}
	else {
		DEBUG_LINEOUT("TestNumberOfLinks: Node %d not found on link ID %d", id, linkID);
	}

Error:
	return r;
}

RESULT TestAMONMap(Console *pc) {
	RESULT r = R_OK;

	AddConsoleFunctionByArgs(g_pConsole, TestCreateAMONMap, "CreateAMONMap", 2, 0);
	AddConsoleFunctionByArgs(g_pConsole, TestAddAMONNode, "AddAMONNode", 3, 0);
	AddConsoleFunctionByArgs(g_pConsole, TestRemoveAMONNode, "RemoveAMONNode", 2, 0);
	AddConsoleFunctionByArgs(g_pConsole, TestPrintAMONMap, "PrintAMONMap", 1, 0);
	AddConsoleFunctionByArgs(g_pConsole, TestNumberOfLinks, "AMONMapNumberOfLinks", 3, 0);

Error:
	return r;
}
