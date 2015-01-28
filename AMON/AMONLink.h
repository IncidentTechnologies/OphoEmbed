#ifndef AMON_LINK_H_
#define AMON_LINK_H_

// The AMON Link

#include "..\Common\RESULT.h"
#include "AMON.h"

#define NUM_LINKS 4
#define LINK_STATUS_COUNTER_THRESHOLD 5

typedef enum {
	AMON_ALL	= -1,
	AMON_NORTH 	= 0,
	AMON_SOUTH	= 1,
	AMON_EAST	= 2,
	AMON_WEST	= 3,
	AMON_LINK_INVALID
} AMON_LINK;

typedef enum {
	AMON_DEVICE_OK 					= 0x00,
	AMON_DEVICE_UNASSIGNED			= 0x01,
	AMON_DEVICE_MASTER				= 0x02,
	AMON_DEVICE_EFFECTIVE_MASTER	= 0x03,
	AMON_DEVICE_INVALID
} AMON_DEVICE_STATUS;

typedef enum {
	AMON_MASTER_FALSE,
	AMON_MASTER_ABSOLUTE,
	AMON_MASTER_SELF_DEFINED,
	AMON_MASTER_NET_DEFINED,
	AMON_MASTER_ASSIGNED,
	AMON_MASTER_INVALID
} AMON_MASTER_STATE;

// AMON Link States
typedef enum {
	AMON_LINK_UNINITIALIZED,
	AMON_LINK_ID_REQUESTED,
	AMON_LINK_ID_SENT,
	AMON_LINK_ESTABLISHING_LINK,
	AMON_LINK_ESTABLISHED,
	AMON_LINK_MAYBE_DISCONNECTED,
	AMON_LINK_STATE_INVALID
} AMON_LINK_STATE;

extern AMON_LINK_STATE g_AMONLinkStates[NUM_LINKS];

typedef struct {
	int id;
	AMON_DEVICE_STATUS Status;
	AMON_MASTER_STATE MasterState;
	int link_id;							// the link id

	unsigned fPendingLinkStatus: 1;
	unsigned char LinkStatusCounter;

	unsigned fLinkToMaster: 1;
} AMON_LINK_INFO;

#define MAX_MSG_LENGTH 256

extern unsigned char link_input[NUM_LINKS][MAX_MSG_LENGTH];
extern int link_input_c[NUM_LINKS];
extern int g_linkMessageLength[NUM_LINKS];

// Link
RESULT CheckLinkStatus(AMON_LINK link);

RESULT InitializeLink(AMON_LINK link);
RESULT DisconnectLink(AMON_LINK link);

RESULT ResetLink(AMON_LINK link);

const char* GetLinkStateString(AMON_LINK_STATE state);

RESULT PrintAMONLinkInfo(AMON_LINK link);

#endif // AMON_LINK_H_
