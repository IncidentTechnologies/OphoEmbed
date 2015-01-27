#include "SIRDebug.h"	// Required for console functions

#include "AMON.h"
#include "AMONPHY.h"
#include "AMONNet.h"	// Needed to funnel bytes to HandleAMONByte

AMON_PHY_STATE g_AMONLinkPhys[NUM_LINKS];

RESULT AMONReceiveByte(AMON_LINK link, unsigned char byte) {
	RESULT r = R_OK;

	SetLEDWithClearTimeout(1, 0, 50, 0, 10);

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
			SetLEDLinkClearTimeout(link, 50, 0, 0, 50);
			SendByte(link, AMON_BYTE_ECHO);		// respond with an echo
		} break;

		case AMON_BYTE_ECHO: {
			SetLEDLinkClearTimeout(link, 0, 50, 0, 50);
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
			g_AMONLinkPhys[link] == AMON_PHY_UNINITIALIZED;
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

	switch(link) {
		case AMON_NORTH: ROM_UARTCharPut(UART1_BASE, byte); break;
		case AMON_SOUTH: ROM_UARTCharPut(UART3_BASE, byte); break;
		case AMON_EAST: ROM_UARTCharPut(UART4_BASE, byte); break;
		case AMON_WEST: ROM_UARTCharPut(UART2_BASE, byte); break;
	}

Error:
	return r;
}

RESULT FlushPHY(AMON_LINK link) {
	switch(link) {
		case AMON_NORTH: return FlushUART(UART1_BASE); break;
		case AMON_SOUTH: return FlushUART(UART3_BASE); break;
		case AMON_EAST: return FlushUART(UART4_BASE); break;
		case AMON_WEST: return FlushUART(UART2_BASE); break;
	}
}

RESULT SendPing(AMON_LINK link) {
	return SendByte(link, AMON_BYTE_PING);
}

RESULT SendEcho(AMON_LINK link) {
	return SendByte(link, AMON_BYTE_ECHO);
}
