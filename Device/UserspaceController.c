#include "UserspaceController.h"

// User Space flash location
// TODO: This should be a memory location, not a struct
// then access interface defined by a struct as provided by the application
// Device side Userspace can use the USER_SPACE template struct but the memory location
// should be a void* or something like that

void *g_pUserSpaceAddr = NULL;
USER_SPACE *g_pUserSpace = NULL;
int g_UserSpace_n = 0;		// User defined userspace size

uint32_t g_UserSpaceAddr = USER_SPACE_ADDRESS;			// This is set to the 122KB point in flash
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

// This will return R_OK if this is a debug unit or R_NO if not
// this is done by checking to see if the serial number is deadbeef
RESULT DebugUnit() {
	 int32_t  i = 0;
	 //int32_t  count = sizeof(g_UserSpace.serial) / sizeof(g_UserSpace.serial[0]);

	 //for(i = 0; i < count; i++)
	 for(i = 0; i < SERIAL_NUMBER_BYTES; i++)
		if(g_pUserSpace->serial[i] != 0)
			return R_FAIL;

	return R_OK;
}

RESULT IsSerialNumberZero() {
	 int32_t  i = 0;

	for(i = 0; i < 16; i++)
		if(g_pUserSpace->serial[i] != 0)
			return R_FALSE;

	return R_OK;
}

RESULT SetSerialNumber(int32_t  val) {
	 int32_t  i = 0;
	 //int32_t  count = sizeof(val);

	memset(g_pUserSpace->serial, 0, sizeof(g_pUserSpace->serial));

	//for(i = 0; i < count; i++)
	for(i = 0; i < SERIAL_NUMBER_BYTES; i++)
		g_pUserSpace->serial[15 - i] = (val >> (i * 8)) & 0xFF;

	// Commit serial number to flash
	OutputSerialToDebug();
	return CommitUserSpace();
}

//inline RESULT OutputSerialToDebug()
RESULT OutputSerialToDebug() {
	int32_t  i = 0;
	//int32_t  count = sizeof(g_pUserSpace->serial) / sizeof(g_pUserSpace->serial[0]);

	DEBUG_MSG_NA("Serial Number: 0x");
	//for(i = 0; i < count; i++)

	for(i = 0; i < SERIAL_NUMBER_BYTES; i++)
		DEBUG_MSG("%x", g_pUserSpace->serial[i]);

	DEBUG_MSG_NA("\n\r");

	return R_OK;
}

void *GetUserspaceSerialAddress() {
	return g_pUserSpace->serial;
}

uint8_t GetDeviceSerialNumber(uint8_t byteNum) {
	return (uint8_t)(g_pUserSpace->serial[byteNum]);
}

void *GetDeviceUserspaceAddress() {
	return (void*)(g_pUserSpace);
}

uint8_t *GetDeviceUserspaceSerialAddress() {
	return (g_pUserSpace->serial);
}

uint8_t GetDeviceUserspaceAddressLength() {
	return g_UserSpace_n;
}

uint8_t GetDeviceUserspaceSerialLength() {
	//return sizeof(g_UserSpace.serial);			// This will not likely work
	return SERIAL_NUMBER_BYTES;
}

RESULT ClearUserSpaceMemory() {
	RESULT r = R_OK;

	CNRM(g_pUserSpaceAddr, "ClearUserSpaceMemory: No userspace address set");
	CBRM(g_UserSpace_n, "ClearUserSpaceMemory: User space size cannot be zero");

	memset(g_pUserSpaceAddr, 0, g_UserSpace_n);

Error:
	return r;
}

RESULT EraseUserSpace() {
	RESULT r = R_OK;

	// Clear the UserSpace
	DEBUG_LINEOUT("+EraseUserSpace 0x%x", USER_SPACE_ADDRESS);

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
	int userspacesize = sizeof(USER_SPACE);

	void *ulPtr = (void*)(g_UserSpaceAddr);
	USER_SPACE *pUserSpace = (USER_SPACE*)(ulPtr);

	DEBUG_LINEOUT_NA("+EraseUserSpacePreserveSerialNumber");
	CNRM(g_pUserSpace, "EraseUserSpacePreserveSerialNumber: Device Userspace not allocated");

	// Save the serial
	memcpy(g_pUserSpace, pUserSpace, sizeof(USER_SPACE));	// Copy in user space from Flash
	memcpy(tempSerial, g_pUserSpace->serial, sizeof(tempSerial));

	// Clear UserSpace
	uint32_t  ulRes = ROM_FlashErase(USER_SPACE_ADDRESS);	// first kb
	CBRM((ulRes == 0), "EraseUserSpace: Failed to erase address 0x%x", USER_SPACE_ADDRESS);
	SysCtlDelay(ROM_SysCtlClockGet() / 100);

	// Read back in userspace into mem
	memcpy(g_pUserSpace, pUserSpace, sizeof(USER_SPACE));

	// Copy serial number back in
	memcpy(g_pUserSpace->serial, tempSerial, sizeof(tempSerial));

	// Commit back to userspace flash
	CRM_NA(CommitUserSpace(),"EraseUserSpacePreserveSerialNumber: Failed to Commit UserSpace");

Error:
	DEBUG_LINEOUT_NA("-EraseUserSpacePreserveSerialNumber");
	return r;
}

