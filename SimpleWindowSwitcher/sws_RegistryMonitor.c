#include "sws_RegistryMonitor.h"

void sws_RegistryMonitor_Clear(sws_RegistryMonitor* _this)
{
	if (_this)
	{
		CloseHandle(_this->hEvCU);
		CloseHandle(_this->hEvLM);
		CloseHandle(_this->hKeyCU);
		CloseHandle(_this->hKeyLM);
		FreeLibrary(_this->hLib);
		free(_this->buffer);
		free(_this->wszPath);
		free(_this->wszValueName);
		memset(_this, 0, sizeof(sws_RegistryMonitor));
	}
}

sws_error_t sws_RegistryMonitor_Notify(sws_RegistryMonitor* _this, DWORD dwWakeMask)
{
	HWND hwndMain = NULL;
	HWND hwndDlgModeless = NULL;
	MSG msg;
	BOOL bRet;
	HACCEL haccel = NULL;
	LSTATUS lRes;
	HANDLE hEvents[3];
	hEvents[0] = _this->hEvEx;
	hEvents[1] = _this->hEvCU;
	hEvents[2] = _this->hEvLM;
	while (TRUE)
	{
		switch (MsgWaitForMultipleObjectsEx(
			3,
			hEvents,
			INFINITE,
			dwWakeMask,
			MWMO_INPUTAVAILABLE
		))
		{
		case WAIT_OBJECT_0 + 0:
			return SWS_ERROR_SUCCESS;
		case WAIT_OBJECT_0 + 1:
			lRes = RegQueryValueExW(
				_this->hKeyCU,
				_this->wszValueName,
				0,
				NULL,
				_this->buffer,
				&(_this->szBuffer)
			);
			if (lRes != ERROR_SUCCESS)
			{
				return sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
			}
			_this->callback(_this->ptr, FALSE, _this->buffer, _this->szBuffer);
			LSTATUS lRes = RegNotifyChangeKeyValue(
				_this->hKeyCU,
				FALSE,
				_this->notifyFilter,
				_this->hEvCU,
				TRUE
			);
			if (lRes != ERROR_SUCCESS && lRes != ERROR_INVALID_HANDLE)
			{
				return sws_error_Report(sws_error_GetFromWin32Error(lRes), NULL);
			}
			break;
		case WAIT_OBJECT_0 + 2:
			lRes = RegQueryValueExW(
				_this->hKeyLM,
				_this->wszValueName,
				0,
				NULL,
				_this->buffer,
				&(_this->szBuffer)
			);
			if (lRes != ERROR_SUCCESS)
			{
				return sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
			}
			_this->callback(_this->ptr, TRUE, _this->buffer, _this->szBuffer);
			lRes = RegNotifyChangeKeyValue(
				_this->hKeyLM,
				FALSE,
				_this->notifyFilter,
				_this->hEvLM,
				TRUE
			);
			if (lRes != ERROR_SUCCESS && lRes != ERROR_INVALID_HANDLE)
			{
				return sws_error_Report(sws_error_GetFromWin32Error(lRes), NULL);
			}
			break;
		case WAIT_OBJECT_0 + 3:
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			break;
		default:
			return sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
		}
	}
	return SWS_ERROR_SUCCESS;
}

