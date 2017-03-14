#include "AMON.h"
#include "../OS/Console.h"
#include "../Common/EHM.h"

AMON g_amon = {
	.id = -1,
	.status = AMON_DEVICE_UNASSIGNED,
	.MasterState = AMON_MASTER_FALSE,
	.fStart = 0
};

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

	DEBUG_LINEOUT_NA("AMON Started");

	return R_OK;
}

RESULT StopAMON() {
	g_amon.fStart = 0;

	DEBUG_LINEOUT_NA("AMON Stopped");

	return R_OK;
}

unsigned char IsAMONMaster() {
	if(g_amon.MasterState != AMON_MASTER_FALSE)
		return true;
	else
		return false;
}

unsigned char AMONLinkRxBusy(AMON_LINK link) {
	return g_amon.links[link].fLinkRxBusy;
}

// TODO: EHM for lock fail?
RESULT LockAMONLinkRx(AMON_LINK link) {
	RESULT r = R_OK;

	//CBRM((g_amon.links[link].fLinkRxBusy == 0), "Can't lock link rx %d, already busy", link);
	//CBR((g_amon.links[link].fLinkRxBusy == 0));
	g_amon.links[link].fLinkRxBusy = 1;

Error:
	return r;
}
RESULT UnlockAMONLinkRx(AMON_LINK link) {
	RESULT r = R_OK;

	//CBRM((g_amon.links[link].fLinkRxBusy == 1), "Can't unlock link rx %d, not busy", link);
	//CBR((g_amon.links[link].fLinkRxBusy == 1));
	g_amon.links[link].fLinkRxBusy = 0;

Error:
	return r;
}

unsigned char AMONLinkTxBusy(AMON_LINK link) {
	return g_amon.links[link].fLinkTxBusy;
}

RESULT LockAMONLinkTx(AMON_LINK link) {
	RESULT r = R_OK;

	//CBRM((g_amon.links[link].fLinkTxBusy == 0), "Can't lock link rx %d, already busy", link);
	//CBR((g_amon.links[link].fLinkTxBusy == 0));
	g_amon.links[link].fLinkTxBusy = 1;

Error:
	return r;
}

RESULT UnlockAMONLinkTx(AMON_LINK link) {
	RESULT r = R_OK;

	//CBRM((g_amon.links[link].fLinkTxBusy == 1), "Can't unlock link rx %d, not busy", link);
	//CBR((g_amon.links[link].fLinkTxBusy == 1));
	g_amon.links[link].fLinkTxBusy = 0;

Error:
	return r;
}

unsigned char AMONLinkBusy(AMON_LINK link) {
	return AMONLinkRxBusy(link) || AMONLinkTxBusy(link);
}
