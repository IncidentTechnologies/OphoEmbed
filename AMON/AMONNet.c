#include "AMONNet.h"

AMON_RX_STATE g_LinkRxState[NUM_LINKS];
unsigned char g_fLinkActivitySinceInterval[NUM_LINKS];

// The message on a given link
AMON_MESSAGE_TYPE g_linkMessageType[NUM_LINKS];

int g_MasterCount = 0;

AMONMap *g_AMONmap = NULL;

cbAMONDeviceRegistered g_AMONDeviceRegisteredCallback = NULL;
RESULT RegisterAMONDeviceRegisteredCallback(cbAMONDeviceRegistered AMONDeviceRegisteredCB) {
	RESULT r = R_OK;

	CBRM_NA((g_AMONDeviceRegisteredCallback == NULL), "RegisterAMONDeviceRegisteredCallback: AMON Device Registered Callback already registered");
	g_AMONDeviceRegisteredCallback = AMONDeviceRegisteredCB;

Error:
	return r;
}

RESULT UnregisterAMONDeviceRegisteredCallback() {
	RESULT r = R_OK;

	CBRM_NA((g_AMONDeviceRegisteredCallback != NULL), "UnregisterAMONDeviceRegisteredCallback: AMON Device Registered Callback not registered");
	g_AMONDeviceRegisteredCallback = NULL;

Error:
	return r;
}

cbAMONDeviceUnregistered g_AMONDeviceUnregisteredCallback = NULL;
RESULT RegisterAMONDeviceUnregisteredCallback(cbAMONDeviceUnregistered AMONDeviceUnregisteredCB) {
	RESULT r = R_OK;

	CBRM_NA((g_AMONDeviceUnregisteredCallback == NULL), "RegisterAMONDeviceUnregisteredCallback: AMON Device Unregistered Callback already registered");
	g_AMONDeviceUnregisteredCallback = AMONDeviceUnregisteredCB;

Error:
	return r;
}

RESULT UnregisterAMONDeviceUnregisteredCallback() {
	RESULT r = R_OK;

	CBRM_NA((g_AMONDeviceUnregisteredCallback != NULL), "UnregisterAMONDeviceUnregisteredCallback: AMON Device Unregistered Callback not registered");
	g_AMONDeviceUnregisteredCallback = NULL;

Error:
	return r;
}

cbGetAMONDevice g_GetAMONDeviceCallback = NULL;
RESULT RegisterGetAMONDeviceCallback(cbGetAMONDevice GetAMONDeviceCB) {
	RESULT r = R_OK;

	CBRM_NA((g_GetAMONDeviceCallback == NULL), "RegisterGetAMONDeviceCallback: Get AMON Device Callback already registered");
	g_GetAMONDeviceCallback = GetAMONDeviceCB;

Error:
	return r;
}

RESULT UnregisterGetAMONDeviceCallback() {
	RESULT r = R_OK;

	CBRM_NA((g_GetAMONDeviceCallback != NULL), "UnegisterGetAMONDeviceCallback: Get AMON Device Callback not registered");
	g_GetAMONDeviceCallback = NULL;

Error:
	return r;
}

// TODO: Fix this rudimentary approach
int GetNumberOfEastWestLinks(int id) {
	return GetNumberOfEastWestMapLinks(g_AMONmap, id, AMON_WEST, AMON_EAST);
}

int GetDepthOfAMONMapLink(AMON_LINK link) {
	return GetDepthOfMapLink(g_AMONmap, link);
}

int GetAMONMapNodeIDOnLinkDepth(AMON_LINK link, int depth) {
	AMONNode *node = GetMapNodeIDOnLinkDepth(g_AMONmap, link, depth);
	return node->m_id;
}

inline RESULT ReportLinkActivity(AMON_LINK link) {
	g_fLinkActivitySinceInterval[link] = TRUE;
	return R_OK;
}

RESULT InitAmon(int ticksPerSecond) {
	RESULT r = R_OK;
	int i = 0;

	g_amon.fStart = 0;
	CRM_NA(InitializeAMON(), "InitAmon: Failed to Initialize AMON");
	//StartAMON();

	CRM_NA(InitializeAMONQueue(), "InitAmon: Failed to initialize AMON Queue");

	//ResetMasterCount();

	g_SysTicksPerSecond = ticksPerSecond;

	SetAMONInterval(1, g_SysTicksPerSecond);

	// Initialize the links
	for(i = 0; i < NUM_LINKS; i++) {
		CRM(InitializeLink(i), "InitAmon: Failed to initialize link %d", i);

		g_fLinkActivitySinceInterval[i] = FALSE;
	}

#ifdef CONSOLE
	// Console Functions
	AddConsoleFunctionByArgs(GetConsole(), SendByteModeCommand, "AMONSendByte", 3, 0);
	AddConsoleFunctionByArgs(GetConsole(), SetAMONMasterConsole, "SetAMONMaster", 2, 0);
	AddConsoleFunctionByArgs(GetConsole(), PrintAMONMasterMap, "PrintAMONMasterMap", 1, 0);
	AddConsoleFunctionByArgs(GetConsole(), SendByteModeCommandDestLink, "AMONSendByteDestLink", 4, 0);

	// TODO: Add variable params / min params
	AddConsoleFunctionByArgs(GetConsole(), SendAMONMessage, "AMONMessage", 3, 0);
	AddConsoleFunctionByArgs(GetConsole(), SendAMONNULLPing, "AMONSendNULLPing", 2, 0);

	AddConsoleFunctionByArgs(GetConsole(), StartAMON, "StartAMON", 1, 0);
	AddConsoleFunctionByArgs(GetConsole(), PrintAMONInfo, "PrintAMONInfo", 1, 0);

	AddConsoleFunctionByArgs(GetConsole(), ConsoleCheckLinkStatus, "AMONCheckLinkStatus", 2, 0);
	AddConsoleFunctionByArgs(GetConsole(), ConsoleSetAMONInterval, "AMONSetInterval", 2, 0);

	AddConsoleFunctionByArgs(GetConsole(), ResetAMONLink, "AMONResetLink", 2, 0);

	AddConsoleFunctionByArgs(GetConsole(), TestAMONMap, "TestAMONMap", 1, 0);

	AddConsoleFunctionByArgs(GetConsole(), TestAMONNumLinks, "TestAMONNumLinks", 2, 0);
#endif

//	SetLEDWithClearTimeout(1, 20, 20, 100, 50);
Error:
	return r;
}

cbHandleAMONPayload g_HandleAMONPayloadCallback = NULL;
RESULT RegisterHandleAMONPayloadCallback(cbHandleAMONPayload handleAMONPayloadCB) {
	RESULT r = R_OK;

	CBRM_NA((g_HandleAMONPayloadCallback == NULL), "RegisterAMONCallback: Callback already registered");
	g_HandleAMONPayloadCallback = handleAMONPayloadCB;

Error:
	return r;
}

RESULT UnregisterHandleAMONPayloadCallback() {
	RESULT r = R_OK;

	CBRM_NA((g_HandleAMONPayloadCallback != NULL), "RegisterAMONCallback: Callback not registered");
	g_HandleAMONPayloadCallback = NULL;

Error:
	return r;
}

#define DEFAULT_AMON_INTERVAL 1000

#define AMON_SYSTICKS_PER_SECOND 76000

int g_SysTicksPerSecond = AMON_SYSTICKS_PER_SECOND;
volatile short m_AMONInterval = DEFAULT_AMON_INTERVAL;
volatile int m_AMONIntervalSystick = DEFAULT_AMON_INTERVAL * (AMON_SYSTICKS_PER_SECOND / 1000);

short GetAMONInterval() { return m_AMONInterval; }
int GetAMONIntervalSystick() { return m_AMONIntervalSystick; }

RESULT SetAMONInterval(short msTime, int ticksPerSecond) {
	m_AMONInterval = msTime;
	m_AMONIntervalSystick = (short)(m_AMONInterval * (ticksPerSecond / 1000));
	return R_OK;
}

RESULT ConsoleSetAMONInterval(Console *pc, char *pszMsTime) {
	RESULT r = R_OK;

	unsigned short msTime = (unsigned short)atoi(pszMsTime);

	CBRM((msTime < 5000 && msTime > 10), "ConsoleSetAMONInterval: Interval %d ms must be between 10 and 5000 ms", msTime);

	CRM(SetAMONInterval(msTime, g_SysTicksPerSecond), "ConsoleSetAMONInterval: Failed to set AMON interval to %d ms", msTime);

Error:
	return r;
}

unsigned int g_uiAMONInterval_c = 0;

// AMON OnInterval - called every interval
RESULT OnAMONInterval() {
	RESULT r = R_OK;
	int i = 0;

	/*
	for(i = 0; i < NUM_LINKS; i++) {
		if(NumPacketsInPendingQueue(i) != 0 && g_AMONLinkPhys[i] == AMON_PHY_READY) {
			#ifdef AMON_VERBOSE
				DEBUG_LINEOUT("Transmit complete received on link %d, pending %d packets", i, NumPacketsInQueue(i));
			#endif
			// Transfer the pending queue to the queue
			CRM(PushPendingQueue(i), "OnAMONInterval: Failed to push pending queue linke %d", i);

			// Send the packets in the queue
			CRM(SendRequestTransmit(i, NumPacketsInQueue(i)), "OnAMONInterval: Failed to send request transmit on link %d", i);
		}
	}*/

	if(g_amon.fStart == 0) {
		return R_OFF;
	}

	g_uiAMONInterval_c++;

	// Every cycle for timeouts
	for(i = 0; i < NUM_LINKS; i++) {
		if(g_AMONLinkPhys[i] == AMON_PHY_INITIATE_REQUEST) {
			if(g_AMONLinkPhyTimeout[i] && --(g_AMONLinkPhyTimeout[i]) == 0) {
				// DEBUG_LINEOUT("OnAMONInterval: timeout on link %d during AMON_INITIATE_REQUEST, resending", i);
				CRM(SendInitiateRequest(i),	"OnAMONInterval: Failed to Send Initiate Request on link %d", i);
			}
		}
	}

	// Establish link and link check cycles
	if((g_uiAMONInterval_c % AMON_LINK_INTERVAL_DIV) == 0) {

#ifdef AMON_VERBOSE
		DEBUG_LINEOUT("OnAMON: Check Link");
#endif

		for(i = 0; i < NUM_LINKS; i++) {
			if(g_AMONLinkStates[i] != AMON_LINK_ESTABLISHED) {
				unsigned char r = (i == 0 || i == 3) ? 50 : 0;
				unsigned char g = (i == 1 || i == 3) ? 50 : 0;
				unsigned char b = (i == 2 || i == 3) ? 50 : 0;

				CRM(SendByte(i, AMON_BYTE_PING), "OnAMONInterval: Failed to send ping on link %d", i);
			}
			else {
				// If link established we want to check the status unless it's sending traffic
				if(AMONLinkBusy(i) == 0 && g_fLinkActivitySinceInterval[i] == FALSE) {
					CRM(CheckLinkStatus(i), "OnAMONInterval: Failed to check link %d status", i);
				}
				else {
					g_fLinkActivitySinceInterval[i] = FALSE;
				}
			}
		}
	}

Error:
	return r;
}

RESULT SetAMONMaster() {
	return SetAMONMasterState(AMON_MASTER_ABSOLUTE);
}

