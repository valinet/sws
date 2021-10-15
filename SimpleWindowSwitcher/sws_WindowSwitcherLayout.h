#ifndef _H_SWS_WINDOWSWITCHERLAYOUT_H_
#define _H_SWS_WINDOWSWITCHERLAYOUT_H_
#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")
#include <shellscalingapi.h>
#pragma comment(lib, "Shcore.lib")
#include "sws_def.h"
#include "sws_error.h"
#include "sws_vector.h"
#include "sws_WindowSwitcherLayoutWindow.h"
#include "sws_WindowHelpers.h"

typedef struct _sws_WindowSwitcherLayout
{
	HMONITOR hMonitor;
	HWND hWnd;

	sws_vector pWindowList;
	int iX;
	int iY;
	unsigned int iWidth;
	unsigned int iHeight;
	unsigned int cbDpiX;
	unsigned int cbDpiY;
	int iIndex;
	MONITORINFO mi;
	double cbMaxHeight;
	double cbMaxWidth;
	double cbRowWidth;
	double cbRowHeight;
	double cbRowTitleHeight;
	double cbMasterTopPadding;
	double cbMasterBottomPadding;
	double cbMasterLeftPadding;
	double cbMasterRightPadding;
	double cbElementTopPadding;
	double cbElementBottomPadding;
	double cbElementLeftPadding;
	double cbElementRightPadding;
	double cbTopPadding;
	double cbBottomPadding;
	double cbLeftPadding;
	double cbRightPadding;
	double cbHDividerPadding;
	double cbVDividerPadding;
	double cbThumbnailAvailableHeight;
	double cbMaxTileWidth;
	unsigned int numTopMost;
	BOOL bIncludeWallpaper;
	BOOL bWallpaperAlwaysLast;
	BOOL bWallpaperToggleBehavior;
	HWND hWndWallpaper;
	HFONT hFontRegular;
	HFONT hFontRegular2;
} sws_WindowSwitcherLayout;

static BOOL CALLBACK _sws_WindowSwitcherLayout_EnumWindowsCallback(_In_ HWND hWnd, _In_ sws_WindowSwitcherLayout* _this);

sws_error_t sws_WindowSwitcherLayout_InvalidateLayout(sws_WindowSwitcherLayout* _this);

sws_error_t sws_WindowSwitcherLayout_ComputeLayout(sws_WindowSwitcherLayout* _this, int direction, HWND hTarget);

void sws_WindowSwitcherLayout_Clear(sws_WindowSwitcherLayout* _this);

sws_error_t sws_WindowSwitcherLayout_Initialize(sws_WindowSwitcherLayout* _this, HMONITOR hMonitor, HWND hWnd, DWORD* settings);

#endif