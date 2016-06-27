#include "Device.h"
#include "../MIDI/USB/usbmidi.h"

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

	DEBUG_LINEOUT("Initialized Opho Device");

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

uint16_t GetDeviceIDEx() {
	if(g_device.cbDeviceIDEx != NULL)
			return g_device.cbDeviceIDEx();
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
	else {
		version.major = 0;
		version.minor = 0;
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

// USB Layer Access
uint8_t GetDeviceUSBStatus() {
	return 	g_USBStatus;
}


