#include "Device.h"

DEVICE g_device;

// Flash location
#define MAX_FIRMWARE_UPGRADE_SIZE SIZE_OF_UPDATE_SPACE_KB
uint32_t  g_FirmwareDownloadAddress = FIRMWARE_DOWNLOAD_ADDRESS;
uint32_t  g_DownloadedFirmwareBytes = 0;
uint32_t  g_DownloadedFirmwarePages = 0;
uint32_t  g_DownloadedFirmwareIsPiezo = 0;
uint32_t  g_DownloadedPiezoFirmware_size = 0;

RESULT PrintFirmwareDownloadedBytes() {
	void *fwPtr = (void*)(g_FirmwareDownloadAddress);
	DEBUG_LINEOUT("PrintFirmwareDownloadedBytes: %d bytes downloaded at address 0x%x", g_DownloadedFirmwareBytes, g_FirmwareDownloadAddress);
	UARTprintfBinaryData(fwPtr, g_DownloadedFirmwareBytes, 20);

	return R_OK;
}

RESULT EraseFirmwareUpdateEraseFlashArea() {
	RESULT r = R_OK;
	uint32_t  ulRes;
	 int32_t  i = 0;

	//DEBUG_LINEOUT_NA("EraseFirmwareUpdateEraseFlashArea: Starting erase");

	for(i = 0; i < MAX_FIRMWARE_UPGRADE_SIZE; i++) {
		uint32_t  tempAddr = (uint32_t )(g_FirmwareDownloadAddress + (0x400 * i));
		ulRes = FlashErase(tempAddr);
		CBRM((ulRes == 0), "EraseFirmwareUpdateFlashArea: Erase of flash address 0x%x failed", tempAddr);
	}

	DEBUG_LINEOUT_NA("EraseFirmwareUpdateEraseFlashArea: Finished erase with no error");

Error:
	return r;
}

#define FW_COPY_FUNC_ADDR 0x1B800

RESULT InitDevice(DEVICE device) {
	RESULT r = R_OK;

	memcpy(&g_device, &device, sizeof(DEVICE));
	g_device.m_fInitialized = true;

	memset(&(g_device.m_services), 0, sizeof(g_device.m_services));

	// Turn off velocity sensing
	g_device.m_fVelocityEnabled = 0;

Error:
	return r;
}

bool IsDeviceInitialized() {
	return (bool)(g_device.m_fInitialized);
}

uint8_t GetDeviceID() {
	if(g_device.cbDeviceID != NULL)
		return g_device.cbDeviceID();
	else
		return 0;
}

DEVICE_FIRMWARE_VERSION GetDeviceFirmwareVersion() {
	DEVICE_FIRMWARE_VERSION version;

	memset(&version, 0, sizeof(version));

	if(g_device.cbFirmwareVersion != NULL) {
		uint8_t tempVer = g_device.cbFirmwareVersion();
		version.major = (tempVer >> 4) & 0x0F;
		version.minor = (tempVer >> 0) & 0x0F;
	}

	return version;
}

bool IsDeviceCharging() {
	if(g_device.cbIsCharging != NULL)
		return g_device.cbIsCharging();
	else
		return false;
}

uint8_t GetDeviceBatteryPercentage() {
	if(g_device.cbBatteryPercentage != NULL)
		return g_device.cbBatteryPercentage();
	else
		return 0;
}

uint8_t GetDeviceSerialNumber(uint8_t byteNum) {
	if(g_device.cbSerialNumber != NULL)
		return g_device.cbSerialNumber((int)byteNum);
	else
		return 0;
}

void *GetDeviceUserspaceAddress() {
	if(g_device.cbGetUserspaceAddress != NULL)
		return g_device.cbGetUserspaceAddress();
	else
		return NULL;
}

uint8_t *GetDeviceUserspaceSerialAddress() {
	if(g_device.cbGetUserspaceSerialAddress != NULL)
		return g_device.cbGetUserspaceSerialAddress();
	else
		return NULL;
}

uint8_t GetDeviceUserspaceAddressLength() {
	if(g_device.cbGetUserspaceSerialLength != NULL)
		return g_device.cbGetUserspaceSerialLength();
	else
		return NULL;
}

// USB Layer Access
uint8_t GetDeviceUSBStatus() {
	return 	g_USBStatus;
}


