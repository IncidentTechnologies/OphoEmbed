#ifndef AMON_PHY_H_
#define AMON_PHY_H_

// The AMON PHY is the actual byte level of the AMON protocol

#include "..\Common\RESULT.h"
#include "AMONLink.h"

#define AMON_HALF_DUPLEX
#ifndef AMON_HALF_DUPLEX
	#define AMON_FULL_DUPLEX
#endif

// Byte Mode Messages
typedef enum {
	AMON_BYTE_LINK_REQUEST 		= 0x11,
	AMON_BYTE_LINK_RESET 		= 0x12,
	AMON_BYTE_LINK_RESET_ACK	= 0x13,
	AMON_BYTE_LINK_AVAILABLE	= 0x22,
	AMON_BYTE_LINK_ACCEPTED		= 0x33,
	AMON_BYTE_LINK_ESTABLISHED 	= 0x44,
	AMON_BYTE_PING				= 0x55,
	AMON_BYTE_ECHO				= 0x66,
	AMON_BYTE_ERROR				= 0x77,
	AMON_CHECK_LINK				= 0x80,
	AMON_LINK_ALIVE				= 0x81,
	AMON_INK_DEAD				= 0x82,
	AMON_LINK_ERROR				= 0x83,
	AMON_REQUEST_TRANSMIT		= 0xD0,		// This message is amorphous as it includes the entire 0xD0 - 0xDF range
	AMON_ACCEPT_TRANSMIT		= 0xF0,
	AMON_TRANSMIT_COMPLETE		= 0xF1,
	AMON_TRANSMIT_COMPLETE_ACK	= 0xF2,
	AMON_BYTE_INVALID
} AMON_BYTE_MODE_MESSAGE;

// The link state machine for PHY
typedef enum {
	AMON_PHY_UNINITIALIZED,		// link is yet to be initialized
	AMON_PHY_IDENTIFIED,			// device identified on link
	AMON_PHY_REQUESTED,
	AMON_PHY_AVAILABLE,
	AMON_PHY_ACCEPTED,
	AMON_PHY_ASSIGNED,
	AMON_PHY_MAYBE_DISCONNECTED,
	AMON_PHY_READY,		// link establsihed and ready

	// Half Duplex Functionality
	//AMON_PHY_READY,						// In half-duplex the link sits in a ready state
	AMON_PHY_REQUEST_TRANSMIT,				// We've requested a transmission (sender)
	AMON_PHY_ACCEPT_TRANSMIT,				// We've accepted a transmission, waiting for the packet traffic (routed to link layer) (receiver)
	AMON_PHY_TRANSMIT_ACTIVE,				// We're currently transmitting, control handed off to the link layer (sender)
	AMON_PHY_TRANSMIT_COMPLETE,				// The transmission is complete and we're waiting to either get a completion ACK or a new transmission (putting us into accept transmit or ready respectively) (sender)
	AMON_PHY_TRANSMIT_COMPLETE_PENDING,		// pending some packets (receiver)
	AMON_PHY_REQUEST_TRANSMIT_RESPONSE,		// We've requested a transmission after receiving a transmission (sender)
	AMON_PHY_STATE_INVALID
} AMON_PHY_STATE;

extern AMON_PHY_STATE g_AMONLinkPhys[NUM_LINKS];

// Half Duplex
extern unsigned char g_AMONLinkPhyPacketCount[NUM_LINKS];

// TODO: Register
typedef RESULT (*cbSendByteOnLink)(unsigned char*);
extern cbSendByteOnLink g_PHYSendByteCallbacks[NUM_LINKS];

RESULT RegisterLinkSendByteCallback(AMON_LINK link, cbSendByteOnLink cbSendByte);
RESULT UnregisterLinkSendByteCallback(AMON_LINK link);

typedef unsigned char (*cbLinkPHYBusy)();
extern cbLinkPHYBusy g_PHYBusyCallbacks[NUM_LINKS];

RESULT RegisterLinkBusyCallback(AMON_LINK link, cbLinkPHYBusy cbBusy);
RESULT UnregisterLinkBusyCallback(AMON_LINK link);

typedef RESULT (*cbFlushLink)();
extern cbFlushLink g_PHYFlushCallbacks[NUM_LINKS];

RESULT RegisterLinkFlushCallback(AMON_LINK link, cbFlushLink cbFlush);
RESULT UnregisterLinkFlushCallback(AMON_LINK link);

typedef RESULT (*cbPHYDelay)();
extern cbPHYDelay g_PHYDelayCallback;

RESULT RegisterLinkDelayCallback(cbPHYDelay cbDelay);
RESULT UnregisterLinkDelayCallback();

RESULT InitAMONPHY();

RESULT AMONErrorLink(AMON_LINK link);
RESULT AMONReceiveByte(AMON_LINK link, unsigned char byte);
RESULT AMONHandlePHYByte(AMON_LINK link, unsigned char byte);
RESULT AMONHandleHalfDuplexPHYByte(AMON_LINK link, unsigned char byte);

// Byte Mode
unsigned char LinkBusy(AMON_LINK link);
RESULT FlushPHY(AMON_LINK link);
RESULT SendByte(AMON_LINK link, unsigned char byte);
RESULT SendRequestTransmit(AMON_LINK link, unsigned char fResponse);
RESULT BroadcastByte(unsigned char byte);
RESULT SendPing(AMON_LINK link);
RESULT SendEcho(AMON_LINK link);

RESULT DelayPHY();

#endif // !AMON_PHY_H_
