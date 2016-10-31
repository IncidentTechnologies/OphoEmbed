#include "MIDIController.h"

#include <stdbool.h>
#include <stdarg.h>
#include <math.h>

uint8_t m_pSysExBuffer[MAX_SYS_EX_SIZE];
uint32_t m_pSysExBuffer_n = 0;
bool m_fSysEx = false;

uint8_t m_CCSetLEDFret = 0;
uint8_t m_CCSetLEDString = 0;
RGBM m_CCSetLEDColor = {.r = 0, .g = 0, .b = 0, .m = 0};
bool m_fCCSetLED = false;

RESULT InitializeMIDIController() {
	RESULT r = R_OK;

	memset(&m_pSysExBuffer, 0, sizeof(m_pSysExBuffer));
	m_pSysExBuffer_n = 0;

	DEBUG_LINEOUT_NA("Device MIDI Controller Initialized");

Error:
	return r;
}

cbHandleCustomDeviceSysEx g_HandleCustomDeviceSysExCallback = NULL;
RESULT RegisterHandleCustomDeviceSysExCallback(cbHandleCustomDeviceSysEx HandleCustomDeviceSysExCB) {
	RESULT r = R_OK;

	CBRM_NA((g_HandleCustomDeviceSysExCallback == NULL), "RegisterHandleCustomDeviceSysExCallback: Handle Custom Device SysEx Callback already registered");
	g_HandleCustomDeviceSysExCallback = HandleCustomDeviceSysExCB;

Error:
	return r;
}

RESULT UnregisterHandleCustomDeviceSysExCallback() {
	RESULT r = R_OK;

	CBRM_NA((g_HandleCustomDeviceSysExCallback != NULL), "UnregisterHandleCustomDeviceSysExCallback: Handle Custom Device SysEx Callback not registered");
	g_HandleCustomDeviceSysExCallback = NULL;

Error:
	return r;
}

cbHandleDebugSysEx g_HandleDebugSysExCallback = NULL;
RESULT RegisterHandleDebugSysExCallback(cbHandleDebugSysEx HandleDebugSysExCB) {
	RESULT r = R_OK;

	CBRM_NA((g_HandleDebugSysExCallback == NULL), "RegisterHandleDebugSysExCallback: Handle Debug SysEx Callback already registered");
	g_HandleDebugSysExCallback = HandleDebugSysExCB;

Error:
	return r;
}

RESULT UnregisterHandleDebugSysExCallback() {
	RESULT r = R_OK;

	CBRM_NA((g_HandleDebugSysExCallback != NULL), "UnregisterHandleDebugSysExCallback: Handle Debug SysEx Callback not registered");
	g_HandleDebugSysExCallback = NULL;

Error:
	return r;
}

cbHandleLEDStateCC g_HandleLEDStateCCCallback = NULL;
RESULT RegisterHandleLEDStateCCCallback(cbHandleLEDStateCC HandleLEDStateCCCB) {
	RESULT r = R_OK;

	CBRM_NA((g_HandleLEDStateCCCallback == NULL), "RegisterHandleLEDStateCCCallback: Handle LED State CC Callback already registered");
	g_HandleLEDStateCCCallback = HandleLEDStateCCCB;

Error:
	return r;
}

RESULT UnregisterHandleLEDStateCCCallback() {
	RESULT r = R_OK;

	CBRM_NA((g_HandleLEDStateCCCallback != NULL), "UnregisterHandleLEDStateCCCallback: Handle LED State CC Callback not registered");
	g_HandleLEDStateCCCallback = NULL;

Error:
	return r;
}

bool IsMIDIConnected() {
	return (IsBLEConnected() || IsUSBConnected());
}

// Device MIDI functions
// Application functions should be declared in the given application MIDI Controller
RESULT SendMidiNoteMsg(uint8_t midiVal, uint8_t midiVelocity, uint8_t channel, uint8_t fOnOff) {
	if(IsBLEConnected())
		return SendSPIMidiNoteMsg(midiVal, midiVelocity, channel, fOnOff);
	else if(IsUSBConnected())
		return SendUSBMidiNoteMsg(midiVal, midiVelocity, channel, fOnOff);

	return R_NOT_CONNECTED;
}

