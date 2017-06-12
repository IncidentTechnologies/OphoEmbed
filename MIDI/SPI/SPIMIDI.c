#include "SPIMIDI.h"

//#include "usbmidi.h"
#include "../../Drivers/TM4C/SPIController.h"

#define MAX_RX 16
int m_SPIMidiConfig = 0;
bool m_fSPIMIDIInitialized = false;

// Pending Incoming SPI MIDI events
MIDI_MSG m_gTarPendingIncomingBLEMsgs[MAX_INCOMING_BLE_PENDING_MSGS];

int32_t  m_gTarPendingIncomingBLEMsgs_n = 0;	// number of events queued
int32_t  m_gTarPendingIncomingBLEMsgs_c = 0;	// event cursor (to add events)
int32_t  m_gTarPendingIncomingBLEMsgs_e = 0;	// event execute

RESULT InitializeIncomingBLEQueue() {
	RESULT r = R_OK;

	// Pending events
	memset(m_gTarPendingIncomingBLEMsgs, 0, sizeof(m_gTarPendingIncomingBLEMsgs));

	m_gTarPendingIncomingBLEMsgs_n = 0;	// number of events queued
	m_gTarPendingIncomingBLEMsgs_c = 0;	// event cursor (to add events)
	m_gTarPendingIncomingBLEMsgs_e = 0;	// event execute

	DEBUG_LINEOUT("Incoming BLE Msgs Queue initialized!");

Error:
	return r;
}

// Critical path - conventions need not apply
inline RESULT QueueNewIncomingBLEMsg(MIDI_MSG midiMsg) {
	//RESULT r = R_OK;

	if(m_gTarPendingIncomingBLEMsgs_n >= MAX_INCOMING_BLE_PENDING_MSGS){
		DEBUG_LINEOUT("QueueNewIncomingBLEMsg: Cannot queue another message as queue is full!");
		return R_FAIL;
	}

	 // copy over the event to avoid dynamic allocation
	m_gTarPendingIncomingBLEMsgs[m_gTarPendingIncomingBLEMsgs_c].type = midiMsg.type;
	m_gTarPendingIncomingBLEMsgs[m_gTarPendingIncomingBLEMsgs_c].data1 = midiMsg.data1;
	m_gTarPendingIncomingBLEMsgs[m_gTarPendingIncomingBLEMsgs_c].data2 = midiMsg.data2;

	m_gTarPendingIncomingBLEMsgs_n++;

	// circular buffer style
	m_gTarPendingIncomingBLEMsgs_c++;
	if(m_gTarPendingIncomingBLEMsgs_c == MAX_INCOMING_BLE_PENDING_MSGS)
		m_gTarPendingIncomingBLEMsgs_c = 0;

	/*
	DEBUG_LINEOUT("Queued up new ble msg type 0x%x c:%d n:%d e:%d",
			midiMsg.type, m_gTarPendingIncomingBLEMsgs_c, m_gTarPendingIncomingBLEMsgs_n, m_gTarPendingIncomingBLEMsgs_e);
	//*/

	return R_OK;
}

uint8_t IsIncomingBLEMsgPending() {
	return m_gTarPendingIncomingBLEMsgs_n;
}

RESULT HandleQueuedIncomingBLEMsg() {
	RESULT r = R_OK;

	CBRM((m_gTarPendingIncomingBLEMsgs_n > 0), "HandleQueuedIncomingBLEMsg: Cannot handle message as queue is empty");

	//DEBUG_LINEOUT("Handling incoming BLE message 0x%x", m_gTarPendingIncomingBLEMsgs[m_gTarPendingIncomingBLEMsgs_e].type);

	CRM(HandleMIDIPacket(m_gTarPendingIncomingBLEMsgs[m_gTarPendingIncomingBLEMsgs_e]), "HandleQueuedIncomingBLEMsg: Failed to handle MIDI Message");

	/*
	DEBUG_LINEOUT("Message handled type 0x%x c:%d n:%d e:%d",
			m_gTarPendingIncomingBLEMsgs[m_gTarPendingIncomingBLEMsgs_e].type, m_gTarPendingIncomingBLEMsgs_c, m_gTarPendingIncomingBLEMsgs_n, m_gTarPendingIncomingBLEMsgs_e);
	//*/

Error:
	m_gTarPendingIncomingBLEMsgs_n--;

	m_gTarPendingIncomingBLEMsgs_e++;
		if(m_gTarPendingIncomingBLEMsgs_e == MAX_INCOMING_BLE_PENDING_MSGS)
			m_gTarPendingIncomingBLEMsgs_e = 0;

	return r;
}

