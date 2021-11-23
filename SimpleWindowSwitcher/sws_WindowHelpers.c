#include "sws_WindowHelpers.h"

NtUserBuildHwndList _sws_pNtUserBuildHwndList;
pCreateWindowInBand _sws_CreateWindowInBand;
pSetWindowCompositionAttribute _sws_SetWindowCompositionAttribute;
pIsShellManagedWindow _sws_IsShellManagedWindow;
pGetWindowBand _sws_GetWindowBand;
pSetWindowBand _sws_SetWindowBand;
BOOL(*_sws_ShouldSystemUseDarkMode)();
void(*_sws_RefreshImmersiveColorPolicyState)();
void(*_sws_SetPreferredAppMode)(INT64);
void(*_sws_AllowDarkModeForWindow)(HWND hWnd, INT64 bAllowDark);
HMODULE _sws_hWin32u = 0;
HINSTANCE _sws_hUser32 = 0;
HINSTANCE _sws_hUxtheme = 0;

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
		propStore->lpVtbl->GetValue(propStore, &pKey, &prop);
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

BOOL sws_WindowHelpers_IsAltTabWindow(
	_In_ HWND hwnd,
	_In_ HWND hWndWallpaper
)
{


	if (_sws_WindowHelpers_IsValidDesktopZOrderBand(hwnd, TRUE) &&
		_sws_WindowHelpers_IsWindowNotDesktopOrTray(hwnd, hWndWallpaper) &&
		IsWindowVisible(hwnd))
	{
		return _sws_WindowHelpers_ShouldAddWindowToTrayHelper(hwnd);
	}
	return FALSE;
	
	BOOL isCloaked;
	DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &isCloaked, sizeof(BOOL));
	if (isCloaked)
	{
		return FALSE;
	}


	const LONG_PTR windowLong = GetWindowLongPtrW(hwnd, GWL_STYLE);
	if (!(windowLong & WS_VISIBLE) || !(windowLong & WS_SYSMENU)) {
		return FALSE;
	}


	// https://forum.rehips.com/index.php?topic=9599.0
	if (IsWindowVisible(hwnd))
	{
		LONG WinExStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
		if ((GetWindow(hwnd, GW_OWNER) == NULL || (WinExStyle & WS_EX_APPWINDOW)) &&
			!(WinExStyle & WS_EX_TOOLWINDOW))
		{
			return TRUE;
		}
	}
	return FALSE;

	//TITLEBARINFO ti;
	HWND hwndTry, hwndWalk = NULL;

	wchar_t wszClassName[100];
	GetClassNameW(hwnd, wszClassName, 100);
	if (!wcscmp(wszClassName, L"#32770"))
	{
		// ??? somwhow allow Explorer dialog boxes
		// but restrict Notepad ones...
		return TRUE;
	}

	if (!IsWindowVisible(hwnd))
		return FALSE;

	hwndTry = GetAncestor(hwnd, GA_ROOTOWNER);
	while (hwndTry != hwndWalk)
	{
		hwndWalk = hwndTry;
		hwndTry = GetLastActivePopup(hwndWalk);
		if (IsWindowVisible(hwndTry))
			break;
	}
	if (hwndWalk != hwnd)
		return FALSE;

	// the following removes some task tray programs and "Program Manager"
	TITLEBARINFO ti;
	ti.cbSize = sizeof(ti);
	GetTitleBarInfo(hwnd, &ti);
	if (ti.rgstate[0] & STATE_SYSTEM_INVISIBLE)
		return FALSE;

	// Tool windows should not be displayed either, these do not appear in the
	// task bar.
	if (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)
		return FALSE;

	return TRUE;
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

