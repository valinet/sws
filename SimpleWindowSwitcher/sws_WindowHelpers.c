#include "sws_WindowHelpers.h"

ULONG_PTR _sws_gdiplus_token;
RTL_OSVERSIONINFOW sws_global_rovi;
DWORD32 sws_global_ubr;
NtUserBuildHwndList _sws_pNtUserBuildHwndList;
pCreateWindowInBand _sws_CreateWindowInBand;
pSetWindowCompositionAttribute _sws_SetWindowCompositionAttribute;
pIsShellManagedWindow _sws_IsShellManagedWindow;
pIsShellManagedWindow sws_IsShellFrameWindow;
pGetWindowBand _sws_GetWindowBand;
pSetWindowBand _sws_SetWindowBand;
BOOL(*_sws_ShouldSystemUseDarkMode)();
void(*_sws_RefreshImmersiveColorPolicyState)();
void(*_sws_SetPreferredAppMode)(INT64);
void(*_sws_AllowDarkModeForWindow)(HWND hWnd, INT64 bAllowDark);
HMODULE _sws_hComctl32 = 0;
HMODULE _sws_hShlwapi = 0;
HMODULE _sws_hWin32u = 0;
HINSTANCE _sws_hUser32 = 0;
HINSTANCE _sws_hUxtheme = 0;
HINSTANCE _sws_hShcore = 0;
pHungWindowFromGhostWindow _sws_HungWindowFromGhostWindow;
pGhostWindowFromHungWindow _sws_GhostWindowFromHungWindow;
pInternalGetWindowIcon _sws_InternalGetWindowIcon;
pIsCoreWindow _sws_IsCoreWindow;
FARPROC sws_SHRegGetValueFromHKCUHKLM;
FARPROC sws_LoadIconWithScaleDown;
BOOL (*sws_SHWindowsPolicy)(REFGUID riid);
DEFINE_GUID(POLID_TurnOffSPIAnimations, 0xD7AF00A, 0xB468, 0x4A39, 0xB0, 0x16, 0x33, 0x3E, 0x22, 0x77, 0xAB, 0xED);

BOOL sws_WindowHelpers_IsValidMonitor(HMONITOR hMonitor, HDC unnamedParam2, LPRECT unnamedParam3, HMONITOR* pMonitor)
{
	if (!pMonitor || !*pMonitor) return FALSE;
	if (hMonitor == *pMonitor)
	{
		*pMonitor = NULL;
		return FALSE;
	}
	return TRUE;
}

sws_error_t sws_WindowHelpers_PermitDarkMode(HWND hWnd)
{
	if (_sws_SetPreferredAppMode && _sws_AllowDarkModeForWindow)
	{
		_sws_SetPreferredAppMode(TRUE);
		if (hWnd)
		{
			_sws_AllowDarkModeForWindow(hWnd, TRUE);
		}
		return SWS_ERROR_SUCCESS;
	}
	else
	{
		return SWS_ERROR_NOT_INITIALIZED;
	}
}

sws_error_t sws_WindowHelpers_RefreshImmersiveColorPolicyState()
{
	if (_sws_RefreshImmersiveColorPolicyState)
	{
		_sws_RefreshImmersiveColorPolicyState();
		return SWS_ERROR_SUCCESS;
	}
	else
	{
		return SWS_ERROR_NOT_INITIALIZED;
	}
}

sws_error_t sws_WindowHelpers_ShouldSystemUseDarkMode(DWORD* dwRes)
{
	RTL_OSVERSIONINFOW rovi;
	if (sws_WindowHelpers_GetOSVersion(&rovi) && rovi.dwBuildNumber < 18985)
	{
		*dwRes = TRUE;
		return SWS_ERROR_SUCCESS;
	}
	if (_sws_ShouldSystemUseDarkMode)
	{
		if (dwRes)
		{
			*dwRes = _sws_ShouldSystemUseDarkMode();
			return SWS_ERROR_SUCCESS;
		}
		else
		{
			return SWS_ERROR_INVALID_PARAMETER;
		}
	}
	else
	{
		return SWS_ERROR_NOT_INITIALIZED;
	}
}

sws_error_t sws_WindowHelpers_SetWindowBlur(HWND hWnd, int type, DWORD Color, DWORD Opacity)
{
	ACCENTPOLICY policy;
	policy.nAccentState = type;
	policy.nFlags = 0;
	policy.nColor = (Opacity << 24) | (Color & 0xFFFFFF); // ACCENT_ENABLE_BLURBEHIND=3... // Color = 0XB32E9A
	policy.nFlags = 0;
	WINCOMPATTRDATA data = { 19, &policy, sizeof(ACCENTPOLICY) }; // WCA_ACCENT_POLICY=19
	if (_sws_SetWindowCompositionAttribute)
	{
		_sws_SetWindowCompositionAttribute(hWnd, &data);
	}
	else
	{
		return SWS_ERROR_NOT_INITIALIZED;
	}
}

