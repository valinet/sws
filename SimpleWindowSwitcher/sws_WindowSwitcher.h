#ifndef _H_SWS_WINDOWSWITCHER_H_
#define _H_SWS_WINDOWSWITCHER_H_
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <initguid.h>
#include <Windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <Uxtheme.h>
#pragma comment(lib, "Uxtheme.lib")
#include <Shlobj_core.h>
#pragma comment(lib, "Shlwapi.lib")
#include <roapi.h>
#pragma comment(lib, "runtimeobject.lib")
#include "sws_def.h"
#include "sws_error.h"
#include "sws_WindowSwitcherLayout.h"
#include "sws_RegistryMonitor.h"

typedef struct _sws_WindowSwitcher
{
    BOOL bIsDynamic;
    BOOL bWithRegMon;
    HRESULT hrCo;
    HRESULT hrRo;
    HMODULE hinstDLL;
    HWND hWnd;
    UINT msgShellHook;
    sws_WindowSwitcherLayout layout;
    HBRUSH hContourBrush;
    HBRUSH hBackgroundBrush;
    HBRUSH hCloseButtonBrush;
    HTHEME hTheme;
    BOOL bPartialRedraw;
    ITaskbarList* pTaskList;
    HWND hWndLast;
    BOOL bWasControl;
    sws_RegistryMonitor rm;
    HMONITOR hMonitor;
    INT cwIndex;
    DWORD cwMask;
    HANDLE hEvExit;
    BOOL bIsDarkMode;

    DWORD dwRowHeight;
    DWORD dwMaxWP;
    DWORD dwMaxHP;
    DWORD bIncludeWallpaper;
    DWORD dwColorScheme;
} sws_WindowSwitcher;

void sws_WindowSwitcher_RefreshTheme(sws_WindowSwitcher* _this);

void sws_WindowSwitcher_SetTransparencyFromRegistry(sws_WindowSwitcher* _this, HKEY hOrigin);

void _sws_WindowSwitcher_SwitchToSelectedItemAndDismiss(sws_WindowSwitcher* _this);

static void CALLBACK _sws_WindowSwitcher_NotifyTransparencyChange(sws_WindowSwitcher* _this, BOOL bIsInHKLM, DWORD* value, size_t size);

static DWORD WINAPI _sws_WindowSwitcher_Show(sws_WindowSwitcher* _this);

static BOOL CALLBACK _sws_WindowSwitcher_EnumWindowsCallback(_In_ HWND hWnd, _In_ sws_WindowSwitcher* _this);

static sws_error_t _sws_WindowSwitcher_GetCloseButtonRectFromIndex(sws_WindowSwitcher* _this, DWORD dwIndex, LPRECT lpRect);

static LRESULT _sws_WindowsSwitcher_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static sws_error_t _sws_WindowSwitcher_RegisterWindowClass(sws_WindowSwitcher* _this);

__declspec(dllexport) sws_error_t sws_WindowSwitcher_RunMessageQueue(sws_WindowSwitcher* _this);

__declspec(dllexport) void sws_WindowSwitcher_Clear(sws_WindowSwitcher* _this);

__declspec(dllexport) sws_error_t sws_WindowSwitcher_Initialize(sws_WindowSwitcher** __this, BOOL bWithRegMon);

#endif
