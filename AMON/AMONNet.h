#ifndef AMON_NET_H_
#define AMON_NET_H_

#include <stdarg.h>

#include "..\Common\RESULT.h"
#include "..\OS\console.h"
#include "amonmap.h"

#include "AMON.h"		// The AMON Device
#include "AMONPHY.h"	// The AMON PHY layer
#include "AMONLink.h"	// The AMON Link

#include "AMONPacket.h"

#define AMON_VALUE 0x8E
#define AMONShort(usValue)       (usValue & 0xff), (usValue >> 8)
#define AMONToShort(lsb, msb) 	 ((msb & 0xFF) << 8) + (lsb & 0xFF)

//#define AMON_VERBOSE

extern int g_SysTicksPerSecond;

extern AMONMap *g_AMONmap;

extern AMON_MESSAGE_TYPE g_linkMessageType[NUM_LINKS];

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

RESULT InitAmon(int ticksPerSecond);

RESULT RegisterNewDevice(int destID, int linkID, int *newID, void *pContext);

RESULT SetAMONMaster();
RESULT SetAMONMasterState(AMON_MASTER_STATE state);
RESULT UnsetAMONMaster();
RESULT SelfAssignedMasterOnLink(AMON_LINK link);

RESULT HandleAMONByte(AMON_LINK link, unsigned char byte);		// Handles the byte coming from the PHY layer

RESULT SendAMONBuffer(AMON_LINK link, unsigned char *pBuffer, int pBuffer_n);
RESULT PassThruAMONBuffer(AMON_LINK link, unsigned char *pBuffer, int pBuffer_n);

RESULT HandleAMONPacket_old(AMON_LINK link);
RESULT HandleAMONPacket(AMON_LINK link, AMONPacket *d_pAMONPacket);

// Packet handlers
RESULT HandleAMONPing(AMON_LINK link, AMONPingPacket *d_pAMONPingPacket);
RESULT HandleAMONEcho(AMON_LINK link, AMONEchoPacket *d_pAMONEchoPacket);
RESULT HandleAMONResetLink(AMON_LINK link, AMONResetLinkPacket *d_pAMONResetLinkPacket);
RESULT HandleAMONResetLinkAck(AMON_LINK link, AMONResetLinkAckPacket *d_pAMONResetLinkAckPacket);
RESULT HandleAMONRequestID(AMON_LINK link, AMONRequestIDPacket *d_pAMONRequestIDPacket);
RESULT HandleAMONAssignID(AMON_LINK link, AMONAssignIDPacket *d_pAMONAssignIDPacket);
RESULT HandleAMONAck(AMON_LINK link, AMONAckPacket *d_pAMONAckPacket);
//RESULT HandleAMONBroadcast(AMON_LINK link, AMONBroadcastPacket *d_pAMONBroadcastPacket);
RESULT HandleAMONSend(AMON_LINK link, AMONSendPacket *d_pAMONSendPacket);
RESULT HandleAMONGetDeviceID(AMON_LINK link, AMONGetDeviceIDPacket *d_pAMONGetDeviceIDPacket);
RESULT HandleAMONSendDeviceID(AMON_LINK link, AMONSendDeviceIDPacket *d_pAMONSendDeviceIDPacket);
RESULT HandleAMONEstablishLink(AMON_LINK link, AMONEstablishLinkPacket *d_pAMONEstablishLinkPacket);
RESULT HandleAMONEstablishLinkAck(AMON_LINK link, AMONEstablishLinkAckPacket *d_pAMONEstablishLinkAckPacket);
RESULT HandleAMONError(AMON_LINK link, AMONErrorPacket *d_pAMONErrorPacket);
RESULT HandleAMONSendByteDestLink(AMON_LINK link, AMONSendByteDestLinkPacket *d_pAMONSendByteDestLinkPacket);

// AMON OnInterval
RESULT OnAMONInterval();
RESULT SetAMONInterval(short msTime, int ticksPerSecond);
short GetAMONInterval();
int GetAMONIntervalSystick();

int GetNumberOfEastWestLinks(int id);
int GetDepthOfAMONMapLink(AMON_LINK link);

// Device Registration / Unregistration
typedef RESULT (*cbAMONDeviceRegistered)(short);
extern cbAMONDeviceRegistered g_AMONDeviceRegisteredCallback;
RESULT RegisterAMONDeviceRegisteredCallback(cbAMONDeviceRegistered AMONDeviceRegisteredCB);
RESULT UnregisterAMONDeviceRegisteredCallback();

typedef RESULT (*cbAMONDeviceUnregistered)(short, short, AMON_LINK);
extern cbAMONDeviceUnregistered g_AMONDeviceUnregisteredCallback;
RESULT RegisterAMONDeviceUnregisteredCallback(cbAMONDeviceUnregistered AMONDeviceUnregisteredCB);
RESULT UnregisterAMONDeviceUnregisteredCallback();

typedef void* (*cbGetAMONDevice)();
extern cbGetAMONDevice g_GetAMONDeviceCallback;
RESULT RegisterGetAMONDeviceCallback(cbGetAMONDevice GetAMONDeviceCB);
RESULT UnregisterGetAMONDeviceCallback();

// Command Mode
typedef RESULT (*cbHandleAMONPayload)(AMON_LINK, short, unsigned char, unsigned char *, int);
extern cbHandleAMONPayload g_HandleAMONPayloadCallback;
RESULT RegisterHandleAMONPayloadCallback(cbHandleAMONPayload handleAMONPayloadCB);
RESULT UnregisterHandleAMONPayloadCallback();

RESULT SendMessagePayload(AMON_LINK link, short destID, unsigned char type, unsigned char *payloadBuffer, int payloadBuffer_n);
RESULT SendMessageType(AMON_MESSAGE_TYPE type, short destID, ...);
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
RESULT SendAMONNULLPing(Console *pc, char *pszLink);
RESULT ConsoleCheckLinkStatus(Console *pc, char *pszLink);
RESULT ConsoleSetAMONInterval(Console *pc, char *pszMsTime);
RESULT PrintAMONInfo(Console *pc);
RESULT TestAMONNumLinks(Console *pc, char *pszID);

RESULT ResetAMONLink(Console *pc, char *pszLink);

// LED Convenience Functions
RESULT SetLEDLinkClearTimeout(AMON_LINK link, int red, int green, int blue, int count);

#endif // AMON_NET_H_
