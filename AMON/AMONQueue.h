#ifndef AMON_QUEUE_H_
#define AMON_QUEUE_H_

#include "../DS/list.h"
#include "AMONPacket.h"

RESULT InitializeAMONQueue();
RESULT PushAMONQueuePacket(AMON_LINK link, AMONPacket *pAMONPacket);
RESULT SendAMONQueue(AMON_LINK link);

RESULT PushAndTransmitAMONQueuePacket(AMON_LINK link, AMONPacket *pAMONPacket);

unsigned char NumPacketsInQueue(AMON_LINK link);

#endif // ! AMON_QUEUE_H_
