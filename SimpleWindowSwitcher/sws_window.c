#include "sws_window.h"

void sws_window_Clear(sws_window* _this)
{
	if (_this->hWnd)
	{
		_this->hWnd = NULL;
		_this->dwProcessId = 0;
		ZeroMemory(_this->wszPath, MAX_PATH);
		_this->bIsApplicationFrameHost = FALSE;
	}
}

sws_error_t sws_window_Initialize(sws_window* _this, HWND hWnd)
{
	sws_error_t rv = SWS_ERROR_SUCCESS;

	if (!rv)
	{
		ZeroMemory(_this->wszPath, MAX_PATH);
		_this->hWnd = hWnd;
	}
	if (!rv)
	{
		if (!GetWindowThreadProcessId(hWnd, &(_this->dwProcessId)))
		{
			rv = sws_error_GetFromWin32Error(GetLastError());
		}
	}
	if (!rv)
	{
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, _this->dwProcessId);
		if (hProcess)
		{
			GetModuleFileNameExW(hProcess, NULL, _this->wszPath, MAX_PATH);
			CloseHandle(hProcess);
		}
	}
	if (!rv)
	{
		wchar_t path[MAX_PATH];
		ZeroMemory(path, MAX_PATH);
		memcpy(path, _this->wszPath, MAX_PATH);
		PathStripPathW(path);
		_this->bIsApplicationFrameHost = !_wcsicmp(path, L"ApplicationFrameHost.exe");
	}

	return rv;
}
