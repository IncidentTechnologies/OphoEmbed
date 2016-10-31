#ifndef CRC_H_
#define CRC_H_

#include "../Common/EHM.h"
#include "../Device/Device.h"

// CRC
// opho/Device/CRC.h
// Functions to reading and checking CRC

extern uint32_t g_CRCheader[4];

#define CRC_BYTE_0 0xFF01FF02
#define CRC_BYTE_1 0xFF03FF04

RESULT InitializeCRC();
RESULT PrintCRC();
//RESULT ValidateCRC();

#endif // ! CRC_H_
