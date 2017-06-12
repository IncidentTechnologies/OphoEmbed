//#include "SIRDebug.h"	// Required for console functions

#include "AMON.h"
#include "AMONPHY.h"
#include "AMONNet.h"	// Needed to funnel bytes to HandleAMONByte

AMON_PHY_STATE g_AMONLinkPhys[NUM_LINKS];
unsigned char g_AMONLinkPhyPacketCount[NUM_LINKS];
unsigned char g_AMONLinkPhyTimeout[NUM_LINKS];

cbSendByteOnLink g_PHYSendByteCallbacks[NUM_LINKS];
cbFlushLink g_PHYFlushCallbacks[NUM_LINKS];
cbLinkPHYBusy g_PHYBusyCallbacks[NUM_LINKS];
cbPHYDelay g_PHYDelayCallback = NULL;

RESULT InitAMONPHY() {
	RESULT r = R_OK;
	int i = 0;

	for(i = 0; i < NUM_LINKS; i++) {

		g_PHYSendByteCallbacks[i] = NULL;
		g_PHYFlushCallbacks[i] = NULL;
		g_PHYBusyCallbacks[i] = NULL;

		ResetAMONPhy(i);
	}

Error:
	return r;
}

RESULT ResetAMONPhy(AMON_LINK link) {
	RESULT r = R_OK;

	g_AMONLinkPhys[link] = AMON_PHY_UNINITIALIZED;
	g_AMONLinkPhyPacketCount[link] = 0;
	g_AMONLinkPhyTimeout[link] = 0;

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

	CBRM((g_PHYDelayCallback == NULL), "RegisterLinkDelayCallback: Failed to register delay callback");
	g_PHYDelayCallback = cbDelay;

Error:
	return r;
}

RESULT UnregisterLinkDelayCallback() {
	RESULT r = R_OK;

	CBRM((g_PHYDelayCallback != NULL), "UnegisterLinkDelayCallback: Failed to unregister delay callback");
	g_PHYDelayCallback = NULL;

Error:
	return r;
}

RESULT AMONHandlePHYError(const char *pszStr, AMON_LINK link) {
	RESULT r = R_OK;

	DEBUG_LINEOUT("AMON_BYTE_ERROR received on link %d, resetting the link", link);
	CRM(AMONErrorLink(link), "AMONHandlePHYError: Failed to send error on phy link %d", link);
	//CRM(ResetLink(link), "AMONHandleHalfDuplexPHYByte: Failed to reset link %d", link);
	CRM(InitializeLink(link), "AMONHandlePHYError: Failed to initialize and reset link %d", link);

Error:
	return r;
}

