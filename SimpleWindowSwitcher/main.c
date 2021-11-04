#define UNICODE
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <Windows.h>
#include <Psapi.h>
#include <stdio.h>
#include "sws_def.h"
#include "sws_dll.h"
#include "sws_error.h"
#include "sws_WindowSwitcher.h"

__declspec(dllexport) sws_error_t main(DWORD unused)
{
	sws_error_t rv = SWS_ERROR_SUCCESS;
    void* sws = NULL;

	if (!rv)
	{
		rv = sws_WindowSwitcher_Initialize(&sws, TRUE);
	}
	if (!rv)
	{
		rv = sws_WindowSwitcher_RunMessageQueue(sws);
	}
	if (!rv)
	{
		sws_WindowSwitcher_Clear(sws);
	}
	return rv;
}

BOOL WINAPI DllMain(
    _In_ HINSTANCE hinstDLL,
    _In_ DWORD     fdwReason,
    _In_ LPVOID    lpvReserved
)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);
        wchar_t exeName[MAX_PATH];
        GetProcessImageFileNameW(
            OpenProcess(
                PROCESS_QUERY_INFORMATION,
                FALSE,
                GetCurrentProcessId()
            ),
            exeName,
            MAX_PATH
        );
        PathStripPathW(exeName);
        wchar_t dllName[MAX_PATH];
        GetModuleFileNameW(hinstDLL, dllName, MAX_PATH);
        PathStripPathW(dllName);
        if (!_wcsicmp(dllName, L"dxgi.dll"))
        {
            wchar_t wszSystemPath[MAX_PATH];
            GetSystemDirectoryW(wszSystemPath, MAX_PATH);
            wcscat_s(wszSystemPath, MAX_PATH, L"\\dxgi.dll");
            HMODULE hModule = LoadLibraryW(wszSystemPath);
#pragma warning(disable : 6387)
            ApplyCompatResolutionQuirkingFunc = GetProcAddress(hModule, "ApplyCompatResolutionQuirking");
            CompatStringFunc = GetProcAddress(hModule, "CompatString");
            CompatValueFunc = GetProcAddress(hModule, "CompatValue");
            CreateDXGIFactoryFunc = GetProcAddress(hModule, "CreateDXGIFactory");
            CreateDXGIFactory1Func = GetProcAddress(hModule, "CreateDXGIFactory1");
            CreateDXGIFactory2Func = GetProcAddress(hModule, "CreateDXGIFactory2");
            DXGID3D10CreateDeviceFunc = GetProcAddress(hModule, "DXGID3D10CreateDevice");
            DXGID3D10CreateLayeredDeviceFunc = GetProcAddress(hModule, "DXGID3D10CreateLayeredDevice");
            DXGID3D10GetLayeredDeviceSizeFunc = GetProcAddress(hModule, "DXGID3D10GetLayeredDeviceSize");
            DXGID3D10RegisterLayersFunc = GetProcAddress(hModule, "DXGID3D10RegisterLayers");
            DXGIDeclareAdapterRemovalSupportFunc = GetProcAddress(hModule, "DXGIDeclareAdapterRemovalSupport");
            DXGIDumpJournalFunc = GetProcAddress(hModule, "DXGIDumpJournal");
            DXGIGetDebugInterface1Func = GetProcAddress(hModule, "DXGIGetDebugInterface1");
            DXGIReportAdapterConfigurationFunc = GetProcAddress(hModule, "DXGIReportAdapterConfiguration");
            PIXBeginCaptureFunc = GetProcAddress(hModule, "PIXBeginCapture");
            PIXEndCaptureFunc = GetProcAddress(hModule, "PIXEndCapture");
            PIXGetCaptureStateFunc = GetProcAddress(hModule, "PIXGetCaptureState");
            SetAppCompatStringPointerFunc = GetProcAddress(hModule, "SetAppCompatStringPointer");
            UpdateHMDEmulationStatusFunc = GetProcAddress(hModule, "UpdateHMDEmulationStatus");
#pragma warning(default : 6387)
        }
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}