HWND* _sws_WindowHelpers_Gui_BuildWindowList
(
	NtUserBuildHwndList pNtUserBuildHwndList,
	HDESK in_hDesk,
	HWND  in_hWnd,
	BOOL  in_EnumChildren,
	BOOL  in_RemoveImmersive,
	UINT  in_ThreadID,
	INT* out_Cnt
)
{
	/* locals */
	UINT  lv_Max;
	UINT  lv_Cnt;
	UINT  lv_NtStatus;
	HWND* lv_List;

	// initial size of list
	lv_Max = 512;

	// retry to get list
	for (;;)
	{
		// allocate list
		if ((lv_List = (HWND*)malloc(lv_Max * sizeof(HWND))) == NULL)
			break;

		// call the api
		lv_NtStatus = pNtUserBuildHwndList(
			in_hDesk, in_hWnd,
			in_EnumChildren, in_RemoveImmersive, in_ThreadID,
			lv_Max, lv_List, &lv_Cnt);

		// success?
		if (lv_NtStatus == NOERROR)
			break;

		// free allocated list
		free(lv_List);

		// clear
		lv_List = NULL;

		// other error then buffersize? or no increase in size?
		if (lv_NtStatus != STATUS_BUFFER_TOO_SMALL || lv_Cnt <= lv_Max)
			break;

		// update max plus some extra to take changes in number of windows into account
		lv_Max = lv_Cnt + 16;
	}

	// return the count
	*out_Cnt = lv_Cnt;

	// return the list, or NULL when failed
	return lv_List;
}

static void _sws_WindowHelpers_FindWindowExReverseOrder(HWND hWnd, WNDENUMPROC in_Proc, LPARAM in_Param)
{
	hWnd = FindWindowEx(NULL, hWnd, NULL, NULL);
	if (!hWnd)
	{
		return;
	}
	test(hWnd, in_Proc, in_Param);
	if (!in_Proc(hWnd, in_Param))
		return;
}

/********************************************************/
/* enumerate all top level windows including metro apps */
/********************************************************/
sws_error_t sws_WindowHelpers_RealEnumWindows(
	WNDENUMPROC in_Proc,
	LPARAM in_Param
)
{
	if (!_sws_pNtUserBuildHwndList)
	{
		return sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_NOT_INITIALIZED), NULL);
	}

	/* locals */
	INT   lv_Cnt;
	HWND  lv_hWnd;
	BOOL  lv_Result;
	HWND  lv_hFirstWnd;
	HWND  lv_hDeskWnd;
	HWND* lv_List;

	// no error yet
	lv_Result = TRUE;

	// first try api to get full window list including immersive/metro apps
	lv_List = _sws_WindowHelpers_Gui_BuildWindowList(_sws_pNtUserBuildHwndList, 0, 0, 0, 0, 0, &lv_Cnt);

	// success?
	if (lv_List)
	{
		// loop through list
		while (lv_Cnt-- > 0 && lv_Result)
		{
			// get handle
			lv_hWnd = lv_List[lv_Cnt];

			// filter out the invalid entry (0x00000001) then call the callback
			if (IsWindow(lv_hWnd))
				lv_Result = in_Proc(lv_hWnd, in_Param);
		}

		// free the list
		free(lv_List);
	}
	else
	{
		// get desktop window, this is equivalent to specifying NULL as hwndParent
		lv_hDeskWnd = GetDesktopWindow();

		// fallback to using FindWindowEx, get first top-level window
		lv_hFirstWnd = FindWindowEx(lv_hDeskWnd, 0, 0, 0);

		// init the enumeration
		lv_Cnt = 0;
		lv_hWnd = lv_hFirstWnd;

		// loop through windows found
		// - since 2012 the EnumWindows API in windows has a problem (on purpose by MS)
		//   that it does not return all windows (no metro apps, no start menu etc)
		// - luckally the FindWindowEx() still is clean and working
		while (lv_hWnd && lv_Result)
		{
			// call the callback
			lv_Result = in_Proc(lv_hWnd, in_Param);

			// get next window
			lv_hWnd = FindWindowEx(lv_hDeskWnd, lv_hWnd, 0, 0);

			// protect against changes in window hierachy during enumeration
			if (lv_hWnd == lv_hFirstWnd || lv_Cnt++ > 10000)
				break;
		}
	}

	// return the result
	//return lv_Result;
	return SWS_ERROR_SUCCESS;
}

BOOL _sws_WindowHelpers_ShouldTreatShellManagedWindowAsNotShellManaged(HWND hWnd)
{
	return (GetPropW(hWnd, L"Microsoft.Windows.ShellManagedWindowAsNormalWindow") != NULL);
}

BOOL _sws_WindowHelpers_IsWindowA920(HWND hWnd)
{
	return GetPropW(hWnd, (LPCWSTR)0xA920);
}

