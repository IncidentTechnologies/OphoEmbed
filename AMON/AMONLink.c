#include "SIRDebug.h"	// Required for console functions

#include "AMON.h"
#include "AMONLink.h"
#include "AMONNet.h"	// For the Message Type

AMON_LINK_STATE g_AMONLinkStates[NUM_LINKS];

unsigned char link_input[NUM_LINKS][MAX_MSG_LENGTH];
int link_input_c[NUM_LINKS];

int g_linkMessageLength[NUM_LINKS];

RESULT InitializeLink(AMON_LINK link) {
	RESULT r = R_OK;

	g_amon.links[link].id = -1;
	g_amon.links[link].MasterState = AMON_MASTER_FALSE;
	g_amon.links[link].Status = AMON_DEVICE_UNASSIGNED;
	g_amon.links[link].link_id = -1;

	// Pending status
	g_amon.links[link].fPendingLinkStatus = 0;
	g_amon.links[link].fLinkToMaster = 0;
	g_amon.links[link].LinkStatusCounter = 0;

	// Reset the PHY
	CRM(ResetLink(link), "InitilizeLink: Failed to reset link %d", link);

Error:
	return r;
}

RESULT ResetLink(AMON_LINK link) {
	RESULT r = R_OK;

	DEBUG_LINEOUT("RESET LINK %d", link);

	//CRM(FlushPHY(link), "ResetLink: Failed to flush link %d PHY", link);

	memset(link_input[link][MAX_MSG_LENGTH], 0, sizeof(unsigned char) * MAX_MSG_LENGTH);
	link_input_c[link] = 0;

	g_LinkRxState[link] = AMON_RX_READY;

	g_linkMessageLength[link] = 0;
	g_linkMessageType[link] = AMON_NULL;

	g_AMONLinkPhys[link] = AMON_PHY_UNINITIALIZED;
	g_AMONLinkStates[link] = AMON_LINK_UNINITIALIZED;

Error:
	return R_OK;
}

RESULT DisconnectLink(AMON_LINK link) {
	RESULT r = R_OK;

	// Save the id for later
	int id = g_amon.links[link].id;

	// Did we lose the link to master, reset our AMON setup
	if(g_amon.links[link].fLinkToMaster != 0)
		CRM(InitializeAMON(), "DisconnectLink: Failed to re-initialize AMON on link %d disconnect", link);

	DEBUG_LINEOUT("DisconnectLink: Link %d has been disconnected from device %d, reinitializing link", link, id);
	CRM(InitializeLink(link), "DisconnectLink: Link %d re-initialization failed", link);

	if(g_amon.MasterState != AMON_MASTER_FALSE) {
		CRM(RemoveAMONNodeByID(g_AMONmap, id), "DisconnectLink: Failed to remove node %d from AMON map", id);
	}

Error:
	return r;
}

RESULT CheckLinkStatus(AMON_LINK link) {
	RESULT r = R_OK;

	CBRM((g_AMONLinkStates[link] == AMON_LINK_ESTABLISHED),
			"CheckLinkStatus: Can't check link %d status on unestablished link with status %d", link, g_AMONLinkStates[link]);

#ifdef AMON_VERBOSE
	DEBUG_LINEOUT("CheckLinkStatus: Check link %d for id %d", link, g_amon.links[link].id);
#endif

	if(g_amon.links[link].fPendingLinkStatus != 0) {
		g_amon.links[link].LinkStatusCounter++;
		g_amon.links[link].fPendingLinkStatus = 0;

		if(g_amon.links[link].LinkStatusCounter > LINK_STATUS_COUNTER_THRESHOLD)
			return DisconnectLink(link);
	}

	g_amon.links[link].fPendingLinkStatus = 1;
	CRM(SendPingNetwork(link, g_amon.links[link].id), "CheckLinkStatus: Failed to echo ID to device %d on link %d", g_amon.links[link].id, link);

Error:
	return r;
}

const char* GetLinkStateString(AMON_LINK_STATE state) {
	switch(state) {
		case AMON_LINK_UNINITIALIZED: return "uninitialized"; break;
		case AMON_LINK_ID_REQUESTED: return "id requested"; break;
		case AMON_LINK_ID_SENT: return "id sent"; break;
		case AMON_LINK_ESTABLISHING_LINK: return "establishing"; break;
		case AMON_LINK_ESTABLISHED: return "established"; break;
		case AMON_LINK_MAYBE_DISCONNECTED: return "maybe disconnected"; break;

		default:
		case AMON_LINK_STATE_INVALID: return "invalid"; break;
	}

	return "invalid";
}

RESULT PrintAMONLinkInfo(AMON_LINK link) {
	RESULT r = R_OK;

	DEBUG_LINEOUT("Link %d status %s", link, GetLinkStateString(g_AMONLinkStates[link]));

	if(g_AMONLinkStates[link] == AMON_LINK_ESTABLISHED)
		DEBUG_LINEOUT("Link %d established to device id %d with link id %d", link, g_amon.links[link].id, g_amon.links[link].link_id);

Error:
	return r;
}


