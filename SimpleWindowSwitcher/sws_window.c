#include "sws_window.h"

extern NTSTATUS NTAPI NtQueryInformationProcess(
	_In_ HANDLE ProcessHandle,
	_In_ DWORD ProcessInformationClass,
	_Out_writes_bytes_(ProcessInformationLength) PVOID ProcessInformation,
	_In_ ULONG ProcessInformationLength,
	_Out_opt_ PULONG ReturnLength);
#define ProcessConsoleHostProcess 49

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
		HWND hWndOfInterest = _sws_HungWindowFromGhostWindow(hWnd);
		if (!hWndOfInterest) hWndOfInterest = hWnd;
		if (!GetWindowThreadProcessId(hWndOfInterest, &(_this->dwProcessId)))
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
			//NtQueryInformationProcess(hProcess, ProcessConsoleHostProcess, &(_this->dwConhostPID), sizeof(UINT_PTR), NULL);
			//_this->dwConhostPID--;
			//wprintf(L"%d - %s\n", _this->dwConhostPID, _this->wszPath);
			CloseHandle(hProcess);
		}
	}
	if (!rv)
	{
		/*
		wchar_t path[MAX_PATH];
		ZeroMemory(path, MAX_PATH);
		memcpy(path, _this->wszPath, MAX_PATH);
		wchar_t syspath[MAX_PATH];
		ZeroMemory(syspath, MAX_PATH);
		GetSystemDirectoryW(syspath, MAX_PATH);
		wcscat_s(syspath, MAX_PATH, L"\\ApplicationFrameHost.exe");
		//wprintf(L"%s %s\n", path, syspath);
		_this->bIsApplicationFrameHost = !_wcsicmp(path, syspath);
		*/
		_this->bIsApplicationFrameHost = sws_IsShellFrameWindow(hWnd);
		//_this->bIsApplicationFrameHost = sws_WindowHelpers_IsWindowUWP(hWnd);
		_this->tshWnd = NULL;
	}

	return rv;
}
