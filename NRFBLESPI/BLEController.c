#include "BLEController.h"

bool m_fBLEConnected = false;

inline RESULT SetBLEConnect(uint8_t fStatus) {
	m_fBLEConnected = fStatus;
	return R_OK;
}

inline bool IsBLEConnected() {
#ifdef SPOOF_BLE
	return SPOOF_BLE;
#else
	return m_fBLEConnected;
#endif
}