BOOL _sws_WindowHelpers_IsValidDesktopZOrderBand(HWND hWnd, int bAdditionalChecks)
{
	BOOL bRet = FALSE;
	DWORD dwBand = ZBID_DEFAULT;
	if (_sws_GetWindowBand && _sws_GetWindowBand(hWnd, &dwBand))
	{
		wchar_t wszWindowText[200];
		GetWindowTextW(hWnd, wszWindowText, 200);
		bRet = _sws_IsBandValidToInclude[dwBand];
		if (_sws_WindowHelpers_IsWindowA920(hWnd))
		{
			bRet = TRUE;
		}
		else if (!bRet)
		{
			return bRet;
		}
		if (bAdditionalChecks)
		{
			//wprintf(L"%s %d %d\n", wszWindowText, dwBand, _sws_IsShellManagedWindow(hWnd));
			return !(_sws_IsShellManagedWindow && _sws_IsShellManagedWindow(hWnd)) || _sws_WindowHelpers_ShouldTreatShellManagedWindowAsNotShellManaged(hWnd) || sws_WindowHelpers_IsWindowUWP(hWnd);
		}
	}
	return bRet;
}

BOOL _sws_WindowHelpers_IsWindowNotDesktopOrTrayHelper(HWND hWnd)
{
	ATOM SecondaryTaskbarAtom = RegisterWindowMessageW(L"Shell_SecondaryTrayWnd");
	if (GetClassWord(hWnd, GCW_ATOM) == SecondaryTaskbarAtom)
	{
		return (SecondaryTaskbarAtom == 0);
	}
	return TRUE;
}

BOOL _sws_WindowHelpers_IsWindowNotDesktopOrTray(HWND hWnd, HWND hWndDesktop)
{
	if (IsWindow(hWnd) &&
		hWnd != FindWindowW(L"Shell_TrayWnd", NULL) &&
		hWnd != hWndDesktop)
	{
		return _sws_WindowHelpers_IsWindowNotDesktopOrTrayHelper(hWnd);
	}
	return FALSE;
}

