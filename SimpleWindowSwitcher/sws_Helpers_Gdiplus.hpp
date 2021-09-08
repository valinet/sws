#ifndef _HPP_SWS_HELPERS_GDIPLUS_
#define _HPP_SWS_HELPERS_GDIPLUS_
#include <Windows.h>
#ifdef __cplusplus
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
#include "sws_error.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif
	ULONG_PTR g_GdiplusToken;
	void sws_Helpers_Gdiplus_Clear();
	sws_error_t sws_Helpers_Gdiplus_Initialize();
#ifdef __cplusplus
}
#endif
#endif