RESULT SetAMONMasterState(AMON_MASTER_STATE state) {
	g_amon.MasterState = state;

	//ResetMasterCount();

	if(state > AMON_MASTER_FALSE && state < AMON_MASTER_INVALID) {
		g_amon.status = AMON_DEVICE_OK;
		g_amon.id = 0;

		// Initialize AMON Map
		if(g_GetAMONDeviceCallback != NULL)
			g_AMONmap = CreateAMONMap(NUM_LINKS, AMON_MASTER_ID, g_GetAMONDeviceCallback());
		else
			g_AMONmap = CreateAMONMap(NUM_LINKS, AMON_MASTER_ID, g_GetAMONDeviceCallback());
	}
	else {
		g_amon.status = AMON_DEVICE_UNASSIGNED;
		g_amon.id = -1;
	}

	return R_OK;
}

RESULT SelfAssignedMasterOnLink(AMON_LINK link) {
	RESULT r = R_OK;

	// If the incoming link is unassigned, and we're a top or left device in respect to it
	// then assign ourselves as a self-defined master, give us a zero ID and set the status to OK
	// We should also be un assigned
	if(g_amon.links[link].Status == AMON_DEVICE_UNASSIGNED && g_amon.status == AMON_DEVICE_UNASSIGNED) {
		if(link == AMON_EAST || link == AMON_SOUTH) {
			SetAMONMasterState(AMON_MASTER_SELF_DEFINED);
			DEBUG_LINEOUT("Self Assigning Master (id 0) off link %d", link);
		}
	}

Error:
	return r;
}

/*
RESULT ResetMasterCount() {
	g_AMONmap->m_mapID = 0;
	return R_OK;
}

RESULT RegisterNewID(int *newId) {
	g_AMONmap->m_mapID++;
	*newId = g_AMONmap->m_mapID;
	return R_OK;
}
*/

// Add a new device
RESULT RegisterNewDevice(int destID, int linkID, int *newID, void *pContext) {
	RESULT r = R_OK;

	*newID = g_AMONmap->m_mapID;
	CRM(AddAMONNode(g_AMONmap, destID, linkID, g_AMONmap->m_mapID, pContext),
			"RegisterNewDevice: Failed to add node to node %d at link %d", destID, linkID);

Error:
	return r;
}

RESULT UnsetAMONMaster() {
	g_amon.MasterState = AMON_MASTER_FALSE;
	return R_OK;
}


inline RESULT HandleAMONByte(AMON_LINK link, unsigned char byte) {
	RESULT r = R_OK;

	CBRM_NA((link_input_c[link] < MAX_MSG_LENGTH), "AMONRx: Buffer full!");

	// Add to the buffer
	link_input[link][link_input_c[link]] = byte;
	link_input_c[link]++;

	switch(g_LinkRxState[link]) {
		case AMON_RX_READY: {
			// Something went wrong
			if(byte != AMON_VALUE) {
				DEBUG_LINEOUT("HandleAMONByte: Byte value received 0x%x, expected AMON_VALUE link rx lock busy: %d", byte,  AMONLinkBusy(link));

				// TODO: Better error handling - fix the reset on both sides
				//CRM_NA(SendErrorResetLink(link, g_linkMessageType[link]), "AMONRx: Failed to send error and reset link");

				// Something is clearly wrong, reinitialize the link
				// This is often happening with ping/echo PHY bytes
				CRM(InitializeLink(link), "SendErrorResetLink: Failed to initialize and reset link %d", link);

				// TODO: This is a bridge gap solution
				// Reset the buffer - ignore the issue (but it won't take down the link for now, just drop the message)
				link_input_c[link] = 0;
			}
			else {
				g_LinkRxState[link] = AMON_RX_AMON_RECEIVED;
				LockAMONLinkRx(link);	// don't fail if this condition isn't met
			}
		} break;

		case AMON_RX_AMON_RECEIVED: {
			// Cannot detect an error here / we are guaranteed to have at least 4 bytes so this is safe
			g_linkMessageLength[link] = (unsigned int)byte;
			g_LinkRxState[link] = AMON_RX_LENGTH_RECEIVED;
		} break;

		case AMON_RX_LENGTH_RECEIVED: {
			g_linkMessageType[link] = (AMON_MESSAGE_TYPE)byte;

			if(g_linkMessageType[link] >= AMON_INVALID || g_linkMessageType[link] == AMON_NULL) {
				DEBUG_LINEOUT("HandleAMONByte: Message type invalid 0x%x", byte);
				CRM_NA(SendErrorResetLink(link, g_linkMessageType[link]), "AMONRx: Failed to send error and reset link");
			}
			else {
				g_LinkRxState[link] = AMON_RX_TYPE_RECEIVED;
			}
		} break;

		case AMON_RX_DATA:
		case AMON_RX_TYPE_RECEIVED: {
			if(link_input_c[link] >= g_linkMessageLength[link]) {
				// Message has been completely received!
				// TODO: Move this to casting the packet to AMONPacket rather than using the buffer
				UnlockAMONLinkRx(link);	// don't fail if this condition isn't met

				//CRM(HandleAMONPacket(link), "AMONRx: Failed to handle packet on link %d", link);

				// Push the packet into the queue
				AMONPacket *pAMONPacket = (AMONPacket*)malloc(sizeof(unsigned char) * link_input_c[link]);
				CNRM_NA(pAMONPacket, "HandleAMONByte: Failed to allocate AMON Packet");

				memcpy(pAMONPacket, link_input[link], link_input_c[link]);
				CRM(PushAMONIncomingQueuePacket(link, pAMONPacket),
						"HandleAMONByte: Failed to copy and push new packet to incoming queue link %d", link);

				// Clear up the static buffer
				link_input_c[link] = 0;
				g_LinkRxState[link] = AMON_RX_READY;
				g_linkMessageLength[link] = 0;
				g_linkMessageType[link] = AMON_NULL;

				g_LinkRxState[link] = AMON_RX_READY;	// Reset the link protocol state

#ifdef AMON_HALF_DUPLEX
				CBRM_NA((g_AMONLinkPhyPacketCount[link] != 0), "HandleAMONByte: Phy packet counter should not be zero");
				g_AMONLinkPhyPacketCount[link]--;

				#ifdef AMON_VERBOSE
					DEBUG_LINEOUT("Packet received %d packets remain on link %d", g_AMONLinkPhyPacketCount[link], link);
				#endif
#endif
			}
			else {
				g_LinkRxState[link] = AMON_RX_DATA;
			}
		} break;

		case AMON_RX_INVALID: {
			DEBUG_LINEOUT("Error: AMON link %d in invalid state, resetting", link);
			CRM_NA(SendErrorResetLink(link, g_linkMessageType[link]), "AMONRx: Failed to send error and reset link");
		} break;
	}	// !switch(g_LinkRxState[link]) {

	return r;

Error:
	// Clear up the static buffer
	link_input_c[link] = 0;
	g_LinkRxState[link] = AMON_RX_READY;
	g_linkMessageLength[link] = 0;
	g_linkMessageType[link] = AMON_NULL;

	g_LinkRxState[link] = AMON_RX_READY;	// Reset the link protocol state

	return r;
}

unsigned char CalculateChecksum(unsigned char *pBuffer, int pBuffer_n) {
	int i = 0;
	unsigned char checksum = 0x00;

	for(i = 0; i < pBuffer_n - 1; i++)
		checksum += (signed char)pBuffer[i];

	checksum *= -1;
	pBuffer[pBuffer_n - 1] = checksum;

	return checksum;
}

RESULT HandleAMONPing(AMON_LINK link, AMONPingPacket *d_pAMONPingPacket) {
	RESULT r = R_OK;

	//#ifdef AMON_VERBOSE
		DEBUG_LINEOUT("Received PING on link %d from device %d to device %d",
				link, d_pAMONPingPacket->m_originID, d_pAMONPingPacket->m_destID);
	//#endif

	if(g_amon.id == d_pAMONPingPacket->m_destID ) {
		CRM(SendEchoNetwork(link, d_pAMONPingPacket->m_originID),
				"HandleAMONPacket: Failed to echo ID to device %d on link %d", d_pAMONPingPacket->m_originID, link);
	}
	else {
		CRM(PassThruAMONBuffer(link, (unsigned char*)d_pAMONPingPacket, d_pAMONPingPacket->m_header.m_length),
				"HandleAMONPacket: Failed to pass through message from link %d", link);
	}

Error:
	if(d_pAMONPingPacket != NULL) {
		free(d_pAMONPingPacket);
		d_pAMONPingPacket = NULL;
	}

	return r;
}

RESULT HandleAMONEcho(AMON_LINK link, AMONEchoPacket *d_pAMONEchoPacket) {
	RESULT r = R_OK;

	//#ifdef AMON_VERBOSE
		DEBUG_LINEOUT("Received ECHO on link %d from device %d to device %d", link, d_pAMONEchoPacket->m_originID, d_pAMONEchoPacket->m_destID);
	//#endif

	if(g_amon.id == d_pAMONEchoPacket->m_destID ) {

		#ifdef AMON_VERBOSE
			DEBUG_LINEOUT("Received ECHO response to prior ping on link %d from %d to address %d", link, d_pAMONEchoPacket->m_originID, d_pAMONEchoPacket->m_destID);
		#endif

		if(g_amon.links[link].fPendingLinkStatus != 0)
			g_amon.links[link].LinkStatusCounter = 0;
	}
	else {
		CRM(PassThruAMONBuffer(link, (unsigned char*)d_pAMONEchoPacket, d_pAMONEchoPacket->m_header.m_length),
				"HandleAMONPacket: Failed to pass through message from link %d", link);
	}

Error:
	if(d_pAMONEchoPacket != NULL) {
		free(d_pAMONEchoPacket);
		d_pAMONEchoPacket = NULL;
	}
	return r;
}

RESULT HandleAMONResetLink(AMON_LINK link, AMONResetLinkPacket *d_pAMONResetLinkPacket) {
	RESULT r = R_OK;

	DEBUG_LINEOUT("HandleAMONPacket: Reset link message on link %d, sending ACK", link);
	CRM(SendResetLinkACK(link), "HandleAMONPacket: Failed to send reset link ACK on link %d", link);

	//CRM(ResetLink(link), "HandleAMONPacket: Failed to reset link %d on reset link", link);
	CRM(DisconnectLink(link), "HandleAMONPacket: Failed to disconnect link %d on reset link", link);

Error:
	if(d_pAMONResetLinkPacket != NULL) {
		free(d_pAMONResetLinkPacket);
		d_pAMONResetLinkPacket = NULL;
	}
	return r;
}

RESULT HandleAMONResetLinkAck(AMON_LINK link, AMONResetLinkAckPacket *d_pAMONResetLinkAckPacket) {
	RESULT r = R_OK;

	DEBUG_LINEOUT("HandleAMONPacket: Reset link message on link %d, sending ACK", link);
	//CRM(ResetLink(link), "HandleAMONPacket: Failed to reset link %d on reset link ACK", link);
	CRM(DisconnectLink(link), "HandleAMONPacket: Failed to disconnect link %d on reset link ACK", link);

Error:
	if(d_pAMONResetLinkAckPacket != NULL) {
		free(d_pAMONResetLinkAckPacket);
		d_pAMONResetLinkAckPacket = NULL;
	}
	return r;
}