bool g_fSPIMIDIInterrupt = false;


// Critical path (interrupt) - conventions need not apply
RESULT HandleSPIMIDIInterrupt(void *pContext) {
	g_fSPIMIDIInterrupt = true;

	SPI_MESSAGE *pSPIMessage = NULL;

	if(SSIReadSPIMessage(&pSPIMessage) != R_OK) {
		DEBUG_LINEOUT("HandleSPIMIDIInterrupt: Failed to read SPI message");
		g_fSPIMIDIInterrupt = false;
		return R_FAIL;
	}

	switch(pSPIMessage->header.type) {
		case SPI_MSG_MIDI: {
			//SPI_MIDI_MESSAGE *pSPIMidiMessage = (SPI_MIDI_MESSAGE *)(pSPIMessage);

			//DEBUG_LINEOUT("spi midi: 0x%x %d %d", pSPIMidiMessage->midiMsg.type, pSPIMidiMessage->midiMsg.data1, pSPIMidiMessage->midiMsg.data2);

			// Queue incoming MIDI
			// CRM(HandleMIDIPacket(pSPIMidiMessage->midiMsg), "HandleSPIMIDI: Failed to handle MIDI Message");

			//if(QueueNewIncomingBLEMsg(pSPIMidiMessage->midiMsg) != R_OK) {
			if(QueueNewIncomingBLEMsg(((SPI_MIDI_MESSAGE *)(pSPIMessage))->midiMsg) != R_OK) {
				DEBUG_LINEOUT("HandleSPIMIDI: Failed to queue BLE MIDI Message");
				g_fSPIMIDIInterrupt = false;
				return R_FAIL;
			}
		} break;

		case SPI_MSG_SYS: {
			SPI_SYS_MESSAGE *pSPISysMessage = (SPI_SYS_MESSAGE *)(pSPIMessage);
			//DEBUG_LINEOUT("spi sys msg");
			switch(pSPISysMessage->msgType) {
				case SPI_SYS_BLE_CONNECTED: {
					SetBLEConnect(true);
					DEBUG_LINEOUT("BLE Connected!");
				} break;

				case SPI_SYS_BLE_DISCONNECTED: {
					SetBLEConnect(false);
					DEBUG_LINEOUT("BLE Disconnected!");
				} break;

				default: {
					DEBUG_LINEOUT("Unhandled SPI SYS MSG 0x%x", pSPISysMessage->msgType);
				} break;
			}
		} break;
	}

	g_fSPIMIDIInterrupt = false;
	return R_OK;
}

// If the INT signal is high, but we didn't hit the ISR
// then likely we have a hang condition
RESULT CheckForSPIMIDIHangCondition() {
	if(g_fSPIMIDIInterrupt == false && m_fSPIMIDIInitialized == true) {
		if(SSICheckInterruptLine(m_SPIMidiConfig) == true) {
			DEBUG_LINEOUT("SPI hang detected");
			return HandleSPIMIDIInterrupt(NULL);
		}
	}

	return R_OK;
}


