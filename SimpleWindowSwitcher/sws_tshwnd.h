#ifndef _H_SWS_TSHWND_
#define _H_SWS_TSHWND_
#include <Windows.h>
#include "sws_def.h"
#include "sws_error.h"

typedef struct _sws_tshwnd
{
	HWND hWnd;
	FILETIME ft;
} sws_tshwnd;

void sws_tshwnd_ModifyTimestamp(sws_tshwnd* _this, FILETIME ft);

void sws_tshwnd_UpdateTimestamp(sws_tshwnd* _this);

int sws_tshwnd_CompareTimestamp(sws_tshwnd* p1, sws_tshwnd* p2, LPARAM flags);

int sws_tshwnd_CompareHWND(sws_tshwnd* p1, sws_tshwnd* p2);

sws_error_t sws_tshwnd_Initialize(sws_tshwnd* _this, HWND hWnd);
#endif