#ifndef CIRCLE_BUFFER_H_
#define CIRCLE_BUFFER_H_

#include "../Common/RESULT.h"
#include "../Common/EHM.h"

typedef struct PSZ_CIRCULAR_BUFFER {
	char** m_buffer;
	int m_buffer_e;
	int m_buffer_c;
	int m_buffer_l;
} PSZCircleBuffer;

PSZCircleBuffer* CreatNewPSZCircleBuffer(int length);
char* PSZCircleBufferPush(PSZCircleBuffer *pb, char *pszItem);
RESULT DeletePSZCircleBuffer(PSZCircleBuffer *pb);

typedef struct PSZ_CIRCULAR_BUFFER_ITERATOR {
	PSZCircleBuffer* m_pbuffer;
	int m_buffer_i;
	int m_buffer_s;
} PSZCircleBufferIterator;

PSZCircleBufferIterator *CreatePSZCircleBufferIterator(PSZCircleBuffer* pbuffer);
char* NextCircBuf(PSZCircleBufferIterator *it);
char* PreviousCircBuf(PSZCircleBufferIterator *it);

#endif // CIRCLE_BUFFER_H_