RESULT SendMidiCC(uint8_t index, uint8_t value) {
	if(IsBLEConnected())
		return SendSPIMidiCC(index, value);
	else if(IsUSBConnected())
		return SendUSBMidiCC(index, value);

	return R_NOT_CONNECTED;
}

RESULT SendFirmwareVersion() {
	if(IsBLEConnected())
		return SendSPIFirmwareVersion();
	else if(IsUSBConnected())
		return SendUSBFirmwareVersion();

	return R_NOT_CONNECTED;
}

RESULT SendFirmwareDownloadAck(uint8_t status) {
	if(IsBLEConnected())
		return SendSPIFirmwareDownloadAck(status);
	else if(IsUSBConnected())
		return SendUSBFirmwareDownloadAck(status);

	return R_NOT_CONNECTED;
}

RESULT SendBatteryStatusAck() {
	if(IsBLEConnected())
		return SendSPIBatteryStatusAck();
	else if(IsUSBConnected())
		return SendUSBBatteryStatusAck();

	return R_NOT_CONNECTED;
}

RESULT SendBatteryChargePercentageAck() {
	if(IsBLEConnected())
		return SendSPIBatteryChargePercentageAck();
	else if(IsUSBConnected())
		return SendUSBBatteryChargePercentageAck();

	return R_NOT_CONNECTED;
}

RESULT SendRequestSerialNumberAck(uint8_t byteNumber) {
	if(IsBLEConnected())
		return SendSPIRequestSerialNumberAck(byteNumber);
	else if(IsUSBConnected())
		return SendUSBRequestSerialNumberAck(byteNumber);

	return R_NOT_CONNECTED;
}

RESULT SendAck(uint8_t SendBuffer[4]) {
	if(IsBLEConnected())
		return SendSPIAck(SendBuffer);
	else if(IsUSBConnected())
		return SendUSBAck(SendBuffer);

	return R_NOT_CONNECTED;
}

RESULT SendCommitUserspaceAck(uint8_t status) {
	if(IsBLEConnected())
		return SendSPICommitUserspaceAck(status);
	else if(IsUSBConnected())
		return SendUSBCommitUserspaceAck(status);

	return R_NOT_CONNECTED;
}

RESULT SendResetUserspaceAck(uint8_t status) {
	if(IsBLEConnected())
		return SendSPICommitUserspaceAck(status);
	else if(IsUSBConnected())
		return SendUSBCommitUserspaceAck(status);

	return R_NOT_CONNECTED;
}

//uint8_t m_pSysExBuffer[MAX_SYS_EX_SIZE];
//uint32_t m_pSysExBuffer_n = 0;
//bool m_fSysEx = false;

RESULT ResetSysEx() {
	m_pSysExBuffer_n = 0;
	m_fSysEx = false;

	return R_OK;
}

RESULT SysExError() {
	ResetSysEx();
	return R_FAIL;
}

RESULT ResetCCSetLED() {
	m_CCSetLEDFret = 0;
	m_CCSetLEDString = 0;
	m_CCSetLEDColor = UintToRGBM(0x00);
	m_fCCSetLED = false;

	return R_OK;
}

RESULT CCSetLEDError() {
	ResetCCSetLED();
	return R_FAIL;
}

RESULT HandleMIDISysExPacket(MIDI_MSG midiPacket) {
	RESULT r = R_OK;
	int i = 0;

	uint8_t *pBuffer = &midiPacket;

	for(i = 0; i < 3; i++) {
		uint8_t dataByte = pBuffer[i];

		if(dataByte == MIDI_SYS_EX_END) {
			// end of sys ex
			m_fSysEx = false;
			return HandleMIDISysExBuffer();
		}
		else {
			if(dataByte == MIDI_SYS_EX) {
				//CRM_NA(SysExError(), "HandleMIDISysExPacket: got 0xF0 inside a sysex message");
				//DEBUG_LINEOUT("Warning: 0xF0 inside of SysEx message");
				// TODO: Fix the iOS app to use the new MRGB format!!
			}
			m_pSysExBuffer[m_pSysExBuffer_n++] = dataByte;
		}
	}

Error:
	return r;
}