RESULT HandleAMONRequestID(AMON_LINK link, AMONRequestIDPacket *d_pAMONRequestIDPacket) {
	RESULT r = R_OK;
	int i = 0;

	DEBUG_LINEOUT("Received ID request on link %d from device connected to %d on link %d",
			link, d_pAMONRequestIDPacket->m_linkDeviceID, d_pAMONRequestIDPacket->m_linkID);

	if(g_amon.id == AMON_MASTER_ID && g_amon.status == AMON_DEVICE_OK && g_amon.MasterState != AMON_MASTER_FALSE) {
		// We are the master, so dispatch a new ID
		int newID;

		// TODO: new device connected CB
		CRM(RegisterNewDevice(d_pAMONRequestIDPacket->m_linkDeviceID, d_pAMONRequestIDPacket->m_linkID, &newID, NULL),
				"HandleAMONPacket: Failed to register new device on %d link %d", d_pAMONRequestIDPacket->m_linkDeviceID, d_pAMONRequestIDPacket->m_linkID);

		// Send assign device out on all established links	// TODO: Is this th right behavior? We know what link the request came in on
		for(i = 0; i < NUM_LINKS; i++) {
			if(g_AMONLinkStates[i] == AMON_LINK_ESTABLISHED) {
				CRM(SendAssignID(i, d_pAMONRequestIDPacket->m_linkDeviceID, d_pAMONRequestIDPacket->m_linkID, newID),
						"HandleAMONPacket: Failed to assign ID %d to dev con to %d link %d on link %d",
							newID, d_pAMONRequestIDPacket->m_linkDeviceID, d_pAMONRequestIDPacket->m_linkID, i);
			}
		}
	}
	else {
		CRM(PassThruAMONBuffer(link, (unsigned char*)d_pAMONRequestIDPacket, d_pAMONRequestIDPacket->m_header.m_length),
				"HandleAMONPacket: Failed to pass through message from link %d", link);
	}

Error:
	if(d_pAMONRequestIDPacket != NULL) {
		free(d_pAMONRequestIDPacket);
		d_pAMONRequestIDPacket = NULL;
	}
	return r;
}

RESULT HandleAMONAssignID(AMON_LINK link, AMONAssignIDPacket *d_pAMONAssignIDPacket) {
	RESULT r = R_OK;

	DEBUG_LINEOUT("Received ID ASSIGN on link %d fom device connected to %d on link %d with ID %d",
			link, d_pAMONAssignIDPacket->m_linkDeviceID, d_pAMONAssignIDPacket->m_linkID, d_pAMONAssignIDPacket->m_newID);

	if(g_amon.links[link].id == d_pAMONAssignIDPacket->m_linkDeviceID && g_amon.links[link].link_id == d_pAMONAssignIDPacket->m_linkID) {

		// We just got an id assigned
		g_amon.id = d_pAMONAssignIDPacket->m_newID;
		g_amon.links[link].fLinkToMaster = 1;
		g_amon.status = AMON_DEVICE_OK;

		CRM(SendACK(link, AMON_MASTER_ID, AMON_ACK_ASSIGN_ID, 0x00),
				"HandleAMONPacket: Failed to send assign ID ack on link %d", link);

		// Send device ID on link so neighbor has the link info
		CRM(SendDeviceID(link), "AMONRx: Failed to Send Device ID on link %d on assign", link);

		// ID assigned, link establisehd - fire call back
		if(g_AMONLinkEstablishedCallback != NULL) {
			CRM(g_AMONLinkEstablishedCallback(link), "HandleAMONAssignID: Failed AMON link %d established CB", link);
		}
	}
	else {
		CRM(PassThruAMONBuffer(link, (unsigned char*)d_pAMONAssignIDPacket, d_pAMONAssignIDPacket->m_header.m_length),
				"HandleAMONPacket: Failed to pass through message from link %d", link);
	}

Error:
	if(d_pAMONAssignIDPacket != NULL) {
		free(d_pAMONAssignIDPacket);
		d_pAMONAssignIDPacket = NULL;
	}
	return r;
}

RESULT HandleAMONAck(AMON_LINK link, AMONAckPacket *d_pAMONAckPacket) {
	RESULT r = R_OK;

	/*int originID = AMONToShort(pBuffer[3], pBuffer[4]);
	int destID = AMONToShort(pBuffer[5], pBuffer[6]);
	AMON_ACK_TYPE type = (AMON_ACK_TYPE)pBuffer[7];
	unsigned char status = (unsigned char)pBuffer[8];*/

	if(g_amon.id == d_pAMONAckPacket->m_destID) {
		DEBUG_LINEOUT("Received ACK on link %d from device %d type 0x%x status 0x%x",
				link, d_pAMONAckPacket->m_originID, d_pAMONAckPacket->m_ackType, d_pAMONAckPacket->m_ackStatus);

		switch(d_pAMONAckPacket->m_ackType) {
			case AMON_ACK_ASSIGN_ID: {
				// Device has confirmed registration, fire the callback
				if(g_AMONDeviceRegisteredCallback != NULL) {
					CRM(g_AMONDeviceRegisteredCallback(d_pAMONAckPacket->m_originID), "HandleAMONAck: Failed device registered CB id %d", d_pAMONAckPacket->m_originID);
				}
			} break;

			case AMON_ACK_SEND: {
				// TODO: Handle ACK
			} break;
		}
	}
	else {
		CRM(PassThruAMONBuffer(link, (unsigned char*)d_pAMONAckPacket, d_pAMONAckPacket->m_header.m_length),
				"HandleAMONPacket: Failed to pass through message from link %d", link);
	}

Error:
	if(d_pAMONAckPacket != NULL) {
		free(d_pAMONAckPacket);
		d_pAMONAckPacket = NULL;
	}

	return r;
}

/* TODO
RESULT HandleAMONBroadcast(AMON_LINK link, AMONBroadcastPacket *pAMONBroadcastPacket) {
	RESULT r = R_OK;

	// TODO

Error:
	return r;
}
*/

RESULT HandleAMONSend(AMON_LINK link, AMONSendPacket *d_pAMONSendPacket) {
	RESULT r = R_OK;
	int i = 0;

	#ifdef AMON_VERBOSE
		DEBUG_LINEOUT("Received AMON_SEND (message) on link %d from device %d to device %d type %d",
				link, d_pAMONSendPacket->m_originID, d_pAMONSendPacket->m_destID, d_pAMONSendPacket->m_sendMessageType);
		PrintToOutputBinaryBuffer(g_pConsole, d_pAMONSendPacket, d_pAMONSendPacket->m_header->m_length, 10);
	#endif

	// Check to see if we're the destination, otherwise pass it on
	if(g_amon.id == d_pAMONSendPacket->m_destID ) {
		// Make a copy of the data so it doesn't get clobbered
		unsigned char *pPayloadBuffer = (unsigned char*)calloc(sizeof(unsigned char), d_pAMONSendPacket->m_payloadLength);

		// TODO: wtf isn't this working
		//memcpy(pPayloadBuffer, (unsigned char*)(d_pAMONSendPacket + 9), sizeof(unsigned char) * d_pAMONSendPacket->m_payloadLength);
		for(i = 0; i < d_pAMONSendPacket->m_payloadLength; i++) {
			pPayloadBuffer[i] = ((unsigned char *)(d_pAMONSendPacket))[9 + i];
		}

		// Note: The handler needs to delete the memory after it's been used
		// TODO: Create an incoming message queue?

		///*

		//CRM(g_HandleAMONPayloadCallback(link, originDeviceID, type, pPayloadBuffer, payload_n),
		//					"HandleAMONPacket: Failed to receive amon msg from device %d on link %d", originDeviceID, link);

		CRM(g_HandleAMONPayloadCallback(link, d_pAMONSendPacket->m_originID, d_pAMONSendPacket->m_sendMessageType, pPayloadBuffer, d_pAMONSendPacket->m_payloadLength),
			"HandleAMONPacket: Failed to receive amon msg from device %d on link %d", d_pAMONSendPacket->m_originID, link);
		//*/
	}
	else {
		CRM(PassThruAMONBuffer(link, (unsigned char*)d_pAMONSendPacket, d_pAMONSendPacket->m_header.m_length),
				"HandleAMONPacket: Failed to pass through message from link %d", link);
	}
Error:
	if(d_pAMONSendPacket != NULL) {
		free(d_pAMONSendPacket);
		d_pAMONSendPacket = NULL;
	}

	return r;
}

RESULT HandleAMONGetDeviceID(AMON_LINK link, AMONGetDeviceIDPacket *d_pAMONGetDeviceIDPacket) {
	RESULT r = R_OK;

	// Other device is requesting our ID
	g_amon.links[link].Status = d_pAMONGetDeviceIDPacket->m_originDeviceStatus;		// get the device status

	if(g_amon.links[link].Status == AMON_DEVICE_OK) {
		g_amon.links[link].id = d_pAMONGetDeviceIDPacket->m_originDeviceID;
		g_AMONLinkStates[link] = AMON_LINK_ESTABLISHING_LINK;

		DEBUG_LINEOUT("Rx: AMON_GET_ID with id %d status 0x%x from OK device",
				g_amon.links[link].id, g_amon.links[link].Status);

		// Send Establish Link Message
		CRM(SendEstablishLink(link), "AMONRx: Failed to Send Establish Link on link %d", link);
	}
	else {
		g_amon.links[link].id = -1;

		DEBUG_LINEOUT("Rx: AMON_GET_ID with id %d status 0x%x from unassigned device",
				g_amon.links[link].id, g_amon.links[link].Status);

		// At this point we can figure out if we might be a master
		CRM_NA(SelfAssignedMasterOnLink(link), "AMONRx: Failed to self assign master");

		CRM(SendDeviceID(link), "AMONRx: Failed to Send Device ID on link %d", link);
		g_AMONLinkStates[link] = AMON_LINK_ID_SENT;
	}

Error:
	if(d_pAMONGetDeviceIDPacket != NULL) {
		free(d_pAMONGetDeviceIDPacket);
		d_pAMONGetDeviceIDPacket = NULL;
	}

	return r;
}

RESULT HandleAMONSendDeviceID(AMON_LINK link, AMONSendDeviceIDPacket *d_pAMONSendDeviceIDPacket) {
	RESULT r = R_OK;

	// Get the device status
	g_amon.links[link].Status = d_pAMONSendDeviceIDPacket->m_deviceStatus;

	// Is the link already established?
	if(g_AMONLinkStates[link] == AMON_LINK_ESTABLISHED) {
		int rxID = d_pAMONSendDeviceIDPacket->m_deviceID;
		DEBUG_LINEOUT("Rx: Neighbor ID %d received on link %d", rxID, link);
		g_amon.links[link].id = rxID;
	}
	else {

		// ID has been received from other device during link establish
		if(g_amon.links[link].Status == AMON_DEVICE_OK) {
			g_amon.links[link].id = d_pAMONSendDeviceIDPacket->m_deviceID;
			g_AMONLinkStates[link] = AMON_LINK_ESTABLISHING_LINK;

			DEBUG_LINEOUT("Rx: AMON_SEND_ID with id %d status 0x%x from OK device",
					g_amon.links[link].id, g_amon.links[link].Status);

			// Send Establish Link Message
			CRM(SendEstablishLink(link), "AMONRx: Failed to Send Establish Link on link %d", link);
		}
		else {
			g_amon.links[link].id = -1;
			g_AMONLinkStates[link] = AMON_LINK_ESTABLISHING_LINK;

			DEBUG_LINEOUT("Rx: AMON_SEND_ID with id %d status 0x%x from unassigned device",
					g_amon.links[link].id, g_amon.links[link].Status);

			// At this point we can figure out if we might be a master
			CRM_NA(SelfAssignedMasterOnLink(link), "AMONRx: Failed to self assign master");

			// We must have established ourselves as a master in the previous stepso Resend our ID
			if(g_amon.MasterState == AMON_MASTER_SELF_DEFINED)
				CRM(SendDeviceID(link), "AMONRx: Failed to Send Device ID on link %d", link);
		}
	}

Error:
	if(d_pAMONSendDeviceIDPacket != NULL) {
		free(d_pAMONSendDeviceIDPacket);
		d_pAMONSendDeviceIDPacket = NULL;
	}

	return r;
}