sws_error_t sws_RegistryMonitor_Initialize(
	sws_RegistryMonitor* _this,
	wchar_t* wszPath,
	wchar_t* wszValueName,
	REGSAM desiredAccess,
	DWORD notifyFilter,
	SRRF srrf,
	char* buffer,
	size_t szBuffer,
	void(*callback)(void* ptr, BOOL bLM, char* buffer, size_t size),
	void* ptr,
	HANDLE hEvEx
)
{
	sws_error_t rv = SWS_ERROR_SUCCESS;

	if (!rv)
	{
		if (!_this)
		{
			rv = SWS_ERROR_NO_MEMORY;
		}
		else
		{
			memset(_this, 0, sizeof(sws_RegistryMonitor));
		}
		_this->ptr = ptr;
	}
	if (!rv)
	{
		if (!wszPath)
		{
			rv = SWS_ERROR_GENERIC_ERROR;
		}
		_this->lenPath = wcslen(wszPath);
	}
	if (!rv)
	{
		_this->wszPath = calloc(1, (_this->lenPath + 1) * sizeof(wchar_t));
		if (!_this->wszPath)
		{
			rv = SWS_ERROR_NO_MEMORY;
		}
	}
	if (!rv)
	{
		rv = sws_error_Report(sws_error_GetFromErrno(wcscpy_s(_this->wszPath, _this->lenPath + 1, wszPath)), NULL);
	}
	if (!rv)
	{
		if (!wszValueName)
		{
			rv = SWS_ERROR_GENERIC_ERROR;
		}
		_this->lenValueName = wcslen(wszValueName);
	}
	if (!rv)
	{
		_this->wszValueName = calloc(1, (_this->lenValueName + 1) * sizeof(wchar_t));
		if (!_this->wszValueName)
		{
			rv = SWS_ERROR_NO_MEMORY;
		}
	}
	if (!rv)
	{
		rv = sws_error_Report(sws_error_GetFromErrno(wcscpy_s(_this->wszValueName, _this->lenValueName + 1, wszValueName)), NULL);
	}
	if (!rv)
	{
		_this->desiredAccess = desiredAccess;
		_this->notifyFilter = notifyFilter;
		_this->szBuffer = szBuffer;
		_this->buffer = calloc(1, szBuffer);
		if (!_this->buffer)
		{
			rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_NO_MEMORY), NULL);
		}
	}
	if (!rv)
	{
		_this->callback = callback;
		_this->hLib = LoadLibraryW(L"Shlwapi.dll");
		if (!_this->hLib)
		{
			rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
		}
	}
	if (!rv)
	{
		_this->SHRegGetValueFromHKCUHKLMFunc = GetProcAddress(_this->hLib, "SHRegGetValueFromHKCUHKLM");
		if (!_this->SHRegGetValueFromHKCUHKLMFunc)
		{
			rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
		}
	}
	if (!rv)
	{
		if (_this->SHRegGetValueFromHKCUHKLMFunc(
			wszPath,
			wszValueName,
			SRRF_RT_REG_DWORD,
			NULL,
			buffer,
			&szBuffer
		) != ERROR_SUCCESS)
		{
			rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
		}
	}
	if (!rv)
	{
		LSTATUS lRes = RegOpenKeyExW(
			HKEY_LOCAL_MACHINE,
			wszPath,
			0,
			desiredAccess,
			&(_this->hKeyLM)
		);
		if (lRes != ERROR_SUCCESS && lRes != ERROR_FILE_NOT_FOUND)
		{
			rv = sws_error_Report(sws_error_GetFromWin32Error(lRes), NULL);
		}
	}
	if (!rv)
	{
		LSTATUS lRes = RegOpenKeyExW(
			HKEY_CURRENT_USER,
			wszPath,
			0,
			desiredAccess,
			&(_this->hKeyCU)
		);
		if (lRes != ERROR_SUCCESS && lRes != ERROR_FILE_NOT_FOUND)
		{
			rv = sws_error_Report(sws_error_GetFromWin32Error(lRes), NULL);
		}
	}
	if (!rv)
	{
		_this->hEvLM = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (!_this->hEvLM)
		{
			rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
		}
	}
	if (!rv)
	{
		_this->hEvCU = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (!_this->hEvCU)
		{
			rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
		}
	}
	if (!rv)
	{
		_this->hEvEx = hEvEx;
	}
	if (!rv)
	{
		LSTATUS lRes = RegNotifyChangeKeyValue(
			_this->hKeyLM,
			FALSE,
			notifyFilter,
			_this->hEvLM,
			TRUE
		);
		if (lRes != ERROR_SUCCESS && lRes != ERROR_INVALID_HANDLE)
		{
			rv = sws_error_Report(sws_error_GetFromWin32Error(lRes), NULL);
		}
	}
	if (!rv)
	{
		LSTATUS lRes = RegNotifyChangeKeyValue(
			_this->hKeyCU,
			FALSE,
			notifyFilter,
			_this->hEvCU,
			TRUE
		);
		if (lRes != ERROR_SUCCESS && lRes != ERROR_INVALID_HANDLE)
		{
			rv = sws_error_Report(sws_error_GetFromWin32Error(lRes), NULL);
		}
	}
	
	return rv;
}