RESULT PrintUserspace() {
	//UARTprintfBinaryData(ulPtr, sizeof(g_UserSpace_n), 20);
	UARTprintfBinaryData(g_pUserSpaceAddr, g_UserSpace_n, 20);
	return R_OK;
}

RESULT AllocateUserSpace() {
	RESULT r = R_OK;

	CBRM((g_UserSpace_n != 0), "AllocateUserSpace: Userspace size cannot be zero");
	CBRM((g_pUserSpaceAddr == NULL), "AllocateUserSpace: Userspace already allocated, dellaoc first");

	g_pUserSpaceAddr = (void*)malloc(g_UserSpace_n);
	CNRM(g_pUserSpaceAddr, "AllocateUserSpace: Failed to allocate %d bytes for userspace", g_UserSpace_n);

	g_pUserSpace = (USER_SPACE*)(g_pUserSpaceAddr);

	return r;
Error:
	if(g_pUserSpaceAddr != NULL) {
		free(g_pUserSpaceAddr);
		g_pUserSpaceAddr = NULL;
	}

	g_pUserSpace = NULL;

	return r;
}

RESULT DeallocateUserSpace() {
	RESULT r = R_OK;

	CBRM((g_pUserSpaceAddr != NULL), "DeallocateUserSpace: Userspace not allocated");

	free(g_pUserSpaceAddr);
	g_pUserSpaceAddr = NULL;

	g_pUserSpace = NULL;

Error:
	return r;
}

RESULT LoadUserSpaceToMemory() {
	RESULT r = R_OK;

	void *ulPtr = (void*)(g_UserSpaceAddr);
	USER_SPACE *pUserSpace = (USER_SPACE*)(ulPtr);

	CBRM((g_pUserSpaceAddr != NULL), "LoadUserSpaceToMemory: Userspace not allocated");
	CNRM(pUserSpace, "LoadUserSpaceToMemory: UserSpace Flash pointer is null");

	memcpy(g_pUserSpaceAddr, pUserSpace, g_UserSpace_n);

Error:
	return r;
}

RESULT FlashProgramUserspace() {
	RESULT r = R_OK;

	CBRM((g_UserSpace_n != 0), "User defined user space size cannot be zero");
	CBRM((g_UserSpace_n % 4 == 0), "User defined user space size must be factor of 4");

	uint32_t  ulRes = ROM_FlashProgram((uint32_t  *)(g_pUserSpace), (void*)g_UserSpaceAddr, g_UserSpace_n);
	CBRM_NA_WARN((ulRes == 0), "SetUSFwUpdateStatus: failed to program new user space into flash");

	ROM_SysCtlDelay(ROM_SysCtlClockGet() / 100);

Error:
	return r;
}

RESULT SetUSFwUpdateStatus(FW_UPDATE_STATUS newStatus) {
	RESULT r = R_OK;
	void *ulPtr = (void*)(g_UserSpaceAddr);

	EraseUserSpace();

	DEBUG_LINEOUT("SetUSFwUpdateStatus: setting to new fw update status: 0x%x", (uint8_t)newStatus);
	g_pUserSpace->fw_update_status = (uint8_t)newStatus;
	g_pUserSpace->fw_downloaded_pages = g_DownloadedFirmwarePages;

	CRM(FlashProgramUserspace(), "SetUSFwUpdateState: Failed to flash userspace");

	// Print buffer
	CRM(PrintUserspace(), "Failed to print userspace");

Error:
	return r;
}

RESULT CommitUserSpace() {
	RESULT r = R_OK;
	uint32_t  ulRes = 0;
	void *ulPtr = (void*)(g_UserSpaceAddr);

	EraseUserSpace();

	SysCtlDelay(ROM_SysCtlClockGet() / 100);

	DEBUG_LINEOUT("Commiting Userspace: sizeof(USER_SPACE):%d", g_UserSpace_n);

	CRM(FlashProgramUserspace(), "CommitUserSpace: Failed to flash userspace");

	// Print buffer
	CRM(PrintUserspace(), "Failed to print userspace");

Error:
	return R_OK;
}