RESULT HandleAMONEstablishLink(AMON_LINK link, AMONEstablishLinkPacket *d_pAMONEstablishLinkPacket) {
	RESULT r = R_OK;

	// Lets check that the ID matches our own
	// link established should only be received when we already have an ID and they don't
	int rxID = (int)(d_pAMONEstablishLinkPacket->m_originID);
	CBRM((rxID == g_amon.id), "Failed to establish link, incoming id %d not self id %d", rxID, g_amon.id);

	DEBUG_LINEOUT("Rx: AMON_ESTABLISH_LINK with id %d on link %d", rxID, link);

	// Establish link receipt lets us set the device's link
	unsigned char rxLinkID = (unsigned char)(d_pAMONEstablishLinkPacket->m_linkID);
	g_amon.links[link].link_id = rxLinkID;

	// Send Establish Link Message ACK
	CRM(SendEstablishLinkAck(link), "AMONRx: Failed to Send Establish Link on link %d", link);

	g_AMONLinkStates[link] = AMON_LINK_ESTABLISHED;

	// Link has been established
	if(g_AMONLinkEstablishedCallback != NULL) {
		CRM(g_AMONLinkEstablishedCallback(link), "HandleAMONEstablishLink: Failed AMON Link %d establish CB", link);
	}

Error:
	if(d_pAMONEstablishLinkPacket != NULL) {
		free(d_pAMONEstablishLinkPacket);
		d_pAMONEstablishLinkPacket = NULL;
	}

	return r;
}

RESULT HandleAMONEstablishLinkAck(AMON_LINK link, AMONEstablishLinkAckPacket *d_pAMONEstablishLinkAckPacket) {
	RESULT r = R_OK;

	// Lets check that the ID matches the link ID
	// link established should only be received when we already have an ID and they don't
	int rxID = d_pAMONEstablishLinkAckPacket->m_senderID;
	CBRM((rxID == g_amon.links[link].id), "Failed to establish link, incoming id %d not correct id expected: %d", rxID, g_amon.links[link].id);

	DEBUG_LINEOUT("Rx: AMON_ESTABLISH_LINK_ACK with id %d on link %d", rxID, link);

	// Get the link info of the other device
	g_amon.links[link].link_id = (int)(d_pAMONEstablishLinkAckPacket->m_linkID);

	// Link is now established
	g_AMONLinkStates[link] = AMON_LINK_ESTABLISHED;

	// Lets request an ID from the network now
	// Since the slave sends the establish link, we know this to be the case
	// TODO: Add more protection here
	CRM_NA(SendRequestIDFromNetwork(link), "AMONRx: Failed to send request ID from network");

Error:
	if(d_pAMONEstablishLinkAckPacket != NULL) {
		free(d_pAMONEstablishLinkAckPacket);
		d_pAMONEstablishLinkAckPacket = NULL;
	}

	return r;
}

RESULT HandleAMONError(AMON_LINK link, AMONErrorPacket *d_pAMONErrorPacket) {
	RESULT r = R_OK;

	// Reset the link - master might try to reconnect
	CRM(InitializeLink(link), "SendErrorResetLink: Failed to initialize and reset link %d", link);
	//CRM(ResetLink(link), "SendErrorResetLink: Failed to reset link %d", link);

	DEBUG_LINEOUT("HandleAMONPacket: Error: Received error on link %d for message type 0x%x", link, d_pAMONErrorPacket->m_messageType);

	#ifdef AMON_VERBOSE
		PrintToOutputBinaryBuffer(g_pConsole, pBuffer, pBuffer_n, 10);
	#endif

Error:
	if(d_pAMONErrorPacket != NULL) {
		free(d_pAMONErrorPacket);
		d_pAMONErrorPacket = NULL;
	}

	return r;
}

RESULT HandleAMONSendByteDestLink(AMON_LINK link, AMONSendByteDestLinkPacket *d_pAMONSendByteDestLinkPacket) {
	RESULT r = R_OK;
//			SetLEDLinkClearTimeout(destLink, 50, 50, 50, 100);

	if(g_amon.id == d_pAMONSendByteDestLinkPacket->m_destID) {
		DEBUG_LINEOUT("Received SEND BYTE ON DEST LINK on link %d from device %d byte 0x%x", link, d_pAMONSendByteDestLinkPacket->m_originID, d_pAMONSendByteDestLinkPacket->m_byte);
//				SetLEDLinkClearTimeout(destLink, 50, 0, 50, 100);
		CRM_NA(SendByte(d_pAMONSendByteDestLinkPacket->m_destLinkID, d_pAMONSendByteDestLinkPacket->m_byte), "HandleAMONPacket: Failed to send byte");
	}
	else {
		CRM(PassThruAMONBuffer(link, (unsigned char*)d_pAMONSendByteDestLinkPacket, d_pAMONSendByteDestLinkPacket->m_header.m_length),
				"HandleAMONPacket: Failed to pass through message from link %d", link);
	}

Error:
	if(d_pAMONSendByteDestLinkPacket != NULL) {
		free(d_pAMONSendByteDestLinkPacket);
		d_pAMONSendByteDestLinkPacket = NULL;
	}

	return r;
}

RESULT HandleAMONPacket(AMON_LINK link, AMONPacket *d_pAMONPacket) {
	RESULT r = R_OK;
	unsigned char checksum = CalculateChecksum(d_pAMONPacket, d_pAMONPacket->m_length);
	unsigned char packetChecksum = ((unsigned char*)(d_pAMONPacket))[d_pAMONPacket->m_length - 1];

#ifdef AMON_VERBOSE
	DEBUG_LINEOUT("rx: AMON packet %d bytes", d_pAMONPacket->m_length);
	PrintToOutputBinaryBuffer(g_pConsole, d_pAMONPacket, d_pAMONPacket->m_length, 10);
#endif

	CBRM((checksum == packetChecksum), "HandleAMONPacket: Checksum mismatch 0x%x 0x%x", checksum, packetChecksum);

	ReportLinkActivity(link);

	switch(d_pAMONPacket->m_type) {
		case AMON_NULL: {
			// TODO: ?
		} break;

		case AMON_PING: {
			CRM(HandleAMONPing(link, (AMONPingPacket*)d_pAMONPacket),
					"HandleAMONPacket: Failed to handle ping packet of link %d", link);
		} break;

		case AMON_ECHO: {
			CRM(HandleAMONEcho(link, (AMONEchoPacket*)d_pAMONPacket),
					"HandleAMONPacket: Failed to handle echo packet of link %d", link);
		} break;

		case AMON_RESET_LINK: {
			CRM(HandleAMONResetLink(link, (AMONResetLinkPacket*)d_pAMONPacket),
					"HandleAMONPacket: Failed to handle reset link packet of link %d", link);
		} break;

		case AMON_RESET_LINK_ACK: {
			CRM(HandleAMONResetLinkAck(link, (AMONResetLinkAckPacket*)d_pAMONPacket),
					"HandleAMONPacket: Failed to handle reset link packet of link %d", link);
		} break;

		case AMON_REQUEST_ID: {
			CRM(HandleAMONRequestID(link, (AMONRequestIDPacket*)d_pAMONPacket),
					"HandleAMONPacket: Failed to handle request id packet of link %d", link);
		} break;

		case AMON_ASSIGN_ID: {
			CRM(HandleAMONAssignID(link, (AMONAssignIDPacket*)d_pAMONPacket),
					"HandleAMONPacket: Failed to handle assign id packet of link %d", link);
		} break;

		case AMON_ACK: {
			CRM(HandleAMONAck(link, (AMONAckPacket*)d_pAMONPacket),
					"HandleAMONPacket: Failed to handle ack packet of link %d", link);
		} break;

		case AMON_BROADCAST: {
			// TODO
			/*CRM(HandleAMONBroadcast(link, (AMONbroadcastPacket*)d_pAMONPacket),
					"HandleAMONPacket: Failed to handle broadcast packet of link %d", link);*/
		} break;

		case AMON_SEND: {
			CRM(HandleAMONSend(link, (AMONSendPacket*)d_pAMONPacket),
					"HandleAMONPacket: Failed to handle send packet of link %d", link);
		} break;

		case AMON_GET_ID: {
			CRM(HandleAMONGetDeviceID(link, (AMONGetDeviceIDPacket*)d_pAMONPacket),
						"HandleAMONPacket: Failed to handle get device id packet of link %d", link);
		} break;

		case AMON_SEND_ID: {
			CRM(HandleAMONSendDeviceID(link, (AMONSendDeviceIDPacket*)d_pAMONPacket),
						"HandleAMONPacket: Failed to handle send device id packet of link %d", link);
		} break;

		case AMON_ESTABLISH_LINK: {
			CRM(HandleAMONEstablishLink(link, (AMONEstablishLinkPacket*)d_pAMONPacket),
						"HandleAMONPacket: Failed to handle Establish Link packet of link %d", link);
		} break;

		case AMON_ESTABLISH_LINK_ACK: {
			CRM(HandleAMONEstablishLinkAck(link, (AMONEstablishLinkAckPacket*)d_pAMONPacket),
						"HandleAMONPacket: Failed to handle Establish Link Ack packet of link %d", link);
		} break;

		case AMON_ERROR: {
			CRM(HandleAMONError(link, (AMONErrorPacket*)d_pAMONPacket),
						"HandleAMONPacket: Failed to handle Error packet of link %d", link);
		} break;

		case AMON_SEND_BYTE_DEST_LINK: {
			CRM(HandleAMONSendByteDestLink(link, (AMONSendByteDestLinkPacket*)d_pAMONPacket),
						"HandleAMONPacket: Failed to handle Send Byte Dest Link packet of link %d", link);
		} break;

		default: {
			DEBUG_LINEOUT("HandleAMONPacket: Unhandled packet of type %d", d_pAMONPacket->m_type);
		} break;
	}

	return r;

Error:
	if(d_pAMONPacket != NULL) {
		free(d_pAMONPacket);
		d_pAMONPacket = NULL;
	}

	CRM_NA(SendErrorResetLink(link, g_linkMessageType[link]), "AMONRx: Failed to send error and reset link");
	return r;
}

