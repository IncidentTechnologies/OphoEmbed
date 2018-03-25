#include "CRC.h"
#include "memorymap.h"

#ifndef _VS_PROJ
#include "driverlib/sw_crc.h"
#endif

uint32_t m_FWLength = 0;
uint32_t m_CRC32Value = 0;

RESULT InitializeCRC() {
	RESULT r = R_PASS;

	CBM((g_CRCheader[0] == CRC_BYTE_0), "CRC byte 0 mismatch");
	CBM((g_CRCheader[1] == CRC_BYTE_1), "CRC byte 0 mismatch");

	m_FWLength = g_CRCheader[2];
	m_CRC32Value = g_CRCheader[3];

	//CRM(ValidateCRC(), "Failed to validate CRC value");

	DEBUG_LINEOUT("CRC initialized");

Error:
	return r;
}

RESULT PrintCRC() {
	RESULT r = R_PASS;
	int i = 0;

	DEBUG_LINEOUT("CRC:");

	for(i = 0; i < 4; i++) {
		DEBUG_LINEOUT("%d: 0x%x", i, g_CRCheader[i]);
	}

Error:
	return r;
}

/*
RESULT ValidateCRC() {
	RESULT r = R_PASS;
	int i = 0;

	for(i = 0; i < 4; i++) {
		g_CRCheader[i] = 0xFFFFFFFF;
	}

	DEBUG_LINEOUT("0x%x", g_CRCheader[2]);

	uint32_t flashLength = (uint32_t)(APP_LENGTH);
	uint32_t crcValue = Crc32(0xFFFFFFFF, (uint8_t *)(APP_BASE), flashLength);

	DEBUG_LINEOUT("Calculated CRC: 0x%x", crcValue);

	CBM((flashLength == g_CRCheader[2]), "Mismatch of length in CRC header 0x%x, expected 0x%x", g_CRCheader[2], flashLength);
	CBM((crcValue == g_CRCheader[3]), "Mismatch of CRC value in CRC header 0x%x, expected 0x%x", g_CRCheader[3], crcValue);

	DEBUG_LINEOUT("CRC Valid");

Error:
	return r;
}
*/
