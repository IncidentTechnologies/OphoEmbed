//#include "SIRDebug.h"	// Required for console functions

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

	g_amon.links[link].fLinkTxBusy = 0;
	g_amon.links[link].fLinkRxBusy = 0;

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

	CRM(ResetAMONPhy(link), "Failed to reset AMON Phy %d", link);
	g_AMONLinkStates[link] = AMON_LINK_UNINITIALIZED;

	// Unlock the link
	UnlockAMONLinkRx(link);
	UnlockAMONLinkTx(link);

Error:
	return R_OK;
}

cbAMONLink g_AMONLinkEstablishedCallback = NULL;
RESULT RegisterAMONLinkEstablishedCallback(cbAMONLink AMONLinkEstablishedCB) {
	RESULT r = R_OK;

	CBRM((g_AMONLinkEstablishedCallback == NULL), "RegisterAMONLinkEstablishedCallback: AMON Link Established callback already registered");
	g_AMONLinkEstablishedCallback = AMONLinkEstablishedCB;

Error:
	return r;
}

RESULT UnegisterAMONLinkEstablishedCallback() {
	RESULT r = R_OK;

	CBRM((g_AMONLinkEstablishedCallback != NULL), "UnregisterAMONLinkEstablishedCallback: AMON Link Established callback not registered");
	g_AMONLinkEstablishedCallback = NULL;

Error:
	return r;
}

cbAMONLink g_AMONLinkDisconnectCallback = NULL;
RESULT RegisterAMONLinkDisconnectCallback(cbAMONLink AMONLinkDisconnectCB) {
	RESULT r = R_OK;

	CBRM((g_AMONLinkDisconnectCallback == NULL), "RegisterAMONLinkDisconnectCallback: AMON Link Disconnect callback already registered");
	g_AMONLinkDisconnectCallback = AMONLinkDisconnectCB;

Error:
	return r;
}

RESULT UnegisterAMONLinkDisconnectCallback()  {
	RESULT r = R_OK;

	CBRM((g_AMONLinkDisconnectCallback != NULL), "UnregisterAMONLinkDisconnectCallback: AMON Link Disconnect callback not registered");
	g_AMONLinkDisconnectCallback = NULL;

Error:
	return r;
}

RESULT DisconnectLink(AMON_LINK link) {
	RESULT r = R_OK;

	// Save the id for later
	int destID = g_amon.links[link].id;
	int id = g_amon.id;

	// Did we lose the link to master, reset our AMON setup
	if(g_amon.links[link].fLinkToMaster != 0)
		CRM(InitializeAMON(), "DisconnectLink: Failed to re-initialize AMON on link %d disconnect", link);

	DEBUG_LINEOUT("DisconnectLink: Link %d has been disconnected from device %d, reinitializing link", link, id);
	CRM(InitializeLink(link), "DisconnectLink: Link %d re-initialization failed", link);

	if(g_amon.MasterState != AMON_MASTER_FALSE) {
		//CRM(RemoveAMONNodeByID(g_AMONmap, destID), "DisconnectLink: Failed to remove node %d from AMON map", id);
		CRM(ResetAMONNodeLink(g_AMONmap, id, link), "DisconnectLink: Failed to reset link %d of node %d from AMON map", link, id);
	}

	if(g_AMONLinkDisconnectCallback != NULL)
		g_AMONLinkDisconnectCallback(link);

Error:
	return r;
}

AMON_LINK_STATE GetLinkState(AMON_LINK link) {
	return g_AMONLinkStates[link];
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

		if(g_amon.links[link].LinkStatusCounter > LINK_STATUS_COUNTER_THRESHOLD) {
			return DisconnectLink(link);
		}
	}

	g_amon.links[link].fPendingLinkStatus = 1;
	CRM(SendPingNetwork(link, g_amon.links[link].id), "CheckLinkStatus: Failed to echo ID to device %d on link %d", g_amon.links[link].id, link);

Error:
	return r;
}

AMON_LINK GetMasterLink() {
	int i = 0;

	for(i = 0; i < NUM_LINKS; i++)
		if(g_amon.links[i].fLinkToMaster)
			return (AMON_LINK)(i);

	return AMON_LINK_INVALID;
}

const char* GetLinkStateString(AMON_LINK_STATE state) {
	switch(state) {
		case AMON_LINK_UNINITIALIZED: return "uninitialized";
		case AMON_LINK_ID_REQUESTED: return "id requested";
		case AMON_LINK_ID_SENT: return "id sent";
		case AMON_LINK_ESTABLISHING_LINK: return "establishing";
		case AMON_LINK_ESTABLISHED: return "established";
		case AMON_LINK_MAYBE_DISCONNECTED: return "maybe disconnected";

		default:
		case AMON_LINK_STATE_INVALID: return "invalid";
	}
}

RESULT PrintAMONLinkInfo(AMON_LINK link) {
	RESULT r = R_OK;

	DEBUG_LINEOUT("Link %d status %s phy stat %s", link, GetLinkStateString(g_AMONLinkStates[link]), GetLinkPhyStateString(g_AMONLinkPhys[link]));

	if(g_AMONLinkStates[link] == AMON_LINK_ESTABLISHED) {
		DEBUG_LINEOUT("Link %d established to device id %d with link id %d", link, g_amon.links[link].id, g_amon.links[link].link_id);
		DEBUG_LINEOUT("Link %d message counter %d with length %d packet count %d", link, link_input_c[link], g_linkMessageLength[link], g_AMONLinkPhyPacketCount[link]);
	}

Error:
	return r;
}