RESULT HandleMIDIPacket(MIDI_MSG midiPacket) {
	RESULT r = R_OK;

	//DEBUG_LINEOUT("SPI H: %02x %02x %02x sysex:%d", midiPacket.type, midiPacket.data1, midiPacket.data2, m_fSysEx);

	if(m_fSysEx) {
		return HandleMIDISysExPacket(midiPacket);
	}

	switch(midiPacket.type.type) {
		case MIDI_NIBBLE_NOTE_ON: {
			DEBUG_LINEOUT("Note on m:0x%x v:0x%x chn: %d!", midiPacket.data1, midiPacket.data2, midiPacket.type.channel);
			// TODO: actually do stuff
		} break;

		case MIDI_NIBBLE_NOTE_OFF: {
			DEBUG_LINEOUT("Note off m:0x%x v:0x%x chn: %d!", midiPacket.data1, midiPacket.data2, midiPacket.type.channel);
			// TODO: actually do stuff
		} break;

		case MIDI_NIBBLE_CONTROL_CHANGE: {
			DEBUG_LINEOUT_NA("CC!");

			if(midiPacket.data1 == GTAR_MIDI_CC_SET_LED_0) {
				m_fCCSetLED = true;
				m_CCSetLEDFret = midiPacket.data2;
				m_CCSetLEDString = midiPacket.type.channel;
			}
			else if(midiPacket.data1 == GTAR_MIDI_CC_SET_LED_1) {
				if(m_fCCSetLED == false) {
					CRM_NA(CCSetLEDError(), "Rx second set CC LED message");
				}

				m_CCSetLEDColor = UintToRGBM(midiPacket.data2);

				// Dispatch the LED message here
				//r = SendStringFretLEDStateRGBM(m_CCSetLEDString, m_CCSetLEDFret, RGBMToUint(m_CCSetLEDColor));
				if(g_HandleLEDStateCCCallback != NULL) {
					r = g_HandleLEDStateCCCallback(m_CCSetLEDString, m_CCSetLEDFret, RGBMToUint(m_CCSetLEDColor));
					ResetCCSetLED();
					CRM_NA(r, "Failed to set CC Set LED msg");
				}
				else {
					DEBUG_LINEOUT("No LED RGBM Handler present");
				}

			}
		} break;

		case MIDI_NIBBLE_SYS_EX: {
			m_fSysEx = true;
			m_pSysExBuffer[m_pSysExBuffer_n++] = midiPacket.data1;
			m_pSysExBuffer[m_pSysExBuffer_n++] = midiPacket.data2;
		} break;

		default: {
			DEBUG_LINEOUT("HandleMIDIPacket: Unhandled MIDI type 0x%x", midiPacket.type);
		} break;
	}

Error:
	return r;
}

