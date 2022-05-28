#ifndef _H_SWS_WINDOWSWITCHERLAYOUTWINDOW_
#define _H_SWS_WINDOWSWITCHERLAYOUTWINDOW_
#include <Windows.h>
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")
#include "sws_error.h"
#include "sws_WindowHelpers.h"
#include "sws_tshwnd.h"

typedef struct _sws_WindowSwitcherLayoutWindow
{
    HWND hWnd;
    SIZE sizWindow;
    HTHUMBNAIL hThumbnail;
    RECT rcThumbnail;
    RECT rcWindow;
    int iRowMax;
    HICON hIcon;
    UINT dwIconSource;
    UINT szIcon;
    RECT rcIcon;
    WCHAR wszPath[MAX_PATH];
    sws_tshwnd* tshWnd;
    sws_tshwnd* last_flashing_tshwnd;
    DWORD dwCount;
    DWORD dwWindowFlags;
} sws_WindowSwitcherLayoutWindow;

void sws_WindowSwitcherLayoutWindow_Erase(sws_WindowSwitcherLayoutWindow* _this);

void sws_WindowSwitcherLayoutWindow_Clear(sws_WindowSwitcherLayoutWindow* _this);

sws_error_t sws_WindowSwitcherLayoutWindow_Initialize(sws_WindowSwitcherLayoutWindow* _this, HWND hWnd, WCHAR* wszPath);
#endif