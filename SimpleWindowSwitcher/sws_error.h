#ifndef _H_SWS_ERROR_H_
#define _H_SWS_ERROR_H_
#include <Windows.h>
#include <stdio.h>
#define DBGHELP_TRANSLATE_TCHAR
#include <DbgHelp.h>
#pragma comment(lib, "Dbghelp.lib")

// References:
// https://github.com/rioki/rex/blob/master/rex/dbg.h#L100

typedef HRESULT sws_error_t;

#define SWS_ERROR_SUCCESS									S_OK
#define SWS_ERROR_SUCCESS_TEXT								"The operation completed successfully"
#define SWS_ERROR_ERROR										S_FALSE
#define SWS_ERROR_ERROR_TEXT								"General failure"
#define SWS_ERROR_GENERIC_ERROR								0xA0010001
#define SWS_ERROR_GENERIC_ERROR_TEXT						"A generic error has occured"
#define SWS_ERROR_NO_MEMORY									0xA0010002
#define SWS_ERROR_NO_MEMORY_TEXT							"Insufficient memory. Please close some applications and try again"
#define SWS_ERROR_NOT_INITIALIZED							0xA0010003
#define SWS_ERROR_NOT_INITIALIZED_TEXT						"Functionality is not initialized"
#define SWS_ERROR_LOADLIBRARY_FAILED						0xA0010004
#define SWS_ERROR_LOADLIBRARY_FAILED_TEXT					"The requested library is not available"
#define SWS_ERROR_FUNCTION_NOT_FOUND						0xA0010005
#define SWS_ERROR_FUNCTION_NOT_FOUND_TEXT					"The requested procedure was not found"
#define SWS_ERROR_UNABLE_TO_SET_DPI_AWARENESS_CONTEXT		0xA0010006
#define SWS_ERROR_UNABLE_TO_SET_DPI_AWARENESS_CONTEXT_TEXT  "Unable to set the requested DPI awareness context"

#ifdef __cplusplus
extern "C"
{
#endif
	void sws_error_PrintStackTrace();

	char* sws_error_NumToDescription(sws_error_t errnum);

	sws_error_t sws_error_Report(sws_error_t errnum, void* data);

	inline sws_error_t sws_error_GetFromInternalError(HRESULT hResult)
	{
		return hResult;
	}

	inline sws_error_t sws_error_GetFromHRESULT(HRESULT hResult)
	{
		return hResult;
	}

	inline sws_error_t sws_error_GetFromWin32Error(DWORD win32err)
	{
		return HRESULT_FROM_WIN32(win32err);
	}

	inline sws_error_t sws_error_GetFromErrno(errno_t err)
	{
		return sws_error_GetFromInternalError(err);
	}
#ifdef __cplusplus
}
#endif
#endif
