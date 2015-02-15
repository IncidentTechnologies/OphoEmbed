#ifndef AMON_QUEUE_H_
#define AMON_QUEUE_H_

#include "../DS/list.h"
#include "AMONPacket.h"

#define MAX_SEND_QUEUE_PACKETS 4
#define MAX_PENDING_QUEUE_PACKETS 30

extern unsigned char g_NumQueuedPackets[NUM_LINKS];

RESULT InitializeAMONQueue();

RESULT PushAMONQueuePacket(AMON_LINK link, AMONPacket *pAMONPacket);
RESULT PushAMONPendingQueuePacket(AMON_LINK link, AMONPacket *pAMONPacket);
RESULT PushAMONIncomingQueuePacket(AMON_LINK link, AMONPacket *pAMONPacket);

AMONPacket *PopAMONIncomingQueuePacket(AMON_LINK link);
RESULT HandleAMONIncomingQueue(AMON_LINK link);

RESULT SendAMONQueue(AMON_LINK link);

RESULT PushAndTransmitAMONQueuePacket(AMON_LINK link, AMONPacket *pAMONPacket);
RESULT PushPendingQueue(AMON_LINK link);

unsigned char NumPacketsInQueue(AMON_LINK link);
unsigned char NumPacketsInPendingQueue(AMON_LINK link);
unsigned char NumPacketsInIncomingQueue(AMON_LINK link);

#endif // ! AMON_QUEUE_H_
