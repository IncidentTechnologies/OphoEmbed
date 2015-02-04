//#include "SIRDebug.h"	// Required for console functions

#include "AMON.h"
#include "AMONPHY.h"
#include "AMONNet.h"	// Needed to funnel bytes to HandleAMONByte

AMON_PHY_STATE g_AMONLinkPhys[NUM_LINKS];

cbSendByteOnLink g_PHYSendByteCallbacks[NUM_LINKS];
cbFlushLink g_PHYFlushCallbacks[NUM_LINKS];
cbLinkPHYBusy g_PHYBusyCallbacks[NUM_LINKS];

RESULT InitAMONPHY() {
	RESULT r = R_OK;
	int i = 0;

	for(i = 0; i < NUM_LINKS; i++) {
		g_PHYSendByteCallbacks[i] = NULL;
		g_PHYFlushCallbacks[i] = NULL;
	}

Error:
	return r;
}

RESULT RegisterLinkSendByteCallback(AMON_LINK link, cbSendByteOnLink cbSendByte) {
	RESULT r = R_OK;

	CBRM((g_PHYSendByteCallbacks[link] == NULL), "RegisterLinkSendByteCallback: Failed to register send byte callback on link %d", link);
	g_PHYSendByteCallbacks[link] = cbSendByte;

Error:
	return r;
}

RESULT UnregisterLinkSendByteCallback(AMON_LINK link) {
	RESULT r = R_OK;

	CBRM((g_PHYSendByteCallbacks[link] != NULL), "UnegisterLinkSendByteCallback: Failed to unregister send byte callback on link %d", link);
	g_PHYSendByteCallbacks[link] = NULL;

Error:
	return r;
}

RESULT RegisterLinkBusyCallback(AMON_LINK link, cbLinkPHYBusy cbBusy){
	RESULT r = R_OK;

	CBRM((g_PHYBusyCallbacks[link] == NULL), "RegisterLinkBusyCallback: Failed to register busy callback on link %d", link);
	g_PHYBusyCallbacks[link] = cbBusy;

Error:
	return r;
}

RESULT UnregisterLinkBusyCallback(AMON_LINK link) {
	RESULT r = R_OK;

	CBRM((g_PHYBusyCallbacks[link] != NULL), "UnegisterLinkBusyCallback: Failed to unregister busy callback on link %d", link);
	g_PHYBusyCallbacks[link] = NULL;

Error:
	return r;
}

RESULT RegisterLinkFlushCallback(AMON_LINK link, cbFlushLink cbFlush) {
	RESULT r = R_OK;

	CBRM((g_PHYFlushCallbacks[link] == NULL), "RegisterLinkFlushCallback: Failed to register flush callback on link %d", link);
	g_PHYFlushCallbacks[link] = cbFlush;

Error:
	return r;
}

RESULT UnregisterLinkFlushCallback(AMON_LINK link) {
	RESULT r = R_OK;

	CBRM((g_PHYFlushCallbacks[link] != NULL), "UnegisterLinkFlushCallback: Failed to unregister flush callback on link %d", link);
	g_PHYFlushCallbacks[link] = NULL;

Error:
	return r;
}

RESULT AMONReceiveByte(AMON_LINK link, unsigned char byte) {
	RESULT r = R_OK;

//	SetLEDWithClearTimeout(1, 0, 50, 0, 10);

	// No Link has yet to be established on this link
	//if(g_amon.links[link].id == - 1 && g_AMONLinkPhys[link] != AMON_PHY_ESTABLISHED) {

	// IF PHY established then go into AMON layer, otherwise go into byte layer
	if(g_AMONLinkPhys[link] != AMON_PHY_ESTABLISHED) {
		CRM(AMONHandlePHYByte(link, byte), "AMONReceiveByte: Failed to handle PHY byte on link %d", link);
	}
	else {
		CRM(HandleAMONByte(link, byte), "AMONReceiveByte: Failed to handle AMON byte on link %d", link);
	}

Error:
	return r;
}

RESULT AMONErrorLink(AMON_LINK link) {
	RESULT r = R_OK;

	g_AMONLinkPhys[link] = AMON_PHY_UNINITIALIZED;
	CRM_NA(SendByte(link, AMON_BYTE_ERROR), "AMONErrorLink: Failed to send Error byte");

Error:
	return R_ERROR;
}

