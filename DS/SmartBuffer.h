#ifndef SMART_BUFFER_H_
#define SMART_BUFFER_H_

#define SMART_BUFFER_BLOCK_SIZE 256

#include "../Common/RESULT.h"
#include "../Common/EHM.h"

typedef struct SMART_BUFFER {
	char *m_pBuffer;
	int m_pBuffer_n;
	int m_pBuffer_bn;
} SmartBuffer;


SmartBuffer* CreateSmartBuffer(const char *pBuffer, int pBuffer_n);
SmartBuffer* CreateSmartBufferFromSB(SmartBuffer *psb);
RESULT DeleteSmartBuffer(SmartBuffer *psb);

RESULT SetBufferToPsz(SmartBuffer *psb, const char *pszBuffer);
RESULT SetBufferToBuffer(SmartBuffer *psb, const char *pBuffer, int pBuffer_n);
RESULT ResetBuffer(SmartBuffer *psb);

// Buffer Tools
char *CreateBufferCopy(SmartBuffer *psb);
unsigned char NotAllWhitespace(SmartBuffer *psb);

// Buffer Manipulation
RESULT IncrementBufferSize(SmartBuffer *psb);
RESULT SBAppendByte(SmartBuffer *psb, char byte);
RESULT SBAppendPsz(SmartBuffer *psb, char *psz, int psz_n);
RESULT SBAppendSB(SmartBuffer *psb, SmartBuffer sb);

RESULT RemoveCharacter(SmartBuffer *psb, char *pChar);    // Removes character at pointer, pointer should point inside the bounds of m_Buffer
RESULT SetBufferEqual(SmartBuffer *psb, const SmartBuffer *rhs);

RESULT Find(SmartBuffer *psb, const char byte, char **ppChar);            // will return a pointer into the buffer of the first found byte

char* GetBuffer(SmartBuffer *psb);
int GetBufferLength(SmartBuffer *psb);
int GetBufferBlockCount(SmartBuffer *psb);

#endif // SMART_BUFFER_H_
