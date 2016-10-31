#ifndef USERSPACE_CONTROLLER_H_
#define USERSPACE_CONTROLLER_H_

#include "../Common/EHM.h"
#include "../Device/Device.h"


#define SERIAL_NUMBER_BYTES 16

typedef enum {
	FW_UPDATE_NONE	  	= 	0x00,
	FW_UPDATE_PENDING 	= 	0x01,
	FW_UPDATE_COMMITTING = 	0x02,
	FW_UPDATE_INVALD
} FW_UPDATE_STATUS;

#pragma pack(push, 1) // exact fit - no padding
typedef struct {
	uint8_t DeviceID;
	uint16_t nSize;				// size in bytes
	uint16_t DeviceIDEx;
	uint8_t fw_major_version;
	uint8_t fw_minor_version;
	uint8_t fw_minor_minor_version;
	uint8_t fw_update_status;
	uint8_t fw_downloaded_pages;

	// Added to make this structure size divisible by 4
	uint8_t serial[SERIAL_NUMBER_BYTES];

	uint8_t reserved_1;
	uint8_t reserved_2;
	//uint8_t reserved_3;
} USER_SPACE;
#pragma pack(pop)

extern void *g_pUserSpaceAddr;
extern USER_SPACE *g_pUserSpace;
extern int g_UserSpace_n;

typedef void (*cbHandleCommitFirmware)();
extern cbHandleCommitFirmware g_HandleCommitFirmware;
RESULT RegisterHandleCommitFirmwareCallback(cbHandleCommitFirmware HandleCommitFirmwareCB);
RESULT UnregisterHandleCommitFirmwareCallback();

RESULT EraseUserSpace();
RESULT EraseUserSpacePreserveSerialNumber();
RESULT SetUSFwUpdateStatus(FW_UPDATE_STATUS newStatus);
RESULT CommitUserSpace();

void *GetUserspaceSerialAddress();

// Opho Device Interfaces
uint8_t GetDeviceSerialNumber(uint8_t byteNum);
void *GetDeviceUserspaceAddress();
uint8_t *GetDeviceUserspaceSerialAddress();
uint8_t GetDeviceUserspaceSerialLength();
uint8_t GetDeviceUserspaceAddressLength();

RESULT DebugUnit();
RESULT OutputSerialToDebug();
RESULT SetSerialNumber( int32_t  val);
RESULT IsSerialNumberZero();

RESULT PrintUserspace();
RESULT FlashProgramUserspace();

typedef RESULT (*cbInitUserSpace)(USER_SPACE *);
RESULT InitUserSpace(int UserSpaceSize, cbInitUserSpace fnInitUserSpace);
RESULT AllocateUserSpace();
RESULT DeallocateUserSpace();
RESULT ClearUserSpaceMemory();
RESULT LoadUserSpaceToMemory();

#endif // ! USERSPACE_CONTROLLER_H_