// TODO: Switch this over to the AMONPacket paradigm
RESULT HandleAMONPacket_old(AMON_LINK link) {
	RESULT r = R_OK;
	unsigned char checksum = CalculateChecksum(link_input[link], link_input_c[link]);
	unsigned char *pBuffer = link_input[link];
	int pBuffer_n = link_input_c[link];
	int i = 0;

#ifdef AMON_VERBOSE
	DEBUG_LINEOUT("rx: AMON packet %d bytes", pBuffer_n);
	PrintToOutputBinaryBuffer(g_pConsole, pBuffer, pBuffer_n, 10);
#endif

	CBRM((checksum == pBuffer[pBuffer_n - 1]), "HandleAMONPacket: Checksum mismatch 0x%x 0x%x", checksum, pBuffer[pBuffer_n - 1]);

	AMON_MESSAGE_TYPE mType = g_linkMessageType[link];
	switch(mType) {
		case AMON_NULL: {

		} break;

		case AMON_PING: {
			int originDeviceID = AMONToShort(pBuffer[3], pBuffer[4]);
			int addressDeviceID = AMONToShort(pBuffer[5], pBuffer[6]);

			//#ifdef AMON_VERBOSE
				DEBUG_LINEOUT("Received PING on link %d from device %d to device %d", link, originDeviceID, addressDeviceID);
			//#endif

			if(g_amon.id == addressDeviceID ) {
				CRM(SendEchoNetwork(link, originDeviceID), "HandleAMONPacket: Failed to echo ID to device %d on link %d", originDeviceID, link);
			}
			else {
				CRM(PassThruAMONBuffer(link, pBuffer, pBuffer_n), "HandleAMONPacket: Failed to pass through message from link %d", link);
			}
		} break;

		case AMON_ECHO: {
			int originDeviceID = AMONToShort(pBuffer[3], pBuffer[4]);
			int addressDeviceID = AMONToShort(pBuffer[5], pBuffer[6]);

			//#ifdef AMON_VERBOSE
				DEBUG_LINEOUT("Received ECHO on link %d from device %d to device %d", link, originDeviceID, addressDeviceID);
			//#endif

			if(g_amon.id == addressDeviceID ) {

				#ifdef AMON_VERBOSE
					DEBUG_LINEOUT("Received ECHO response to prior ping on link %d from %d to address %d", link, originDeviceID, addressDeviceID);
				#endif

				if(g_amon.links[link].fPendingLinkStatus != 0)
					g_amon.links[link].LinkStatusCounter = 0;
			}
			else {
				CRM(PassThruAMONBuffer(link, pBuffer, pBuffer_n), "HandleAMONPacket: Failed to pass through message from link %d", link);
			}
		} break;

		case AMON_RESET_LINK: {
			DEBUG_LINEOUT("HandleAMONPacket: Reset link message on link %d, sending ACK", link);
			CRM(SendResetLinkACK(link), "HandleAMONPacket: Failed to send reset link ACK on link %d", link);

			//CRM(ResetLink(link), "HandleAMONPacket: Failed to reset link %d on reset link", link);
			CRM(DisconnectLink(link), "HandleAMONPacket: Failed to disconnect link %d on reset link", link);

		} break;

		case AMON_RESET_LINK_ACK: {
			DEBUG_LINEOUT("HandleAMONPacket: Reset link message on link %d, sending ACK", link);
			//CRM(ResetLink(link), "HandleAMONPacket: Failed to reset link %d on reset link ACK", link);
			CRM(DisconnectLink(link), "HandleAMONPacket: Failed to disconnect link %d on reset link ACK", link);
		} break;

		case AMON_REQUEST_ID: {
			unsigned char linkID = pBuffer[3];
			unsigned short addressDeviceID = AMONToShort(pBuffer[4], pBuffer[5]);

			DEBUG_LINEOUT("Received ID request on link %d from device connected to %d on link %d", link, addressDeviceID, linkID);

			if(g_amon.id == AMON_MASTER_ID && g_amon.status == AMON_DEVICE_OK && g_amon.MasterState != AMON_MASTER_FALSE) {
				// We are the master, so dispatch a new ID
				int newID;
				CRM(RegisterNewDevice(addressDeviceID, linkID, &newID, NULL),
						"HandleAMONPacket: Failed to register new device on %d link %d", addressDeviceID, linkID);

				// Send assign device out on all established links	// TODO: Is this th right behavior? We know what link the request came in on
				for(i = 0; i < NUM_LINKS; i++) {
					if(g_AMONLinkStates[i] == AMON_LINK_ESTABLISHED) {
						CRM(SendAssignID(i, addressDeviceID, linkID, newID),
								"HandleAMONPacket: Failed to assign ID %d to dev con to %d link %d on link %d",
								newID, addressDeviceID, linkID, i);
					}
				}
			}
			else {
				CRM(PassThruAMONBuffer(link, pBuffer, pBuffer_n), "HandleAMONPacket: Failed to pass through message from link %d", link);
			}
		} break;

		case AMON_ASSIGN_ID: {
			unsigned char linkID = pBuffer[3];
			int addressDeviceID = AMONToShort(pBuffer[4], pBuffer[5]);
			int newID = AMONToShort(pBuffer[6], pBuffer[7]);

			DEBUG_LINEOUT("Received ID ASSIGN on link %d fom device connected to %d on link %d with ID %d", link, addressDeviceID, linkID, newID);

			if(g_amon.links[link].id == addressDeviceID && g_amon.links[link].link_id == linkID) {
				// We just got an id assigned
				g_amon.id = newID;
				g_amon.links[link].fLinkToMaster = 1;
				g_amon.status = AMON_DEVICE_OK;
				CRM(SendACK(link, AMON_MASTER_ID, AMON_ASSIGN_ID, 0x00), "HandleAMONPacket: Failed to send assign ID ack on link %d", link);

				// Send device ID on link so neighbor has the link info
				CRM(SendDeviceID(link), "AMONRx: Failed to Send Device ID on link %d on assign", link);
			}
			else {
				CRM(PassThruAMONBuffer(link, pBuffer, pBuffer_n), "HandleAMONPacket: Failed to pass through message from link %d", link);
			}

		} break;

		case AMON_ACK: {
			int originID = AMONToShort(pBuffer[3], pBuffer[4]);
			int destID = AMONToShort(pBuffer[5], pBuffer[6]);
			AMON_ACK_TYPE type = (AMON_ACK_TYPE)pBuffer[7];
			unsigned char status = (unsigned char)pBuffer[8];

			if(g_amon.id == destID) {
				DEBUG_LINEOUT("Received ACK on link %d from device %d type 0x%x status 0x%x", link, originID, type, status);

				switch(type) {
					case AMON_ACK_ASSIGN_ID: {
						// TODO: Handle ACK?
					} break;

					case AMON_ACK_SEND: {
						// TODO: Handle ACK
					} break;
				}
			}
			else {
				CRM(PassThruAMONBuffer(link, pBuffer, pBuffer_n), "HandleAMONPacket: Failed to pass through message from link %d", link);
			}
		} break;

		case AMON_BROADCAST: {

		} break;

		// Check to see if we're the destination, otherwise pass it on
		case AMON_SEND: {
			int originDeviceID = AMONToShort(pBuffer[3], pBuffer[4]);
			int addressDeviceID = AMONToShort(pBuffer[5], pBuffer[6]);
			unsigned char type = pBuffer[7];
			unsigned char payload_n = pBuffer[8];

			#ifdef AMON_VERBOSE
				DEBUG_LINEOUT("Received AMON_SEND (message) on link %d from device %d to device %d type %d", link, originDeviceID, addressDeviceID, type);
				PrintToOutputBinaryBuffer(g_pConsole, pBuffer, pBuffer_n, 10);
			#endif

			if(g_amon.id == addressDeviceID ) {
				// Make a copy of the data so it doesn't get clobbered
				unsigned char *pPayloadBuffer = (unsigned char*)calloc(sizeof(unsigned char), payload_n);
				memcpy((void*)(pPayloadBuffer), (void*)(pBuffer + 9), sizeof(unsigned char) * payload_n);

				// Note: The handler needs to delete the memory after it's been used
				// TODO: Create an incoming message queue?
				/*
				CRM(g_HandleAMONPayloadCallback(link, originDeviceID, type, pPayloadBuffer, payload_n),
					"HandleAMONPacket: Failed to receive amon msg from device %d on link %d", originDeviceID, link);
				*/
			}
			else {
				CRM(PassThruAMONBuffer(link, pBuffer, pBuffer_n), "HandleAMONPacket: Failed to pass through message from link %d", link);
			}
		} break;

		case AMON_GET_ID: {
			// Other device is requesting our ID
			g_amon.links[link].Status = pBuffer[3];		// get the device status
			unsigned short originDeviceID = AMONToShort(pBuffer[4], pBuffer[5]);

			// If we already have a non zero id then send the establish link
			if(g_amon.links[link].Status == AMON_DEVICE_OK) {
				g_amon.links[link].id = originDeviceID;
				g_AMONLinkStates[link] = AMON_LINK_ESTABLISHING_LINK;

				DEBUG_LINEOUT("Rx: AMON_GET_ID with id %d status 0x%x from OK device", g_amon.links[link].id, g_amon.links[link].Status);

				// Send Establish Link Message
				CRM(SendEstablishLink(link), "AMONRx: Failed to Send Establish Link on link %d", link);
			}
			else {
				g_amon.links[link].id = -1;

				DEBUG_LINEOUT("Rx: AMON_GET_ID with id %d status 0x%x from unassigned device", g_amon.links[link].id, g_amon.links[link].Status);

				// At this point we can figure out if we might be a master
				CRM_NA(SelfAssignedMasterOnLink(link), "AMONRx: Failed to self assign master");

				CRM(SendDeviceID(link), "AMONRx: Failed to Send Device ID on link %d", link);
				g_AMONLinkStates[link] = AMON_LINK_ID_SENT;
			}
		} break;

		case AMON_SEND_ID: {

			// Is the link already established?
			if(g_AMONLinkStates[link] == AMON_LINK_ESTABLISHED) {
				int rxID = AMONToShort(pBuffer[4], pBuffer[5]);
				DEBUG_LINEOUT("Rx: Neighbor ID %d received on link %d", rxID, link);
				g_amon.links[link].id = rxID;
			}
			else {
				// ID has been received from other device during link establish
				g_amon.links[link].Status = pBuffer[3];		// get the device status
				unsigned short originDeviceID = AMONToShort(pBuffer[4], pBuffer[5]);

				if(g_amon.links[link].Status == AMON_DEVICE_OK) {
					g_amon.links[link].id = originDeviceID;
					g_AMONLinkStates[link] = AMON_LINK_ESTABLISHING_LINK;

					DEBUG_LINEOUT("Rx: AMON_SEND_ID with id %d status 0x%x from OK device", g_amon.links[link].id, g_amon.links[link].Status);

					// Send Establish Link Message
					CRM(SendEstablishLink(link), "AMONRx: Failed to Send Establish Link on link %d", link);
				}
				else {
					g_amon.links[link].id = -1;
					g_AMONLinkStates[link] = AMON_LINK_ESTABLISHING_LINK;

					DEBUG_LINEOUT("Rx: AMON_SEND_ID with id %d status 0x%x from unassigned device", g_amon.links[link].id, g_amon.links[link].Status);

					// At this point we can figure out if we might be a master
					CRM_NA(SelfAssignedMasterOnLink(link), "AMONRx: Failed to self assign master");

					// We must have established ourselves as a master in the previous stepso Resend our ID
					if(g_amon.MasterState == AMON_MASTER_SELF_DEFINED)
						CRM(SendDeviceID(link), "AMONRx: Failed to Send Device ID on link %d", link);
				}
			}
		} break;

		case AMON_ESTABLISH_LINK: {
			// Lets check that the ID matches our own
			// link established should only be received when we already have an ID and they don't
			int rxID = (int)(AMONToShort(pBuffer[3], pBuffer[4]));
			CBRM((rxID == g_amon.id), "Failed to establish link, incoming id %d not self id %d", rxID, g_amon.id);

			DEBUG_LINEOUT("Rx: AMON_ESTABLISH_LINK with id %d on link %d", rxID, link);

			// Establish link receipt lets us set the device's link
			unsigned char rxLinkID = (unsigned char)(pBuffer[5]);
			g_amon.links[link].link_id = rxLinkID;

			// Send Establish Link Message ACK
			CRM(SendEstablishLinkAck(link), "AMONRx: Failed to Send Establish Link on link %d", link);

			g_AMONLinkStates[link] = AMON_LINK_ESTABLISHED;
		} break;

		case AMON_ESTABLISH_LINK_ACK: {
			// Lets check that the ID matches the link ID
			// link established should only be received when we already have an ID and they don't
			int rxID = AMONToShort(pBuffer[3], pBuffer[4]);
			CBRM((rxID == g_amon.links[link].id), "Failed to establish link, incoming id %d not correct id expected: %d", rxID, g_amon.links[link].id);

			DEBUG_LINEOUT("Rx: AMON_ESTABLISH_LINK_ACK with id %d on link %d", rxID, link);

			// Get the link info of the other device
			g_amon.links[link].link_id = (int)(pBuffer[5]);

			// Link is now established
			g_AMONLinkStates[link] = AMON_LINK_ESTABLISHED;

			// Lets request an ID from the network now
			// Since the slave sends the establish link, we know this to be the case
			// TODO: Add more protection here
			CRM_NA(SendRequestIDFromNetwork(link), "AMONRx: Failed to send request ID from network");
		} break;

		case AMON_ERROR: {
			unsigned char errorType = pBuffer[3];

			// Reset the link - master might try to reconnect
			CRM(InitializeLink(link), "SendErrorResetLink: Failed to initialize and reset link %d", link);
			//CRM(ResetLink(link), "SendErrorResetLink: Failed to reset link %d", link);

			DEBUG_LINEOUT("HandleAMONPacket: Error: Received error on link %d for message type 0x%x", link, errorType);

			#ifdef AMON_VERBOSE
				PrintToOutputBinaryBuffer(g_pConsole, pBuffer, pBuffer_n, 10);
			#endif
		} break;

		case AMON_SEND_BYTE_DEST_LINK: {
			int originID = AMONToShort(pBuffer[3], pBuffer[4]);
			int destID = AMONToShort(pBuffer[5], pBuffer[6]);
			AMON_LINK destLink = (AMON_LINK)pBuffer[7];
			unsigned char byte = (unsigned char)pBuffer[8];

//			SetLEDLinkClearTimeout(destLink, 50, 50, 50, 100);

			if(g_amon.id == destID) {
				DEBUG_LINEOUT("Received SEND BYTE ON DEST LINK on link %d from device %d byte 0x%x", link, originID, byte);
//				SetLEDLinkClearTimeout(destLink, 50, 0, 50, 100);
				CRM_NA(SendByte(destLink, byte), "HandleAMONPacket: Failed to send byte");
			}
			else {
				CRM(PassThruAMONBuffer(link, pBuffer, pBuffer_n), "HandleAMONPacket: Failed to pass through message from link %d", link);
			}
		}
	}

	// Reset the link (rx adapter) (no error)
	//ResetLink(link);

	link_input_c[link] = 0;
	g_LinkRxState[link] = AMON_RX_READY;
	g_linkMessageLength[link] = 0;
	g_linkMessageType[link] = AMON_NULL;

	return r;

Error:
	CRM_NA(SendErrorResetLink(link, g_linkMessageType[link]), "AMONRx: Failed to send error and reset link");
	return r;
}

