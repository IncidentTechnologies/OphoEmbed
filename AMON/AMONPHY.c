//#include "SIRDebug.h"	// Required for console functions

#include "AMON.h"
#include "AMONPHY.h"
#include "AMONNet.h"	// Needed to funnel bytes to HandleAMONByte

AMON_PHY_STATE g_AMONLinkPhys[NUM_LINKS];
unsigned char g_AMONLinkPhyPacketCount[NUM_LINKS];

cbSendByteOnLink g_PHYSendByteCallbacks[NUM_LINKS];
cbFlushLink g_PHYFlushCallbacks[NUM_LINKS];
cbLinkPHYBusy g_PHYBusyCallbacks[NUM_LINKS];
cbPHYDelay g_PHYDelayCallback = NULL;

RESULT InitAMONPHY() {
	RESULT r = R_OK;
	int i = 0;

	for(i = 0; i < NUM_LINKS; i++) {
		g_AMONLinkPhys[i] = AMON_PHY_UNINITIALIZED;


		g_PHYSendByteCallbacks[i] = NULL;
		g_PHYFlushCallbacks[i] = NULL;
		g_PHYBusyCallbacks[i] = NULL;

		g_AMONLinkPhyPacketCount[i] = 0;
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

RESULT RegisterLinkDelayCallback(cbPHYDelay cbDelay) {
	RESULT r = R_OK;

	CBRM_NA((g_PHYDelayCallback == NULL), "RegisterLinkDelayCallback: Failed to register delay callback");
	g_PHYDelayCallback = cbDelay;

Error:
	return r;
}

RESULT UnregisterLinkDelayCallback() {
	RESULT r = R_OK;

	CBRM_NA((g_PHYDelayCallback != NULL), "UnegisterLinkDelayCallback: Failed to unregister delay callback");
	g_PHYDelayCallback = NULL;

Error:
	return r;
}

RESULT AMONHandleHalfDuplexPHYByte(AMON_LINK link, unsigned char byte) {
	RESULT r = R_OK;

	switch(g_AMONLinkPhys[link]) {

		// We're in the ready state, sender and receiver
		case AMON_PHY_READY: {
			if((byte & AMON_REQUEST_TRANSMIT) == AMON_REQUEST_TRANSMIT) {
				unsigned char numPackets = (unsigned char)(byte & 0x0F);
				g_AMONLinkPhyPacketCount[link] = numPackets;
				g_AMONLinkPhys[link] = AMON_PHY_ACCEPT_TRANSMIT;

				#ifdef AMON_VERBOSE
					DEBUG_LINEOUT("Transmit request of %d packets received on link %d", numPackets, link);
				#endif

				CRM(SendByte(link, AMON_ACCEPT_TRANSMIT), "AMONHandleHalfDuplexPHYByte: Failed to send accept transmit byte on link %d", link);
			}
			else {
				DEBUG_LINEOUT("Byte 0x%x received on link %d is not request transmit, resetting the link", byte, link);
				CRM(AMONErrorLink(link), "AMONHandlePHYByte: Failed to send error on phy link %d", link);
				CRM(ResetLink(link), "AMONHandlePHYByte: Failed to reset link %d", link);
			}
		} break;

		// Receiver - We've accepted a transmit and are receiving data
		case AMON_PHY_ACCEPT_TRANSMIT: {
			if(g_AMONLinkPhyPacketCount[link] != 0) {
				CRM(HandleAMONByte(link, byte), "AMONHandleHalfDuplexPHYByte: Failed to handle AMON byte on link %d", link);
			}
			else {
				if(byte == AMON_TRANSMIT_COMPLETE) {

					#ifdef AMON_VERBOSE
						DEBUG_LINEOUT("Transmit complete received on link %d state %d, send %d packets, pending %d ", link,
								g_AMONLinkPhys[link], NumPacketsInQueue(link), NumPacketsInPendingQueue(link));
					#endif

					// Now we parse through the packets in the queue

					// Check to see if we have pending packets in the queue otherwise, return transmit complete ACK
					if(NumPacketsInPendingQueue(link) != 0) {
						#ifdef AMON_VERBOSE
							DEBUG_LINEOUT("Transmit complete received on link %d, pending %d packets", link, NumPacketsInQueue(link));
						#endif

						g_AMONLinkPhys[link] = AMON_PHY_TRANSMIT_COMPLETE_PENDING;

						// Transfer the pending queue to the queue
						CRM(PushPendingQueue(link), "AMONHandleHalfDuplexPHYByte: Failed to push pending queue linke %d", link);

						// Send the packets in the queue
						CRM(SendRequestTransmit(link, FALSE), "AMONHandleHalfDuplexPHYByte: Failed to send request transmit on link %d", link);
					}
					else {
						#ifdef AMON_VERBOSE
							DEBUG_LINEOUT("Transmit complete received on link %d", link);
						#endif

						g_AMONLinkPhys[link] = AMON_PHY_READY;
						CRM(SendByte(link, AMON_TRANSMIT_COMPLETE_ACK), "AMONHandleHalfDuplexPHYByte: Failed to send transmit complete ack byte on link %d", link);
					}
				}
				else {
					DEBUG_LINEOUT("Byte 0x%x received on link %d is not transmit complete byte, resetting the link", byte, link);
					CRM(AMONErrorLink(link), "AMONHandlePHYByte: Failed to send error on phy link %d", link);
					CRM(ResetLink(link), "AMONHandlePHYByte: Failed to reset link %d", link);
				}
			}
		} break;

		// Sender - We've sent a request to transmit
		case AMON_PHY_REQUEST_TRANSMIT_RESPONSE:
		case AMON_PHY_REQUEST_TRANSMIT: {
			if(byte == AMON_ACCEPT_TRANSMIT) {

				#ifdef AMON_VERBOSE
					DEBUG_LINEOUT("Accept transmit received on link %d state %d, send %d packets, pending %d ...", link,
							g_AMONLinkPhys[link], NumPacketsInQueue(link), NumPacketsInPendingQueue(link));
				#endif

				g_AMONLinkPhys[link] = AMON_PHY_TRANSMIT_ACTIVE;

				unsigned char numPacketsToSend = NumPacketsInQueue(link);
				// Send the AMON Queue
				CRM(SendAMONQueue(link), "AMONHandleHalfDuplexPHYByte: Failed to send AMON Queue on link %d state %d", link, g_AMONLinkPhys[link]);

				// Queue send is complete
				#ifdef AMON_VERBOSE
					DEBUG_LINEOUT("Queue send complete on link %d packets in queue %d pending %d", link,
							NumPacketsInQueue(link), NumPacketsInPendingQueue(link));
				#endif

				#ifdef AMON_VERBOSE
					DEBUG_LINEOUT("Sending transmit complete on link %d for %d packets", link, numPacketsToSend);
				#endif

				CRM_NA(DelayPHY(), "Failed to delay PHY");
				CRM_NA(DelayPHY(), "Failed to delay PHY");

				CRM(SendByte(link, AMON_TRANSMIT_COMPLETE), "AMONHandleHalfDuplexPHYByte: Failed to send transmit complete byte on link %d state %d", link, g_AMONLinkPhys[link]);
				g_AMONLinkPhys[link] = AMON_PHY_TRANSMIT_COMPLETE;
			}
			else {
				/*
				// TODO: If we get here we should request again
				DEBUG_LINEOUT("Byte 0x%x received on link %d in state %d is not accept transmit, retry", byte, link, g_AMONLinkPhys[link]);
				g_AMONLinkPhys[link] = AMON_PHY_READY;
				*/

				/*
				CRM(SendRequestTransmit(link, NumPacketsInQueue(link), (g_AMONLinkPhys[link] == AMON_PHY_REQUEST_TRANSMIT_RESPONSE) ? 1 : 0),
						"AMONHandleHalfDuplexPHYByte: Failed to retry send on link %d state %d", link, g_AMONLinkPhys[link]);
				*/


				// TODO: CLEAN DA CODE
				// If they're requesting a transmit at the same time as us, and they have a higher ID, let them go first
				// our data will go into a pending state - if we win, wait for a accept response
				if((byte & AMON_REQUEST_TRANSMIT) == AMON_REQUEST_TRANSMIT) {
					if(g_amon.links[link].id > g_amon.id) {
						unsigned char numPackets = (unsigned char)(byte & 0x0F);
						g_AMONLinkPhyPacketCount[link] = numPackets;
						g_AMONLinkPhys[link] = AMON_PHY_ACCEPT_TRANSMIT;

						#ifdef AMON_VERBOSE
							DEBUG_LINEOUT("Simultaneous transmit request of %d packets received on link %d state %d, send accept ...", numPackets, link, g_AMONLinkPhys[link]);
						#endif

						// Need to delay a moment
						CRM_NA(DelayPHY(), "Failed to delay");
						CRM_NA(DelayPHY(), "Failed to delay");

						CRM(SendByte(link, AMON_ACCEPT_TRANSMIT), "AMONHandleHalfDuplexPHYByte: Failed to send accept transmit byte on link %d", link);
					}
					else {
						#ifdef AMON_VERBOSE
							DEBUG_LINEOUT("Byte 0x%x received on link %d in state %d is simultaneous transmit, wait ... ", byte, link, g_AMONLinkPhys[link]);
						#endif
						break;
					}
				}
				else {
					DEBUG_LINEOUT("Byte 0x%x received on link %d in state %d is not accept transmit, reset link", byte, link, g_AMONLinkPhys[link]);
					CRM(AMONErrorLink(link), "AMONHandlePHYByte: Failed to send error on phy link %d state %d", link, g_AMONLinkPhys[link]);
					CRM(ResetLink(link), "AMONHandlePHYByte: Failed to reset link %d state %d", link, g_AMONLinkPhys[link]);
				}
			}
		} break;

		// Sender is sending
		case AMON_PHY_TRANSMIT_ACTIVE: {
			DEBUG_LINEOUT("Byte 0x%x received on link %d during transmission - not allowed, resetting the link", byte, link);
			CRM(AMONErrorLink(link), "AMONHandlePHYByte: Failed to send error on phy link %d", link);
			CRM(ResetLink(link), "AMONHandlePHYByte: Failed to reset link %d", link);
		} break;

		// Sender
		case AMON_PHY_TRANSMIT_COMPLETE: {
			if(byte == AMON_TRANSMIT_COMPLETE_ACK) {
				// Transmission has completed with ACK, link goes back to ready
				#ifdef AMON_VERBOSE
					DEBUG_LINEOUT("Transmit complete ACK received on link %d", link);
				#endif

				// Check to see if we have pending packets in our own queue that have accumulated
				// otherwise, revert to PHY  ready
				if(NumPacketsInPendingQueue(link) != 0) {
					#ifdef AMON_VERBOSE
						DEBUG_LINEOUT("Transmit complete received on link %d, pending %d packets", link, NumPacketsInQueue(link));
					#endif

					g_AMONLinkPhys[link] = AMON_PHY_TRANSMIT_COMPLETE_PENDING;

					// Transfer the pending queue to the queue
					CRM(PushPendingQueue(link), "AMONHandleHalfDuplexPHYByte: Failed to push pending queue linke %d", link);

					// Send the packets in the queue
					CRM(SendRequestTransmit(link, TRUE), "AMONHandleHalfDuplexPHYByte: Failed to send request transmit on link %d", link);
				}
				else {
					g_AMONLinkPhys[link] = AMON_PHY_READY;
				}
			}
			else if((byte & AMON_REQUEST_TRANSMIT) == AMON_REQUEST_TRANSMIT) {
				// They're trying to transmit right after
				unsigned char numPackets = (unsigned char)(byte & 0x0F);
				g_AMONLinkPhyPacketCount[link] = numPackets;
				g_AMONLinkPhys[link] = AMON_PHY_ACCEPT_TRANSMIT;
				#ifdef AMON_VERBOSE
					DEBUG_LINEOUT("Transmit request response of %d packets received on link %d", numPackets, link);
				#endif
				CRM(SendByte(link, AMON_ACCEPT_TRANSMIT), "AMONHandleHalfDuplexPHYByte: Failed to send accept transmit byte on link %d", link);
			}
			else {
				DEBUG_LINEOUT("Byte 0x%x received on link %d is not transmit complete ACK or request response 0x%x, resetting the link", byte, (byte & AMON_REQUEST_TRANSMIT), link);
				CRM(AMONErrorLink(link), "AMONHandlePHYByte: Failed to send error on phy link %d", link);
				CRM(ResetLink(link), "AMONHandlePHYByte: Failed to reset link %d", link);
			}
		} break;

		default: {
			DEBUG_LINEOUT("Byte 0x%x received on link %d is unhandled", byte, link);
		} break;
	}

Error:
	return r;
}

RESULT AMONReceiveByte(AMON_LINK link, unsigned char byte) {
	RESULT r = R_OK;

	// IF PHY established then go into AMON layer, otherwise go into byte layer
	if(g_AMONLinkPhys[link] < AMON_PHY_READY) {
		CRM(AMONHandlePHYByte(link, byte), "AMONReceiveByte: Failed to handle PHY byte on link %d", link);
	}
	else {
#ifdef AMON_HALF_DUPLEX
		CRM(AMONHandleHalfDuplexPHYByte(link, byte), "AMONReceiveByte: Failed to handle half duplex PHY byte on link %d", link);
#else
		CRM(HandleAMONByte(link, byte), "AMONReceiveByte: Failed to handle AMON byte on link %d", link);
#endif
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
			g_AMONLinkPhys[link] = AMON_PHY_READY;
			SendByte(link, AMON_BYTE_LINK_ESTABLISHED);
		} break;

		case AMON_BYTE_LINK_ESTABLISHED: {
			if(g_AMONLinkPhys[link] != AMON_PHY_ACCEPTED) {
				DEBUG_LINEOUT("Error: AMONRx: Link established on state %d on link", g_AMONLinkPhys[link], link);
				CRM(AMONErrorLink(link), "AMONRx: Error failed on link %d", link);
			}

			DEBUG_LINEOUT("Link established on link %d, get device ID", link);
			g_AMONLinkPhys[link] = AMON_PHY_READY;

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
			CRM(ResetLink(link), "AMONHandlePHYByte: Failed to reset link %d", link);
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

RESULT SendRequestTransmit(AMON_LINK link, unsigned char fResponse) {
	RESULT r = R_OK;

	// If the link is not ready, we quit this - the packets are queued and will get picked
	// up on completion
	if(g_AMONLinkPhys[link] == AMON_PHY_READY || g_AMONLinkPhys[link] == AMON_PHY_TRANSMIT_COMPLETE_PENDING)
		g_AMONLinkPhys[link] = (fResponse == 0) ? AMON_PHY_REQUEST_TRANSMIT : AMON_PHY_REQUEST_TRANSMIT_RESPONSE;
	else
		return R_OK;

	g_AMONLinkPhyPacketCount[link] = NumPacketsInQueue(link);
	unsigned char byte = AMON_REQUEST_TRANSMIT + (g_AMONLinkPhyPacketCount[link] & 0x0F);

	DEBUG_LINEOUT("Sending request transmit 0x%x packets on link %d", byte, link);

	CBRM((g_PHYSendByteCallbacks[link] != NULL), "SendRequestTransmit: Failed to send byte on link %d, cb not present", link);
	CRM(g_PHYSendByteCallbacks[link](byte), "SendRequestTransmit: Link %d send byte callback failed", link);

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

RESULT DelayPHY() {
	RESULT r = R_OK;

	CBRM_NA((g_PHYDelayCallback != NULL), "DelayPHY: Delay callback not present");
	CRM_NA(g_PHYDelayCallback(), "DelayPHY: Delay phy callback failed");

Error:
	return r;
}

RESULT SendPing(AMON_LINK link) {
	return SendByte(link, AMON_BYTE_PING);
}

RESULT SendEcho(AMON_LINK link) {
	return SendByte(link, AMON_BYTE_ECHO);
}

const char* GetLinkPhyStateString(AMON_PHY_STATE state) {
	switch(state) {
		case AMON_PHY_UNINITIALIZED: return "uninitialized"; break;
		case AMON_PHY_IDENTIFIED: return "identified"; break;
		case AMON_PHY_REQUESTED: return "requested"; break;
		case AMON_PHY_AVAILABLE: return "available"; break;
		case AMON_PHY_ACCEPTED: return "accepted"; break;
		case AMON_PHY_ASSIGNED: return "assigned"; break;
		case AMON_PHY_MAYBE_DISCONNECTED: return "maybe disconnect"; break;
		case AMON_PHY_READY: return "ready"; break;
		case AMON_PHY_REQUEST_TRANSMIT: return "request transmit"; break;
		case AMON_PHY_ACCEPT_TRANSMIT: return "accept transmit"; break;
		case AMON_PHY_TRANSMIT_ACTIVE: return "transmit active"; break;
		case AMON_PHY_TRANSMIT_COMPLETE: return "transmit complete"; break;
		case AMON_PHY_TRANSMIT_COMPLETE_PENDING: return "transmit complete pending"; break;
		case AMON_PHY_REQUEST_TRANSMIT_RESPONSE: return "request transmit response"; break;

		default:
		case AMON_PHY_STATE_INVALID: return "invalid"; break;
	}

	return "invalid";
}
