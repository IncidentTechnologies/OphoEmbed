#ifndef AMONMAP_H_
#define AMONMAP_H_

#include "../Common/RESULT.h"
#include "../OS/Console.h"

typedef struct AMON_NODE{
	int m_id;
	struct AMON_NODE **m_links;
	int m_links_n;
} AMONNode;

typedef struct {
	int m_links;
	AMONNode *m_root;
	int m_mapID;
	int m_count;
} AMONMap;

AMONMap *CreateAMONMap(int numLinks, int rootID);
AMONNode * CreateAMONNode(int numLinks, int ID);

RESULT AddAMONNode(AMONMap* map, int destID, int linkID, int ID);
RESULT RemoveAMONNodeByID(AMONMap* map, int ID);
RESULT RemoveAMONNode(AMONNode* node);

AMONNode *FindAMONNode(AMONMap *map, int id);
AMONNode *FindAMONNodeParent(AMONMap *map, int id);	// This will acquire the AMON node parent

RESULT PrintAMONMap(Console *pc, AMONMap *map);

// Testing
RESULT TestAMONMap(Console *pc);

#endif // AMONMAP_H_