RESULT AMONHandleHalfDuplexPHYByte(AMON_LINK link, unsigned char byte) {
	RESULT r = R_OK;

	switch(g_AMONLinkPhys[link]) {
		// We're in the ready state, sender and receiver
		case AMON_PHY_READY: {
			if(byte == AMON_INITIATE_REQUEST) {
				g_AMONLinkPhys[link] = AMON_PHY_PACKET_PENDING;

				CRM(SendByte(link, AMON_INITIATE_REQUEST_ACK), "AMONHandleHalfDuplexPHYByte: Failed to send initiate request ack on link %d", link);
			}
			else if(byte == AMON_BYTE_ERROR) {
				DEBUG_LINEOUT("AMON_BYTE_ERROR received on link %d, resetting the link", link);
				CRM(AMONErrorLink(link), "AMONHandleHalfDuplexPHYByte: Failed to send error on phy link %d", link);
				//CRM(ResetLink(link), "AMONHandleHalfDuplexPHYByte: Failed to reset link %d", link);
				CRM(InitializeLink(link), "SendErrorResetLink: Failed to initialize and reset link %d", link);
			} /*else {
				DEBUG_LINEOUT("Byte 0x%x received on link %d is not initiate request, ignoring", byte, link);
//				DEBUG_LINEOUT("Byte 0x%x received on link %d is not initiate request, resetting the link", byte, link);
//				CRM(AMONErrorLink(link), "AMONHandleHalfDuplexPHYByte: Failed to send error on phy link %d", link);
//				CRM(ResetLink(link), "AMONHandleHalfDuplexPHYByte: Failed to reset link %d", link);
			}*/
		} break;

		case AMON_PHY_PACKET_PENDING: {
			if((byte & 0xF0) == AMON_REQUEST_TRANSMIT) {
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
				CRM(AMONErrorLink(link), "AMONHandleHalfDuplexPHYByte: Failed to send error on phy link %d", link);
				//CRM(ResetLink(link), "AMONHandleHalfDuplexPHYByte: Failed to reset link %d", link);
				CRM(InitializeLink(link), "SendErrorResetLink: Failed to initialize and reset link %d", link);
			}
		} break;

		// Receiver - We've accepted a transmit and are receiving data
		// TODO: What if we disconnect in this state - need to have a fall back or timeout
		case AMON_PHY_ACCEPT_TRANSMIT: {
			if(g_AMONLinkPhyPacketCount[link] != 0) {
				CRM(HandleAMONByte(link, byte), "AMONHandleHalfDuplexPHYByte: Failed to handle AMON byte on link %d", link);
			}
			else {
				if(byte == AMON_TRANSMIT_COMPLETE || byte == AMON_TRANSMIT_COMPLETE_WITH_PENDING) {

					#ifdef AMON_VERBOSE
						DEBUG_LINEOUT("Transmit complete received on link %d state %d, send %d packets, pending %d ", link,
								g_AMONLinkPhys[link], NumPacketsInQueue(link), NumPacketsInPendingQueue(link));
					#endif

					// Now we parse through the packets in the queue
					CRM(HandleAMONIncomingQueue(link), "Failed to handle received packets link %d", link);

					// Check to see if we have pending packets in the queue otherwise, return transmit complete ACK
					if(NumPacketsInPendingQueue(link) != 0) {
						#ifdef AMON_VERBOSE
							DEBUG_LINEOUT("Transmit complete received on link %d, pending %d packets", link, NumPacketsInQueue(link));
						#endif

						g_AMONLinkPhys[link] = AMON_PHY_TRANSMIT_COMPLETE_PENDING;

						// Transfer the pending queue to the queue
						CRM(PushPendingQueue(link), "AMONHandleHalfDuplexPHYByte: Failed to push pending queue linke %d", link);

						// Send the packets in the queue
						g_AMONLinkPhys[link] = AMON_PHY_REQUEST_TRANSMIT_RESPONSE;
						g_AMONLinkPhyPacketCount[link] = NumPacketsInQueue(link);
						unsigned char temp = AMON_REQUEST_TRANSMIT + (g_AMONLinkPhyPacketCount[link] & 0x0F);
						CRM(SendByte(link, temp), "AMONHandleHalfDuplexPHYByte: Failed to send request transmit on link %d", link);
					}
					else if(byte == AMON_TRANSMIT_COMPLETE_WITH_PENDING) {
						g_AMONLinkPhys[link] = AMON_PHY_PACKET_PENDING;

						CRM(SendByte(link, AMON_INITIATE_REQUEST_ACK), "AMONHandleHalfDuplexPHYByte: Failed to send initiate request ack on link %d", link);
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
					CRM(AMONErrorLink(link), "AMONHandleHalfDuplexPHYByte: Failed to send error on phy link %d", link);
					//CRM(ResetLink(link), "AMONHandleHalfDuplexPHYByte: Failed to reset link %d", link);
					CRM(InitializeLink(link), "SendErrorResetLink: Failed to initialize and reset link %d", link);
				}
			}
		} break;

		// initiating a request
		case AMON_PHY_INITIATE_REQUEST: {
			if(byte == AMON_INITIATE_REQUEST_ACK) {
				g_AMONLinkPhyTimeout[link] = 0;
				 // Send the packets in the queue
				g_AMONLinkPhys[link] = AMON_PHY_REQUEST_TRANSMIT;
				g_AMONLinkPhyPacketCount[link] = NumPacketsInQueue(link);
				unsigned char temp = AMON_REQUEST_TRANSMIT + (g_AMONLinkPhyPacketCount[link] & 0x0F);
				CRM(SendByte(link, temp), "AMONHandleHalfDuplexPHYByte: Failed to send request transmit on link %d", link);
			}
			else {
				// If they're requesting a transmit at the same time as us, and they have a higher ID, let them go first
				// our data will go into a pending state - if we win, wait for a initiate ack
				if(byte == AMON_INITIATE_REQUEST) {
					if(g_amon.links[link].id > g_amon.id) {
						g_AMONLinkPhyTimeout[link] = 0;
						g_AMONLinkPhys[link] = AMON_PHY_PACKET_PENDING;

						#ifdef AMON_VERBOSE
							DEBUG_LINEOUT("Simultaneous transmit request of %d packets received on link %d state %d, send accept ...", numPackets, link, g_AMONLinkPhys[link]);
						#endif

						// Need to delay a moment
//						CRM(DelayPHY(), "Failed to delay");
//						CRM(DelayPHY(), "Failed to delay");

						CRM(SendByte(link, AMON_INITIATE_REQUEST_ACK), "AMONHandleHalfDuplexPHYByte: Failed to send ack initiate byte on link %d", link);
					}
					else {
						g_AMONLinkPhyTimeout[link] = AMON_PHY_DEFAULT_TIMEOUT;
					}
				} /*
				else {
					DEBUG_LINEOUT("Byte 0x%x received on link %d in state %d is not initiate ack or initiate request, ignoring", byte, link, g_AMONLinkPhys[link]);
//					DEBUG_LINEOUT("Byte 0x%x received on link %d in state %d is not initiate ack or initiate request, reset link", byte, link, g_AMONLinkPhys[link]);
//					CRM(AMONErrorLink(link), "AMONHandleHalfDuplexPHYByte: Failed to send error on phy link %d state %d", link, g_AMONLinkPhys[link]);
//					CRM(ResetLink(link), "AMONHandleHalfDuplexPHYByte: Failed to reset link %d state %d", link, g_AMONLinkPhys[link]);
				}*/
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

//				CRM(DelayPHY(), "Failed to delay PHY");
//				CRM(DelayPHY(), "Failed to delay PHY");

				unsigned char temp = NumPacketsInPendingQueue(link) == 0 ? AMON_TRANSMIT_COMPLETE : AMON_TRANSMIT_COMPLETE_WITH_PENDING;
				CRM(SendByte(link, temp), "AMONHandleHalfDuplexPHYByte: Failed to send transmit complete byte on link %d state %d", link, g_AMONLinkPhys[link]);
				g_AMONLinkPhys[link] = AMON_PHY_TRANSMIT_COMPLETE;
			}
			else {
				DEBUG_LINEOUT("Byte 0x%x received on link %d in state %d is not accept transmit, reset link", byte, link, g_AMONLinkPhys[link]);
				CRM(AMONErrorLink(link), "AMONHandleHalfDuplexPHYByte: Failed to send error on phy link %d state %d", link, g_AMONLinkPhys[link]);
				//CRM(ResetLink(link), "AMONHandleHalfDuplexPHYByte: Failed to reset link %d state %d", link, g_AMONLinkPhys[link]);
				CRM(InitializeLink(link), "SendErrorResetLink: Failed to initialize and reset link %d", link);
			}
		} break;

		// Sender is sending
		case AMON_PHY_TRANSMIT_ACTIVE: {
			DEBUG_LINEOUT("Byte 0x%x received on link %d during transmission - not allowed, resetting the link", byte, link);
			CRM(AMONErrorLink(link), "AMONHandleHalfDuplexPHYByte: Failed to send error on phy link %d", link);
			//CRM(ResetLink(link), "AMONHandleHalfDuplexPHYByte: Failed to reset link %d", link);
			CRM(InitializeLink(link), "SendErrorResetLink: Failed to initialize and reset link %d", link);
		} break;

		// Sender
		case AMON_PHY_TRANSMIT_COMPLETE: {
			if(byte == AMON_TRANSMIT_COMPLETE_ACK) {
				// Transmission has completed with ACK, link goes back to ready
				#ifdef AMON_VERBOSE
					DEBUG_LINEOUT("Transmit complete ACK received on link %d", link);
				#endif

				// Check to see if we have pending packets in the queue otherwise, return to ready state
				if(NumPacketsInPendingQueue(link) != 0) {
					#ifdef AMON_VERBOSE
						DEBUG_LINEOUT("Transmit complete received on link %d, pending %d packets", link, NumPacketsInQueue(link));
					#endif

					g_AMONLinkPhys[link] = AMON_PHY_TRANSMIT_COMPLETE_PENDING;

					// Transfer the pending queue to the queue
					CRM(PushPendingQueue(link), "AMONHandleHalfDuplexPHYByte: Failed to push pending queue linke %d", link);

					//reinitiate transmission
					g_AMONLinkPhys[link] = AMON_PHY_INITIATE_REQUEST;
					CRM(SendInitiateRequest(link),
							"PushAndTransmitAMONQueuePacket: Failed to Send Initiate Request on link %d", link);
				} else
					g_AMONLinkPhys[link] = AMON_PHY_READY;

			} else if(byte == AMON_INITIATE_REQUEST_ACK) {
				#ifdef AMON_VERBOSE
					DEBUG_LINEOUT("Initiate request ack received on link %d, pending %d packets", link, NumPacketsInQueue(link));
				#endif

				g_AMONLinkPhys[link] = AMON_PHY_TRANSMIT_COMPLETE_PENDING;

				// Transfer the pending queue to the queue
				CRM(PushPendingQueue(link), "AMONHandleHalfDuplexPHYByte: Failed to push pending queue linke %d", link);

				// Send the packets in the queue
				g_AMONLinkPhys[link] = AMON_PHY_REQUEST_TRANSMIT;
				g_AMONLinkPhyPacketCount[link] = NumPacketsInQueue(link);
				unsigned char temp = AMON_REQUEST_TRANSMIT + (g_AMONLinkPhyPacketCount[link] & 0x0F);
				CRM(SendByte(link, temp), "AMONHandleHalfDuplexPHYByte: Failed to send request transmit on link %d", link);
			} else if((byte & 0xF0) == AMON_REQUEST_TRANSMIT) {
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
				DEBUG_LINEOUT("Byte 0x%x received on link %d is not transmit complete ACK or request transmit, resetting the link", byte, link);
				CRM(AMONErrorLink(link), "AMONHandleHalfDuplexPHYByte: Failed to send error on phy link %d", link);
				//CRM(ResetLink(link), "AMONHandleHalfDuplexPHYByte: Failed to reset link %d", link);
				CRM(InitializeLink(link), "SendErrorResetLink: Failed to initialize and reset link %d", link);
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
	CRM(SendByte(link, AMON_BYTE_ERROR), "AMONErrorLink: Failed to send Error byte");

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
			//CRM(ResetLink(link), "AMONHandlePHYByte: Link %d failed to be reset", link);
			CRM(InitializeLink(link), "SendErrorResetLink: Failed to initialize and reset link %d", link);
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
			//CRM(ResetLink(link), "AMONHandlePHYByte: Failed to reset link %d", link);
			CRM(InitializeLink(link), "SendErrorResetLink: Failed to initialize and reset link %d", link);
		} break;
	}

Error:
	return r;
}

RESULT BroadcastByte(unsigned char byte) {
	RESULT r = R_OK;

	CRM(SendByte(AMON_NORTH, byte), "Broadcast failed on north link");
	CRM(SendByte(AMON_SOUTH, byte), "Broadcast failed on south link");
	CRM(SendByte(AMON_EAST, byte), "Broadcast failed on east link");
	CRM(SendByte(AMON_WEST, byte), "Broadcast failed on west link");

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

RESULT SendInitiateRequest(AMON_LINK link) {
	RESULT r = R_OK;

	// If the link is not ready, we quit this - the packets are queued and will get picked
	// up on completion
	if(g_AMONLinkPhys[link] == AMON_PHY_READY || g_AMONLinkPhys[link] == AMON_PHY_INITIATE_REQUEST)
		g_AMONLinkPhys[link] = AMON_PHY_INITIATE_REQUEST;
	else
		return R_OK;

	unsigned char byte = AMON_INITIATE_REQUEST;

#ifdef AMON_VERBOSE
	DEBUG_LINEOUT("Sending AMON_INITIATE_REQUEST 0x%x packets on link %d", byte, link);
#endif

	CBRM((g_PHYSendByteCallbacks[link] != NULL), "SendInitiateRequest: Failed to send byte on link %d, cb not present", link);
	CRM(g_PHYSendByteCallbacks[link](byte), "SendInitiateRequest: Link %d send byte callback failed", link);

	g_AMONLinkPhyTimeout[link] = g_amon.links[link].id > g_amon.id ? AMON_PHY_DEFAULT_TIMEOUT + 1 : AMON_PHY_DEFAULT_TIMEOUT;

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

	CBRM((g_PHYDelayCallback != NULL), "DelayPHY: Delay callback not present");
	CRM(g_PHYDelayCallback(), "DelayPHY: Delay phy callback failed");

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
		case AMON_PHY_UNINITIALIZED: return "uninitialized";
		case AMON_PHY_IDENTIFIED: return "identified";
		case AMON_PHY_REQUESTED: return "requested";
		case AMON_PHY_AVAILABLE: return "available";
		case AMON_PHY_ACCEPTED: return "accepted";
		case AMON_PHY_ASSIGNED: return "assigned";
		case AMON_PHY_MAYBE_DISCONNECTED: return "maybe disconnect";
		case AMON_PHY_READY: return "ready";
		case AMON_PHY_REQUEST_TRANSMIT: return "request transmit";
		case AMON_PHY_ACCEPT_TRANSMIT: return "accept transmit";
		case AMON_PHY_TRANSMIT_ACTIVE: return "transmit active";
		case AMON_PHY_TRANSMIT_COMPLETE: return "transmit complete";
		case AMON_PHY_TRANSMIT_COMPLETE_PENDING: return "transmit complete pending";
		case AMON_PHY_REQUEST_TRANSMIT_RESPONSE: return "request transmit response";

		default:
		case AMON_PHY_STATE_INVALID: return "invalid";
	}

	//return "invalid";
}
