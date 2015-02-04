#include "AMONNet.h"

AMON_RX_STATE g_LinkRxState[NUM_LINKS];

// The message on a given link
AMON_MESSAGE_TYPE g_linkMessageType[NUM_LINKS];

int g_MasterCount = 0;

AMONMap *g_AMONmap = NULL;

int GetNumberOfEastWestLinks(int id) {
	return GetNumberOfEastWestMapLinks(g_AMONmap, id, AMON_WEST, AMON_EAST);
}

RESULT InitAmon(int ticksPerSecond) {
	RESULT r = R_OK;
	int i = 0;

	g_amon.fStart = 0;
	CRM_NA(InitializeAMON(), "InitAmon: Failed to Initialize AMON");
	//StartAMON();

	//ResetMasterCount();

	g_SysTicksPerSecond = ticksPerSecond;

	// Initialize the links
	for(i = 0; i < NUM_LINKS; i++)
		CRM(InitializeLink(i), "InitAmon: Failed to initialize link %d", i);

//	// Console Functions
//	AddConsoleFunctionByArgs(g_pConsole, SendByteModeCommand, "AMONSendByte", 3, 0);
//	AddConsoleFunctionByArgs(g_pConsole, SetAMONMasterConsole, "SetAMONMaster", 2, 0);
//	AddConsoleFunctionByArgs(g_pConsole, PrintAMONMasterMap, "PrintAMONMasterMap", 1, 0);
//	AddConsoleFunctionByArgs(g_pConsole, SendByteModeCommandDestLink, "AMONSendByteDestLink", 4, 0);
//
//	// TODO: Add variable params / min params
//	AddConsoleFunctionByArgs(g_pConsole, SendAMONMessage, "AMONMessage", 3, 0);
//	AddConsoleFunctionByArgs(g_pConsole, StartAMON, "StartAMON", 1, 0);
//	AddConsoleFunctionByArgs(g_pConsole, PrintAMONInfo, "PrintAMONInfo", 1, 0);
//
//	AddConsoleFunctionByArgs(g_pConsole, ConsoleCheckLinkStatus, "AMONCheckLinkStatus", 2, 0);
//	AddConsoleFunctionByArgs(g_pConsole, ConsoleSetAMONInterval, "AMONSetInterval", 2, 0);
//
//	AddConsoleFunctionByArgs(g_pConsole, ResetAMONLink, "AMONResetLink", 2, 0);

	AddConsoleFunctionByArgs(g_pConsole, TestAMONMap, "TestAMONMap", 1, 0);

	AddConsoleFunctionByArgs(g_pConsole, TestAMONNumLinks, "TestAMONNumLinks", 2, 0);

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

// AMON OnInterval - called every interval
RESULT OnAMONInterval() {
	RESULT r = R_OK;
	int i = 0;

	if(g_amon.fStart == 0)
		return R_OFF;

	//SetLEDWithClearTimeout(1, 20, 20, 20, 50);
	for(i = 0; i < NUM_LINKS; i++) {
		if(g_AMONLinkStates[i] != AMON_LINK_ESTABLISHED) {
			unsigned char r = (i == 0 || i == 3) ? 50 : 0;
			unsigned char g = (i == 1 || i == 3) ? 50 : 0;
			unsigned char b = (i == 2 || i == 3) ? 50 : 0;

			//SetLEDLinkClearTimeout(i, r, g, b, 50);

			CRM(SendByte(i, AMON_BYTE_PING), "OnAMONInterval: Failed to send ping on link %d", i);
		}
		else {
			// If link established we want to check the status
			CRM(CheckLinkStatus(i), "OnAMONInterval: Failed to check link %d status", i);
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
		g_AMONmap = CreateAMONMap(NUM_LINKS, AMON_MASTER_ID);
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
RESULT RegisterNewDevice(int destID, int linkID, int *newID) {
	RESULT r = R_OK;

	*newID = g_AMONmap->m_mapID;
	CRM(AddAMONNode(g_AMONmap, destID, linkID, g_AMONmap->m_mapID), "RegisterNewDevice: Failed to add node to node %d at link %d", destID, linkID);

Error:
	return r;
}

RESULT UnsetAMONMaster() {
	g_amon.MasterState = AMON_MASTER_FALSE;
	return R_OK;
}


RESULT HandleAMONByte(AMON_LINK link, unsigned char byte) {
	RESULT r = R_OK;

	CBRM_NA((link_input_c[link] < MAX_MSG_LENGTH), "AMONRx: Buffer full!");

	// Add to the buffer
	link_input[link][link_input_c[link]] = byte;
	link_input_c[link]++;

	switch(g_LinkRxState[link]) {
		case AMON_RX_READY: {
			// Something went wrong
			if(byte != AMON_VALUE)
				CRM_NA(SendErrorResetLink(link, g_linkMessageType[link]), "AMONRx: Failed to send error and reset link");

			g_LinkRxState[link] = AMON_RX_AMON_RECEIVED;
		} break;

		case AMON_RX_AMON_RECEIVED: {
			// Cannot detect an error here / we are guaranteed to have at least 4 bytes so this is safe
			g_linkMessageLength[link] = (unsigned int)byte;
			g_LinkRxState[link] = AMON_RX_LENGTH_RECEIVED;
		} break;

		case AMON_RX_LENGTH_RECEIVED: {
			g_linkMessageType[link] = (AMON_MESSAGE_TYPE)byte;
			if(g_linkMessageType[link] >= AMON_INVALID || g_linkMessageType[link] == AMON_NULL)
				CRM_NA(SendErrorResetLink(link, g_linkMessageType[link]), "AMONRx: Failed to send error and reset link");

			g_LinkRxState[link] = AMON_RX_TYPE_RECEIVED;
		} break;

		case AMON_RX_DATA:
		case AMON_RX_TYPE_RECEIVED: {
			if(link_input_c[link] >= g_linkMessageLength[link]) {
				// Message has been completely received!
				CRM(HandleAMONPacket(link), "AMONRx: Failed to handle packet on link %d", link);
				g_LinkRxState[link] = AMON_RX_READY;	// Reset the link protocol state
			}
			else {
				g_LinkRxState[link] = AMON_RX_DATA;
			}
		} break;

		case AMON_RX_INVALID: {
			DEBUG_LINEOUT("Error: AMON link %d in invalid state, resetting", link);
			CRM_NA(SendErrorResetLink(link, g_linkMessageType[link]), "AMONRx: Failed to send error and reset link");
		} break;
			}

Error:
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

RESULT HandleAMONPacket(AMON_LINK link) {
	RESULT r = R_OK;
	unsigned char checksum = CalculateChecksum(link_input[link], link_input_c[link]);
	unsigned char *pBuffer = link_input[link];
	int pBuffer_n = link_input_c[link];
	int i = 0;

#ifdef AMON_VERBOSE
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

			#ifdef AMON_VERBOSE
				DEBUG_LINEOUT("Received PING on link %d from device %d to device %d", link, originDeviceID, addressDeviceID);
			#endif

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

			#ifdef AMON_VERBOSE
				DEBUG_LINEOUT("Received ECHO on link %d from device %d to device %d", link, originDeviceID, addressDeviceID);
			#endif

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
			int addressDeviceID = AMONToShort(pBuffer[4], pBuffer[5]);

			DEBUG_LINEOUT("Received ID request on link %d from device connected to %d on link %d", link, addressDeviceID, linkID);

			if(g_amon.id == AMON_MASTER_ID && g_amon.status == AMON_DEVICE_OK && g_amon.MasterState != AMON_MASTER_FALSE) {
				// We are the master, so dispatch a new ID
				int newID;
				CRM(RegisterNewDevice(addressDeviceID, linkID, &newID), "HandleAMONPacket: Failed to register new device on %d link %d", addressDeviceID, linkID);

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
				CRM(g_HandleAMONPayloadCallback(link, originDeviceID, type, pPayloadBuffer, payload_n),
						"HandleAMONPacket: Failed to receive amon msg from device %d on link %d", originDeviceID, link);
			}
			else {
				CRM(PassThruAMONBuffer(link, pBuffer, pBuffer_n), "HandleAMONPacket: Failed to pass through message from link %d", link);
			}
		} break;

		case AMON_GET_ID: {
			// Other device is requesting our ID
			g_amon.links[link].Status = pBuffer[3];		// get the device status
			if(g_amon.links[link].Status == AMON_DEVICE_OK) {
				g_amon.links[link].id = AMONToShort(pBuffer[4], pBuffer[5]);
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
			if(g_AMONLinkStates[i] == AMON_LINK_ESTABLISHED) {
				int rxID = AMONToShort(pBuffer[4], pBuffer[5]);
				DEBUG_LINEOUT("Rx: Neighbor ID %d received on link %d", rxID, link);
				g_amon.links[link].id = rxID;
			}
			else {
				// ID has been received from other device during link establish
				g_amon.links[link].Status = pBuffer[3];		// get the device status
				if(g_amon.links[link].Status == AMON_DEVICE_OK) {
					g_amon.links[link].id = AMONToShort(pBuffer[4], pBuffer[5]);
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
			DEBUG_LINEOUT("HandleAMONPacket: Error: Received error on link %d for message type 0x%x", link, errorType);
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
	DEBUG_LINEOUT("Error: 0x%x, resetting link %d", type, link);

	//SetLEDLinkClearTimeout(AMON_ALL, 50, 0, 0, 600);
	ResetLink(link);

	return SendError(link, type);
}

// This will pass a buffer through to all established links other than the incoming link
RESULT PassThruAMONBuffer(AMON_LINK link, unsigned char *pBuffer, int pBuffer_n) {
	RESULT r = R_OK;
	int i = 0;

	// We're not the master, so send this back out on all links not including this link
	for(i = 0; i < NUM_LINKS; i++) {
		if(i != link && g_AMONLinkStates[i] == AMON_LINK_ESTABLISHED) {
			CRM(SendAMONBuffer(i, pBuffer, pBuffer_n), "HandleAMONPacket: Failed to pass message on to link %d", i);
		}
	}

Error:
	return r;
}

RESULT SendAMONBuffer(AMON_LINK link, unsigned char *pBuffer, int pBuffer_n) {
	RESULT r = R_OK;
	int i = 0;
	unsigned char checksum = CalculateChecksum(pBuffer, pBuffer_n);

#ifdef AMON_VERBOSE
	DEBUG_LINEOUT("Tx: %d bytes", pBuffer_n);
	PrintToOutputBinaryBuffer(g_pConsole, pBuffer, pBuffer_n, 10);
#endif

	for(i = 0; i < pBuffer_n; i++)
		CRM(SendByte(link, pBuffer[i]), "SendAMONBuffer: Failed SendByte on %d link", link);

Error:
	return r;
}

RESULT SendMessageType(AMON_MESSAGE_TYPE type, short destID, ...) {
	RESULT r = R_OK;
	int i = 0;

	switch(type) {

		case AMON_PING: {
			for(i = 0; i < NUM_LINKS; i++)
				if(g_AMONLinkStates[i] == AMON_LINK_ESTABLISHED)
					CRM(SendPingNetwork(i, destID), "SendMessage: Failed to ping device %d on link %d", destID, i);
		} break;

		default: {

		} break;
	}

Error:
	return r;
}

RESULT SendMessagePayload(AMON_LINK link, short destID, unsigned char type, unsigned char *payloadBuffer, int payloadBuffer_n) {
	RESULT r = R_OK;
	unsigned char *pBuffer = NULL;

	CBRM((payloadBuffer_n != 0), "SendMessagePayload: Cannot send message of %d bytes", payloadBuffer_n);

	unsigned char linkID = (unsigned char)(g_amon.links[link].link_id);
	unsigned short originID = g_amon.id;

	int pBuffer_n = 10 + payloadBuffer_n;
	pBuffer = (unsigned char *)calloc(sizeof(unsigned char), pBuffer_n);
	CNRM_NA(pBuffer, "SendMessagePayload: Failed to initialize buffer to send");

	// TODO: uhhh - fill out the buffer doofus
	pBuffer[0] = AMON_VALUE;							// AMON Value
	pBuffer[1] = pBuffer_n;								// length
	pBuffer[2] = AMON_SEND;								// Message Type
	pBuffer[3] = (unsigned char)(originID & 0xff);		// origin ID - lower byte
	pBuffer[4] = (unsigned char)(originID >> 8);		// origin ID - upper byte
	pBuffer[5] = (unsigned char)(destID & 0xff);		// dest ID - lower byte
	pBuffer[6] = (unsigned char)(destID >> 8);			// dest ID - upper byte
	pBuffer[7] = (unsigned char)(type);					// Send message type
	pBuffer[8] = (unsigned char)(payloadBuffer_n);		// payload buffer length

	// Copy over the payload
	memcpy((void*)(pBuffer + 9), (void*)(payloadBuffer), sizeof(unsigned char) * payloadBuffer_n);

	pBuffer[9 + payloadBuffer_n] = 0x00;				// Check sum (calculated later)

#ifdef AMON_VERBOSE
	DEBUG_LINEOUT("SendMessagePayload: Sending payload buffer length %d to destID %d of type 0x%x", pBuffer_n, destID, type);
	PrintToOutputBinaryBuffer(g_pConsole, pBuffer, pBuffer_n, 10);
#endif

	CRM(SendAMONBuffer(link, pBuffer, pBuffer_n), "SendMessagePayload: Failed to SendAMONBuffer %d bytes on link %d", pBuffer_n, link);

Error:
	if(pBuffer != NULL) {
		free(pBuffer);
		pBuffer = NULL;
	}

	return r;
}

RESULT SendRequestIDFromNetwork(AMON_LINK link) {
	RESULT r = R_OK;

	unsigned char linkID = (unsigned char)(g_amon.links[link].link_id);
	unsigned short deviceID = g_amon.links[link].id;

	unsigned char pBuffer[] = {
		AMON_VALUE,					// AMON Value
		0x07,						// length
		AMON_REQUEST_ID,			// Message Type
		(unsigned char)(linkID),	// link ID
		AMONShort(deviceID),		// Origin Device ID
		0x00						// Check sum (calculated later)
	};
	int pBuffer_n = sizeof(pBuffer) / sizeof(pBuffer[0]);

	CRM(SendAMONBuffer(link, pBuffer, pBuffer_n), "SendRequestIDFromNetwork: Failed to SendAMONBuffer %d bytes", pBuffer_n);

Error:
	return r;
}

RESULT SendAssignID(AMON_LINK link, short destID, AMON_LINK destLink, short newID) {
	RESULT r = R_OK;

	unsigned char pBuffer[] = {
		AMON_VALUE,					// AMON Value
		0x09,						// length
		AMON_ASSIGN_ID,				// Message Type
		(unsigned char)(destLink),	// link ID
		AMONShort(destID),			// Destination Device ID
		AMONShort(newID),			// New Device ID
		0x00						// Check sum (calculated later)
	};
	int pBuffer_n = sizeof(pBuffer) / sizeof(pBuffer[0]);

	CRM(SendAMONBuffer(link, pBuffer, pBuffer_n), "SendAssignID: Failed to SendAMONBuffer %d bytes", pBuffer_n);

Error:
	return r;
}

RESULT SendEstablishLink(AMON_LINK link) {
	RESULT r = R_OK;

	unsigned short deviceID = g_amon.links[link].id;

	unsigned char pBuffer[] = {
		AMON_VALUE,				// AMON Value
		0x07,					// length
		AMON_ESTABLISH_LINK,	// Message Type
		AMONShort(deviceID),	// Origin Device ID
		(unsigned char)(link),	// link ID
		0x00					// Check sum (calculated later)
	};
	int pBuffer_n = sizeof(pBuffer) / sizeof(pBuffer[0]);

	CRM(SendAMONBuffer(link, pBuffer, pBuffer_n), "SendEstablishLink: Failed to SendAMONBuffer %d bytes", pBuffer_n);

Error:
	return r;
}

RESULT SendEstablishLinkAck(AMON_LINK link) {
	RESULT r = R_OK;

	unsigned short deviceID = g_amon.id;

	unsigned char pBuffer[] = {
		AMON_VALUE,					// AMON Value
		0x07,						// length
		AMON_ESTABLISH_LINK_ACK,	// Message Type
		AMONShort(deviceID),		// Origin Device ID
		(unsigned char)(link),		// link ID
		0x00						// Check sum (calculated later)
	};
	int pBuffer_n = sizeof(pBuffer) / sizeof(pBuffer[0]);

	CRM(SendAMONBuffer(link, pBuffer, pBuffer_n), "SendDeviceID: Failed to SendAMONBuffer %d bytes", pBuffer_n);

Error:
	return r;
}

RESULT SendError(AMON_LINK link, AMON_MESSAGE_TYPE type) {
	RESULT r = R_OK;

	unsigned char pBuffer[] = {
		AMON_VALUE,				// AMON Value
		0x05,					// length
		AMON_ERROR,			// Message Type
		type,			// Message Type
		0x00					// Check sum (calculated later)
	};
	int pBuffer_n = sizeof(pBuffer) / sizeof(pBuffer[0]);

	CRM(SendAMONBuffer(link, pBuffer, pBuffer_n), "SendError: Failed to SendAMONBuffer %d bytes", pBuffer_n);

Error:
	return r;
}

RESULT SendDeviceID(AMON_LINK link) {
	RESULT r = R_OK;

	unsigned char deviceStatus = g_amon.status;
	unsigned short deviceID = g_amon.id;

	unsigned char pBuffer[] = {
		AMON_VALUE,				// AMON Value
		0x07,					// length
		AMON_SEND_ID,			// Message Type
		deviceStatus,			// Origin Device Status
		AMONShort(deviceID),	// Origin Device ID
		0x00					// Check sum (calculated later)
	};
	int pBuffer_n = sizeof(pBuffer) / sizeof(pBuffer[0]);

	CRM(SendAMONBuffer(link, pBuffer, pBuffer_n), "SendDeviceID: Failed to SendAMONBuffer %d bytes", pBuffer_n);

Error:
	return r;
}

RESULT SendGetDeviceID(AMON_LINK link) {
	RESULT r = R_OK;

	unsigned char deviceStatus = g_amon.status;
	unsigned short deviceID = g_amon.id;

	unsigned char pBuffer[] = {
		AMON_VALUE,				// AMON Value
		0x07,					// length
		AMON_GET_ID,			// Message Type
		deviceStatus,			// Origin Device Status
		AMONShort(deviceID),	// Origin Device ID
		0x00					// Check sum (calculated later)
	};
	int pBuffer_n = sizeof(pBuffer) / sizeof(pBuffer[0]);

	CRM(SendAMONBuffer(link, pBuffer, pBuffer_n), "SendGetDeviceID: Failed to SendAMONBuffer %d bytes", pBuffer_n);

Error:
	return r;
}

RESULT SendResetLink(AMON_LINK link) {
	RESULT r = R_OK;

	unsigned char pBuffer[] = {
		AMON_VALUE,				// AMON Value
		0x04,					// length
		AMON_RESET_LINK,		// Message Type
		0x00					// Check sum (calculated later)
	};
	int pBuffer_n = sizeof(pBuffer) / sizeof(pBuffer[0]);

	CRM(SendAMONBuffer(link, pBuffer, pBuffer_n), "SendResetLink: Failed to SendAMONBuffer %d bytes", pBuffer_n);

Error:
	return r;
}

RESULT SendResetLinkACK(AMON_LINK link) {
	RESULT r = R_OK;

	unsigned char pBuffer[] = {
		AMON_VALUE,				// AMON Value
		0x04,					// length
		AMON_RESET_LINK_ACK,	// Message Type
		0x00					// Check sum (calculated later)
	};
	int pBuffer_n = sizeof(pBuffer) / sizeof(pBuffer[0]);

	CRM(SendAMONBuffer(link, pBuffer, pBuffer_n), "SendResetLinkACK: Failed to SendAMONBuffer %d bytes", pBuffer_n);

Error:
	return r;
}

RESULT SendPingNetwork(AMON_LINK link, short destID) {
	RESULT r = R_OK;

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

Error:
	return r;
}

RESULT SendEchoNetwork(AMON_LINK link, short destID) {
	RESULT r = R_OK;

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

Error:
	return r;
}

RESULT SendACK(AMON_LINK link, short destID, AMON_ACK_TYPE type, unsigned char status) {
	RESULT r = R_OK;

	unsigned char pBuffer[] = {
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

	CRM(SendAMONBuffer(link, pBuffer, pBuffer_n), "SendGetDeviceID: Failed to SendAMONBuffer %d bytes", pBuffer_n);

Error:
	return r;
}

RESULT SendByteDestLink(AMON_LINK link, short destID, AMON_LINK destLink, unsigned char byte) {
	RESULT r = R_OK;

	unsigned char pBuffer[] = {
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

	CRM(SendAMONBuffer(link, pBuffer, pBuffer_n), "SendByteDestLink: Failed to SendAMONBuffer %d bytes", pBuffer_n);

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

RESULT ResetAMONLink(Console *pc, char *pszLink) {
	RESULT r = R_OK;

	AMON_LINK link = GetLinkFromString(pszLink);
	CBRM_NA((link != AMON_LINK_INVALID), "ResetAMONLink: Failed to get link");

	// Make sure link is established
	CBRM((g_AMONLinkPhys[link] == AMON_PHY_ESTABLISHED), "ResetAMONLink: Failed to reset link %d", link);

	CRM(SendResetLink(link);, "ResetAMONLink: Failed to reset link %d", link);

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

	if(strcmp(pszfMaster, "true") == 0)
		return SetAMONMaster();
	else
		return UnsetAMONMaster();

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



