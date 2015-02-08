#include "AMONPacket.h"
#include "AMONNet.h"

RESULT SendAMONPacket(AMON_LINK link, AMONPacket *d_pAMONPacket) {
	RESULT r = R_OK;

	CRM(SendAMONBuffer(link, (unsigned char *)(d_pAMONPacket), d_pAMONPacket->m_length),
			"SendAMONPacket: Failed to SendAMONBuffer %d bytes", d_pAMONPacket->m_length);

Error:
	if(d_pAMONPacket != NULL) {
		free(d_pAMONPacket);
		d_pAMONPacket = NULL;
	}
	return r;
}

AMONPingPacket *CreateAMONPingPacket(unsigned char destID) {
	AMONPingPacket *pAMONPingPacket = (AMONPingPacket*)calloc(1, sizeof(AMONPingPacket));

	pAMONPingPacket->m_header.m_AMONID = AMON_VALUE;

	//pAMONPingPacket->m_length = sizeof(AMONPingPacket);
	pAMONPingPacket->m_header.m_length = 8;	// Alignment screws this up

	pAMONPingPacket->m_header.m_type = AMON_PING;
	pAMONPingPacket->m_originID = g_amon.id;
	pAMONPingPacket->m_destID = destID;

	return pAMONPingPacket;
}

AMONEchoPacket *CreateAMONEchoPacket(unsigned char destID) {
	AMONEchoPacket *pAMONEchoPacket = (AMONEchoPacket*)calloc(1, sizeof(AMONEchoPacket));

	pAMONEchoPacket->m_header.m_AMONID = AMON_VALUE;

	//pAMONPingPacket->m_length = sizeof(AMONPingPacket);
	pAMONEchoPacket->m_header.m_length = 8;	// Alignment screws this up

	pAMONEchoPacket->m_header.m_type = AMON_ECHO;
	pAMONEchoPacket->m_originID = g_amon.id;
	pAMONEchoPacket->m_destID = destID;

	return pAMONEchoPacket;
}

AMONRequestIDPacket *CreateAMONRequestIDPacket(unsigned char linkID, unsigned short linkDeviceID) {
	AMONRequestIDPacket *pAMONRequestIDPacket = (AMONRequestIDPacket*)calloc(1, sizeof(AMONRequestIDPacket));

	pAMONRequestIDPacket->m_header.m_AMONID = AMON_VALUE;

	pAMONRequestIDPacket->m_header.m_length = 7;	// Alignment screws this up

	pAMONRequestIDPacket->m_header.m_type = AMON_REQUEST_ID;
	pAMONRequestIDPacket->m_linkID = linkID;
	pAMONRequestIDPacket->m_linkDeviceID = linkDeviceID;

	return pAMONRequestIDPacket;
}

AMONAssignIDPacket *CreateAMONAssignIDPacket(AMON_LINK destLink, unsigned short destID, unsigned short newID) {
	AMONAssignIDPacket *pAMONAssignIDPacket = (AMONAssignIDPacket*)calloc(1, sizeof(AMONAssignIDPacket));

	pAMONAssignIDPacket->m_header.m_AMONID = AMON_VALUE;

	pAMONAssignIDPacket->m_header.m_length = 9;	// Alignment screws this up

	pAMONAssignIDPacket->m_header.m_type = AMON_ASSIGN_ID;
	pAMONAssignIDPacket->m_linkID = (unsigned char)destLink;
	pAMONAssignIDPacket->m_linkDeviceID = destID;
	pAMONAssignIDPacket->m_newID = newID;

	return pAMONAssignIDPacket;
}

AMONEstablishLinkPacket *CreateAMONEstablishLinkPacket(AMON_LINK link, unsigned short originID) {
	AMONEstablishLinkPacket *pAMONEstablishLinkPacket = (AMONEstablishLinkPacket*)calloc(1, sizeof(AMONEstablishLinkPacket));

	pAMONEstablishLinkPacket->m_header.m_AMONID = AMON_VALUE;

	pAMONEstablishLinkPacket->m_header.m_length = 7;	// Alignment screws this up

	pAMONEstablishLinkPacket->m_header.m_type = AMON_ESTABLISH_LINK;
	pAMONEstablishLinkPacket->m_linkID = (unsigned char)link;
	pAMONEstablishLinkPacket->m_originID = originID;

	return pAMONEstablishLinkPacket;
}

AMONEstablishLinkAckPacket *CreateAMONEstablishLinkAckPacket(AMON_LINK link, unsigned short senderID) {
	AMONEstablishLinkAckPacket *pAMONEstablishLinkAckPacket = (AMONEstablishLinkAckPacket*)calloc(1, sizeof(AMONEstablishLinkAckPacket));

	pAMONEstablishLinkAckPacket->m_header.m_AMONID = AMON_VALUE;

	pAMONEstablishLinkAckPacket->m_header.m_length = 7;	// Alignment screws this up

	pAMONEstablishLinkAckPacket->m_header.m_type = AMON_ESTABLISH_LINK_ACK;
	pAMONEstablishLinkAckPacket->m_linkID = (unsigned char)link;
	pAMONEstablishLinkAckPacket->m_senderID = senderID;

	return pAMONEstablishLinkAckPacket;
}

AMONErrorPacket *CreateAMONErrorPacket(unsigned short type) {
	AMONErrorPacket *pAMONErrorPacket = (AMONErrorPacket*)calloc(1, sizeof(AMONErrorPacket));

	pAMONErrorPacket->m_header.m_AMONID = AMON_VALUE;

	pAMONErrorPacket->m_header.m_length = 5;	// Alignment screws this up

	pAMONErrorPacket->m_header.m_type = AMON_ERROR;
	pAMONErrorPacket->m_messageType = type;

	return pAMONErrorPacket;
}















