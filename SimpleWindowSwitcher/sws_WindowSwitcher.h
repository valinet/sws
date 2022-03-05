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
#include <oleacc.h>
#include "sws_def.h"
#include "sws_error.h"
#include "sws_window.h"
#include "sws_utility.h"
#include "sws_WindowSwitcherLayout.h"
#include "sws_RegistryMonitor.h"
#include "sws_tshwnd.h"

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
    //sws_WindowSwitcherLayout layouts[SWS_WINDOWSWITCHER_MAX_NUM_MONITORS];
    //sws_WindowSwitcherLayout minilayouts[SWS_WINDOWSWITCHER_MAX_NUM_MONITORS];
    //UINT numLayouts;
    //UINT numLayoutsMax;
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
    BOOL bIsMouseClicking;
    long long last_change;
    sws_vector pHWNDList;
    HDPA htshwnds;
    BOOL bWallpaperAlwaysLast;
    HWND hWndWallpaper;
    UINT mode;
    HWND lastMiniModehWnd;
    HWINEVENTHOOK global_hook;
    DWORD dwColorScheme;
    DWORD dwTheme;
    DWORD dwCornerPreference;
    DWORD dwShowDelay;
    BOOL bPrimaryOnly;
    sws_IInputSwitchCallback InputSwitchCallback;
    sws_IInputSwitchControl* pInputSwitchControl;
    UINT vkTilde;
    UINT dwOverrideMode;
    UINT opacity;
    HANDLE hShowThread;
    HANDLE hShowSignal;
    BOOL bNoPerApplicationListPrevious;
    BOOL bIsInitialized;
    HWND hWndAccessible;
    IAccPropServices* pAccPropServices;
    HBRUSH hFlashBrush;
    DWORD dwPaintFlags;
    HANDLE hFlashAnimationThread;
    HANDLE hFlashAnimationSignal;
    HDC hdcWindow;
    HDC hdcPaint;
    HPAINTBUFFER hBufferedPaint;
    INT cwOldIndex;
    DWORD cwOldMask;
    HDPA hLastClosedWnds;
    long long lastUpdateTime;

    DWORD dwRowHeight;
    DWORD dwMaxWP;
    DWORD dwMaxHP;
    DWORD bIncludeWallpaper;
    DWORD bPerMonitor;
    DWORD dwMaxAbsoluteWP;
    DWORD dwMaxAbsoluteHP;
    DWORD bNoPerApplicationList;
    DWORD dwMasterPadding;
    DWORD dwWallpaperSupport;
    DWORD bSwitcherIsPerApplication;
    DWORD bAlwaysUseWindowTitleAndIcon;
} sws_WindowSwitcher;

typedef struct _sws_WindowSwitcher_EndTaskThreadParams
{
    HWND hWnd;
    sws_WindowSwitcher* sws;
    HDESK hDesktop;
} sws_WindowSwitcher_EndTaskThreadParams;

static void _sws_WindowSwitcher_UpdateAccessibleText(sws_WindowSwitcher* _this);

sws_error_t sws_WindowSwitcher_RegisterHotkeys(sws_WindowSwitcher* _this, HKL hkl);

void sws_WindowSwitcher_UnregisterHotkeys(sws_WindowSwitcher* _this);

void sws_WindowSwitcher_RefreshTheme(sws_WindowSwitcher* _this);

void _sws_WindowSwitcher_SwitchToSelectedItemAndDismiss(sws_WindowSwitcher* _this);

static DWORD WINAPI _sws_WindowSwitcher_Calculate(sws_WindowSwitcher* _this);

static sws_error_t _sws_WindowSwitcher_GetCloseButtonRectFromIndex(sws_WindowSwitcher* _this, DWORD dwIndex, LPRECT lpRect);

static LRESULT _sws_WindowsSwitcher_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static sws_error_t _sws_WindowSwitcher_RegisterWindowClass(sws_WindowSwitcher* _this);

__declspec(dllexport) sws_error_t sws_WindowSwitcher_RunMessageQueue(sws_WindowSwitcher* _this);

__declspec(dllexport) void sws_WindowSwitcher_Clear(sws_WindowSwitcher* _this);

__declspec(dllexport) sws_error_t sws_WindowSwitcher_Initialize(sws_WindowSwitcher** __this, BOOL bWithRegMon);

void sws_WindowSwitcher_Paint(sws_WindowSwitcher* _this, DWORD dwFlags);

void sws_WindowSwitcher_InitializeDefaultSettings(sws_WindowSwitcher* _this);

#endif