RESULT SendErrorResetLink(AMON_LINK link, AMON_MESSAGE_TYPE type) {
	RESULT r = R_OK;

	DEBUG_LINEOUT("Error: 0x%x, resetting link %d", type, link);

	// SetLEDLinkClearTimeout(AMON_ALL, 50, 0, 0, 600);
	CRM(SendError(link, type), "SendErrorResetLink: Failed to send error %d on link %d", type, link);

	//CRM(ResetLink(link), "SendErrorResetLink: Failed to reset link %d", link);
	CRM(InitializeLink(link), "SendErrorResetLink: Failed to initialize and reset link %d", link);

Error:
	return r;
}

// This will pass a buffer through to all established links other than the incoming link
RESULT PassThruAMONBuffer(AMON_LINK link, unsigned char *pBuffer, int pBuffer_n) {
	RESULT r = R_OK;
	int i = 0;

	// We're not the master, so send this back out on all links not including this link
	for(i = 0; i < NUM_LINKS; i++) {
		if(i != link && g_AMONLinkStates[i] == AMON_LINK_ESTABLISHED) {

			//CRM(SendAMONBuffer(i, pBuffer, pBuffer_n), "HandleAMONPacket: Failed to pass message on to link %d", i);

			// This memory gets deallocated in the queue
			AMONPacket *pAMONPacket = (AMONPacket *)calloc(pBuffer_n, sizeof(unsigned char));
			CNRM_NA(pAMONPacket, "PassThruAMONBuffer: Failed to allocate AMON Packet");

			// Copy over the packet
			memcpy(pAMONPacket, pBuffer, pBuffer_n * sizeof(unsigned char));

			CRM(PushAndTransmitAMONQueuePacket(i, (AMONPacket *)pAMONPacket),
					"PassThruAMONBuffer: Failed to push and transmit AMON packet on link %d", link);
		}
	}

Error:
	return r;
}

RESULT SendAMONBuffer(AMON_LINK link, unsigned char *pBuffer, int pBuffer_n) {
	RESULT r = R_OK;
	int i = 0;

	// Spin wait to ensure we're not receiving - seems to hang once in a while
	//while(AMONLinkRxBusy(link));

	LockAMONLinkTx(link);	// don't fail if this condition isn't met
	unsigned char checksum = CalculateChecksum(pBuffer, pBuffer_n);

#ifdef AMON_VERBOSE
	DEBUG_LINEOUT("Tx: %d bytes", pBuffer_n);
	PrintToOutputBinaryBuffer(g_pConsole, pBuffer, pBuffer_n, 10);
#endif

	for(i = 0; i < pBuffer_n; i++) {
		// Spin wait until link not busy
		while(LinkBusy(link) == TRUE);
		CRM(SendByte(link, pBuffer[i]),
				"SendAMONBuffer: Failed SendByte on %d link", link);

		//DelayPHY();
	}

Error:
	UnlockAMONLinkTx(link);	// Don't fail if this condition isn't met
	return r;
}

RESULT SendMessageType(AMON_MESSAGE_TYPE type, short destID, ...) {
	RESULT r = R_OK;
	int i = 0;

	switch(type) {

		case AMON_PING: {
			for(i = 0; i < NUM_LINKS; i++) {
				if(g_AMONLinkStates[i] == AMON_LINK_ESTABLISHED) {
					CRM(SendPingNetwork(i, destID), "SendMessage: Failed to ping device %d on link %d", destID, i);
				}
			}
		} break;

		default: {

		} break;
	}

Error:
	return r;
}


// TODO: Make this work haha
RESULT SendMessagePayloadLink(AMON_LINK link, short destID, unsigned char type, unsigned char *payloadBuffer, int payloadBuffer_n) {
	RESULT r = R_OK;

	CBRM((payloadBuffer_n != 0), "SendMessagePayloadLink: Cannot send message of %d bytes", payloadBuffer_n);

	AMONSendPacket *pAMONSendPacket = CreateAMONSendPacket(destID, type, payloadBuffer, payloadBuffer_n);
	CNRM_NA(pAMONSendPacket, "SendMessagePayloadLink: Failed to allocate Send Packet");

#ifdef AMON_VERBOSE
	DEBUG_LINEOUT("SendMessagePayloadLink: Sending AMONSendPacket length %d to destID %d of type 0x%x", pAMONSendPacket->m_header.m_length, destID, type);
	PrintToOutputBinaryBuffer(g_pConsole, pAMONSendPacket, pAMONSendPacket->m_header.m_length, 10);
#endif

	CRM(PushAndTransmitAMONQueuePacket(link, (AMONPacket *)pAMONSendPacket),
			"SendMessagePayloadLink: Failed to push and transmit Send packet on link %d", link);

Error:
	return r;
}

RESULT SendMessagePayload(short destID, unsigned char type, unsigned char *payloadBuffer, int payloadBuffer_n) {
	RESULT r = R_OK;
	int i = 0;

	CBRM((payloadBuffer_n != 0), "SendMessagePayloadLink: Cannot send message of %d bytes", payloadBuffer_n);

	AMONSendPacket *pAMONSendPacket = CreateAMONSendPacket(destID, type, payloadBuffer, payloadBuffer_n);
	CNRM_NA(pAMONSendPacket, "SendMessagePayloadLink: Failed to allocate Send Packet");

#ifdef AMON_VERBOSE
	DEBUG_LINEOUT("SendMessagePayload: Sending AMONSendPacket length %d to destID %d of type 0x%x", pAMONSendPacket->m_header.m_length, destID, type);
	PrintToOutputBinaryBuffer(g_pConsole, pAMONSendPacket, pAMONSendPacket->m_header.m_length, 10);
#endif

	for(i = 0; i < NUM_LINKS; i++) {
		if(g_AMONLinkStates[i] == AMON_LINK_ESTABLISHED) {
			CRM(PushAndTransmitAMONQueuePacket(i, (AMONPacket *)pAMONSendPacket),
					"SendMessagePayload: Failed to push and transmit Send packet on link %d", i);
		}
	}

Error:
	return r;
}

RESULT SendRequestIDFromNetwork(AMON_LINK link) {
	RESULT r = R_OK;

	unsigned char linkID = (unsigned char)(g_amon.links[link].link_id);
	unsigned short deviceID = g_amon.links[link].id;

	/*
	unsigned char pBuffer[] = {
		AMON_VALUE,					// AMON Value
		0x07,						// length
		AMON_REQUEST_ID,			// Message Type
		(unsigned char)(linkID),	// link ID
		AMONShort(deviceID),		// Origin Device ID
		0x00						// Check sum (calculated later)
	};
	int pBuffer_n = sizeof(pBuffer) / sizeof(pBuffer[0]);

	CRM(SendAMONBuffer(link, pBuffer, pBuffer_n), "SendRequestIDFromNetwork: Failed to SendAMONBuffer %d bytes", pBuffer_n);*/

	AMONRequestIDPacket *pAMONRequestIDPacket = CreateAMONRequestIDPacket(linkID, deviceID);
	CNRM_NA(pAMONRequestIDPacket, "SendRequestIDFromNetwork: Failed to allocate Request ID Packet");

	CRM(PushAndTransmitAMONQueuePacket(link, (AMONPacket *)pAMONRequestIDPacket),
			"SendRequestIDFromNetwork: Failed to push and transmit request ID packet on link %d", link);

Error:
	return r;
}

RESULT SendAssignID(AMON_LINK link, short destID, AMON_LINK destLink, short newID) {
	RESULT r = R_OK;

	/*unsigned char pBuffer[] = {
		AMON_VALUE,					// AMON Value
		0x09,						// length
		AMON_ASSIGN_ID,				// Message Type
		(unsigned char)(destLink),	// link ID
		AMONShort(destID),			// Destination Device ID
		AMONShort(newID),			// New Device ID
		0x00						// Check sum (calculated later)
	};
	int pBuffer_n = sizeof(pBuffer) / sizeof(pBuffer[0]);

	CRM(SendAMONBuffer(link, pBuffer, pBuffer_n), "SendAssignID: Failed to SendAMONBuffer %d bytes", pBuffer_n);*/

	AMONAssignIDPacket *pAMONAssignIDPacket = CreateAMONAssignIDPacket(destLink, destID, newID);
	CNRM_NA(pAMONAssignIDPacket, "SendAssignID: Failed to allocate Assign ID Packet");

	CRM(PushAndTransmitAMONQueuePacket(link, (AMONPacket *)pAMONAssignIDPacket),
			"SendAssignID: Failed to push and transmit Assign ID packet on link %d", link);

Error:
	return r;
}

RESULT SendEstablishLink(AMON_LINK link) {
	RESULT r = R_OK;

	unsigned short deviceID = g_amon.links[link].id;

	/*unsigned char pBuffer[] = {
		AMON_VALUE,				// AMON Value
		0x07,					// length
		AMON_ESTABLISH_LINK,	// Message Type
		AMONShort(deviceID),	// Origin Device ID
		(unsigned char)(link),	// link ID
		0x00					// Check sum (calculated later)
	};
	int pBuffer_n = sizeof(pBuffer) / sizeof(pBuffer[0]);

	CRM(SendAMONBuffer(link, pBuffer, pBuffer_n), "SendEstablishLink: Failed to SendAMONBuffer %d bytes", pBuffer_n);*/

	AMONEstablishLinkPacket *pAMONEstablishLinkPacket = CreateAMONEstablishLinkPacket(link, (unsigned short)deviceID);
	CNRM_NA(pAMONEstablishLinkPacket, "SendEstablishLink: Failed to allocate Establish Link Packet");

	CRM(PushAndTransmitAMONQueuePacket(link, (AMONPacket *)pAMONEstablishLinkPacket),
			"SendEstablishLink: Failed to push and transmit Establish Link packet on link %d", link);

Error:
	return r;
}