BOOL _sws_WindowHelpers_IsValidTabWindowForTray(HWND hWnd)
{
	if (!hWnd)
	{
		return FALSE;
	}
	if (!_sws_WindowHelpers_IsWindowA920(hWnd))
	{
		return TRUE;
	}
	IPropertyStore* propStore = NULL;
	SHGetPropertyStoreForWindow(
		hWnd,
		&__uuidof_IPropertyStore,
		&propStore
	);
	if (propStore)
	{
		PROPERTYKEY pKey;
		pKey.fmtid = __uuidof_AppUserModelIdProperty;
		pKey.pid = 5;
		PROPVARIANT prop;
		ZeroMemory(&prop, sizeof(PROPVARIANT));
		if (SUCCEEDED(propStore->lpVtbl->GetValue(propStore, &pKey, &prop))) PropVariantClear(&prop);
		propStore->lpVtbl->Release(propStore);
		IShellItem2* item;
		HRESULT hr = SHCreateItemInKnownFolder(
			&FOLDERID_AppsFolder,
			KF_FLAG_DONT_VERIFY,
			prop.bstrVal,
			&__uuidof_IShellItem2,
			&item
		);
		if (SUCCEEDED(hr))
		{
			item->lpVtbl->Release(item);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL _sws_WindowHelpers_ShouldAddWindowToTrayHelper(HWND hWnd)
{
	LONG WinExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
	if (_sws_WindowHelpers_IsValidTabWindowForTray(hWnd) &&
		(GetWindow(hWnd, GW_OWNER) == NULL || (WinExStyle & WS_EX_APPWINDOW)) &&
		!(WinExStyle & WS_EX_TOOLWINDOW))
	{
		return TRUE;
	}
	return FALSE;
}

BOOL _sws_TestExStyle(HWND hWnd, DWORD dwExStyle)
{
	return dwExStyle == (dwExStyle & (DWORD)GetWindowLongPtrW(hWnd, GWL_EXSTYLE));
}

BOOLEAN _sws_IsOwnerToolWindow(HWND hwnd)
{
	BOOLEAN bRet = FALSE;

	HWND hwndCurrent = hwnd;
	HWND hwndOwner = GetWindow(hwnd, GW_OWNER);
	while (!_sws_TestExStyle(hwndCurrent, WS_EX_APPWINDOW) && hwndOwner)
	{
		HWND hwndPrev = hwndCurrent;
		hwndCurrent = hwndOwner;
		hwndOwner = GetWindow(hwndOwner, GW_OWNER);
		if (_sws_TestExStyle(hwndCurrent, WS_EX_TOOLWINDOW))
		{
			bRet = !_sws_TestExStyle(hwndPrev, WS_EX_CONTROLPARENT) || hwndOwner != NULL;
			break;
		}
	}

	return bRet;
}

BOOL _sws_IsReallyVisible(HWND hWnd)
{
	RECT rc;
	GetWindowRect(hWnd, &rc);
	return IsWindowVisible(hWnd) && !IsRectEmpty(&rc);
}

BOOL _sws_IsGhosted(HWND hwnd)
{
	return _sws_GhostWindowFromHungWindow(hwnd) != NULL;
}

BOOL _sws_ShouldListWindowInAltTab(HWND hwnd)
{
	BOOL bRet = FALSE;

	if (IsWindow(hwnd) /*&& hwnd != _hwnd*/)
	{
		DWORD dwExStyle = (DWORD)GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
		HWND hwndOwner = GetWindow(hwnd, GW_OWNER);
		BOOLEAN bOwnerVisible = IsWindow(hwndOwner) && IsWindowEnabled(hwndOwner) && _sws_IsReallyVisible(hwndOwner);
		BOOLEAN bNoActivate = (dwExStyle & WS_EX_NOACTIVATE) != 0 || (dwExStyle & WS_EX_TOOLWINDOW) != 0;
		BOOLEAN bAppWindow = (dwExStyle & WS_EX_APPWINDOW) != 0;
		if (bAppWindow)
		{
			bNoActivate = FALSE;
		}
		bRet = _sws_IsReallyVisible(hwnd)
			// && IsWindowEnabled(hwnd)
			&& !bNoActivate
			&& (bAppWindow || !bOwnerVisible && !_sws_IsOwnerToolWindow(hwnd))
			&& !_sws_IsGhosted(hwnd);
	}

	return bRet;
}

BOOL _sws__IsTaskWindow(HWND hwnd)
{
	DWORD dwExStyle = (DWORD)GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
	return ((dwExStyle & WS_EX_APPWINDOW) != 0 || ((dwExStyle & WS_EX_TOOLWINDOW) == 0 && (dwExStyle & WS_EX_NOACTIVATE) == 0))
		&& IsWindowVisible(hwnd)
		&& !_sws_IsGhosted(hwnd);
}

BOOL _sws_IsTaskWindow(HWND hwnd, HWND* phwndTaskWindow)
{
	BOOL bRet = FALSE;

	if (_sws_ShouldListWindowInAltTab(hwnd))
	{
		HWND hwndTaskWindow = hwnd;
		HWND hwndCurrent = hwnd;
		while ((hwndCurrent = GetWindow(hwndCurrent, GW_OWNER)))
		{
			if (!_sws__IsTaskWindow(hwndCurrent))
				break;
			hwndTaskWindow = hwndCurrent;
		}
		*phwndTaskWindow = hwndTaskWindow;
		bRet = TRUE;
	}

	return bRet;
}

wchar_t* sws_WindowHelpers_GetAUMIDForHWND(HWND hWnd)
{
	WCHAR* pszAppId;
	if (SUCCEEDED(sws_AppResolver->lpVtbl->GetAppIDForWindow(sws_AppResolver, hWnd, &pszAppId, NULL, NULL, NULL)) && pszAppId) return pszAppId;
	return NULL;
}

BOOL sws_WindowHelpers_IsTaskbarWindow(HWND hWnd, HWND hWndWallpaper)
{
	if (_sws_WindowHelpers_IsValidDesktopZOrderBand(hWnd, TRUE) &&
		_sws_WindowHelpers_IsWindowNotDesktopOrTray(hWnd, hWndWallpaper) &&
		IsWindowVisible(hWnd) &&
		!_sws_HungWindowFromGhostWindow(hWnd))
	{
		return _sws_WindowHelpers_ShouldAddWindowToTrayHelper(hWnd);
	}
	return FALSE;
}

BOOL sws_WindowHelpers_IsWindowShellManagedByExplorerPatcher(HWND hWnd)
{
	return GetPropW(hWnd, L"valinet.ExplorerPatcher.ShellManagedWindow") != 0;
}

BOOL sws_WindowHelpers_ShouldTreatShellManagedWindowAsNotShellManaged(HWND hWnd)
{
	return GetPropW(hWnd, L"Microsoft.Windows.ShellManagedWindowAsNormalWindow") != 0;
}

BOOL sws_WindowHelpers_IsAltTabWindow(HWND hWnd)
{
	// This identifies whether a window is a shell frame and includes those
	// A shell frame corresponds to, as far as I can tell, the frame of a UWP app
	// and we want those in the Alt-Tab list
	// Bugfix: Exclude hung shell frame (immersive) UWP windows, as we already include
	// ghost app windows in their place already
	if (sws_IsShellFrameWindow(hWnd) && !_sws_GhostWindowFromHungWindow(hWnd))
	{
		return TRUE;
	}
	// Next, we need to check whether the window is shell managed and exclude it if so
	// Shell managed windows, as far as I can tell, represent all immersive UI the
	// Windows shell might present the user with, like: Start menu, Search (Win+Q),
	// notifications, taskbars etc
	if (_sws_IsShellManagedWindow(hWnd) && !sws_WindowHelpers_ShouldTreatShellManagedWindowAsNotShellManaged(hWnd))
	{
		return FALSE;
	}
	// Also, exclude some windows created by ExplorerPatcher
	if (sws_WindowHelpers_IsWindowShellManagedByExplorerPatcher(hWnd))
	{
		return FALSE;
	}
	// Lastly, this check works with the remaining classic window and determines if it is a
	// "task window" and only includes it in Alt-Tab if so; this check is taken from
	// "AltTab.dll" in Windows 7 and this is how that OS decided to include a window in its
	// window switcher
	HWND hwndTaskWindow = NULL;
	return _sws_IsTaskWindow(hWnd, &hwndTaskWindow);
}

void sws_WindowHelpers_GetDesktopText(wchar_t* wszTitle)
{
	HANDLE hExplorerFrame = GetModuleHandleW(L"ExplorerFrame.dll");
	if (hExplorerFrame)
	{
		LoadStringW(hExplorerFrame, 13140, wszTitle, MAX_PATH);
	}
	else
	{
		wcscat_s(wszTitle, MAX_PATH, L"Desktop");
	}
}

static BOOL CALLBACK _sws_WindowHelpers_GetWallpaperHWNDCallback(HWND hwnd, LPARAM lParam)
{
	HWND* ret = (HWND*)lParam;

	HWND p = FindWindowExW(hwnd, NULL, L"SHELLDLL_DefView", NULL);
	if (p)
	{
		HWND t = FindWindowExW(NULL, hwnd, L"WorkerW", NULL);
		if (t)
		{
			*ret = t;
		}
	}
	return TRUE;
}

BOOL sws_WindowHelpers_EnsureWallpaperHWND()
{
	BOOL bRet = FALSE;

	// See: https://github.com/valinet/ExplorerPatcher/issues/525
	HWND hProgman = GetShellWindow();
	if (hProgman)
	{
		DWORD_PTR res0 = -1, res1 = -1, res2 = -1, res3 = -1;
		// CDesktopBrowser::_IsDesktopWallpaperInitialized and CWallpaperRenderer::ExpireImages
		SendMessageTimeoutW(hProgman, 0x052C, 10, 0, SMTO_NORMAL, 1000, &res0);
		if (SUCCEEDED(res0))
		{
			// Generate wallpaper window
			SendMessageTimeoutW(hProgman, 0x052C, 13, 0, SMTO_NORMAL, 1000, &res1);
			SendMessageTimeoutW(hProgman, 0x052C, 13, 1, SMTO_NORMAL, 1000, &res2);
			SendMessageTimeoutW(hProgman, 0x052C, 0, 0, SMTO_NORMAL, 1000, &res3);
			bRet = !res1 && !res2 && !res3;
		}
	}

	return bRet;
}

HWND sws_WindowHelpers_GetWallpaperHWND()
{
	HWND hWnd = NULL;

	// 24H2 has the Progman window as the whole desktop (icons + wallpaper)
	// Check if SHELLDLL_DefView is a direct child of Progman
	HWND hWndProgman = GetShellWindow();
	if (hWndProgman)
	{
		HWND hWndDefView = FindWindowExW(hWndProgman, NULL, L"SHELLDLL_DefView", NULL);
		if (hWndDefView)
		{
			hWnd = hWndProgman; // We're running on 24H2 or newer
		}
	}

	// Otherwise find a top-level WorkerW window with a SHELLDLL_DefView child, that's our desktop icons
	if (!hWnd)
	{
		EnumWindows(_sws_WindowHelpers_GetWallpaperHWNDCallback, &hWnd);
	}

	return hWnd;
}

BOOL CALLBACK sws_WindowHelpers_AddAltTabWindowsToTimeStampedHWNDList(HWND hWnd, HDPA hdpa)
{
	if (!hdpa)
	{
		return FALSE;
	}
	if (sws_WindowHelpers_IsAltTabWindow(hWnd))
	{
		sws_tshwnd* tshWnd = malloc(sizeof(sws_tshwnd));
		if (tshWnd)
		{
			sws_tshwnd_Initialize(tshWnd, hWnd);
			sws_tshwnd_ModifyTimestamp(tshWnd, sws_WindowHelpers_GetStartTime());
			DPA_AppendPtr(hdpa, tshWnd);
		}
	}
	return TRUE;
}

// https://stackoverflow.com/questions/5309914/updatelayeredwindow-and-drawtext
HBITMAP sws_WindowHelpers_CreateAlphaTextBitmap(LPCWSTR inText, HFONT inFont, DWORD dwTextFlags, SIZE size, COLORREF inColour)
{
	// Create DC and select font into it 
	HDC hTextDC = CreateCompatibleDC(NULL);
	HFONT hOldFont = (HFONT)SelectObject(hTextDC, inFont);
	HBITMAP hMyDIB = NULL;

	// Get text area 
	RECT TextArea = { 0, 0, size.cx, size.cy };
	if ((TextArea.right > TextArea.left) && (TextArea.bottom > TextArea.top))
	{
		BITMAPINFOHEADER BMIH;
		memset(&BMIH, 0x0, sizeof(BITMAPINFOHEADER));
		void* pvBits = NULL;

		// Specify DIB setup 
		BMIH.biSize = sizeof(BMIH);
		BMIH.biWidth = TextArea.right - TextArea.left;
		BMIH.biHeight = TextArea.bottom - TextArea.top;
		BMIH.biPlanes = 1;
		BMIH.biBitCount = 32;
		BMIH.biCompression = BI_RGB;

		// Create and select DIB into DC 
		hMyDIB = CreateDIBSection(hTextDC, (LPBITMAPINFO)&BMIH, 0, (LPVOID*)&pvBits, NULL, 0);
		HBITMAP hOldBMP = (HBITMAP)SelectObject(hTextDC, hMyDIB);
		if (hOldBMP != NULL)
		{
			// Set up DC properties 
			SetTextColor(hTextDC, 0x00FFFFFF);
			SetBkColor(hTextDC, 0x00000000);
			SetBkMode(hTextDC, OPAQUE);

			// Draw text to buffer 
			DrawTextW(hTextDC, inText, -1, &TextArea, dwTextFlags);
			BYTE* DataPtr = (BYTE*)pvBits;
			BYTE FillR = GetRValue(inColour);
			BYTE FillG = GetGValue(inColour);
			BYTE FillB = GetBValue(inColour);
			BYTE ThisA;
			for (int LoopY = 0; LoopY < BMIH.biHeight; LoopY++) {
				for (int LoopX = 0; LoopX < BMIH.biWidth; LoopX++) {
					ThisA = *DataPtr; // Move alpha and pre-multiply with RGB 
					*DataPtr++ = (FillB * ThisA) >> 8;
					*DataPtr++ = (FillG * ThisA) >> 8;
					*DataPtr++ = (FillR * ThisA) >> 8;
					*DataPtr++ = ThisA; // Set Alpha 
				}
			}

			// De-select bitmap 
			SelectObject(hTextDC, hOldBMP);
		}
	}

	// De-select font and destroy temp DC 
	SelectObject(hTextDC, hOldFont);
	DeleteDC(hTextDC);

	// Return DIBSection 
	return hMyDIB;
}

BOOL sws_WindowHelpers_AreAnimationsAllowed()
{
	if (sws_SHWindowsPolicy(&POLID_TurnOffSPIAnimations))
	{
		return FALSE;
	}

	BOOL bAnimationsEnabled = FALSE;
	SystemParametersInfoW(SPI_GETCLIENTAREAANIMATION, 0, &bAnimationsEnabled, 0);
	return bAnimationsEnabled;
}

void sws_WindowHelpers_GetWindowText(HWND hWnd, LPCWSTR lpWStr, DWORD dwLength)
{
	HWND hWndGhost = _sws_GhostWindowFromHungWindow(hWnd);
	if (hWndGhost)
	{
		sws_InternalGetWindowText(hWndGhost, lpWStr, dwLength);
	}
	else
	{
		sws_InternalGetWindowText(hWnd, lpWStr, dwLength);
	}
}

HWND sws_WindowHelpers_GetLastActivePopup(HWND hWnd)
{
	HWND hOwner = GetWindow(hWnd, GW_OWNER);
	return GetLastActivePopup(hOwner ? hOwner : hWnd);
}

void sws_WindowHelpers_Clear()
{
	GdiplusShutdown(_sws_gdiplus_token);
	_sws_gdiplus_token = 0;
	if (sws_DefAppIcon)
	{
		DestroyIcon(sws_DefAppIcon);
		sws_DefAppIcon = NULL;
	}
	if (_sws_hWin32u)
	{
		FreeLibrary(_sws_hWin32u);
		_sws_hWin32u = NULL;
	}
	if (_sws_hUser32)
	{
		FreeLibrary(_sws_hUser32);
		_sws_hUser32 = NULL;
	}
	if (_sws_hUxtheme)
	{
		FreeLibrary(_sws_hUxtheme);
		_sws_hUxtheme = NULL;
	}
	if (_sws_hShlwapi)
	{
		FreeLibrary(_sws_hShlwapi);
		_sws_hShlwapi = NULL;
	}
	if (_sws_hComctl32)
	{
		FreeLibrary(_sws_hComctl32);
		_sws_hComctl32 = NULL;
	}
	if (_sws_hShcore)
	{
		FreeLibrary(_sws_hShcore);
		_sws_hShcore = NULL;
	}
	if (sws_AppResolver)
	{
		sws_AppResolver->lpVtbl->Release(sws_AppResolver);
		sws_AppResolver = NULL;
	}
}

sws_error_t sws_WindowHelpers_Initialize()
{
	sws_error_t rv = SWS_ERROR_SUCCESS;

	if (_sws_gdiplus_token)
	{
		return rv;
	}
	sws_global_ubr = sws_WindowHelpers_GetOSVersionAndUBR(&sws_global_rovi);
	if (sws_global_rovi.dwMajorVersion == 0)
	{
		return SWS_ERROR_GENERIC_ERROR;
	}
	GetSystemTimeAsFileTime(&sws_ancient_ft);
	GetSystemTimeAsFileTime(&sws_start_ft);
	if (!rv)
	{
		_sws_hComctl32 = LoadLibraryW(L"Comctl32.dll");
		if (!_sws_hComctl32)
		{
			rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_LOADLIBRARY_FAILED), NULL);
		}
	}
	if (!rv)
	{
		sws_LoadIconWithScaleDown = GetProcAddress(_sws_hComctl32, "LoadIconWithScaleDown");
		if (!sws_LoadIconWithScaleDown)
		{
			rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_FUNCTION_NOT_FOUND), NULL);
		}
	}
	if (!rv)
	{
		sws_LoadIconWithScaleDown(
			(HINSTANCE)NULL,
			(PCWSTR)MAKEINTRESOURCEW(IDI_APPLICATION),
			(int)128,
			(int)128,
			(HICON*)(&(sws_DefAppIcon))
		);
		sws_LoadIconWithScaleDown(
			(HINSTANCE)NULL,
			(PCWSTR)MAKEINTRESOURCEW(IDI_APPLICATION),
			(int)128,
			(int)128,
			(HICON*)(&(sws_LegacyDefAppIcon))
		);
		//sws_LegacyDefAppIcon = LoadIconW(NULL, MAKEINTRESOURCEW(IDI_APPLICATION));
	}
	if (!rv)
	{
		UINT32 gdiplusStartupInput[100];
		ZeroMemory(gdiplusStartupInput, 100);
		gdiplusStartupInput[0] = 1;
		rv = sws_error_Report(sws_error_GetFromGdiplusStatus(GdiplusStartup(&_sws_gdiplus_token, &gdiplusStartupInput)), NULL);
	}
	if (!rv)
	{
		_sws_hShlwapi = LoadLibraryW(L"Shlwapi.dll");
		if (!_sws_hShlwapi)
		{
			rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_LOADLIBRARY_FAILED), NULL);
		}
	}
	if (!rv)
	{
		sws_SHRegGetValueFromHKCUHKLM = GetProcAddress(_sws_hShlwapi, "SHRegGetValueFromHKCUHKLM");
		if (!sws_SHRegGetValueFromHKCUHKLM)
		{
			rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_FUNCTION_NOT_FOUND), NULL);
		}

	}
	if (!rv)
	{
		if (!_sws_hWin32u)
		{
			_sws_hWin32u = LoadLibraryW(L"win32u.dll");
			if (!_sws_hWin32u)
			{
				rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_LOADLIBRARY_FAILED), NULL);
			}
		}
	}
	if (!rv)
	{
		if (!_sws_pNtUserBuildHwndList)
		{
			_sws_pNtUserBuildHwndList = (NtUserBuildHwndList)GetProcAddress(_sws_hWin32u, "NtUserBuildHwndList");
			if (!_sws_pNtUserBuildHwndList)
			{
				rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_FUNCTION_NOT_FOUND), NULL);
			}
		}
	}
	if (!rv)
	{
		if (!_sws_hUser32)
		{
			_sws_hUser32 = LoadLibraryW(L"user32.dll");
			if (!_sws_hUser32)
			{
				rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_LOADLIBRARY_FAILED), NULL);
			}
		}
	}
	if (!rv)
	{
		if (!_sws_HungWindowFromGhostWindow)
		{
			_sws_HungWindowFromGhostWindow = (pHungWindowFromGhostWindow)GetProcAddress(_sws_hUser32, "HungWindowFromGhostWindow");
			if (!_sws_HungWindowFromGhostWindow)
			{
				rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_FUNCTION_NOT_FOUND), NULL);
			}
		}
	}
	if (!rv)
	{
		if (!_sws_GhostWindowFromHungWindow)
		{
			_sws_GhostWindowFromHungWindow = (pGhostWindowFromHungWindow)GetProcAddress(_sws_hUser32, "GhostWindowFromHungWindow");
			if (!_sws_GhostWindowFromHungWindow)
			{
				rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_FUNCTION_NOT_FOUND), NULL);
			}
		}
	}
	if (!rv)
	{
		if (!_sws_SetWindowCompositionAttribute)
		{
			_sws_SetWindowCompositionAttribute = (pSetWindowCompositionAttribute)GetProcAddress(_sws_hUser32, "SetWindowCompositionAttribute");
			if (!_sws_SetWindowCompositionAttribute)
			{
				rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_FUNCTION_NOT_FOUND), NULL);
			}
		}
	}
	if (!rv)
	{
		if (!_sws_IsShellManagedWindow)
		{
			_sws_IsShellManagedWindow = (pSetWindowCompositionAttribute)GetProcAddress(_sws_hUser32, (LPCSTR)2574);
			if (!_sws_IsShellManagedWindow)
			{
				rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_FUNCTION_NOT_FOUND), NULL);
			}
		}
	}
	if (!rv)
	{
		if (!sws_IsShellFrameWindow)
		{
			sws_IsShellFrameWindow = (pSetWindowCompositionAttribute)GetProcAddress(_sws_hUser32, (LPCSTR)2573);
			if (!sws_IsShellFrameWindow)
			{
				rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_FUNCTION_NOT_FOUND), NULL);
			}
		}
	}
	if (!rv)
	{
		if (!_sws_CreateWindowInBand)
		{
			_sws_CreateWindowInBand = (pCreateWindowInBand)GetProcAddress(_sws_hUser32, "CreateWindowInBand");
			if (!_sws_CreateWindowInBand)
			{
				rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_FUNCTION_NOT_FOUND), NULL);
			}
		}
	}
	if (!rv)
	{
		if (!_sws_GetWindowBand)
		{
			_sws_GetWindowBand = GetProcAddress(_sws_hUser32, "GetWindowBand");
			if (!_sws_GetWindowBand)
			{
				rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_FUNCTION_NOT_FOUND), NULL);
			}
		}
	}
	if (!rv)
	{
		if (!_sws_SetWindowBand)
		{
			_sws_SetWindowBand = GetProcAddress(_sws_hUser32, "SetWindowBand");
			if (!_sws_SetWindowBand)
			{
				rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_FUNCTION_NOT_FOUND), NULL);
			}
		}
	}
	if (!rv)
	{
		if (!_sws_IsTopLevelWindow)
		{
			_sws_IsTopLevelWindow = GetProcAddress(_sws_hUser32, "IsTopLevelWindow");
			if (!_sws_IsTopLevelWindow)
			{
				rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_FUNCTION_NOT_FOUND), NULL);
			}
		}
	}
	/*if (!rv)
	{
		if (!_sws_GetProgmanWindow)
		{
			_sws_GetProgmanWindow = GetProcAddress(_sws_hUser32, "GetProgmanWindow");
			if (!_sws_GetProgmanWindow)
			{
				rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_FUNCTION_NOT_FOUND), NULL);
			}
		}
	}*/
	if (!rv)
	{
		if (!sws_InternalGetWindowText)
		{
			sws_InternalGetWindowText = GetProcAddress(_sws_hUser32, "InternalGetWindowText");
			if (!sws_InternalGetWindowText)
			{
				rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_FUNCTION_NOT_FOUND), NULL);
			}
		}
	}
	if (!rv)
	{
		if (!_sws_InternalGetWindowIcon)
		{
			_sws_InternalGetWindowIcon = GetProcAddress(_sws_hUser32, "InternalGetWindowIcon");
			if (!_sws_InternalGetWindowIcon)
			{
				rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_FUNCTION_NOT_FOUND), NULL);
			}
		}
	}
	/*if (!rv)
	{
		if (!_sws_IsCoreWindow)
		{
			_sws_IsCoreWindow = GetProcAddress(_sws_hUser32, (LPCSTR)2572);
			if (!_sws_IsCoreWindow)
			{
				rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_FUNCTION_NOT_FOUND), NULL);
			}
		}
	}*/
	if (!rv)
	{
		if (!_sws_hUxtheme)
		{
			_sws_hUxtheme = LoadLibraryW(L"uxtheme.dll");
			if (!_sws_hUxtheme)
			{
				rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_LOADLIBRARY_FAILED), NULL);
			}
		}
	}
	if (!rv)
	{
		if (!_sws_ShouldSystemUseDarkMode)
		{
			_sws_ShouldSystemUseDarkMode = GetProcAddress(_sws_hUxtheme, (LPCSTR)138);
			if (!_sws_ShouldSystemUseDarkMode)
			{
				rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_FUNCTION_NOT_FOUND), NULL);
			}
		}
	}
	if (!rv)
	{
		if (!_sws_SetPreferredAppMode)
		{
			_sws_SetPreferredAppMode = GetProcAddress(_sws_hUxtheme, (LPCSTR)135);
			if (!_sws_SetPreferredAppMode)
			{
				rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_FUNCTION_NOT_FOUND), NULL);
			}
		}
	}
	if (!rv)
	{
		if (!_sws_AllowDarkModeForWindow)
		{
			_sws_AllowDarkModeForWindow = GetProcAddress(_sws_hUxtheme, (LPCSTR)133);
			if (!_sws_AllowDarkModeForWindow)
			{
				rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_FUNCTION_NOT_FOUND), NULL);
			}
		}
	}
	if (!rv)
	{
		if (!_sws_RefreshImmersiveColorPolicyState)
		{
			_sws_RefreshImmersiveColorPolicyState = GetProcAddress(_sws_hUxtheme, (LPCSTR)104);
			if (!_sws_RefreshImmersiveColorPolicyState)
			{
				rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_FUNCTION_NOT_FOUND), NULL);
			}
		}
	}
	if (!rv)
	{
		if (!_sws_hShcore)
		{
			_sws_hShcore = LoadLibraryW(L"shcore.dll");
			if (!_sws_hShcore)
			{
				rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_LOADLIBRARY_FAILED), NULL);
			}
		}
	}
	if (!rv)
	{
		if (!sws_SHWindowsPolicy)
		{
			sws_SHWindowsPolicy = GetProcAddress(_sws_hShcore, (LPCSTR)190);
			if (!sws_SHWindowsPolicy)
			{
				rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_FUNCTION_NOT_FOUND), NULL);
			}
		}
	}
	if (!rv)
	{
		if (!sws_AppResolver)
		{
			CoCreateInstance(&CLSID_StartMenuCacheAndAppResolver, NULL, CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER, &IID_IAppResolver_8, (void**)&sws_AppResolver);
			if (!sws_AppResolver)
			{
				rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_APPRESOLVER_NOT_AVAILABLE), NULL);
			}
		}
	}
	return rv;
}
