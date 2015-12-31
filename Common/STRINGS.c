#include "STRINGS.h"

#ifdef STRING_DEBUG
	const int8_t *g_MIDIputErrorMSG = "Failed to put EP3 data in FIFO";
	const int8_t *g_MIDIsendErrorMSG = "SendUSBMidiNoteMsg: Failed to send EP3 data";


	const int8_t *g_SendUSBMidiNoteMsg_errmsg 			= "ExecuteQueuedMidiEvent: Failed to send usb midi note msg";
	const int8_t *g_SendUSBMidiFret_errmsg 				= "ExecuteQueuedMidiEvent: Failed to send usb fret midi msg";
	const int8_t *g_SendFirmwareVersion_errmsg 			= "ExecuteQueuedMidiEvent: SendFirmwareVersion failed";
	const int8_t *g_SendFirmwareDownloadAck_errmsg 		= "ExecuteQueuedMidiEvent: Failed to FW downlaod ACK";
	const int8_t *g_SendPiezoFirmwareDownloadAck_errmsg 	= "ExecuteQueuedMidiEvent: Failed to piezo FW downlaod ACK";
	const int8_t *g_SendBatteryStatusAck_errmsg 			= "ExecuteQueuedMidiEvent: Failed to send battery status ACK";
	const int8_t *g_SendBatteryint8_tgePercentageAck_errmsg = "ExecuteQueuedMidiEvent: Failed to send battery int8_tge percentage ACK";
	const int8_t *g_SendRequestSerialNumberAck_errmsg 	= "ExecuteQueueMidiEvent: Failed to send RequestSerialNumberAck";
	const int8_t *g_SendGetPiezoCrossTalkMatrixAck_errmsg = "ExecuteQueueMidiEvent: Failed to send GetPiezoCrossTalkMatrixAck";
	const int8_t *g_SendGetPiezoSensitivityAck_errmsg 	= "ExecuteQueueMidiEvent: Failed to send GetPiezoSensitivityAck";
	const int8_t *g_SendGetPiezoWindowAck_errmsg 			= "ExecuteQueueMidiEvent: Failed to send GetPiezoWindowAck";
	const int8_t *g_SendCalibratePiezoStringAck_errmsg 	= "ExecuteQueueMidiEvent: Failed to send CalibratePiezoStringAck";
	const int8_t *g_SendPiezoCmdAck_errmsg 				= "ExecuteQueueMidiEvent: Failed to send PiezoCmdAck";
	const int8_t *g_SendAck_errmsg 						= "ExecuteQueueMidiEvent: Failed to send PiezoCmdResponse";
	const int8_t *g_SendCommitUserspaceAck_errmsg 		= "ExecuteQueueMidiEvent: Failed to send CommitUserspaceaAck";
	const int8_t *g_SendResetUserspaceAck_errmsg 			= "ExecuteQueueMidiEvent: Failed to send ResetUserspaceaAck";
#else
	const int8_t *g_MIDIputErrorMSG = "";
	const int8_t *g_MIDIsendErrorMSG = "";

	const int8_t *g_SendUSBMidiNoteMsg_errmsg 			= "";
	const int8_t *g_SendUSBMidiFret_errmsg 				= "";
	const int8_t *g_SendFirmwareVersion_errmsg 			= "";
	const int8_t *g_SendFirmwareDownloadAck_errmsg 		= "";
	const int8_t *g_SendPiezoFirmwareDownloadAck_errmsg 	= "";
	const int8_t *g_SendBatteryStatusAck_errmsg 			= "";
	const int8_t *g_SendBatteryChargePercentageAck_errmsg = "";
	const int8_t *g_SendRequestSerialNumberAck_errmsg 	= "";
	const int8_t *g_SendGetPiezoCrossTalkMatrixAck_errmsg = "";
	const int8_t *g_SendGetPiezoSensitivityAck_errmsg 	= "";
	const int8_t *g_SendGetPiezoWindowAck_errmsg 			= "";
	const int8_t *g_SendCalibratePiezoStringAck_errmsg 	= "";
	const int8_t *g_SendPiezoCmdAck_errmsg 				= "";
	const int8_t *g_SendAck_errmsg 						= "";
	const int8_t *g_SendCommitUserspaceAck_errmsg 		= "";
	const int8_t *g_SendResetUserspaceAck_errmsg 			= "";
#endif


