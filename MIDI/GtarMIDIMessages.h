#ifndef GTAR_MIDI_MESSAGES_H_
#define GTAR_MIDI_MESSAGES_H_

#include <stdint.h>
#include "midi.h"

typedef enum gTarSendMessage {
	GTAR_SEND_MSG_FRET_UP = 0x30,
	GTAR_SEND_MSG_FRET_DOWN = 0x31,
	GTAR_SEND_MSG_FIRMWARE_VERSION = 0x32,
	GTAR_SEND_MSG_INVALID
} GTAR_SEND_MESSAGE;

typedef enum gTarMidiCCMessage {
	GTAR_MIDI_CC_SET_LED_0 = 0x33,
	GTAR_MIDI_CC_SET_LED_1 = 0x34,
	GTAR_MIDI_CC_INVALID
} GTAR_MIDI_CC_MSG;

typedef enum gTarAckType {
	GTAR_ACK_DOWNLOAD_FW = 0x35,
	GTAR_ACK_BATTERY_STATUS = 0x36,
	GTAR_ACK_BATTERY_PERCENTAGE = 0x37,
	GTAR_ACK_GET_PIEZO_CT_MATRIX = 0x38,
	GTAR_ACK_GET_PIEZO_SENSITIVITY = 0x39,
	GTAR_ACK_GET_PIEZO_WINDOW = 0x3A,
	GTAR_ACK_GET_PIEZO_TIMEOUT = 0x3B,
	GTAR_ACK_REQUEST_SERIAL_NUMBER = 0x3C,
	GTAR_ACK_CALIBRATE_PIEZO_STRING = 0x3D,
	GTAR_ACK_COMMIT_USERSPACE = 0x3E,
	GTAR_ACK_RESET_USERSPACE = 0x3F,
	GTAR_ACK_DOWNLOAD_PIEZO_FW = 0x40,
	GTAR_ACK_PIEZO_CMD = 0x41,
	GTAR_ACK_PIEZO_CMD_RESPONSE = 0x42,
	GTAR_ACK_INVALID
} GTAR_ACK_TYPE;

typedef enum gTarMidiMessage {
	GTAR_MSG_SET_LED = 0x00,
	GTAR_MSG_SET_NOTE_ACTIVE = 0x01,
	GTAR_MSG_SET_FRET_FOLLOW = 0x02,
	GTAR_MSG_REQ_AUTH_REDOWNLOAD = 0x03,
	GTAR_MSG_REQ_FW_VERSION = 0x04,
	GTAR_MSG_DOWNLOAD_NEW_FW_PACKAGE = 0x05,
	GTAR_MSG_EXEC_FW_UPDATE = 0x06,
	GTAR_MSG_SET_PIEZO_STATE = 0x07,
	GTAR_MSG_SET_PIEZO_THRESH = 0x08,
	GTAR_MSG_SET_SMART_PICK_THRESH = 0x09,
	GTAR_MSG_SET_LED_EX = 0x0A,

	GTAR_MSG_CALIBRATE_PIEZO_STRING = 0x0B,

	GTAR_MSG_ENABLE_VELOCITY = 0x0C,
	GTAR_MSG_DISABLE_VELOCITY = 0x0D,
	GTAR_MSG_REQ_BATTERY_STATUS = 0x0E,
	GTAR_MSG_SET_FRETBOARD_THRESH = 0x0F,

	GTAR_MSG_ENABLE_DEBUG = 0x11,
	GTAR_MSG_DISABLE_DEBUG = 0x12,

	// Piezo Sensor
	GTAR_MSG_SET_PIEZO_CT_MATRIX = 0x13,
	GTAR_MSG_GET_PIEZO_CT_MATRIX = 0x14,
	GTAR_MSG_SET_PIEZO_SENSITIVITY = 0x15,
	GTAR_MSG_GET_PIEZO_SENSITIVITY = 0x16,
	GTAR_MSG_SET_PIEZO_WINDOW = 0x17,
	GTAR_MSG_GET_PIEZO_WINDOW = 0x10,

	GTAR_MSG_GET_PIEZO_TIMEOUT = 0x1B,
	GTAR_MSG_SET_PIEZO_TIMEOUT = 0x1C,

	GTAR_MSG_REQ_SERIAL_NUM = 0x18,
	GTAR_MSG_SET_SERIAL_NUMBER = 0x19,
	GTAR_MSG_SET_ACCELEROMETER_STATE = 0x1A,

	GTAR_MSG_COMMIT_USERSPACE = 0x1D,
	GTAR_MSG_RESET_USERSPACE = 0x1E,
	GTAR_MSG_DOWNLOAD_NEW_PIEZO_FW_PACKAGE = 0x1F,
	GTAR_MSG_PIEZO_CMD = 0x20,
	GTAR_MSG_INVALID
} GTAR_MIDI_MSG;

typedef struct rgbm {
	unsigned m: 2;
	unsigned b: 2;
	unsigned g: 2;
	unsigned r: 2;		// MSB
} RGBM;

typedef struct mrgb {
	unsigned b: 2;
	unsigned g: 2;
	unsigned r: 2;
	unsigned m: 2;		// MSB
} MRGB;

RGBM UintToRGBM(uint8_t uin);
uint8_t RGBMToUint(RGBM rgbm);
MRGB UintToMRGB(uint8_t uin);
uint8_t MRGBToUint(MRGB rgbm);

typedef struct GtarMsgHeader {
	uint8_t gtarid;
	GTAR_MIDI_MSG msgType;
} GTAR_MSG_HEADER;