RESULT UnwrapBuffer7F(uint8_t *pBuffer7F, int pBuffer7F_n, uint8_t **n_pBuffer, int *pn_pBuffer_n) {
	RESULT r = R_OK;

	signed char k = 1;
	int32_t serial_nc = 0, i = 0;

	// Count the bits of septets and cut off any remainder
	int count = (pBuffer7F_n * 7) / 8;

	*pn_pBuffer_n = count;
	*n_pBuffer = (uint8_t *)malloc(sizeof(uint8_t) * count);

	memset((*n_pBuffer), 0, sizeof(sizeof(uint8_t) * count));
	CNRM_NA((*n_pBuffer), "Failed to allocate memory for the unwrapped buffer");

	for(i = 0; i < pBuffer7F_n; i++) {

		DEBUG_LINEOUT("0x%x", pBuffer7F[i]);

		if(i != pBuffer7F_n - 1)
			(*n_pBuffer)[serial_nc] = (pBuffer7F[i] << k) + (pBuffer7F[i + 1] >> (7 - k));
		else
			(*n_pBuffer)[serial_nc] += (pBuffer7F[i] << k);

		k++;
		serial_nc++;

		if(k == 8) {
			k = 1;
			i++;

			DEBUG_LINEOUT("0x%x", pBuffer7F[i]);

			if(i < pBuffer7F_n) {
				(*n_pBuffer)[serial_nc] = pBuffer7F[i - 1] & 0x01;
			}

			//printf("yo %d l:%d!\n", i, pBuffer7F_n);

		}
	}

	/*
	DEBUG_LINEOUT("*** unwrapped buffer: %d bytes ***", *pn_pBuffer_n);
	UARTprintfBinaryData(*n_pBuffer, *pn_pBuffer_n, 20);
	//*/

Error:
	return r;
}

RESULT WrapBuffer7F(uint8_t *pBuffer, int pBuffer_n, uint8_t **n_pBuffer7F, int *pn_pBuffer7F_n) {
	RESULT r = R_OK;

	int i = 0, j = 0, c = 0;

	// For every byte over 8 bytes we need another 7F byte
	int count = pBuffer_n + ceil(((float)(pBuffer_n))/((float)(7.0f)));
	*pn_pBuffer7F_n = count;
	(*n_pBuffer7F) = (uint8_t *)malloc(sizeof(uint8_t) * count);

	memset((*n_pBuffer7F), 0x00, sizeof(sizeof(uint8_t) * count));
	CNRM_NA((*n_pBuffer7F), "Failed to allocate memory for the wrapped buffer");

	uint8_t left, right;
	j = 1;
	c = 0;

	for(i = 0; i < pBuffer_n; i++) {
		left = (pBuffer[i] >> j) & 0x7F;
		right = ((pBuffer[i]) & (0xFF >> (8 - j))) & 0x7F;

		j++;

		(*n_pBuffer7F)[c] += left;
		(*n_pBuffer7F)[c + 1] = right << (8 - j);

		if(j == 8) {
			c++;

			(*n_pBuffer7F)[c] = pBuffer[i] & 0x7F;
			(*n_pBuffer7F)[c + 1] = right << (7);

			j = 1;
		}

		c++;
	}

Error:
	return r;
}

long int Convert7FToInt(int num, ...) {
	long int retVal = 0x00;
	int i = 0;

	va_list valist;
	va_start(valist, num);

	for(i = 0; i < num; i++) {
		unsigned char temp = va_arg(valist, unsigned char);
		retVal += (temp & 0x7F) << (7 * i);
	}

	va_end(valist);

	return retVal;
}

