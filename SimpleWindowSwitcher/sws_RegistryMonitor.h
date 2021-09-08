#ifndef _H_REGISTRYMONITOR_
#define _H_REGISTRYMONITOR_
#include <Windows.h>
#include <Shlwapi.h>
#include "sws_error.h"

typedef struct sws_RegistryMonitor
{
	HKEY hKeyLM;
	HKEY hKeyCU;
	HANDLE hEvLM;
	HANDLE hEvCU;
	HANDLE hEvEx;
	wchar_t* wszPath;
	size_t lenPath;
	wchar_t* wszValueName;
	size_t lenValueName;
	REGSAM desiredAccess;
	DWORD notifyFilter;
	SRRF srrf;
	char* buffer;
	size_t szBuffer;
	void(*callback)(void* ptr, BOOL bLM, char* buffer, size_t size);
	HANDLE hOp;
	HMODULE hLib;
	FARPROC SHRegGetValueFromHKCUHKLMFunc;
	void* ptr;
} sws_RegistryMonitor;

sws_error_t sws_RegistryMonitor_Notify(sws_RegistryMonitor* _this, DWORD dwWakeMask);

void sws_RegistryMonitor_Clear(sws_RegistryMonitor* _this);

sws_error_t sws_RegistryMonitor_Initialize(
	sws_RegistryMonitor* _this,
	wchar_t* wszPath,
	wchar_t* wszValueName,
	REGSAM desiredAccess,
	DWORD notifyFilter,
	SRRF srrf,
	char* buffer,
	size_t szBuffer,
	void(*callback)(void* ptr, char* buffer, size_t size),
	void* ptr
);
#endif