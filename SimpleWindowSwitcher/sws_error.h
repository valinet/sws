#ifndef _H_SWS_ERROR_H_
#define _H_SWS_ERROR_H_
#include <Windows.h>
#include <stdio.h>
#define DBGHELP_TRANSLATE_TCHAR
#include <DbgHelp.h>
#pragma comment(lib, "Dbghelp.lib")

// References:
// https://github.com/rioki/rex/blob/master/rex/dbg.h#L100

typedef unsigned int sws_error_t;

#define SWS_ERROR_SUCCESS 0
#define SWS_ERROR_GENERIC_ERROR 1
#define SWS_ERROR_NO_MEMORY 2
#define SWS_ERROR_NOT_INITIALIZED 3
#define SWS_ERROR_LOADLIBRARY_FAILED 4
#define SWS_ERROR_FUNCTION_NOT_FOUND 5
#define SWS_ERROR_UNABLE_TO_SET_DPI_AWARENESS_CONTEXT 6

#ifdef __cplusplus
extern "C"
{
#endif
	void sws_error_PrintStackTrace();

	char* sws_error_NumToDescription(sws_error_t errnum);

	sws_error_t sws_error_Report(sws_error_t errnum);

	sws_error_t sws_error_GetFromHRESULT(HRESULT hResult);

	sws_error_t sws_error_GetFromWin32Error(DWORD win32err);

	sws_error_t sws_error_GetFromGdiplusStatus(int status);
#ifdef __cplusplus
}
#endif
#endif
