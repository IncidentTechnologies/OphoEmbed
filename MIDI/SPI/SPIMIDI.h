#ifndef SPI_MIDI_H_
#define SPI_MIDI_H_

//#include "gtar.h"
#include "../../Common/EHM.h"
#include <stdbool.h>

#include "../GtarMIDIMessages.h"

#define MIDI_SPI_BASE SYSCTL_PERIPH_SSI1

#define MAX_INCOMING_BLE_PENDING_MSGS 20
#define INCOMING_BLE_PENDING_MSG_INTERVAL 10000

typedef enum SPIMessageType {
	SPI_MSG_RESERVED = 0x00,
	SPI_MSG_MIDI,
	SPI_MSG_SYS,
	SPI_MSG_INVALID
} SPI_MESSAGE_TYPE;

#pragma pack(push, 1) // exact fit - no padding
typedef struct SPIMessageHeader {
	uint8_t type;
	uint8_t length;
} SPI_MESSAGE_HEADER;
#pragma pack(pop)

#pragma pack(push, 1) // exact fit - no padding
typedef struct SPIMIDIMessage {
	SPI_MESSAGE_HEADER header;
	MIDI_MSG midiMsg;
	uint8_t reserved;
} SPI_MIDI_MESSAGE;
#pragma pack(pop)

typedef enum SPISysMessageType {
	SPI_SYS_BLE_RESERVED = 0x00,
	SPI_SYS_BLE_CONNECTED,
	SPI_SYS_BLE_DISCONNECTED,
	SPI_SYS_BLE_INVALID
} SPI_SYS_MESSAGE_TYPE;

#pragma pack(push, 1) // exact fit - no padding
typedef struct SPISYSMessage {
	SPI_MESSAGE_HEADER header;
	uint8_t msgType;
	uint8_t optParam;
} SPI_SYS_MESSAGE;
#pragma pack(pop)

#pragma pack(push, 1) // exact fit - no padding
typedef struct SPIMessage {
	SPI_MESSAGE_HEADER header;
} SPI_MESSAGE;
#pragma pack(pop)

extern bool g_fSPIMIDIInterrupt;
RESULT HandleSPIMIDIInterrupt(void *pContext);
RESULT CheckForSPIMIDIHangCondition();

RESULT InitializeSPIMIDI();

// MIDI functions
RESULT SendSPIMidiNoteMsg(uint8_t midiVal, uint8_t midiVelocity, uint8_t channel, uint8_t fOnOff);

//RESULT SendSPIMidiCC(uint8_t index, uint8_t value, uint8_t channel);
RESULT SendSPIFirmwareVersion();
RESULT SendSPIFirmwareDownloadAck(uint8_t status);

RESULT SendSPIBatteryStatusAck();
RESULT SendSPIBatteryChargePercentageAck();

RESULT SendSPIRequestSerialNumberAck(uint8_t byteNumber);

RESULT SendSPIAck(uint8_t SendBuffer[4]);

RESULT SendSPICommitUserspaceAck(uint8_t status);
RESULT SendSPIResetUserspaceAck(uint8_t status);

RESULT SSIReadSPIMessage(SPI_MESSAGE **n_ppSPIMessage);

// Incoming BLE Messages
RESULT InitializeIncomingBLEQueue();
inline RESULT QueueNewIncomingBLEMsg(MIDI_MSG midiMsg);
uint8_t IsIncomingBLEMsgPending();
inline RESULT HandleQueuedIncomingBLEMsg();

#endif // !SPI_MIDI_H_
