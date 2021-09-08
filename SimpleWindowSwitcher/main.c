#define UNICODE
#include <Windows.h>
#include <stdio.h>
#include <roapi.h>
#include "sws_def.h"
#include "sws_dll.h"
#include "sws_error.h"
#include "sws_Helpers_Gdiplus.hpp"
#include "sws_WindowSwitcher.h"
#include "sws_RegistryMonitor.h"
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

sws_WindowSwitcher sws;

__declspec(dllexport) sws_error_t main(int argc, char** argv)
{
	sws_error_t rv = SWS_ERROR_SUCCESS;

#ifndef COMPILE_AS_LIBRARY
    if (!rv)
	{
		BOOL b = SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
		if (!b)
		{
			rv = sws_error_Report(SWS_ERROR_UNABLE_TO_SET_DPI_AWARENESS_CONTEXT);
		}
	}
#endif
	if (!rv)
	{
		rv = sws_error_Report(sws_error_GetFromHRESULT(CoInitializeEx(NULL, COINIT_MULTITHREADED)));
	}
	if (!rv)
	{
		rv = sws_error_Report(sws_error_GetFromHRESULT(RoInitialize(RO_INIT_MULTITHREADED)));
	}
	if (!rv)
	{
		rv = sws_Helpers_Gdiplus_Initialize();
	}
	if (!rv)
	{
		rv = sws_WindowSwitcher_Initialize(&sws);
	}
	if (!rv)
	{
		rv = sws_WindowSwitcher_RunMessageQueue(&sws);
	}
	if (!rv)
	{
		sws_WindowSwitcher_Clear(&sws);
		sws_Helpers_Gdiplus_Clear();
	}
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
	_CrtDumpMemoryLeaks();

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
        wchar_t exeName[MAX_PATH + 1];
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
        wchar_t wszSystemPath[MAX_PATH + 1];
        GetSystemDirectoryW(wszSystemPath, MAX_PATH + 1);
        wcscat_s(wszSystemPath, MAX_PATH + 1, L"\\dxgi.dll");
        HMODULE hModule = LoadLibraryW(wszSystemPath);
        CharLowerW(exeName);
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
        if (!wcscmp(exeName, L"explorer.exe") || !wcscmp(exeName, L"applicationframehost.exe"))
        {
            CreateThread(0, 0, main, 0, 0, 0);
        }
#pragma warning(default : 6387)
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