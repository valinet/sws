#ifndef _H_SWS_WINDOWHELPERS_H_
#define _H_SWS_WINDOWHELPERS_H_
#include <initguid.h>
#include <Windows.h>
#include <psapi.h>
#include <propsys.h>
#include <appmodel.h>
#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")
#include <ShlObj.h>
#include <commctrl.h>
#pragma comment(lib, "Comctl32.lib")
#include "sws_def.h"
#include "sws_error.h"

// References:
// RealEnumWindows: https://stackoverflow.com/questions/38205375/enumwindows-function-in-win10-enumerates-only-desktop-apps
// IsAltTabWindow: https://devblogs.microsoft.com/oldnewthing/20071008-00/?p=24863
// GetIconFromHWND: https://github.com/cairoshell/ManagedShell/blob/master/src/ManagedShell.WindowsTasks/ApplicationWindow.cs

// bcc18b79-ba16-442f-80c4-8a59c30c463b
DEFINE_GUID(__uuidof_IShellItemImageFactory,
	0xbcc18b79,
	0xba16, 0x442f, 0x80, 0xc4,
	0x8a, 0x59, 0xc3, 0x0c, 0x46, 0x3b
);

DEFINE_GUID(__uuidof_IShellItem2,
	0x7e9fb0d3,
	0x919f, 0x4307, 0xab, 0x2e,
	0x9b, 0x18, 0x60, 0x31, 0x0c, 0x93
);

DEFINE_GUID(FOLDERID_AppsFolder,
	0x1e87508d,
	0x89c2, 0x42f0, 0x8a, 0x7e,
	0x64, 0x5a, 0x0f, 0x50, 0xca, 0x58
);

DEFINE_GUID(__uuidof_IPropertyStore,
	0x886D8EEB,
	0x8CF2, 0x4446, 0x8D, 0x02,
	0xCD, 0xBA, 0x1D, 0xBD, 0xCF, 0x99
);

DEFINE_GUID(__uuidof_AppUserModelIdProperty,
	0x9F4C2855,
	0x9F79, 0x4B39, 0xA8, 0xD0,
	0xE1, 0xD4, 0x2D, 0xE1, 0xD5, 0xF3
);

#define STATUS_BUFFER_TOO_SMALL 0xC0000023
typedef NTSTATUS(WINAPI* NtUserBuildHwndList)
(
	HDESK in_hDesk,
	HWND  in_hWndNext,
	BOOL  in_EnumChildren,
	BOOL  in_RemoveImmersive,
	DWORD in_ThreadID,
	UINT  in_Max,
	HWND* out_List,
	UINT* out_Cnt
	);
extern HMODULE _sws_hWin32u;
extern NtUserBuildHwndList _sws_pNtUserBuildHwndList;

typedef struct
{
	int nAccentState;
	int nFlags;
	int nColor;
	int nAnimationId;
} ACCENTPOLICY;
typedef struct
{
	int nAttribute;
	PVOID pData;
	ULONG ulDataSize;
} WINCOMPATTRDATA;
extern HINSTANCE _sws_hUser32;
typedef BOOL(WINAPI* pSetWindowCompositionAttribute)(HWND, WINCOMPATTRDATA*);
extern pSetWindowCompositionAttribute _sws_SetWindowCompositionAttribute;
typedef BOOL(WINAPI* pIsShellManagedWindow)(HWND);
extern pIsShellManagedWindow _sws_IsShellManagedWindow;

typedef HWND(WINAPI* pCreateWindowInBand)(
	_In_ DWORD dwExStyle,
	_In_opt_ ATOM atom,
	_In_opt_ LPCWSTR lpWindowName,
	_In_ DWORD dwStyle,
	_In_ int X,
	_In_ int Y,
	_In_ int nWidth,
	_In_ int nHeight,
	_In_opt_ HWND hWndParent,
	_In_opt_ HMENU hMenu,
	_In_opt_ HINSTANCE hInstance,
	_In_opt_ LPVOID lpParam,
	DWORD band
	);
extern pCreateWindowInBand _sws_CreateWindowInBand;

