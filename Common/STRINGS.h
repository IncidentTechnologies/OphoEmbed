#ifndef STRING_DEFINES_H_
#define STRING_DEFINES_H_

// ****************************************************************************
// *                           * STRINGS for Embedded *                        *
// ****************************************************************************
/*  STRINGS is a set of strings that will align with the RESULT messages for many
 *  Opho Device calls
// ****************************************************************************/

#include "EHM.h"
#include <stdint.h>

//#define STRING_DEBUG

extern const int8_t *g_MIDIputErrorMSG;
extern const int8_t *g_MIDIsendErrorMSG;

extern const int8_t *g_SendUSBMidiNoteMsg_errmsg;
extern const int8_t *g_SendUSBMidiFret_errmsg;
extern const int8_t *g_SendFirmwareVersion_errmsg;
extern const int8_t *g_SendFirmwareDownloadAck_errmsg;
extern const int8_t *g_SendPiezoFirmwareDownloadAck_errmsg;
extern const int8_t *g_SendBatteryStatusAck_errmsg;
extern const int8_t *g_SendBatteryChargePercentageAck_errmsg;
extern const int8_t *g_SendRequestSerialNumberAck_errmsg;
extern const int8_t *g_SendGetPiezoCrossTalkMatrixAck_errmsg;
extern const int8_t *g_SendGetPiezoSensitivityAck_errmsg;
extern const int8_t *g_SendGetPiezoWindowAck_errmsg;
extern const int8_t *g_SendCalibratePiezoStringAck_errmsg;
extern const int8_t *g_SendPiezoCmdAck_errmsg;
extern const int8_t *g_SendAck_errmsg;
extern const int8_t *g_SendCommitUserspaceAck_errmsg;
extern const int8_t *g_SendResetUserspaceAck_errmsg;

#endif /* STRING_DEFINES_H_ */
