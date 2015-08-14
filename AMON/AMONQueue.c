#include "AMONQueue.h"
#include "AMONPHY.h"

list *g_pAMONQueue[NUM_LINKS];
list *g_pAMONPendingQueue[NUM_LINKS];
list *g_pAMONIncomingQueue[NUM_LINKS];

unsigned char g_AMONQueueSending[NUM_LINKS];

// Using this to make sure we send the right number
unsigned char g_NumQueuedPackets[NUM_LINKS];

RESULT InitializeAMONQueue() {
	RESULT r = R_OK;
	int i = 0;

	for(i = 0; i < NUM_LINKS; i++) {
		//CBRM((g_pAMONQueue[i] == NULL), "InitializeAMONQueue: Failed to initialize AMON Queue %d since already initialized", i);

		g_pAMONQueue[i] = NULL;

		g_pAMONQueue[i] = CreateList();
		g_pAMONPendingQueue[i] = CreateList();
		g_pAMONIncomingQueue[i] = CreateList();

		g_AMONQueueSending[i] = FALSE;

		CNRM_NA(g_pAMONQueue[i], "InitializeAMONQueue: Failed to initialize AMON Queue");

		g_NumQueuedPackets[i] = 0;
	}

	DEBUG_LINEOUT_NA("InitializeAMONQueue: AMON Queue initialized")

Error:
	return r;
}

unsigned char NumPacketsInQueue(AMON_LINK link) {
	if(g_pAMONQueue[link] == NULL)
		return 0;

	return g_pAMONQueue[link]->m_count;
}

unsigned char NumPacketsInPendingQueue(AMON_LINK link) {
	if(g_pAMONPendingQueue[link] == NULL)
			return 0;

	return g_pAMONPendingQueue[link]->m_count;
}

unsigned char NumPacketsInIncomingQueue(AMON_LINK link) {
	if(g_pAMONIncomingQueue[link] == NULL)
			return 0;

	return g_pAMONIncomingQueue[link]->m_count;
}

RESULT PushAMONQueuePacket(AMON_LINK link, AMONPacket *pAMONPacket) {
	RESULT r = R_OK;

	CNRM(g_pAMONQueue[link], "PushAMONQueuePacket: AMON Queue %d not initialized", link);
	CRM(PushItem(g_pAMONQueue[link], (void*)(pAMONPacket)), "PushAMONQueuePacket: Failed to push packet link %d", link);

Error:
	return r;
}

RESULT PushAMONPendingQueuePacket(AMON_LINK link, AMONPacket *pAMONPacket) {
	RESULT r = R_OK;

	CNRM(g_pAMONQueue[link], "PushAMONQueuePacket: AMON Queue %d not initialized", link);
	CBRM((g_pAMONPendingQueue[link]->m_count <= MAX_PENDING_QUEUE_PACKETS), "PushAMONQueuePacket: Pending packets maxed out at %d pending packets", MAX_PENDING_QUEUE_PACKETS);
	CRM(PushItem(g_pAMONQueue[link], (void*)(pAMONPacket)), "PushAMONQueuePacket: Failed to push packet link %d", link);

Error:
	return r;
}

RESULT PushAMONIncomingQueuePacket(AMON_LINK link, AMONPacket *pAMONPacket) {
	RESULT r = R_OK;

	CNRM(g_pAMONIncomingQueue[link], "PushAMONIncomingQueuePacket: AMON Incoming Queue %d not initialized", link);
	CRM(PushItem(g_pAMONIncomingQueue[link], (void*)(pAMONPacket)), "PushAMONIncomingQueuePacket: Failed to push packet link %d", link);

Error:
	return r;
}

AMONPacket *PopAMONIncomingQueuePacket(AMON_LINK link) {
	AMONPacket *pAMONPacket = (AMONPacket *)(PopFrontItem(g_pAMONIncomingQueue[link]));
	return pAMONPacket;
}

