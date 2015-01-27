#ifndef AMON_PHY_H_
#define AMON_PHY_H_

// The AMON PHY is the actual byte level of the AMON protocol

#include "RESULT.h"
#include "AMONLink.h"

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
	AMON_BYTE_INVALID
} AMON_BYTE_MODE_MESSAGE;

// The link state machine for PHY
typedef enum {
	AMON_PHY_UNINITIALIZED,		// link is yet to be initialized
	AMON_PHY_IDENTIFIED,			// device identified on link
	AMON_PHY_REQUESTED,
	AMON_PHY_AVAILABLE,
	AMON_PHY_ACCEPTED,
	AMON_PHY_ESTABLISHED,
	AMON_PHY_ASSIGNED,
	AMON_PHY_MAYBE_DISCONNECTED,
	AMON_PHY_STATE_INVALID
} AMON_PHY_STATE;

extern AMON_PHY_STATE g_AMONLinkPhys[NUM_LINKS];

RESULT AMONErrorLink(AMON_LINK link);
RESULT AMONReceiveByte(AMON_LINK link, unsigned char byte);
RESULT AMONHandlePHYByte(AMON_LINK link, unsigned char byte);

// Byte Mode
RESULT FlushPHY(AMON_LINK link);
RESULT SendByte(AMON_LINK link, unsigned char byte);
RESULT BroadcastByte(unsigned char byte);
RESULT SendPing(AMON_LINK link);
RESULT SendEcho(AMON_LINK link);

#endif // !AMON_PHY_H_