typedef BOOL(WINAPI* pSetWindowBand)(HWND hWnd, HWND hwndInsertAfter, DWORD dwBand);
extern pSetWindowBand _sws_SetWindowBand;

typedef BOOL(WINAPI* pGetWindowBand)(HWND hWnd, PDWORD pdwBand);
extern pGetWindowBand _sws_GetWindowBand;

extern BOOL(*_sws_ShouldSystemUseDarkMode)();
extern void(*_sws_RefreshImmersiveColorPolicyState)();
extern HINSTANCE _sws_hUxtheme;

BOOL(*_sws_IsTopLevelWindow)(HWND);

enum ZBID
{
	ZBID_DEFAULT = 0,
	ZBID_DESKTOP = 1,
	ZBID_UIACCESS = 2,
	ZBID_IMMERSIVE_IHM = 3,
	ZBID_IMMERSIVE_NOTIFICATION = 4,
	ZBID_IMMERSIVE_APPCHROME = 5,
	ZBID_IMMERSIVE_MOGO = 6,
	ZBID_IMMERSIVE_EDGY = 7,
	ZBID_IMMERSIVE_INACTIVEMOBODY = 8,
	ZBID_IMMERSIVE_INACTIVEDOCK = 9,
	ZBID_IMMERSIVE_ACTIVEMOBODY = 10,
	ZBID_IMMERSIVE_ACTIVEDOCK = 11,
	ZBID_IMMERSIVE_BACKGROUND = 12,
	ZBID_IMMERSIVE_SEARCH = 13,
	ZBID_GENUINE_WINDOWS = 14,
	ZBID_IMMERSIVE_RESTRICTED = 15,
	ZBID_SYSTEM_TOOLS = 16,
	ZBID_LOCK = 17,
	ZBID_ABOVELOCK_UX = 18,
};

static BOOL _sws_IsBandValidToInclude[19] = { FALSE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE };

HWND* _sws_WindowHelpers_Gui_BuildWindowList
(
	NtUserBuildHwndList pNtUserBuildHwndList,
	HDESK in_hDesk,
	HWND  in_hWnd,
	BOOL  in_EnumChildren,
	BOOL  in_RemoveImmersive,
	UINT  in_ThreadID,
	INT* out_Cnt
);

/********************************************************/
/* enumerate all top level windows including metro apps */
/********************************************************/
sws_error_t sws_WindowHelpers_RealEnumWindows(
	WNDENUMPROC in_Proc,
	LPARAM in_Param
);
sws_error_t sws_WindowHelpers_PermitDarkMode(HWND hWnd);

sws_error_t sws_WindowHelpers_ShouldSystemUseDarkMode(DWORD* dwRes);

sws_error_t sws_WindowHelpers_SetWindowBlur(HWND hWnd, int type, DWORD Color, DWORD Opacity);

BOOL sws_WindowHelpers_IsAltTabWindow(_In_ HWND hwnd, _In_ HWND hWndWallpaper);

void sws_WindowHelpers_GetDesktopText(wchar_t* wszTitle);

__declspec(dllexport) HICON sws_WindowHelpers_GetIconFromHWND(HWND hWnd, BOOL* bOwnProcess, BOOL bIsDesktop, UINT* szIcon);

static BOOL CALLBACK _sws_WindowHelpers_GetWallpaperHWNDCallback(_In_ HWND hwnd, _Out_ LPARAM lParam);

HWND sws_WindowHelpers_GetWallpaperHWND();

void sws_WindowHelpers_Release();

sws_error_t sws_WindowHelpers_Initialize();

inline void _sws_WindowHelpers_ToggleDesktop()
{
	PostMessageW(FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL), WM_HOTKEY, 513, 0);
	//PostMessageW(FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL), 0x579, 3 - 1, 0); // 1 to restore
	//PostMessageW(FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL), 0x579, 3 - 0, 0); // 0 to show
}

inline BOOL sws_WindowHelpers_IsWindowUWP(HWND hWnd)
{
	return (GetPropW(hWnd, (LPCWSTR)0xA914));
}
#endif