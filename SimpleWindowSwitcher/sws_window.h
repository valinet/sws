#ifndef _H_SWS_WINDOW_
#define _H_SWS_WINDOW_
#include <Windows.h>
#include <Psapi.h>
#include <Shlwapi.h>
#include "sws_WindowHelpers.h"
#include "sws_error.h"
typedef struct _sws_window
{
	HWND hWnd;
	DWORD dwProcessId;
	wchar_t wszPath[MAX_PATH];
	BOOL bIsApplicationFrameHost;
} sws_window;

void sws_window_Clear(sws_window* _this);

sws_error_t sws_window_Initialize(sws_window* _this, HWND hWnd);
#endif