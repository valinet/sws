#ifndef _H_SWS_WINDOWSWITCHER_H_
#define _H_SWS_WINDOWSWITCHER_H_
#include <initguid.h>
#include <Windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <Uxtheme.h>
#pragma comment(lib, "Uxtheme.lib")
#include <Shlobj_core.h>
#pragma comment(lib, "Shlwapi.lib")
//#include <Shldisp.h>
#include "sws_def.h"
#include "sws_error.h"
#include "sws_WindowSwitcherLayout.h"
#include "sws_RegistryMonitor.h"

DEFINE_GUID(__CLSID_Shell,
    0x13709620,
    0xC279, 0x11CE, 0xA4, 0x9E,
    0x44, 0x45, 0x53, 0x54, 0x00, 0x00
);
DEFINE_GUID(__IID_IDispatch,
    0x00020400,
    0x0000, 0x0000, 0xC0, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x46
);
DEFINE_GUID(__uuidof_TaskbarList,
    0x56FDF344,
    0xFD6D, 0x11d0, 0x95, 0x8A,
    0x00, 0x60, 0x97, 0xC9, 0xA0, 0x90
);
DEFINE_GUID(__uuidof_ITaskbarList,
    0x56FDF342,
    0xFD6D, 0x11d0, 0x95, 0x8A,
    0x00, 0x60, 0x97, 0xC9, 0xA0, 0x90
);

typedef struct _sws_WindowSwitcher
{
    HWND hWnd;
    sws_WindowSwitcherLayout layout;
    HBRUSH hContourBrush;
    HBRUSH hBackgroundBrush;
    HTHEME hTheme;
    BOOL bPartialRedraw;
    ITaskbarList* pTaskList;
    HWND hWndLast;
    RECT rcPrev;
    BOOL bWasControl;
    sws_RegistryMonitor rm;
    HMONITOR hMonitor;
} sws_WindowSwitcher;

static DWORD WINAPI _sws_WindowSwitcher_ToggleDesktop(sws_WindowSwitcher* _this);

static void CALLBACK _sws_WindowSwitcher_NotifySettingsChange(sws_WindowSwitcher* _this, BOOL bIsInHKLM, DWORD* value, size_t size);

static DWORD WINAPI _sws_WindowSwitcher_PrepareShow(sws_WindowSwitcher* _this);

static BOOL CALLBACK _sws_WindowSwitcher_EnumWindowsCallback(_In_ HWND hWnd, _In_ sws_WindowSwitcher* _this);

static LRESULT _sws_WindowsSwitcher_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static sws_error_t _sws_WindowSwitcher_RegisterWindowClass();

sws_error_t sws_WindowSwitcher_RunMessageQueue(sws_WindowSwitcher* _this);

void sws_WindowSwitcher_Clear(sws_WindowSwitcher* _this);

sws_error_t sws_WindowSwitcher_Initialize(sws_WindowSwitcher* _this);

#endif
