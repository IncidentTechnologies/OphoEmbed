#ifndef AMONMAP_H_
#define AMONMAP_H_

#include "../Common/RESULT.h"
#include "../OS/Console.h"

typedef struct AMON_NODE{
	int m_id;
	struct AMON_NODE **m_links;
	int m_links_n;
	void *m_pContext;
} AMONNode;

typedef struct {
	int m_links;
	AMONNode *m_root;
	int m_mapID;
	int m_count;
} AMONMap;

AMONMap *CreateAMONMap(int numLinks, int rootID, void *pContext);
AMONNode * CreateAMONNode(int numLinks, int ID, void *pContext);

RESULT AddAMONNode(AMONMap* map, int destID, int linkID, int ID, void *pContext);
RESULT RemoveAMONNodeByID(AMONMap* map, int ID);
RESULT RemoveAMONNode(AMONNode* node);
RESULT ResetAMONNodeLink(AMONMap* map, int destID, int linkID);

int GetNumberOfMapLinks(AMONNode* node, int id, int linkID, int depth);
int GetNumberOfEastWestMapLinks(AMONMap *map, int id, int westLinkId, int eastLinkId);

int GetNodeLinkDepth(AMONNode* node, int linkID, int depth);
int GetDepthOfMapLink(AMONMap *map, int linkID);

// Will traverse the given link to the depth provided and return the node
AMONNode *GetMapNodeIDOnLinkDepth(AMONMap *map, int linkID, int depth);

AMONNode *FindAMONNode(AMONMap *map, int id);
AMONNode *FindAMONNodeParent(AMONMap *map, int id);	// This will acquire the AMON node parent

// Testing
RESULT TestCreateAMONMap(Console *pc, char *pszLinks_n);
RESULT TestAddAMONNode(Console *pc, char *pszDestID, char *pszLinkID);
RESULT TestRemoveAMONNode(Console *pc, char *pszID);
RESULT PrintAMONMap(Console *pc, AMONMap *map);
RESULT TestNumberOfLinks(Console *pc, char *pszID, char *pszLinkID);
RESULT TestAMONMap(Console *pc);

#endif // AMONMAP_H_
