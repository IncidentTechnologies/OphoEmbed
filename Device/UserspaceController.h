#ifndef USERSPACE_CONTROLLER_H_
#define USERSPACE_CONTROLLER_H_

#include "../Common/EHM.h"
#include "../Device/Device.h"

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
	uint8_t serial[16];

	uint8_t reserved_1;
	uint8_t reserved_2;
	uint8_t reserved_3;
} USER_SPACE;
#pragma pack(pop)

typedef void (*cbHandleCommitFirmware)();
extern cbHandleCommitFirmware g_HandleCommitFirmware;
RESULT RegisterHandleCommitFirmwareCallback(cbHandleCommitFirmware HandleCommitFirmwareCB);
RESULT UnregisterHandleCommitFirmwareCallback();

RESULT EraseUserSpace();
RESULT EraseUserSpacePreserveSerialNumber();
RESULT SetUSFwUpdateStatus(FW_UPDATE_STATUS newStatus);
RESULT CommitUserSpace();

#endif // ! USERSPACE_CONTROLLER_H_
