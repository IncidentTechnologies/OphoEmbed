#include "UserspaceController.h"

// User Space flash location
USER_SPACE g_UserSpace;
uint32_t  g_UserSpaceAddr = USER_SPACE_ADDRESS;	// this is set to the 122KB point in flash
volatile uint8_t spacer;

//volatile void (*g_vdFnPtr)() = CommitFirmware;

volatile cbHandleCommitFirmware g_HandleCommitFirmwareCallback = NULL;
RESULT RegisterHandleCommitFirmwareCallback(cbHandleCommitFirmware HandleCommitFirmwareCB) {
	RESULT r = R_OK;

	CBRM_NA((g_HandleCommitFirmwareCallback == NULL), "RegisterHandleCommitFirmwareCallback: Handle Commit Firmware Callback already registered");
	g_HandleCommitFirmwareCallback = HandleCommitFirmwareCB;

Error:
	return r;
}

RESULT UnregisterHandleCommitFirmwareCallback() {
	RESULT r = R_OK;

	CBRM_NA((g_HandleCommitFirmwareCallback != NULL), "UnregisterHandleCommitFirmwareCallback: Handle Commit Firmware Callback not registered");
	g_HandleCommitFirmwareCallback = NULL;

Error:
	return r;
}

RESULT EraseUserSpace() {
	RESULT r = R_OK;

	// Clear the UserSpace
	DEBUG_LINEOUT_NA("+EraseUserSpace");

	uint32_t  ulRes = ROM_FlashErase(USER_SPACE_ADDRESS);	// first kb
	CBRM((ulRes == 0), "EraseUserSpace: Failed to erase address 0x%x", USER_SPACE_ADDRESS);
	SysCtlDelay(ROM_SysCtlClockGet() / 100);

	DEBUG_LINEOUT_NA("-EraseUserSpace");

Error:
	return r;
}

RESULT EraseUserSpacePreserveSerialNumber() {
	RESULT r = R_OK;
	uint8_t tempSerial[16];
	 int32_t  i = 0;
	void *ulPtr = (void*)(g_UserSpaceAddr);
	USER_SPACE *pUserSpace = (USER_SPACE*)(ulPtr);

	DEBUG_LINEOUT_NA("+EraseUserPsacePreserveSerialNumber");

	// Save the serial
	memcpy(&g_UserSpace, pUserSpace, sizeof(USER_SPACE));	// copy in user space
	memcpy(tempSerial, g_UserSpace.serial, sizeof(tempSerial));

	// Clear UserSpace
	uint32_t  ulRes = ROM_FlashErase(USER_SPACE_ADDRESS);	// first kb
	CBRM((ulRes == 0), "EraseUserSpace: Failed to erase address 0x%x", USER_SPACE_ADDRESS);
	SysCtlDelay(ROM_SysCtlClockGet() / 100);

	// Read back in userspace into mem
	memcpy(&g_UserSpace, pUserSpace, sizeof(USER_SPACE));

	// Copy serial number back in
	memcpy(g_UserSpace.serial, tempSerial, sizeof(tempSerial));

	// Commit back to userspace flash
	CRM_NA(CommitUserSpace(),"EraseUserSpacePreserveSerialNumber: Failed to Commit UserSpace");

Error:
	DEBUG_LINEOUT_NA("-EraseUserPsacePreserveSerialNumber");
	return r;
}

RESULT SetUSFwUpdateStatus(FW_UPDATE_STATUS newStatus) {
	RESULT r = R_OK;
	void *ulPtr = (void*)(g_UserSpaceAddr);

	EraseUserSpace();

	DEBUG_LINEOUT("SetUSFwUpdateStatus: setting to new fw update status: 0x%x", (uint8_t)newStatus);
	g_UserSpace.fw_update_status = (uint8_t)newStatus;
	g_UserSpace.fw_downloaded_pages = g_DownloadedFirmwarePages;

	uint32_t  ulRes = FlashProgram((uint32_t  *)(&g_UserSpace), (void*)g_UserSpaceAddr, sizeof(USER_SPACE));
	CBRM_NA_WARN((ulRes == 0), "SetUSFwUpdateStatus: failed to program new user space into flash");

	SysCtlDelay(ROM_SysCtlClockGet() / 100);

	// print buffer
	UARTprintfBinaryData(ulPtr, sizeof(g_UserSpace), 20);

Error:
	return r;
}


RESULT CommitUserSpace() {
	uint32_t  ulRes = 0;
	void *ulPtr = (void*)(g_UserSpaceAddr);

	EraseUserSpace();

	SysCtlDelay(ROM_SysCtlClockGet() / 100);

	DEBUG_LINEOUT("Commiting Userspace: sizeof(USER_SPACE):%d", sizeof(USER_SPACE));

	ulRes = ROM_FlashProgram((uint32_t  *)(&g_UserSpace), (void*)g_UserSpaceAddr, sizeof(USER_SPACE));
	CBRM_NA_WARN((ulRes == 0), "InitUserSpace: failed to program new user space into flash");

	SysCtlDelay(ROM_SysCtlClockGet() / 100);

	// print buffer
	UARTprintfBinaryData(ulPtr, sizeof(g_UserSpace), 20);

	return R_OK;
}