// This will read a message by sending shorts, this will copy the data into new memory
// TODO: This will discard the RX data for now
// TODO: Should this go into the SPI Controller?
inline RESULT SSIReadSPIMessage(SPI_MESSAGE **n_ppSPIMessage) {
	RESULT r = R_OK;
	int i = 0;
	uint16_t tempShort = 0xFFFF;
	uint16_t rxBuffer[MAX_SPI_MSG_LENGTH];
	uint8_t spiNum = m_SPIMidiConfig;			// TODO: Make this more generic later?

	SSI_PERIPHERAL_INFO *pSSIPeripheral = GetSSIConfig(m_SPIMidiConfig);

	CNRM(pSSIPeripheral, "SSIReadSPIMessage: Failed to get config");
	CBRM((*n_ppSPIMessage == NULL), "SSIReadSPIMessage: spi message must be null");

	memset(&rxBuffer, 0, sizeof(rxBuffer));

	if(pSSIPeripheral->select.fEnabled)
		GPIOPinWrite(pSSIPeripheral->select.GPIOPort, pSSIPeripheral->select.pin, 0x00);		// active low

	SSIDataPut(pSSIPeripheral->gpio.SSIBase, 0xFFFF);
	SSIDataGet(pSSIPeripheral->gpio.SSIBase, &rxBuffer[0]);

	SPI_MESSAGE_HEADER SPIMessageHeader = {
		.type = (rxBuffer[0] & 0xFF00) >> 8,
		.length = (rxBuffer[0] & 0x00FF)
	};

	for(i = 1; i < (SPIMessageHeader.length / 2); i++) {
		SSIDataPut(pSSIPeripheral->gpio.SSIBase, 0xFFFF);
		SSIDataGet(pSSIPeripheral->gpio.SSIBase, &rxBuffer[i]);
	}

	while(SSIBusy(pSSIPeripheral->gpio.SSIBase)){ /* wait while busy */ };

	if(pSSIPeripheral->select.fEnabled)
		GPIOPinWrite(pSSIPeripheral->select.GPIOPort, pSSIPeripheral->select.pin, 0xFF);		// active low

	*n_ppSPIMessage = (SPI_MESSAGE*)malloc(SPIMessageHeader.length);

	// Stupid endianess thing
	for(i = 0; i < SPIMessageHeader.length; i += 2) {
		((uint8_t*)(*n_ppSPIMessage))[i] = (uint8_t)((rxBuffer[i/2] & 0xFF00) >> 8);
		((uint8_t*)(*n_ppSPIMessage))[i + 1] = (uint8_t)((rxBuffer[i/2] & 0x00FF));
	}

	/*
	DEBUG_LINEOUT("SPI RX %d bytes", SPIMessageHeader.length);
	UARTprintfBinaryData((unsigned char*)(n_ppSPIMessage), SPIMessageHeader.length, 20);
	//*/

Error:
	return r;
}

RESULT SetSPIMIDIConfig(int SPIMidiConfig) {
	m_SPIMidiConfig = SPIMidiConfig;
	return R_OK;
}

RESULT InitializeSPIMIDI(int spiConfigNum) {
	RESULT r = R_OK;
	m_SPIMidiConfig = spiConfigNum;

	// Register a CB for the first SPI config
	// TODO: Eventually have this set up the SPI config and kick it off
	SSI_PERIPHERAL_INFO *pConfig = GetSSIConfig(m_SPIMidiConfig);
	CNRM(pConfig, "Could not find SSI Configuration %d for SPI", m_SPIMidiConfig);

	pConfig->intpin.cbInt = HandleSPIMIDIInterrupt;

	CRM(InitializeIncomingBLEQueue(), "Failed to initialize BLE Msg Queue");

	//CRM(SPI1Init(), "init: Failed to initialize SPI1 at %d bitrate");
	CRM(SSIInit(m_SPIMidiConfig), "init: Failed to initialize SPI%d at %d bitrate", m_SPIMidiConfig);

	m_fSPIMIDIInitialized = true;
	DEBUG_LINEOUT("SPI MIDI initialized on config %d", m_SPIMidiConfig);

Error:
	return r;
}

// MIDI functions
RESULT SendSPIMidiNoteMsg(uint8_t midiVal, uint8_t midiVelocity, uint8_t channel, uint8_t fOnOff) {
	// Set up the Midi Note Message
	SPI_MIDI_MESSAGE spiMidiNoteMessage;
	memset(&spiMidiNoteMessage, 0, sizeof(spiMidiNoteMessage));

	spiMidiNoteMessage.header.type = SPI_MSG_MIDI;
	spiMidiNoteMessage.header.length = sizeof(spiMidiNoteMessage);
	spiMidiNoteMessage.midiMsg = MakeMIDINoteMessage(fOnOff, channel, midiVal, midiVelocity);

	DEBUG_LINEOUT("Sending SPI note %s val %d chan %d", (fOnOff) ? "on" : "off", channel, midiVal);

	//return SSI1SendBuffer((uint16_t*)(&spiMidiNoteMessage), sizeof(spiMidiNoteMessage) / 2);
	return SSISendBuffer(m_SPIMidiConfig, (uint16_t*)(&spiMidiNoteMessage), sizeof(spiMidiNoteMessage) / 2);
}