__declspec(dllexport) HICON sws_WindowHelpers_GetIconFromHWND(HWND hWnd, BOOL* bOwnProcess, BOOL bIsDesktop, UINT* szIcon)
{
	HICON hIcon = NULL;

	wchar_t wszPath[MAX_PATH];
	ZeroMemory(wszPath, MAX_PATH * sizeof(wchar_t));

	if (bIsDesktop)
	{
		GetSystemDirectory(wszPath, MAX_PATH);
		wcscat_s(wszPath, MAX_PATH, L"\\imageres.dll");
		return ExtractIconW(
			GetModuleHandle(NULL),
			wszPath,
			-110
		);
	}

	DWORD dwHWndPid;
	GetWindowThreadProcessId(hWnd, &dwHWndPid);
	if (dwHWndPid == GetCurrentProcessId())
	{
		*bOwnProcess = TRUE;
	}

	DWORD dwProcessId, dwSize = MAX_PATH;
	GetWindowThreadProcessId(hWnd, &dwProcessId);
	HANDLE hProcess;
	hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwProcessId);
	if (hProcess)
	{
		////wchar_t exeName[MAX_PATH + 1];
		////QueryFullProcessImageNameW(GetCurrentProcess(), 0, exeName, &dwSize);
		////CharLowerW(exeName);
		//QueryFullProcessImageNameW(hProcess, 0, wszPath, dwSize);
		GetModuleFileNameExW((HMODULE)hProcess, NULL, wszPath, MAX_PATH);
		CharLowerW(wszPath);

		////if (!wcscmp(exeName, wszPath))
		////{
		////	*bOwnProcess = TRUE;
		////}

		////if (wcsstr(wszPath, L"applicationframehost.exe"))
		HRESULT hr = S_OK;
		IShellItemImageFactory* imageFactory = NULL;
		SIIGBF flags = SIIGBF_RESIZETOFIT;
		BOOL bIsUWP = sws_WindowHelpers_IsWindowUWP(hWnd);
		if (bIsUWP)
		{
			flags |= SIIGBF_ICONBACKGROUND;

			IPropertyStore* propStore = NULL;
			hr = SHGetPropertyStoreForWindow(
				hWnd,
				&__uuidof_IPropertyStore,
				&propStore
			);
			if (SUCCEEDED(hr))
			{
				PROPERTYKEY pKey;
				pKey.fmtid = __uuidof_AppUserModelIdProperty;
				pKey.pid = 5;
				PROPVARIANT prop;
				ZeroMemory(&prop, sizeof(PROPVARIANT));
				propStore->lpVtbl->GetValue(propStore, &pKey, &prop);
				propStore->lpVtbl->Release(propStore);
				if (prop.bstrVal)
				{
					SHCreateItemInKnownFolder(
						&FOLDERID_AppsFolder,
						KF_FLAG_DONT_VERIFY,
						prop.bstrVal,
						&__uuidof_IShellItemImageFactory,
						&imageFactory
					);
				}
			}
		}
		else
		{
			/*SHCreateItemFromParsingName(
				wszPath,
				NULL,
				&__uuidof_IShellItemImageFactory,
				&imageFactory
			);*/
		}

		if (imageFactory)
		{
			SIZE size;
			size.cx = *szIcon;
			size.cy = *szIcon;
			HBITMAP hBitmap;
			hr = imageFactory->lpVtbl->GetImage(
				imageFactory,
				size,
				flags,
				&hBitmap
			);
			if (SUCCEEDED(hr))
			{
				// Easiest way to get an HICON from an HBITMAP
				// I have turned the Internet upside down and was unable to find this
				// Only a convoluted example using GDI+
				// This is from the disassembly of StartIsBack/StartAllBack
				HIMAGELIST hImageList = ImageList_Create(size.cx, size.cy, ILC_COLOR32, 1, 0);
				if (ImageList_Add(hImageList, hBitmap, NULL) != -1)
				{
					hIcon = ImageList_GetIcon(hImageList, 0, 0);
					ImageList_Destroy(hImageList);
					if (bIsUWP) *szIcon = 0;
				}
				DeleteObject(hBitmap);
			}
		}
		if (!hIcon)
		{
			SendMessageTimeoutW(hWnd, WM_GETICON, ICON_BIG, 0, SMTO_ABORTIFHUNG, 1000, &hIcon);
		}
		if (!hIcon)
		{
			SendMessageTimeoutW(hWnd, WM_GETICON, ICON_SMALL, 0, SMTO_ABORTIFHUNG, 1000, &hIcon);
		}
		if (!hIcon)
		{
#ifdef _WIN64
			GetClassLongPtr(hWnd, -34);
#else
			GetClassLong(hWnd, -34);
#endif
		}
		if (!hIcon)
		{
#ifdef _WIN64
			GetClassLongPtr(hWnd, -14);
#else
			GetClassLong(hWnd, -14);
#endif
		}
		if (!hIcon)
		{
			SendMessageTimeoutW(hWnd, WM_QUERYDRAGICON, 0, 0, 0, 1000, &hIcon);
		}
		if (*bOwnProcess)
		{
			hIcon = CopyIcon(hIcon);
		}
		// here, if hIcon was supplied, one should set its alpha channel properly:
		// 255 for the actual icon contents
		// 0 for the rest (the background), probably with a mask
		if (!hIcon)
		{
			SHFILEINFOW shinfo;
			ZeroMemory(&shinfo, sizeof(SHFILEINFOW));
			SHGetFileInfoW(
				wszPath,
				FILE_ATTRIBUTE_NORMAL,
				&shinfo,
				sizeof(SHFILEINFOW),
				SHGFI_ICON
			);
			hIcon = shinfo.hIcon;
		}
		if (!hIcon)
		{
			hIcon = LoadIconW(GetModuleHandleW(NULL), IDI_APPLICATION);
		}
		CloseHandle(hProcess);
	}

	return hIcon;
}

static BOOL CALLBACK _sws_WindowHelpers_GetWallpaperHWNDCallback(_In_ HWND hwnd, _Out_ LPARAM lParam)
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

HWND sws_WindowHelpers_GetWallpaperHWND()
{
	HWND hWnd = NULL;
	HWND progman = FindWindowW(L"Progman", NULL);
	if (progman)
	{
		SendMessageTimeoutW(progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, NULL);
		EnumWindows(_sws_WindowHelpers_GetWallpaperHWNDCallback, &hWnd);
	}
	return hWnd;
}

void sws_WindowHelpers_Release()
{
	FreeLibrary(_sws_hWin32u);
	FreeLibrary(_sws_hUser32);
	FreeLibrary(_sws_hUxtheme);
}

sws_error_t sws_WindowHelpers_Initialize()
{
	sws_error_t rv = SWS_ERROR_SUCCESS;

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
	return rv;
}