RESULT InitUserSpace()
{
	 int32_t  i = 0;
	RESULT r = R_OK;
	uint8_t tempSerial[16];

	DEBUG_LINEOUT_NA("+InitUserSpace()");
	//DEBUG_LINEOUT("sizeof(USER_SPACE):%d", sizeof(USER_SPACE));

	SysCtlDelay(ROM_SysCtlClockGet() / 10);

	void *ulPtr = (void*)(g_UserSpaceAddr);

	USER_SPACE *pUserSpace = (USER_SPACE*)(ulPtr);
	if(pUserSpace->GtarDeviceID == GTAR_DEVICE_ID && pUserSpace->GtarDeviceIDEx == GTAR_DEVICE_ID_EX)
	{
		switch(pUserSpace->fw_update_status)
		{
			case FW_UPDATE_PENDING:
			{
				// First erase the userspace so we don't have to do it in the loader
				//EraseUserSpace();
				EraseUserSpacePreserveSerialNumber();
				DEBUG_LINEOUT("InitUserSpace: Pending FW Update: 0x%x", (uint32_t )(g_vdFnPtr));
				SysCtlDelay(ROM_SysCtlClockGet() / 10);
				g_vdFnPtr();
			} break;
		}

		// Found the user space
		DEBUG_LINEOUT("InitUserSpace: Found valid user space, GtarDeviceID:0x%x GtarDeviceIDEx:0x%x fw_stat:0x%x",
				pUserSpace->GtarDeviceID, pUserSpace->GtarDeviceIDEx, pUserSpace->fw_update_status);
		memcpy(&g_UserSpace, pUserSpace, sizeof(USER_SPACE));

		SysCtlDelay(ROM_SysCtlClockGet() / 100);
	}
	else
	{
		// No user space clear the space
		//EraseUserSpace();
		EraseUserSpacePreserveSerialNumber();

		// Save before wiping user space
		memcpy(tempSerial, g_UserSpace.serial, sizeof(tempSerial));

		// 0xFF all not allowed
		i = 0;
		while(tempSerial[i] == 0xFF)
		{
			i++;
			if(i == 16)
			{
				DEBUG_LINEOUT_NA("Overwriting 0xFF all serial with 0");
				memset(tempSerial, 0, sizeof(tempSerial));
				break;
			}
		}

		// Set up a clean user space object
		memset(&g_UserSpace, 0, sizeof(USER_SPACE));

		// Copy serial number back in
		memcpy(g_UserSpace.serial, tempSerial, sizeof(tempSerial));

		g_UserSpace.GtarDeviceID = (uint8_t)GTAR_DEVICE_ID;
		g_UserSpace.GtarDeviceIDEx = (uint16_t)GTAR_DEVICE_ID_EX;
		g_UserSpace.fw_major_version = (uint8_t)FW_MAJOR_VERSION;
		g_UserSpace.fw_minor_version = (uint8_t)FW_MINOR_VERSION;
		g_UserSpace.fw_minor_minor_version = (uint8_t)FW_MINOR_MINOR_VERSION;

		// Assign Default values
		uint8_t DefaultCrossTalkMatrix[6][6] = {
			{ 0,  55,  40,  70,  70,  70},
			{55,   0,  55,  70,  70,  70},
			{35,  55,   0,  70,  70,  70},
			{35,  35,  55,   0,  60,  60},
			{30,  30,  30,  55,   0,  65},
			{30,  30,  30,  55,  65,   0}
		};

		memcpy(g_UserSpace.CrossTalkMatrix, DefaultCrossTalkMatrix, sizeof(DefaultCrossTalkMatrix));

		uint8_t DefaultSensitivity[6] = {11,
											   11,
											   11,
											   11,
											   11,
											   11
											  };

		memcpy(g_UserSpace.Sensitivity, DefaultSensitivity, sizeof(DefaultSensitivity));

		g_UserSpace.Window = 40;
		g_UserSpace.WindowIncrement = 5;

		g_UserSpace.DownCount = 10;
		g_UserSpace.UpCount = 2;
		g_UserSpace.UpSlope = 3;
		g_UserSpace.DownSlope = -1;
		g_UserSpace.MaxValue = 135;	// 2.7V

		CommitUserSpace();
	}

	// print the buffer quick
	UARTprintfBinaryData(ulPtr, sizeof(g_UserSpace), 20);

	DEBUG_LINEOUT_NA("-InitUserSpace()");

Error:
	return r;
}