RESULT SendSPIMidiCC(uint8_t index, uint8_t value, uint8_t channel) {
	// Set up the Midi Note Message
	SPI_MIDI_MESSAGE spiMidiMessage;
	memset(&spiMidiMessage, 0, sizeof(spiMidiMessage));

	// HEADER
	spiMidiMessage.header.type = SPI_MSG_MIDI;
	spiMidiMessage.header.length = sizeof(spiMidiMessage);

	// MIDI MSG
	spiMidiMessage.midiMsg.type.type = MIDI_NIBBLE_CONTROL_CHANGE;
	spiMidiMessage.midiMsg.type.channel = channel;
	spiMidiMessage.midiMsg.data1 = index;
	spiMidiMessage.midiMsg.data2 = value;

	//return SSI1SendBuffer((uint16_t*)(&spiMidiMessage), sizeof(spiMidiMessage) / 2);
	return SSISendBuffer(m_SPIMidiConfig, (uint16_t*)(&spiMidiMessage), sizeof(spiMidiMessage) / 2);
}

// SendFirmwareVersion
// This will send the major and minor firmware version to the host
RESULT SendSPIFirmwareVersion() {

	DEVICE_FIRMWARE_VERSION fwVersion = GetDeviceFirmwareVersion();

#ifdef USB_VERBOSE
	DEBUG_LINEOUT("Sending SPI Firmware Version %d.%d", fwVersion.major, fwVersion.minor);
#endif

	//return SendSPIMidiCC(GTAR_SEND_MSG_FIRMWARE_VERSION, ((FW_MAJOR_VERSION & 0xF) << 4) + ((FW_MINOR_VERSION) & 0xF), 0);
	return SendSPIMidiCC(DEVICE_SEND_MSG_FIRMWARE_VERSION, ((fwVersion.major & 0xF) << 4) + ((fwVersion.minor) & 0xF), 0);
}

// SendFirmwareDownloadAck
// This will send the major and minor firmware version to the host
RESULT SendSPIFirmwareDownloadAck(uint8_t status) {
	return SendSPIMidiCC(DEVICE_ACK_DOWNLOAD_FW, status, 0);
}

// Send Battery Status Ack
RESULT SendSPIBatteryStatusAck() {
	return SendSPIMidiCC(DEVICE_ACK_BATTERY_STATUS, ((uint8_t)IsDeviceCharging() & 0x7F), 0);
}

RESULT SendSPIAck(uint8_t SendBuffer[4]) {
	// Simply strip away the first byte and translate to midi msg

	// Set up the Midi Note Message
	SPI_MIDI_MESSAGE spiMidiMessage;
	memset(&spiMidiMessage, 0, sizeof(spiMidiMessage));

	spiMidiMessage.header.type = SPI_MSG_MIDI;
	spiMidiMessage.header.length = sizeof(spiMidiMessage);

	memcpy(&spiMidiMessage.midiMsg.type, &SendBuffer[1], sizeof(MIDI_TYPE));
	spiMidiMessage.midiMsg.data1 = SendBuffer[2];
	spiMidiMessage.midiMsg.data2 = SendBuffer[3];

	//return SSI1SendBuffer((uint16_t*)(&spiMidiMessage), sizeof(spiMidiMessage) / 2);
	return SSISendBuffer(m_SPIMidiConfig, (uint16_t*)(&spiMidiMessage), sizeof(spiMidiMessage) / 2);
}

// Send Battery Status Ack
RESULT SendSPIBatteryChargePercentageAck() {
	uint8_t percentage = GetDeviceBatteryPercentage();
	return SendSPIMidiCC(DEVICE_ACK_BATTERY_PERCENTAGE, percentage & 0x7F, 0);
}

RESULT SendSPICommitUserspaceAck(uint8_t status) {
	return SendSPIMidiCC(DEVICE_ACK_COMMIT_USERSPACE, (uint8_t)(status), 0);
}

RESULT SendSPIResetUserspaceAck(uint8_t status) {
	return SendSPIMidiCC(DEVICE_ACK_RESET_USERSPACE, (uint8_t)(status), 0);
}

RESULT SendSPIRequestSerialNumberAck(uint8_t byteNumber) {
	// max 16 bytes in serial number
	if(byteNumber > 0x0F)
		return R_OUT_OF_BOUNDS;

#ifdef USB_VERBOSE
	DEBUG_LINEOUT("Serial: Sending byte %d val:0x%x", byteNumber, (uint8_t)(g_UserSpace.serial[byteNumber]));
#endif

	uint8_t serialByte = GetDeviceSerialNumber(byteNumber);
	return SendSPIMidiCC(DEVICE_ACK_REQUEST_SERIAL_NUMBER, serialByte, 0);
}

