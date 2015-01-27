#ifndef AMON_NET_H_
#define AMON_NET_H_

#include <stdarg.h>

#include "RESULT.h"
#include "console.h"
#include "amonmap.h"

#include "AMON.h"		// The AMON Device
#include "AMONPHY.h"	// The AMON PHY layer
#include "AMONLink.h"	// The AMON Link

#define AMON_VALUE 0x8E
#define AMONShort(usValue)       (usValue & 0xff), (usValue >> 8)
#define AMONToShort(lsb, msb) 	 ((msb & 0xFF) << 8) + (lsb & 0xFF)

//#define AMON_VERBOSE

extern AMONMap *g_AMONmap;

typedef enum {
	AMON_NULL					= 0x00,
	AMON_PING 					= 0x01,
	AMON_ECHO					= 0x02,
	AMON_REQUEST_ID				= 0x03,
	AMON_ASSIGN_ID				= 0x04,
	AMON_ACK					= 0x05,
	AMON_BROADCAST				= 0x06,
	AMON_SEND					= 0x07,
	AMON_GET_ID					= 0x08,
	AMON_SEND_ID				= 0x09,
	AMON_ESTABLISH_LINK			= 0x0A,
	AMON_ESTABLISH_LINK_ACK		= 0x0B,
	AMON_ERROR					= 0x0C,
	AMON_SEND_BYTE_DEST_LINK	= 0x0D,
	AMON_RESET_LINK				= 0x0E,
	AMON_RESERVED_0				= 0x0F,
	AMON_RESET_LINK_ACK			= 0x10,
	AMON_INVALID				= 0xFF
} AMON_MESSAGE_TYPE;

extern AMON_MESSAGE_TYPE g_linkMessageType[NUM_LINKS];

typedef enum {
	AMON_ACK_ASSIGN_ID 		= 0x01,
	AMON_ACK_SEND			= 0x02,
	AMON_ACK_INVALID
} AMON_ACK_TYPE;

typedef struct {
	unsigned char amon;
	unsigned char length;
	unsigned char type;
	unsigned char *payload;
	unsigned char checksum;
} AMON_MESSAGE;

// Link Packet Received State
// This is specific to the AMON protocol
typedef enum {
	AMON_RX_READY,
	AMON_RX_AMON_RECEIVED,
	AMON_RX_LENGTH_RECEIVED,
	AMON_RX_TYPE_RECEIVED,
	AMON_RX_DATA,
	AMON_RX_INVALID
} AMON_RX_STATE;
extern AMON_RX_STATE g_LinkRxState[NUM_LINKS];

// Done by AMONMap
//RESULT ResetMasterCount();
//RESULT RegisterNewID(int *newId);

RESULT InitAmon();

RESULT RegisterNewDevice(int destID, int linkID, int *newID);

RESULT SetAMONMaster();
RESULT SetAMONMasterState(AMON_MASTER_STATE state);
RESULT UnsetAMONMaster();
RESULT SelfAssignedMasterOnLink(AMON_LINK link);

RESULT HandleAMONByte(AMON_LINK link, unsigned char byte);		// Handles the byte coming from the PHY layer

RESULT SendAMONBuffer(AMON_LINK link, unsigned char *pBuffer, int pBuffer_n);
RESULT PassThruAMONBuffer(AMON_LINK link, unsigned char *pBuffer, int pBuffer_n);

RESULT HandleAMONPacket(AMON_LINK link);

// AMON OnInterval
RESULT OnAMONInterval();
RESULT SetAMONInterval(short msTime);
short GetAMONInterval();
int GetAMONIntervalSystick();

// Command Mode
RESULT SendMessage(AMON_MESSAGE_TYPE type, short destID, ...);
RESULT SendGetDeviceID(AMON_LINK link);
RESULT SendDeviceID(AMON_LINK link);
RESULT SendEstablishLink(AMON_LINK link);
RESULT SendEstablishLinkAck(AMON_LINK link);
RESULT SendRequestIDFromNetwork(AMON_LINK link);
RESULT SendAssignID(AMON_LINK link, short destID, AMON_LINK destLink, short newID);
RESULT SendACK(AMON_LINK link, short destID, AMON_ACK_TYPE type, unsigned char status);
RESULT SendByteDestLink(AMON_LINK link, short destID, AMON_LINK destLink, unsigned char byte);

RESULT SendResetLink(AMON_LINK link);
RESULT SendResetLinkACK(AMON_LINK link);

RESULT SendPingNetwork(AMON_LINK link, short destID);
RESULT SendEchoNetwork(AMON_LINK link, short destID);

RESULT SendError(AMON_LINK link, AMON_MESSAGE_TYPE type);
RESULT SendErrorResetLink(AMON_LINK link, AMON_MESSAGE_TYPE type);

// Console functions
RESULT SendByteModeCommand(Console *pc, char *pszCmd, char *pszLink);
RESULT SendByteModeCommandDestLink(Console *pc, char *pszCmd, char *pszDestID, char *pszLink);
RESULT SetAMONMasterConsole(Console *pc, unsigned char *pszfMaster);
RESULT PrintAMONMasterMap(Console *pc);
RESULT SendAMONMessage(Console *pc, char *pszCmd, char *pszDestID);
RESULT ConsoleCheckLinkStatus(Console *pc, char *pszLink);
RESULT ConsoleSetAMONInterval(Console *pc, char *pszMsTime);
RESULT PrintAMONInfo(Console *pc);

RESULT ResetAMONLink(Console *pc, char *pszLink);

// LED Convenience Functions
RESULT SetLEDLinkClearTimeout(AMON_LINK link, int red, int green, int blue, int count);

#endif // AMON_NET_H_
