#include "AMONQueue.h"

list *g_pAMONQueue[NUM_LINKS];

RESULT InitializeAMONQueue() {
	RESULT r = R_OK;
	int i = 0;

	for(i = 0; i < NUM_LINKS; i++) {
		CBRM((g_pAMONQueue[i] == NULL), "InitializeAMONQueue: Failed to initialize AMON Queue %d since already initialized", i);

		g_pAMONQueue[i] = NULL;
		g_pAMONQueue[i] = CreateList();
		CNRM_NA(g_pAMONQueue[i], "InitializeAMONQueue: Failed to initialize AMON Queue");
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

RESULT PushAMONQueuePacket(AMON_LINK link, AMONPacket *pAMONPacket) {
	RESULT r = R_OK;

	CNRM(g_pAMONQueue[link], "PushAMONQueuePacket: AMON Queue %d not initialized", link);

	CRM(PushItem(g_pAMONQueue[link], (void*)(pAMONPacket)), "PushAMONQueuePacket: Failed to push packet link %d", link);

Error:
	return r;
}

RESULT SendAMONQueue(AMON_LINK link) {
	RESULT r = R_OK;

	CNRM(g_pAMONQueue[link], "SendAMONQueue: AMON Queue %d not initialized", link);

	while(g_pAMONQueue[link]->m_count > 0) {

		if(g_pAMONQueue[link]->m_count > 1) {
			CRM_NA(DelayPHY(), "SendAMONQueue: Failed to delay PHY");
		}

		AMONPacket *pAMONPacket = (AMONPacket*)PopFrontItem(g_pAMONQueue[link]);

		// Note that SendAMONPacket will deallocate the packet memory after it's been sent
		CRM(SendAMONPacket(link, pAMONPacket), "SendAMONQueue: Failed to SendAMONPacket length %d", pAMONPacket->m_length);
	}

Error:
	return r;
}

RESULT PushAndTransmitAMONQueuePacket(AMON_LINK link, AMONPacket *pAMONPacket) {
	RESULT r = R_OK;

	// Push the packet
	CRM(PushAMONQueuePacket(link, pAMONPacket), "PushAndTransmitAMONQueuePacket: Failed to push AMON Queue packet link %d", link);

	// This will send the request transmit byte - then the PHY will actually send the AMON Queue
	CRM(SendRequestTransmit(link, g_pAMONQueue[link]->m_count), "PushAndTransmitAMONQueuePacket: Failed to Send Request Transmit on link %d", link);

Error:
	return r;
}






