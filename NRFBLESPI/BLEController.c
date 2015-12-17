#include "BLEController.h"

bool m_fBLEConnected = false;

RESULT SetBLEConnect(uint8_t fStatus) {
	m_fBLEConnected = fStatus;
	return R_OK;
}

bool IsBLEConnected() {
#ifdef SPOOF_BLE
	return SPOOF_BLE;
#else
	return m_fBLEConnected;
#endif
}