RESULT InitUserSpace(int UserSpaceSize, cbInitUserSpace fnInitUserSpace) {
	int32_t  i = 0;
	RESULT r = R_OK;
	uint8_t tempSerial[16];

	DEBUG_LINEOUT("+InitUserSpace(%d)", UserSpaceSize);

	void *ulPtr = (void*)(g_UserSpaceAddr);
	USER_SPACE *pUserSpace = (USER_SPACE*)(ulPtr);

	CBRM((UserSpaceSize != 0), "InitUserSpace: UserSpaceSize cannot be zero");
	CBRM((UserSpaceSize >= sizeof(USER_SPACE)), "InitUserSpace: UserSpaceSize %d is less than UserSpace size %d", UserSpaceSize, sizeof(USER_SPACE));
	CBRM((UserSpaceSize % 4 == 0), "InitUserSpace: User defined space must be factor of 4 in size");
	g_UserSpace_n = UserSpaceSize;

	// Allocate and load userspace
	CRM(AllocateUserSpace(), "InitUserSpace: Failed to allocate UserSpace");
	CRM(LoadUserSpaceToMemory(), "InitUserSpace: Failed to load user space to local memory");

	if(pUserSpace->DeviceID == GetDeviceID() && pUserSpace->DeviceIDEx == GetDeviceIDEx()) {
		switch(pUserSpace->fw_update_status) {
			case FW_UPDATE_PENDING: {
				// First erase the userspace so we don't have to do it in the loader
				//EraseUserSpace();
				EraseUserSpacePreserveSerialNumber();
				//DEBUG_LINEOUT("InitUserSpace: Pending FW Update: 0x%x", (uint32_t )(g_vdFnPtr));

				DEBUG_LINEOUT("InitUserSpace: Pending FW Update: 0x%x", (uint32_t )(g_HandleCommitFirmwareCallback));
				SysCtlDelay(ROM_SysCtlClockGet() / 10);

				//g_vdFnPtr();

				if(g_HandleCommitFirmwareCallback != NULL)
					g_HandleCommitFirmwareCallback();

			} break;
		}

		// Found the user space
		DEBUG_LINEOUT("InitUserSpace: Found valid user space, GtarDeviceID:0x%x GtarDeviceIDEx:0x%x fw_stat:0x%x",
				pUserSpace->DeviceID, pUserSpace->DeviceIDEx, pUserSpace->fw_update_status);

		SysCtlDelay(ROM_SysCtlClockGet() / 100);
	}
	else {
		// No user space clear the space
		EraseUserSpacePreserveSerialNumber();

		// Save before wiping user space
		memcpy(tempSerial, g_pUserSpace->serial, sizeof(tempSerial));

		// 0xFF all not allowed
		i = 0;
		while(tempSerial[i] == 0xFF) {
			i++;
			if(i == 16) {
				DEBUG_LINEOUT_NA("Overwriting 0xFF all serial with 0");
				memset(tempSerial, 0, sizeof(tempSerial));
				break;
			}
		}

		// Set up a clean user space object
		CRM(ClearUserSpaceMemory(), "Failed to clear userspace memory");

		// Copy serial number back in
		memcpy(g_pUserSpace->serial, tempSerial, sizeof(tempSerial));

		DEVICE_FIRMWARE_VERSION fwv = GetDeviceFirmwareVersion();

		g_pUserSpace->DeviceID = (uint8_t)GetDeviceID();
		g_pUserSpace->DeviceIDEx = (uint16_t)GetDeviceIDEx();
		g_pUserSpace->fw_major_version = (uint8_t)fwv.major;
		g_pUserSpace->fw_minor_version = (uint8_t)fwv.minor;
		//g_pUserSpace->fw_minor_minor_version = (uint8_t)FW_MINOR_MINOR_VERSION;
		// TODO: Remove this entirely
		g_pUserSpace->fw_minor_minor_version = (uint8_t)0;

		// Let the client have a chance to initialize it's own values
		if(fnInitUserSpace != NULL)
			CRM(fnInitUserSpace(g_pUserSpace), "InitUserSpace: User provided Userspace init function failed");

		CommitUserSpace();
	}

	// Print the Userspace
	CRM(PrintUserspace(), "Failed to print userspace");
	SysCtlDelay(ROM_SysCtlClockGet() / 10);

	DEBUG_LINEOUT_NA("-InitUserSpace()");

Error:
	return r;
}
