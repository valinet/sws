#ifndef _H_SWS_VECTOR_H_
#define _H_SWS_VECTOR_H_
#include <stdio.h>
#include <stdint.h>
#include "sws_def.h"
#include "sws_error.h"

typedef struct _sws_vector
{
	void* pList;
	int cbSize;
	int cbCapacity;
	int cbElementSize;
} sws_vector;

sws_error_t sws_vector_PushBack(sws_vector* _this, void* pElement);

void sws_vector_Clear(sws_vector* _this);

sws_error_t sws_vector_Initialize(sws_vector* _this, unsigned int cbElementSize);
#endif