RESULT AMONHandlePHYByte(AMON_LINK link, unsigned char byte) {
	RESULT r = R_OK;

	// This establishes the PHY link
	switch(byte) {
		case AMON_BYTE_LINK_REQUEST: {
			if(g_AMONLinkPhys[link] != AMON_PHY_UNINITIALIZED) {
				DEBUG_LINEOUT("Error: AMONRx: Link requested on state %d on link %d", g_AMONLinkPhys[link], link);
				CRM(AMONErrorLink(link), "AMONRx: Error failed on link %d", link);
			}

			// Make the link available
			DEBUG_LINEOUT("Link requested on link %d, link available", link);
			g_AMONLinkPhys[link] = AMON_PHY_AVAILABLE;
			SendByte(link, AMON_BYTE_LINK_AVAILABLE);
		} break;

		// The device has reset the link, we should as well
		case AMON_BYTE_LINK_RESET: {
			CRM(ResetLink(link), "AMONHandlePHYByte: Link %d failed to be reset", link);
			g_AMONLinkPhys[link] = AMON_PHY_UNINITIALIZED;

			SendByte(link, AMON_BYTE_LINK_RESET_ACK);
		} break;

		case AMON_BYTE_LINK_RESET_ACK: {
			DEBUG_LINEOUT("AMONHandlePHYByte: Link %d reset received ACK", link);
		} break;

		case AMON_BYTE_LINK_AVAILABLE: {
			if(g_AMONLinkPhys[link] != AMON_PHY_REQUESTED) {
				DEBUG_LINEOUT("Error: AMONRx: Link available on state %d on link %d", g_AMONLinkPhys[link], link);
				CRM(AMONErrorLink(link), "AMONRx: Error failed on link %d", link);
			}

			// Accept the link
			DEBUG_LINEOUT("Link available on link %d, accepting link", link);
			g_AMONLinkPhys[link] = AMON_PHY_ACCEPTED;
			SendByte(link, AMON_BYTE_LINK_ACCEPTED);
		} break;

		case AMON_BYTE_LINK_ACCEPTED: {
			if(g_AMONLinkPhys[link] != AMON_PHY_AVAILABLE) {
				DEBUG_LINEOUT("Error: AMONRx: Link accepted on state %d in link %d", g_AMONLinkPhys[link], link);
				CRM(AMONErrorLink(link), "AMONRx: Error failed on link %d", link);
			}

			// Accept the link
			DEBUG_LINEOUT("Link accepted sent on link %d, establishing link", link);
			g_AMONLinkPhys[link] = AMON_PHY_ESTABLISHED;
			SendByte(link, AMON_BYTE_LINK_ESTABLISHED);
		} break;

		case AMON_BYTE_LINK_ESTABLISHED: {
			if(g_AMONLinkPhys[link] != AMON_PHY_ACCEPTED) {
				DEBUG_LINEOUT("Error: AMONRx: Link established on state %d on link", g_AMONLinkPhys[link], link);
				CRM(AMONErrorLink(link), "AMONRx: Error failed on link %d", link);
			}

			DEBUG_LINEOUT("Link established on link %d, get device ID", link);
			g_AMONLinkPhys[link] = AMON_PHY_ESTABLISHED;

			// PHY Link established, launch into AMON handshake
			CRM(SendGetDeviceID(link), "AMONRx: Failed to Send Get Device ID on link %d", link);
			g_AMONLinkStates[link] = AMON_LINK_ID_REQUESTED;
		} break;

		case AMON_BYTE_PING: {
			// TODO
//			SetLEDLinkClearTimeout(link, 50, 0, 0, 50);
			SendByte(link, AMON_BYTE_ECHO);		// respond with an echo
		} break;

		case AMON_BYTE_ECHO: {
//			SetLEDLinkClearTimeout(link, 0, 50, 0, 50);
			DEBUG_LINEOUT("Echo received on link %d, requesting link", link);

			// If we get an echo and the link is uninitialized
			if(g_AMONLinkPhys[link] == AMON_PHY_UNINITIALIZED) {
				// Request the link
				g_AMONLinkPhys[link] = AMON_PHY_REQUESTED;
				SendByte(link, AMON_BYTE_LINK_REQUEST);
			}

		} break;

		// If we get a link error, reset this link so we can try again
		case AMON_BYTE_ERROR: {
			g_AMONLinkPhys[link] = AMON_PHY_UNINITIALIZED;
			DEBUG_LINEOUT("Error received on link %d, resetting the link", link);
		} break;
	}

Error:
	return r;
}

RESULT BroadcastByte(unsigned char byte) {
	RESULT r = R_OK;

	CRM_NA(SendByte(AMON_NORTH, byte), "Broadcast failed on north link");
	CRM_NA(SendByte(AMON_SOUTH, byte), "Broadcast failed on south link");
	CRM_NA(SendByte(AMON_EAST, byte), "Broadcast failed on east link");
	CRM_NA(SendByte(AMON_WEST, byte), "Broadcast failed on west link");

Error:
	return r;
}

RESULT SendByte(AMON_LINK link, unsigned char byte) {
	RESULT r = R_OK;

	CBRM((g_PHYSendByteCallbacks[link] != NULL), "SendByte: Failed to send byte on link %d, cb not present", link);

	CRM(g_PHYSendByteCallbacks[link](byte), "SendByte: Link %d send byte callback failed", link);

Error:
	return r;
}

unsigned char LinkBusy(AMON_LINK link) {
	if((g_PHYSendByteCallbacks[link] == NULL)) {
		DEBUG_LINEOUT("LinkBusy: Link busy callback for link %d is NULL", link);
	}
	else {
		return g_PHYBusyCallbacks[link]();
	}

	return FALSE;
}

RESULT FlushPHY(AMON_LINK link) {
	RESULT r = R_OK;

	CBRM((g_PHYFlushCallbacks[link] != NULL), "FlushPHY: Flush callback on link %d not present", link);

	CRM(g_PHYFlushCallbacks[link](), "FlushPHY: Link %d flush phy callback failed", link);

Error:
	return r;
}

RESULT SendPing(AMON_LINK link) {
	return SendByte(link, AMON_BYTE_PING);
}

RESULT SendEcho(AMON_LINK link) {
	return SendByte(link, AMON_BYTE_ECHO);
}