RESULT SendEstablishLinkAck(AMON_LINK link) {
	RESULT r = R_OK;

	unsigned short deviceID = g_amon.id;

	/*unsigned char pBuffer[] = {
		AMON_VALUE,					// AMON Value
		0x07,						// length
		AMON_ESTABLISH_LINK_ACK,	// Message Type
		AMONShort(deviceID),		// Origin Device ID
		(unsigned char)(link),		// link ID
		0x00						// Check sum (calculated later)
	};
	int pBuffer_n = sizeof(pBuffer) / sizeof(pBuffer[0]);

	CRM(SendAMONBuffer(link, pBuffer, pBuffer_n), "SendDeviceID: Failed to SendAMONBuffer %d bytes", pBuffer_n);*/

	AMONEstablishLinkAckPacket *pAMONEstablishLinkAckPacket = CreateAMONEstablishLinkAckPacket(link, (unsigned short)deviceID);
	CNRM_NA(pAMONEstablishLinkAckPacket, "SendEstablishLinkAck: Failed to allocate Establish Link Ack Packet");

	CRM(PushAndTransmitAMONQueuePacket(link, (AMONPacket *)pAMONEstablishLinkAckPacket),
			"SendEstablishLinkAck: Failed to push and transmit Establish Link Ack packet on link %d", link);

Error:
	return r;
}

RESULT SendError(AMON_LINK link, AMON_MESSAGE_TYPE type) {
	RESULT r = R_OK;

	/*unsigned char pBuffer[] = {
		AMON_VALUE,				// AMON Value
		0x05,					// length
		AMON_ERROR,			// Message Type
		type,			// Message Type
		0x00					// Check sum (calculated later)
	};
	int pBuffer_n = sizeof(pBuffer) / sizeof(pBuffer[0]);

	CRM(SendAMONBuffer(link, pBuffer, pBuffer_n), "SendError: Failed to SendAMONBuffer %d bytes", pBuffer_n);*/

	AMONErrorPacket *pAMONErrorPacket = CreateAMONErrorPacket(type);
	CNRM_NA(pAMONErrorPacket, "SendError: Failed to allocate Error Packet");

	CRM(PushAndTransmitAMONQueuePacket(link, (AMONPacket *)pAMONErrorPacket),
		"SendError: Failed to push and transmit Error packet on link %d", link);

Error:
	return r;
}

RESULT SendDeviceID(AMON_LINK link) {
	RESULT r = R_OK;

	unsigned char deviceStatus = g_amon.status;
	unsigned short deviceID = g_amon.id;

	/*unsigned char pBuffer[] = {
		AMON_VALUE,				// AMON Value
		0x07,					// length
		AMON_SEND_ID,			// Message Type
		deviceStatus,			// Origin Device Status
		AMONShort(deviceID),	// Origin Device ID
		0x00					// Check sum (calculated later)
	};
	int pBuffer_n = sizeof(pBuffer) / sizeof(pBuffer[0]);

	CRM(SendAMONBuffer(link, pBuffer, pBuffer_n), "SendDeviceID: Failed to SendAMONBuffer %d bytes", pBuffer_n);*/

	AMONSendDeviceIDPacket *pAMONSendDeviceIDPacket = CreateAMONSendDeviceIDPacket(deviceStatus, deviceID);
	CNRM_NA(pAMONSendDeviceIDPacket, "SendDeviceID: Failed to allocate Send Device ID Packet");

	CRM(PushAndTransmitAMONQueuePacket(link, (AMONPacket *)pAMONSendDeviceIDPacket),
		"SendDeviceID: Failed to push and transmit Send Device ID packet on link %d", link);

Error:
	return r;
}

RESULT SendGetDeviceID(AMON_LINK link) {
	RESULT r = R_OK;

	unsigned char deviceStatus = g_amon.status;
	unsigned short deviceID = g_amon.id;

	/*unsigned char pBuffer[] = {
		AMON_VALUE,				// AMON Value
		0x07,					// length
		AMON_GET_ID,			// Message Type
		deviceStatus,			// Origin Device Status
		AMONShort(deviceID),	// Origin Device ID
		0x00					// Check sum (calculated later)
	};
	int pBuffer_n = sizeof(pBuffer) / sizeof(pBuffer[0]);

	CRM(SendAMONBuffer(link, pBuffer, pBuffer_n), "SendGetDeviceID: Failed to SendAMONBuffer %d bytes", pBuffer_n);*/

	AMONGetDeviceIDPacket *pAMONGetDeviceIDPacket = CreateAMONGetDeviceIDPacket(deviceStatus, deviceID);
	CNRM_NA(pAMONGetDeviceIDPacket, "SendGetDeviceID: Failed to allocate Get Device ID Packet");

	CRM(PushAndTransmitAMONQueuePacket(link, (AMONPacket *)pAMONGetDeviceIDPacket),
		"SendGetDeviceID: Failed to push and transmit Get Device ID packet on link %d", link);

Error:
	return r;
}

RESULT SendResetLink(AMON_LINK link) {
	RESULT r = R_OK;

	/*unsigned char pBuffer[] = {
		AMON_VALUE,				// AMON Value
		0x04,					// length
		AMON_RESET_LINK,		// Message Type
		0x00					// Check sum (calculated later)
	};
	int pBuffer_n = sizeof(pBuffer) / sizeof(pBuffer[0]);

	CRM(SendAMONBuffer(link, pBuffer, pBuffer_n), "SendResetLink: Failed to SendAMONBuffer %d bytes", pBuffer_n);*/

	AMONResetLinkPacket *pAMONResetLinkPacket = CreateAMONResetLinkPacket();
	CNRM_NA(pAMONResetLinkPacket, "SendResetLink: Failed to allocate Reset Link Packet");

	CRM(PushAndTransmitAMONQueuePacket(link, (AMONPacket *)pAMONResetLinkPacket),
		"SendResetLink: Failed to push and transmit Reset Link packet on link %d", link);

Error:
	return r;
}

RESULT SendResetLinkACK(AMON_LINK link) {
	RESULT r = R_OK;

	/*unsigned char pBuffer[] = {
		AMON_VALUE,				// AMON Value
		0x04,					// length
		AMON_RESET_LINK_ACK,	// Message Type
		0x00					// Check sum (calculated later)
	};
	int pBuffer_n = sizeof(pBuffer) / sizeof(pBuffer[0]);

	CRM(SendAMONBuffer(link, pBuffer, pBuffer_n), "SendResetLinkACK: Failed to SendAMONBuffer %d bytes", pBuffer_n);*/

	AMONResetLinkAckPacket *pAMONResetLinkAckPacket = CreateAMONResetLinkAckPacket();
	CNRM_NA(pAMONResetLinkAckPacket, "SendResetLinkACK: Failed to allocate Reset Link Ack Packet");

	CRM(PushAndTransmitAMONQueuePacket(link, (AMONPacket *)pAMONResetLinkAckPacket),
		"SendResetLinkACK: Failed to push and transmit Reset Link Ack packet on link %d", link);

Error:
	return r;
}

RESULT SendPingNetwork(AMON_LINK link, short destID) {
	RESULT r = R_OK;

	/*
	unsigned char pBuffer[] = {
		AMON_VALUE,									// AMON Value
		0x08,										// length
		AMON_PING,									// Message Type
		AMONShort((unsigned short)(g_amon.id)),		// Origin ACK device ID
		AMONShort(destID),							// destination device ACK ID
		0x00										// Check sum (calculated later)
	};
	int pBuffer_n = sizeof(pBuffer) / sizeof(pBuffer[0]);

	CRM(SendAMONBuffer(link, pBuffer, pBuffer_n), "SendGetDeviceID: Failed to SendPingNetwork %d bytes", pBuffer_n);
	*/

	AMONPingPacket *pAMONPingPacket = CreateAMONPingPacket(destID);
	CNRM_NA(pAMONPingPacket, "SendPingNetwork: Failed to allocate Ping Packet");

	CRM(PushAndTransmitAMONQueuePacket(link, (AMONPacket *)pAMONPingPacket),
			"SendPingNetwork: Failed to push and transmit ping packet on link %d", link);

Error:
	return r;
}

RESULT SendEchoNetwork(AMON_LINK link, short destID) {
	RESULT r = R_OK;

	/*
	unsigned char pBuffer[] = {
		AMON_VALUE,									// AMON Value
		0x08,										// length
		AMON_ECHO,									// Message Type
		AMONShort((unsigned short)(g_amon.id)),		// Origin ACK device ID
		AMONShort(destID),							// destination device ACK ID
		0x00										// Check sum (calculated later)
	};
	int pBuffer_n = sizeof(pBuffer) / sizeof(pBuffer[0]);

	CRM(SendAMONBuffer(link, pBuffer, pBuffer_n), "SendGetDeviceID: Failed to SendEchoNetwork %d bytes", pBuffer_n);
	*/

	AMONEchoPacket *pAMONEchoPacket = CreateAMONEchoPacket(destID);
	CNRM_NA(pAMONEchoPacket, "SendEchoNetwork: Failed to allocate Echo Packet");

	CRM(PushAndTransmitAMONQueuePacket(link, (AMONPacket *)pAMONEchoPacket),
			"SendEchoNetwork: Failed to push and transmit echo packet on link %d", link);

Error:
	return r;
}

RESULT SendACK(AMON_LINK link, short destID, AMON_ACK_TYPE type, unsigned char status) {
	RESULT r = R_OK;

	/*unsigned char pBuffer[] = {
		AMON_VALUE,									// AMON Value
		0x0A,										// length
		AMON_ACK,									// Message Type
		AMONShort((unsigned short)(g_amon.id)),		// Origin ACK device ID
		AMONShort(destID),							// destination device ACK ID
		(unsigned char)(type),						// ACK Type
		(unsigned char)(status),					// ACK Status
		0x00										// Check sum (calculated later)
	};
	int pBuffer_n = sizeof(pBuffer) / sizeof(pBuffer[0]);

	CRM(SendAMONBuffer(link, pBuffer, pBuffer_n), "SendGetDeviceID: Failed to SendAMONBuffer %d bytes", pBuffer_n);*/

	AMONAckPacket *pAMONAckPacket = CreateAMONAckPacket(destID, type, status);
	CNRM_NA(pAMONAckPacket, "SendACK: Failed to allocate Ack Packet");

	CRM(PushAndTransmitAMONQueuePacket(link, (AMONPacket *)pAMONAckPacket),
		"SendACK: Failed to push and transmit Ack packet on link %d", link);

Error:
	return r;
}

RESULT SendByteDestLink(AMON_LINK link, short destID, AMON_LINK destLink, unsigned char byte) {
	RESULT r = R_OK;

	/*unsigned char pBuffer[] = {
		AMON_VALUE,									// AMON Value
		0x0A,										// length
		AMON_SEND_BYTE_DEST_LINK,									// Message Type
		AMONShort((unsigned short)(g_amon.id)),		// Origin ACK device ID
		AMONShort(destID),							// destination device ACK ID
		(unsigned char)(destLink),					// destination Link
		(unsigned char)(byte),						// byte
		0x00										// Check sum (calculated later)
	};
	int pBuffer_n = sizeof(pBuffer) / sizeof(pBuffer[0]);

	CRM(SendAMONBuffer(link, pBuffer, pBuffer_n), "SendByteDestLink: Failed to SendAMONBuffer %d bytes", pBuffer_n);*/

	AMONSendByteDestLinkPacket *pAMONSendByteDestLinkPacket = CreateAMONSendByteDestLinkPacket(destID, destLink, byte);
	CNRM_NA(pAMONSendByteDestLinkPacket, "SendByteDestLink: Failed to allocate Send Byte Dest Link Packet");

	CRM(PushAndTransmitAMONQueuePacket(link, (AMONPacket *)pAMONSendByteDestLinkPacket),
		"SendByteDestLink: Failed to push and transmit Send Byte Dest Link packet on link %d", link);

Error:
	return r;
}

