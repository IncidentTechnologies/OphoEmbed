#ifndef AMON_PACKET_H_
#define AMON_PACKET_H_

#include "AMONLink.h"

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

typedef enum {
	AMON_ACK_ASSIGN_ID 		= 0x01,
	AMON_ACK_SEND			= 0x02,
	AMON_ACK_INVALID
} AMON_ACK_TYPE;

typedef struct {
	unsigned char m_AMONID;
	unsigned char m_length;
	//AMON_MESSAGE_TYPE m_type;
	unsigned char m_type;
} AMONPacket;

typedef struct {
	AMONPacket m_header;
	unsigned short m_originID;
	unsigned short m_destID;
	unsigned char m_checksum;
} AMONPingPacket;

typedef struct {
	AMONPacket m_header;
	unsigned short m_originID;
	unsigned short m_destID;
	unsigned char m_checksum;
} AMONEchoPacket;

typedef struct {
	AMONPacket m_header;
	unsigned char m_linkID;
	unsigned short m_linkDeviceID;
	unsigned char m_checksum;
} AMONRequestIDPacket;

typedef struct {
	AMONPacket m_header;
	unsigned char m_linkID;
	unsigned short m_linkDeviceID;
	unsigned short m_newID;
	unsigned char m_checksum;
} AMONAssignIDPacket;

typedef struct {
	AMONPacket m_header;
	unsigned short m_originID;
	unsigned char m_linkID;
	unsigned char m_checksum;
} AMONEstablishLinkPacket;

typedef struct {
	AMONPacket m_header;
	unsigned short m_senderID;
	unsigned char m_linkID;
	unsigned char m_checksum;
} AMONEstablishLinkAckPacket;

typedef struct {
	AMONPacket m_header;
	unsigned char m_messageType;
	unsigned char m_checksum;
} AMONErrorPacket;

RESULT SendAMONPacket(AMON_LINK link, AMONPacket *d_pAMONPacket);

AMONPingPacket *CreatePingPacket(unsigned char destID);
AMONEchoPacket *CreateAMONEchoPacket(unsigned char destID);
AMONRequestIDPacket *CreateAMONRequestIDPacket(unsigned char linkID, unsigned short linkDeviceID);
AMONAssignIDPacket *CreateAMONAssignIDPacket(AMON_LINK destLink, unsigned short destID, unsigned short newID);
AMONEstablishLinkPacket *CreateAMONEstablishLinkPacket(AMON_LINK link, unsigned short originID);
AMONEstablishLinkAckPacket *CreateAMONEstablishLinkAckPacket(AMON_LINK link, unsigned short senderID);
AMONErrorPacket *CreateAMONErrorPacket(unsigned short type);

#endif // ! AMON_PACKET_H_