RESULT HandleMIDISysExBuffer() {
	RESULT r = R_OK;
	int i = 0;

	// Handle MIDI Sys Ex!

	///*
	DEBUG_LINEOUT("*** rx sysex buffer: %d bytes ***", m_pSysExBuffer_n);
	UARTprintfBinaryData(m_pSysExBuffer, m_pSysExBuffer_n, 20);
	//*/

	DEVICE_MSG *pDeviceMsg = (DEVICE_MSG *)m_pSysExBuffer;

	CBRM((pDeviceMsg->header.deviceID == GetDeviceID()), "MIDI Sys Ex Buffer with incorrcect DEVICE ID: 0x%x", pDeviceMsg->header.deviceID);

	switch(pDeviceMsg->header.msgType) {

		case DEVICE_MSG_ENABLE_DEBUG: {
			// This needs to be a callback
			//CRM_NA(InitJTAGStrings(), "Failed to disable debug mode");
			if(g_HandleDebugSysExCallback != NULL) {
				CRM_NA(g_HandleDebugSysExCallback(TRUE), "Failed to enable debug mode");
			}
			else {
				DEBUG_LINEOUT("No enable debug callback registered");
			}
		} break;

		case DEVICE_MSG_DISABLE_DEBUG: {
			//CRM_NA(InitJTAG(), "Failed to enter debug mode");
			if(g_HandleDebugSysExCallback != NULL) {
				CRM_NA(g_HandleDebugSysExCallback(FALSE), "Failed to disable debug mode");
			}
			else {
				DEBUG_LINEOUT("No disable debug callback registered");
			}
		} break;

		// TODO: Generalize MIDI Queue Arch
		case DEVICE_MSG_REQ_FW_VERSION: {
			DEBUG_LINEOUT("Firmware Version Requested");

			DEVICE_MIDI_EVENT gme;
			gme.m_gmet = DEVICE_SEND_FW_VERSION;
			gme.m_params_n = 0;
			QueueNewMidiEvent(gme);
		} break;

		case DEVICE_MSG_REQ_BATTERY_STATUS: {
			// Queue up send battery status and send battery charge
			DEVICE_MIDI_EVENT gme_status;
			gme_status.m_gmet = DEVICE_SEND_BATTERY_STATUS;
			gme_status.m_params_n = 0;
			QueueNewMidiEvent(gme_status);

			DEVICE_MIDI_EVENT gme_charge;
			gme_charge.m_gmet = DEVICE_SEND_BATTERY_CHARGE;
			gme_charge.m_params_n = 0;
			QueueNewMidiEvent(gme_charge);
		} break;

		case DEVICE_MSG_ENABLE_VELOCITY: {
			g_device.m_fVelocityEnabled = 1;
		} break;

		case DEVICE_MSG_DISABLE_VELOCITY: {
			g_device.m_fVelocityEnabled = 1;
		} break;

		// TODO: Generalize Userspace
		case DEVICE_MSG_COMMIT_USERSPACE: {
			CommitUserSpace();

			// Pretty sure this was here for debugging,
			// PrintPiezoSettingsUserspace();

			DEVICE_MIDI_EVENT gme;
			gme.m_gmet = DEVICE_SEND_COMMIT_USERSPACE;
			gme.m_params_n = 1;
			gme.m_params[0] = 0;
			QueueNewMidiEvent(gme);
		} break;

		case DEVICE_MSG_RESET_USERSPACE: {
			EraseUserSpace();
			InitUserSpace();

			DEVICE_MIDI_EVENT gme;
			gme.m_gmet = DEVICE_SEND_RESET_USERSPACE;
			gme.m_params_n = 1;
			gme.m_params[0] = 0;
			QueueNewMidiEvent(gme);
		} break;

		/*
		case DEVICE_MSG_SET_ACCELEROMETER_STATE: {
			DEVICE_SET_STATE *pDeviceSetAccelState = pDeviceMsg;
			SetAccelerometerState(pDeviceSetAccelState);
		} break;
		*/

		case DEVICE_MSG_REQ_SERIAL_NUM: {
			DEVICE_REQUEST_BYTE_NUMBER *pDeviceRequestSerialNumber = pDeviceMsg;

			// Request serial number
			DEVICE_MIDI_EVENT gme_status;
			gme_status.m_gmet = DEVICE_SEND_SERIAL_NUMBER;
			gme_status.m_params_n = 1;
			gme_status.m_params[0] = pDeviceRequestSerialNumber->byteNumber;
			QueueNewMidiEvent(gme_status);
		} break;

		case DEVICE_MSG_EXEC_FW_UPDATE: {
			DEBUG_LINEOUT_NA("Execute FW update not currently implemented");
		} break;

		// TODO: Verify ALL wrap / unwrap functions!
		case DEVICE_MSG_SET_SERIAL_NUMBER: {
			DEVICE_SET_SERIAL_NUMBER *pDeviceSetSerialNumber = pDeviceMsg;
			//*/
			DEBUG_LINEOUT_NA("Set Serial Number");
			UARTprintfBinaryData(pDeviceSetSerialNumber, sizeof(DEVICE_SET_SERIAL_NUMBER), 20);
			//*/

			uint8_t *pSerialBuffer = NULL;
			int pSerialBuffer_n = -1;

			CRM_NA(UnwrapBuffer7F(pDeviceSetSerialNumber->serialNumber7F, 16, &pSerialBuffer, &pSerialBuffer_n), "Failed to unwrap 7F buffer for serial number");

			// Copy over serial number
			uint8_t *pUserspaceSerialAddress = GetDeviceUserspaceSerialAddress();
			int pUserspaceSerialAddress_n = GetDeviceUserspaceSerialLength();

			//memset(g_UserSpace.serial, 0, sizeof(g_UserSpace.serial));
			memset(pUserspaceSerialAddress, 0, pUserspaceSerialAddress_n);
			for(i = 0; i < pSerialBuffer_n; i++) {
				if(i + 2 < 16) {
					pUserspaceSerialAddress[i + 2] = pSerialBuffer[i];
				}
			}

			// Commit serial number to flash
			OutputSerialToDebug();
			CommitUserSpace();

			if(pSerialBuffer != NULL) {
				free(pSerialBuffer);
				pSerialBuffer = NULL;
			}
		} break;

		/*
		case GTAR_MSG_REQ_AUTH_REDOWNLOAD: {
#ifdef IPHONE_IAP
			// Erase and re-download cert
			EraseAuthCert();
			DownloadAuthCertificate();
#else
			DEBUG_LINEOUT_NA("Auth redownload not supported");
#endif
		} break;
		*/

		// TODO: Test the firmware update function
		// TODO: Add DFU as a back up still
		case DEVICE_MSG_DOWNLOAD_NEW_FW_PACKAGE: {
			DEVICE_DOWNLOAD_NEW_FW_PAGE_HEADER *pDeviceDownloadFWPageHeader = pDeviceMsg;

			// Convert the header to the page
			DEVICE_DOWNLOAD_NEW_FW_PAGE DeviceDownloadFWPage;
			memset(&DeviceDownloadFWPage, 0, sizeof(DEVICE_DOWNLOAD_NEW_FW_PAGE));

			DeviceDownloadFWPage.firmwarePageBytes = (int)Convert7FToInt(3, pDeviceDownloadFWPageHeader->size_lb_7f, pDeviceDownloadFWPageHeader->size_mb_7f, pDeviceDownloadFWPageHeader->size_ub_7f);

			DeviceDownloadFWPage.totalPageCount = pDeviceDownloadFWPageHeader->totalPageCount;
			DeviceDownloadFWPage.currentPage = pDeviceDownloadFWPageHeader->currentPage;
			DeviceDownloadFWPage.transmittedBytes = (int)Convert7FToInt(2, pDeviceDownloadFWPageHeader->transmitted_lb_7f, pDeviceDownloadFWPageHeader->transmitted_ub_7f);

			// The firmware buffer memory starts a byte after reserved 1
			DeviceDownloadFWPage.pFirmwareBuffer = (unsigned char *)(&(pDeviceDownloadFWPageHeader->reserved_1)) + 1;

			uint8_t *pChecksum = pDeviceMsg + sizeof(DEVICE_DOWNLOAD_NEW_FW_PAGE_HEADER) + DeviceDownloadFWPage.firmwarePageBytes;
			DeviceDownloadFWPage.checksum = *pChecksum;

			// With some luck we'll have a firmware package here

			// TODO: Flash the page
		} break;

		default: {
			//DEBUG_LINEOUT("Handling device specifc SysEx type 0x%x", pDeviceMsg->header.msgType);

			if(g_HandleCustomDeviceSysExCallback != NULL) {
				CRM(g_HandleCustomDeviceSysExCallback(pDeviceMsg), "Device handler failed");
			}
			else {
				DEBUG_LINEOUT("Unhandled SysEx type 0x%x", pDeviceMsg->header.msgType);
			}
		} break;
	}

Error:
	ResetSysEx();
	return r;
}





