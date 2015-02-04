#ifndef AMON_H_
#define AMON_H_

// The actual AMON device and high level control mechanisms

#include "../Common/RESULT.h"
#include "AMONLink.h"

#define AMON_MASTER_ID 0

typedef struct {
	AMON_MASTER_STATE MasterState;

	unsigned fInitialized: 1;
	unsigned fStart: 1;
	AMON_DEVICE_STATUS status;
	int id;
	AMON_LINK_INFO links[NUM_LINKS];
} AMON;

extern AMON g_amon;

RESULT InitializeAMON();

// Start / Stop the AMON device
RESULT StartAMON();
RESULT StopAMON();

unsigned char IsAMONMaster();

#endif // ! AMON_H_
