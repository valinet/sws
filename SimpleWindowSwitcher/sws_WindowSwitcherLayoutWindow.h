#ifndef _H_SWS_WINDOWSWITCHERLAYOUTWINDOW_
#define _H_SWS_WINDOWSWITCHERLAYOUTWINDOW_
#include <Windows.h>
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")
#include "sws_error.h"

typedef struct _sws_WindowSwitcherLayoutWindow
{
    HWND hWnd;
    SIZE sizWindow;
    HTHUMBNAIL hThumbnail;
    RECT rcThumbnail;
    RECT rcWindow;
    int iRowMax;
    HICON hIcon;
    BOOL bOwnProcess;
} sws_WindowSwitcherLayoutWindow;

void sws_WindowSwitcherLayoutWindow_Erase(sws_WindowSwitcherLayoutWindow* _this);

void sws_WindowSwitcherLayoutWindow_Clear(sws_WindowSwitcherLayoutWindow* _this);

sws_error_t sws_WindowSwitcherLayoutWindow_Initialize(sws_WindowSwitcherLayoutWindow* _this, HWND hWnd);
#endif