// Convenience function, specific to debug board
//RESULT SetLEDLinkClearTimeout(AMON_LINK link, int red, int green, int blue, int count) {
//	RESULT r = R_OK;
//
//	switch(link) {
//		case AMON_NORTH: {
//			SetLEDWithClearTimeout(2, red, green, blue, count);
//			SetLEDWithClearTimeout(4, red, green, blue, count);
//		} break;
//
//		case AMON_SOUTH: {
//			SetLEDWithClearTimeout(0, red, green, blue, count);
//			SetLEDWithClearTimeout(3, red, green, blue, count);
//		} break;
//
//		case AMON_EAST: {
//			SetLEDWithClearTimeout(3, red, green, blue, count);
//			SetLEDWithClearTimeout(2, red, green, blue, count);
//		} break;
//
//		case AMON_WEST: {
//			SetLEDWithClearTimeout(4, red, green, blue, count);
//			SetLEDWithClearTimeout(0, red, green, blue, count);
//		} break;
//
//		case AMON_ALL: {
//			SetLEDWithClearTimeout(0, red, green, blue, count);
//			SetLEDWithClearTimeout(1, red, green, blue, count);
//			SetLEDWithClearTimeout(2, red, green, blue, count);
//			SetLEDWithClearTimeout(3, red, green, blue, count);
//			SetLEDWithClearTimeout(4, red, green, blue, count);
//		} break;
//	}
//
//Error:
//	return r;
//}

AMON_LINK GetLinkFromString(char *pszLink) {
	int i = 0;

	for(i = 0; i < strlen(pszLink); i++)
		pszLink[i] = tolower(pszLink[i]);

	if(strcmp(pszLink, "north") == 0)
		return AMON_NORTH;
	else if(strcmp(pszLink, "south") == 0)
		return AMON_SOUTH;
	else if(strcmp(pszLink, "east") == 0)
		return AMON_EAST;
	else if(strcmp(pszLink, "west") == 0)
		return AMON_WEST;
	else if(strcmp(pszLink, "all") == 0)
		return AMON_ALL;

	return AMON_INVALID;
}

AMON_BYTE_MODE_MESSAGE GetByteModeMessageFromString(char *pszCmd) {
	int i = 0;

	for(i = 0; i < strlen(pszCmd); i++)
		pszCmd[i] = tolower(pszCmd[i]);

	if(strcmp(pszCmd, "linkrequest") == 0)
		return AMON_BYTE_LINK_REQUEST;
	else if(strcmp(pszCmd, "linkavailable") == 0)
		return AMON_BYTE_LINK_AVAILABLE;
	else if(strcmp(pszCmd, "linkaccepted") == 0)
		return AMON_BYTE_LINK_ACCEPTED;
	else if(strcmp(pszCmd, "linkestablished") == 0)
		return AMON_BYTE_LINK_ESTABLISHED;
	else if(strcmp(pszCmd, "ping") == 0)
		return AMON_BYTE_PING;
	else if(strcmp(pszCmd, "echo") == 0)
		return AMON_BYTE_ECHO;
	else if(strcmp(pszCmd, "reset") == 0)
		return AMON_BYTE_LINK_RESET;
	else if(strcmp(pszCmd, "request_one") == 0)
		return (AMON_REQUEST_TRANSMIT + 0x01);
	else if(strcmp(pszCmd, "accept") == 0)
		return AMON_ACCEPT_TRANSMIT;
	else if(strcmp(pszCmd, "complete") == 0)
		return AMON_TRANSMIT_COMPLETE;
	else if(strcmp(pszCmd, "complete_ack") == 0)
			return AMON_TRANSMIT_COMPLETE_ACK;

	return AMON_BYTE_INVALID;
}

AMON_MESSAGE_TYPE GetMessageTypeFromString(char *pszCmd) {
	int i = 0;

	for(i = 0; i < strlen(pszCmd); i++)
		pszCmd[i] = tolower(pszCmd[i]);

	if(strcmp(pszCmd, "ping") == 0)
		return AMON_PING;
	else if(strcmp(pszCmd, "echo") == 0)
		return AMON_ECHO;
	else if(strcmp(pszCmd, "reset") == 0)
		return AMON_RESET_LINK;

	return AMON_INVALID;
}

// AMON Level Commands
RESULT SendAMONMessage(Console *pc, char *pszCmd, char *pszDestID) {
	RESULT r = R_OK;

	AMON_MESSAGE_TYPE message = GetMessageTypeFromString(pszCmd);
	CBRM_NA((message != AMON_INVALID), "SendAMONMessage: Failed to get message");

	unsigned short destID = (unsigned short)atoi(pszDestID);

	// Make sure we can find the ID in our map
	CBRM((FindAMONNode(g_AMONmap, destID) != NULL), "SendAMONMessage: Failed to find device id %d in map", destID);

	// TODO: Params etc
	CRM(SendMessageType(message, destID), "SendAMONMessage: Failed to send message %d to device %d", message, destID);

Error:
	return r;
}

// This will queue and send a ping
RESULT SendAMONNULLPing(Console *pc, char *pszLink) {
	RESULT r = R_OK;

	AMON_LINK link = GetLinkFromString(pszLink);
	CBRM_NA((link != AMON_LINK_INVALID), "SendAMONNULLPing: Failed to get link");

	//CRM(SendPingNetwork(link, 0), "SendAMONNULLPing: Failed to null ping link %d", link);

	AMONPingPacket *pAMONPingPacket = CreateAMONPingPacket(0);
	CNRM_NA(pAMONPingPacket, "SendAMONNULLPing: Failed to allocate Ping Packet");

	CRM(PushAndTransmitAMONQueuePacket(link, (AMONPacket *)pAMONPingPacket),
			"SendAMONNULLPing: Failed to push and transmit ping packet on link %d", link);

Error:
	return r;
}

RESULT ResetAMONLink(Console *pc, char *pszLink) {
	RESULT r = R_OK;

	AMON_LINK link = GetLinkFromString(pszLink);
	CBRM_NA((link != AMON_LINK_INVALID), "ResetAMONLink: Failed to get link");

	// Make sure link is established
	CBRM((g_AMONLinkPhys[link] == AMON_PHY_READY), "ResetAMONLink: Failed to reset link %d", link);

	CRM(SendResetLink(link), "ResetAMONLink: Failed to reset link %d", link);

Error:
	return r;
}

RESULT ConsoleCheckLinkStatus(Console *pc, char *pszLink) {
	RESULT r = R_OK;

	AMON_LINK link = GetLinkFromString(pszLink);
	CBRM_NA((link != AMON_LINK_INVALID), "ConsoleCheckLinkStatus: Failed to get link");

	CRM(CheckLinkStatus(link), "ConsoleCheckLinkStatus: Failed to check link %d status", link);

Error:
	return r;
}

// UART Manipulation Funcitons
RESULT SendByteModeCommand(Console *pc, char *pszCmd, char *pszLink) {
	RESULT r = R_OK;
	int i = 0;

	AMON_LINK link = GetLinkFromString(pszLink);
	CBRM_NA((link != AMON_LINK_INVALID), "SendByteModeCommand: Failed to get link");

	AMON_BYTE_MODE_MESSAGE byte = GetByteModeMessageFromString(pszCmd);
	CBRM_NA((byte != AMON_BYTE_INVALID), "SendByteModeCommand: Failed to get cmd byte");

	if(link != AMON_ALL) {
//		SetLEDLinkClearTimeout(link, 0, 0, 50, 100);
		CRM_NA(SendByte(link, byte), "SendByteModeCommand: Failed to send byte");
	}
	else {
//		SetLEDLinkClearTimeout(AMON_ALL, 0, 0, 50, 100);
		CRM_NA(BroadcastByte(byte), "SendByteModeCommand: Failed to broadcast byte");
	}

Error:
	return r;
}

RESULT SendByteModeCommandDestLink(Console *pc, char *pszCmd, char *pszDestID, char *pszLink) {
	RESULT r = R_OK;
	int i = 0;

	// Need to be MSTR for this
	CBRM_NA((g_amon.MasterState != AMON_MASTER_FALSE), "SendByteModeCommandDestLink: Cannot send Byte mode message when not master");

	unsigned short destID = (unsigned short)atoi(pszDestID);

	// Make sure we can find the ID in our map
	CBRM((FindAMONNode(g_AMONmap, destID) != NULL), "SendByteModeCommandDestLink: Failed to find device id %d in map", destID);

	AMON_LINK link = GetLinkFromString(pszLink);
	CBRM_NA((link != AMON_INVALID), "SendByteModeCommandDestLink: Failed to get link");

	AMON_BYTE_MODE_MESSAGE byte = GetByteModeMessageFromString(pszCmd);
	CBRM_NA((byte != AMON_BYTE_INVALID), "SendByteModeCommandDestLink: Failed to get cmd byte");

	for(i = 0; i < NUM_LINKS; i++)
		if(g_AMONLinkStates[i] == AMON_LINK_ESTABLISHED)
			CRM(SendByteDestLink(i, destID, link, (unsigned char)byte), "SendByteModeCommandDestLink: Failed to send 0x%x on link %d to dest %d", byte, link, destID);

Error:
	return r;
}

RESULT SetAMONMasterConsole(Console *pc, unsigned char *pszfMaster) {
	RESULT r = R_OK;
	int i = 0;

	for(i = 0; i < strlen(pszfMaster); i++)
		pszfMaster[i] = tolower(pszfMaster[i]);

	if(strcmp(pszfMaster, "true") == 0) {
		//return SetAMONMaster();
		if(g_GetAMONDeviceCallback != NULL)
			SetAMONMaster(g_GetAMONDeviceCallback());
		else
			SetAMONMaster(NULL);
	}
	else {
		return UnsetAMONMaster();
	}

Error:
	return r;
}

RESULT PrintAMONMasterMap(Console *pc) {
	RESULT r = R_OK;

	CBRM_NA((g_amon.MasterState != AMON_MASTER_FALSE), "PrintAMONMasterMap: Failed, this is not a master node");
	CRM_NA(PrintAMONMap(pc, g_AMONmap), "PrintAMONMasterMap: Failed to print map");

Error:
	return r;
}

RESULT TestAMONNumLinks(Console *pc, char *pszID) {
	RESULT r = R_OK;

	int id = atoi(pszID);
	int links = GetNumberOfEastWestLinks(id);

	if(links != 0) {
		DEBUG_LINEOUT("TestNumLinks: Node %d found at depth %d", id, links);
	}
	else {
		DEBUG_LINEOUT("TestNumLinks: Node %d not found", id);
	}

Error:
	return r;
}

RESULT PrintAMONInfo(Console *pc) {
	RESULT r = R_OK;
	int i = 0;

	DEBUG_LINEOUT_NA("----------------------------");
	DEBUG_LINEOUT("AMON ID: %d", g_amon.id);
	for(i = 0; i < NUM_LINKS; i++)
		CRM(PrintAMONLinkInfo(i), "PrintAMONInfo: Failed to print AMON link %d info", i);
	DEBUG_LINEOUT_NA("----------------------------");

Error:
	return r;
}