RESULT SendAMONQueue(AMON_LINK link) {
	RESULT r = R_OK;

	CNRM(g_pAMONQueue[link], "SendAMONQueue: AMON Queue %d not initialized", link);

	while(g_pAMONQueue[link]->m_count > 0) {

		if(g_AMONLinkPhyPacketCount[link] == 0) {
			DEBUG_LINEOUT("ERROR: We're trying to send more than the original number we wanted to send on link %d", link);
			goto Error;
		}

		if(g_pAMONQueue[link]->m_count > 1) {
			CRM_NA(DelayPHY(), "SendAMONQueue: Failed to delay PHY");
		}

		AMONPacket *pAMONPacket = (AMONPacket*)PopFrontItem(g_pAMONQueue[link]);

		// Note that SendAMONPacket will deallocate the packet memory after it's been sent
		if((r = SendAMONPacket(link, pAMONPacket)) != R_OK) {
			if(pAMONPacket != NULL) {
				free(pAMONPacket);
				pAMONPacket = NULL;
			}

			CRM(R_FAIL, "SendAMONQueue: Failed to SendAMONPacket length %d", pAMONPacket->m_length);
		}

		g_AMONLinkPhyPacketCount[link]--;
	}

Error:
	g_AMONQueueSending[link] = FALSE;
	return r;
}

RESULT PushPendingQueue(AMON_LINK link) {
	RESULT r = R_OK;

	CNRM(g_pAMONQueue[link], "SendAMONQueue: AMON Queue %d not initialized", link);
	CNRM(g_pAMONPendingQueue[link], "SendAMONQueue: AMON Pending Queue %d not initialized", link);

	while(g_pAMONPendingQueue[link]->m_count > 0 && g_pAMONQueue[link]->m_count <= MAX_SEND_QUEUE_PACKETS) {
		AMONPacket *pAMONPacket = (AMONPacket*)PopFrontItem(g_pAMONPendingQueue[link]);
		CRM(PushItem(g_pAMONQueue[link], (void*)(pAMONPacket)), "PushPendingQueue: Failed to push packet link %d", link);
	}

Error:
	return r;
}

RESULT PushAndTransmitAMONQueuePacket(AMON_LINK link, AMONPacket *pAMONPacket) {
	RESULT r = R_OK;

	if(g_AMONQueueSending[link] == FALSE && g_AMONLinkPhys[link] == AMON_PHY_READY ) {
		// Push the packet
		CRM(PushAMONQueuePacket(link, pAMONPacket), "PushAndTransmitAMONQueuePacket: Failed to push AMON Queue packet link %d", link);

		// This will send the request transmit byte - then the PHY will actually send the AMON Queue
		CRM(SendInitiateRequest(link),
				"PushAndTransmitAMONQueuePacket: Failed to Send Initiate Request on link %d", link);
	}
	else {
		CNRM(g_pAMONPendingQueue[link], "PushAndTransmitAMONQueuePacket: AMON Pending Queue %d not initialized", link);
		CRM(PushItem(g_pAMONPendingQueue[link], (void*)(pAMONPacket)), "PushAMONQueuePacket: Failed to push packet link %d", link);

		//CRM(PushAMONPendingQueuePacket(link, (void*)(pAMONPacket)), "PushAMONQueuePacket: Failed to push pending packet on link %d", link);
		//DEBUG_LINEOUT("%d Messages pending on link %d queue sending %d link state %d", g_pAMONPendingQueue[link]->m_count, link, g_AMONQueueSending[link], g_AMONLinkPhys[link]);
	}

Error:
	return r;
}

RESULT HandleAMONIncomingQueue(AMON_LINK link) {
	RESULT r = R_OK;

	while(g_pAMONIncomingQueue[link]->m_count > 0) {
		AMONPacket *pAMONPacket = PopAMONIncomingQueuePacket(link);

		if(HandleAMONPacket(link, pAMONPacket) != R_OK)
			DEBUG_LINEOUT("HandleAMONIncomingQueue: Failed to handle incoming queue packet link %d", link);
	}

Error:
	return r;
}