typedef struct GtarMsg {
	GTAR_MSG_HEADER header;
} GTAR_MSG;

typedef struct GtarSetState {
	GTAR_MSG_HEADER header;
	uint8_t state;
} GTAR_SET_STATE;

typedef struct GtarSetThreshold {
	GTAR_MSG_HEADER header;
	uint8_t thresh;
} GTAR_SET_THRESH;

typedef struct GtarSetThresholdShort {
	GTAR_MSG_HEADER header;
	uint8_t thresh_upper;
	uint8_t thresh_lower;
} GTAR_SET_THRESH_SHORT;

typedef struct GtarSetRGBM {
	GTAR_MSG_HEADER header;
	RGBM color;
} GTAR_SET_RGBM;

typedef struct GtarSetMRGB {
	GTAR_MSG_HEADER header;
	MRGB color;
} GTAR_SET_MRGB;

typedef struct GtarSetLEDMsg {
	GTAR_MSG_HEADER header;
	uint8_t string;
	uint8_t fret;
	RGBM color;
} GTAR_SET_LED;

typedef struct GtarSetLEDEx {
	GTAR_MSG_HEADER header;
	uint8_t string;
	uint8_t fret;
	MRGB color;
} GTAR_SET_LED_EX;

typedef struct GtarSetPiezoThreshold {
	GTAR_MSG_HEADER header;
	uint8_t string;			// 1 based
	uint8_t threshold;		// rec = 6
} GTAR_SET_PIEZO_THRESHOLD;

typedef struct GtarSetTimeout {
	GTAR_MSG_HEADER header;
	uint8_t mstimeout;
} GTAR_SET_TIMEOUT;

typedef struct GtarRequestByteNumber {
	GTAR_MSG_HEADER header;
	uint8_t byteNumber;
} GTAR_REQUEST_BYTE_NUMBER;

typedef struct GtarSetSerialNumber {
	GTAR_MSG_HEADER header;
	uint8_t serialNumber7F[16];
} GTAR_SET_SERIAL_NUMBER;

typedef struct GtarSetPiezoCTMatrixValue {
	GTAR_MSG_HEADER header;
	unsigned row: 4;
	unsigned column: 4;
	uint8_t value;
} GTAR_SET_PIEZO_CT_MATRIX_VALUE;

typedef struct GtarGetPiezoCTMatrixValue {
	GTAR_MSG_HEADER header;
	unsigned row: 4;
	unsigned column: 4;
} GTAR_GET_PIEZO_CT_MATRIX_VALUE;

typedef struct GtarGetPiezoValue {
	GTAR_MSG_HEADER header;
	uint8_t channel;
} GTAR_GET_PIEZO_VALUE;

typedef struct GtarSetPiezoValue {
	GTAR_MSG_HEADER header;
	uint8_t channel;
	uint8_t value;
} GTAR_SET_PIEZO_VALUE;

typedef struct GtarSetAccelState {
	GTAR_MSG_HEADER header;

	struct {
		unsigned xaxis: 1;
		unsigned yaxis: 1;
		unsigned zaxis: 1;
		unsigned reserved: 5;
	} state;

	uint8_t speed;
} GTAR_SET_ACCEL_STATE;

typedef struct GtarUpdateBootloader {
	GTAR_MSG_HEADER header;
	/*
	uint8_t size_ub_7f;				// bootloader size upper byte 7f wrapped
	uint8_t size_mb_7f;				// bootloader size middle byte 7f wrapped
	uint8_t size_lb_7f;				// bootloader size lower byte 7f wrapped
	uint8_t transmitted_ub_7f;		// transmitted size upper byte 7f wrapped
	uint8_t transmitted_lb_7f;		// transmitted size lower byte 7f wrapped
	*/
	int bootloaderBytes;			// Size - converted from 3 7f wrapped bytes
	int transmittedBytes;			// transmitted data - converted from 2 7f wrapped bytes

	uint8_t *pBootloaderBuffer;		// This memory is allocated, so ensure it's freed (although not important since this operation requires restart)

	uint8_t checksum;
} GTAR_UPDATE_BOOTLOADER;

typedef struct GtarDownloadNewFirmwarePageHeader {
	GTAR_MSG_HEADER header;

	uint8_t size_ub_7f;				// bootloader size upper byte 7f wrapped
	uint8_t size_mb_7f;				// bootloader size middle byte 7f wrapped
	uint8_t size_lb_7f;				// bootloader size lower byte 7f wrapped
	uint8_t totalPageCount;
	uint8_t currentPage;
	uint8_t reserved_0;
	uint8_t transmitted_ub_7f;		// transmitted size upper byte 7f wrapped
	uint8_t transmitted_lb_7f;		// transmitted size lower byte 7f wrapped
	uint8_t reserved_1;
} GTAR_DOWNLOAD_NEW_FW_PAGE_HEADER;

typedef struct GtarDownloadNewFirmwarePage {
	GTAR_MSG_HEADER header;

	int firmwarePageBytes;			// Size - converted from 3 7f wrapped bytes
	uint8_t totalPageCount;
	uint8_t currentPage;

	int transmittedBytes;			// transmitted data - converted from 2 7f wrapped bytes

	uint8_t *pFirmwareBuffer;		// This memory is allocated, so ensure it's freed (although not important since this operation requires restart)

	uint8_t checksum;
} GTAR_DOWNLOAD_NEW_FW_PAGE;


MIDI_MSG MakeMIDIGtarFretMessage(uint8_t fFretDown, uint8_t channel, uint8_t fret);


#endif // !GTAR_MIDI_MESSAGES_H_
