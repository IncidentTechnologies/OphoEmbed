#include "AMON.h"

AMON g_amon;

RESULT InitializeAMON() {
	RESULT r = R_OK;

	unsigned char fLastAmonState = g_amon.fStart;

	memset(&g_amon, 0, sizeof(AMON));

	g_amon.status = AMON_DEVICE_UNASSIGNED;
	g_amon.MasterState = AMON_MASTER_FALSE;
	g_amon.id = -1;

	g_amon.fStart = fLastAmonState;

Error:
	return r;
}

RESULT StartAMON() {
	g_amon.fStart = 1;
	return R_OK;
}

RESULT StopAMON() {
	g_amon.fStart = 0;
	return R_OK;
}
