#ifndef GTAR_MIDI_MESSAGES_H_
#define GTAR_MIDI_MESSAGES_H_

#include <stdint.h>
#include "midi.h"

typedef enum DeviceSendMessage {
	DEVICE_SEND_MSG_FIRMWARE_VERSION = 0x32,
	DEVICE_SEND_MSG_INVALID
} DEVICE_SEND_MESSAGE;

typedef enum DeviceMidiCCMessage {
	GTAR_MIDI_CC_SET_LED_0 = 0x33,
	GTAR_MIDI_CC_SET_LED_1 = 0x34,
	GTAR_MIDI_CC_INVALID
} DEVICE_MIDI_CC_MSG;

typedef enum DeviceAckType {
	DEVICE_ACK_DOWNLOAD_FW = 0x35,
	DEVICE_ACK_BATTERY_STATUS = 0x36,
	DEVICE_ACK_BATTERY_PERCENTAGE = 0x37,
	DEVICE_ACK_REQUEST_SERIAL_NUMBER = 0x3C,
	DEVICE_ACK_COMMIT_USERSPACE = 0x3E,
	DEVICE_ACK_RESET_USERSPACE = 0x3F,
	DEVICE_ACK_INVALID
} DEVICE_ACK_TYPE;

typedef enum DeviceMidiMessage {
	DEVICE_MSG_REQ_FW_VERSION = 0x04,
	DEVICE_MSG_DOWNLOAD_NEW_FW_PACKAGE = 0x05,
	DEVICE_MSG_EXEC_FW_UPDATE = 0x06,
	DEVICE_MSG_ENABLE_VELOCITY = 0x0C,
	DEVICE_MSG_DISABLE_VELOCITY = 0x0D,
	DEVICE_MSG_REQ_BATTERY_STATUS = 0x0E,
	DEVICE_MSG_ENABLE_DEBUG = 0x11,
	DEVICE_MSG_DISABLE_DEBUG = 0x12,
	DEVICE_MSG_REQ_SERIAL_NUM = 0x18,
	DEVICE_MSG_SET_SERIAL_NUMBER = 0x19,
	DEVICE_MSG_SET_ACCELEROMETER_STATE = 0x1A,
	DEVICE_MSG_COMMIT_USERSPACE = 0x1D,
	DEVICE_MSG_RESET_USERSPACE = 0x1E,
	DEVICE_MSG_INVALID
} DEVICE_MIDI_MSG;

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

typedef struct DeviceMsgHeader {
	uint8_t deviceID;
	DEVICE_MIDI_MSG msgType;
} DEVICE_MSG_HEADER;

typedef struct DeviceMsg {
	DEVICE_MSG_HEADER header;
} DEVICE_MSG;

typedef struct DeviceSetState {
	DEVICE_MSG_HEADER header;
	uint8_t state;
} DEVICE_SET_STATE;

typedef struct DeviceSetThreshold {
	DEVICE_MSG_HEADER header;
	uint8_t thresh;
} DEVICE_SET_THRESH;

typedef struct DeviceSetThresholdShort {
	DEVICE_MSG_HEADER header;
	uint8_t thresh_upper;
	uint8_t thresh_lower;
} DEVICE_SET_THRESH_SHORT;

typedef struct DeviceSetRGBM {
	DEVICE_MSG_HEADER header;
	RGBM color;
} DEVICE_SET_RGBM;

typedef struct DeviceSetMRGB {
	DEVICE_MSG_HEADER header;
	MRGB color;
} DEVICE_SET_MRGB;

typedef struct DeviceSetTimeout {
	DEVICE_MSG_HEADER header;
	uint8_t mstimeout;
} DEVICE_SET_TIMEOUT;

typedef struct DeviceRequestByteNumber {
	DEVICE_MSG_HEADER header;
	uint8_t byteNumber;
} DEVICE_REQUEST_BYTE_NUMBER;

typedef struct DeviceSetSerialNumber {
	DEVICE_MSG_HEADER header;
	uint8_t serialNumber7F[16];
} DEVICE_SET_SERIAL_NUMBER;

typedef struct DeviceUpdateBootloader {
	DEVICE_MSG_HEADER header;
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
} DEVICE_UPDATE_BOOTLOADER;

typedef struct DeviceDownloadNewFirmwarePageHeader {
	DEVICE_MSG_HEADER header;

	uint8_t size_ub_7f;				// bootloader size upper byte 7f wrapped
	uint8_t size_mb_7f;				// bootloader size middle byte 7f wrapped
	uint8_t size_lb_7f;				// bootloader size lower byte 7f wrapped
	uint8_t totalPageCount;
	uint8_t currentPage;
	uint8_t reserved_0;
	uint8_t transmitted_ub_7f;		// transmitted size upper byte 7f wrapped
	uint8_t transmitted_lb_7f;		// transmitted size lower byte 7f wrapped
	uint8_t reserved_1;
} DEVICE_DOWNLOAD_NEW_FW_PAGE_HEADER;

typedef struct DeviceDownloadNewFirmwarePage {
	DEVICE_MSG_HEADER header;

	int firmwarePageBytes;			// Size - converted from 3 7f wrapped bytes
	uint8_t totalPageCount;
	uint8_t currentPage;

	int transmittedBytes;			// transmitted data - converted from 2 7f wrapped bytes

	uint8_t *pFirmwareBuffer;		// This memory is allocated, so ensure it's freed (although not important since this operation requires restart)

	uint8_t checksum;
} DEVICE_DOWNLOAD_NEW_FW_PAGE;

// TODO: Should this be here?  Not all devices may have an accelerometer
typedef struct DeviceSetAccelState {
	DEVICE_MSG_HEADER header;

	struct {
		unsigned xaxis: 1;
		unsigned yaxis: 1;
		unsigned zaxis: 1;
		unsigned reserved: 5;
	} state;

	uint8_t speed;
} DEVICE_SET_ACCEL_STATE;


#endif // !GTAR_MIDI_MESSAGES_H_
