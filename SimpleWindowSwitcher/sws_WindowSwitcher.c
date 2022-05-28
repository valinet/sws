#include "sws_WindowSwitcher.h"

static void _sws_WindowSwitcher_UpdateAccessibleText(sws_WindowSwitcher* _this)
{
    sws_WindowSwitcherLayoutWindow* pWindowList = _this->layout.pWindowList.pList;
    if (pWindowList)
    {
        if (!_this->layout.pWindowList.cbSize)
        {
            SetWindowTextW(_this->hWndAccessible, L"");
        }
        else
        {
            WCHAR wszAccText[MAX_PATH * 2], wszTitle[MAX_PATH];
            ZeroMemory(wszAccText, MAX_PATH * 2 * sizeof(WCHAR));
            ZeroMemory(wszTitle, MAX_PATH * sizeof(WCHAR));
            if (_this->layout.bIncludeWallpaper && pWindowList[_this->layout.iIndex].hWnd == _this->layout.hWndWallpaper)
            {
                sws_WindowHelpers_GetDesktopText(wszTitle);
            }
            else
            {
                WCHAR wszRundll32Path[MAX_PATH];
                GetSystemDirectoryW(wszRundll32Path, MAX_PATH);
                wcscat_s(wszRundll32Path, MAX_PATH, L"\\rundll32.exe");
                if (_this->bAlwaysUseWindowTitleAndIcon || _this->mode != SWS_WINDOWSWITCHER_LAYOUTMODE_FULL || !_this->bSwitcherIsPerApplication || (pWindowList[_this->layout.iIndex].dwWindowFlags & SWS_WINDOWSWITCHERLAYOUT_WINDOWFLAGS_ISUWP) || !pWindowList[_this->layout.iIndex].wszPath || (pWindowList[_this->layout.iIndex].wszPath && !_wcsicmp(pWindowList[_this->layout.iIndex].wszPath, wszRundll32Path)))
                {
                    sws_WindowHelpers_GetWindowText(pWindowList[_this->layout.iIndex].hWnd, wszTitle, MAX_PATH);
                }
                else
                {
                    if (pWindowList[_this->layout.iIndex].dwCount > 1)
                    {
                        DWORD dwPrefixLen = 0;
                        //swprintf_s(wszTitle, MAX_PATH, L"%d: ", dwCount);
                        dwPrefixLen = 0;// wcslen(wszTitle);
                        WCHAR wszExplorerPath[MAX_PATH];
                        GetWindowsDirectoryW(wszExplorerPath, MAX_PATH);
                        wcscat_s(wszExplorerPath, MAX_PATH, L"\\explorer.exe");
                        if (pWindowList[_this->layout.iIndex].wszPath && !_wcsicmp(wszExplorerPath, pWindowList[_this->layout.iIndex].wszPath))
                        {
                            HANDLE hExplorer = GetModuleHandleW(NULL);
                            if (hExplorer)
                            {
                                LoadStringW(hExplorer, 6020, wszTitle + dwPrefixLen, MAX_PATH - dwPrefixLen);
                            }
                            if (!hExplorer || !wszTitle)
                            {
                                wcscat_s(wszTitle + dwPrefixLen, MAX_PATH - dwPrefixLen, L"File Explorer");
                            }
                        }
                        else
                        {
                            IShellItem2* pIShellItem2 = NULL;
                            if (SUCCEEDED(SHCreateItemFromParsingName(pWindowList[_this->layout.iIndex].wszPath, NULL, &IID_IShellItem2, &pIShellItem2)))
                            {
                                LPWSTR wszOutText = NULL;
                                if (SUCCEEDED(pIShellItem2->lpVtbl->GetString(pIShellItem2, &PKEY_FileDescription, &wszOutText)))
                                {
                                    int len = wcslen(wszOutText);
                                    if (len >= 4 && wszOutText[len - 1] == L'e' && wszOutText[len - 2] == L'x' && wszOutText[len - 3] == L'e' && wszOutText[len - 4] == L'.')
                                    {
                                        CoTaskMemFree(wszOutText);
                                        if (SUCCEEDED(pIShellItem2->lpVtbl->GetString(pIShellItem2, &PKEY_Software_ProductName, &wszOutText)))
                                        {
                                            wcscpy_s(wszTitle + dwPrefixLen, MAX_PATH - dwPrefixLen, wszOutText);
                                            CoTaskMemFree(wszOutText);
                                        }
                                        else
                                        {
                                            sws_WindowHelpers_GetWindowText(pWindowList[_this->layout.iIndex].hWnd, wszTitle + dwPrefixLen, MAX_PATH - dwPrefixLen);
                                        }
                                    }
                                    else
                                    {
                                        wcscpy_s(wszTitle + dwPrefixLen, MAX_PATH - dwPrefixLen, wszOutText);
                                        CoTaskMemFree(wszOutText);
                                    }
                                }
                                else
                                {
                                    sws_WindowHelpers_GetWindowText(pWindowList[_this->layout.iIndex].hWnd, wszTitle + dwPrefixLen, MAX_PATH - dwPrefixLen);
                                }
                                pIShellItem2->lpVtbl->Release(pIShellItem2);
                            }
                        }
                        WCHAR wszTitle2[MAX_PATH];
                        wcscpy_s(wszTitle2, MAX_PATH, wszTitle);

                        WCHAR wszFormat[MAX_PATH];
                        HANDLE hExplorer = GetModuleHandleW(NULL);
                        if (hExplorer)
                        {
                            if (pWindowList[_this->layout.iIndex].dwCount)
                            {
                                LoadStringW(hExplorer, 11115, wszFormat, MAX_PATH);
                            }
                            else
                            {
                                LoadStringW(hExplorer, 11114, wszFormat, MAX_PATH);
                            }
                        }
                        if (!hExplorer || !wszFormat)
                        {
                            if (pWindowList[_this->layout.iIndex].dwCount)
                            {
                                wcscat_s(wszFormat, MAX_PATH, L"%s - %d running windows");
                            }
                            else
                            {
                                wcscat_s(wszFormat, MAX_PATH, L"%s - 1 running window");
                            }

                        }
                        if (pWindowList[_this->layout.iIndex].dwCount)
                        {
                            swprintf_s(wszTitle, MAX_PATH, wszFormat, wszTitle2, pWindowList[_this->layout.iIndex].dwCount);
                        }
                        else
                        {
                            swprintf_s(wszTitle, MAX_PATH, wszFormat, wszTitle2);
                        }
                    }
                    else
                    {
                        sws_WindowHelpers_GetWindowText(pWindowList[_this->layout.iIndex].hWnd, wszTitle, MAX_PATH);
                    }
                }
            }
            swprintf_s(
                wszAccText,
                MAX_PATH * 2,
                L"%s: %d of %d",
                wszTitle,
                _this->layout.pWindowList.cbSize - _this->layout.iIndex,
                _this->layout.pWindowList.cbSize
            );
            //wprintf(L"[sws] Accesible text: %s.\n", wszAccText);
            SetWindowTextW(_this->hWndAccessible, wszAccText);
            NotifyWinEvent(
                EVENT_OBJECT_LIVEREGIONCHANGED,
                _this->hWndAccessible,
                OBJID_CLIENT,
                CHILDID_SELF
            );
        }
    }
}

static int _sws_WindowSwitcher_free_stub(void* p, void *pData)
{
    // This enables correct reporting of DPAs being freed in Debug builds
#if defined(DEBUG) | defined(_DEBUG)
    printf("[sws] tshwnd::free: destroy [[ %p ]]\n", p);
#endif
    free(p);
    return 1;
}

static HRESULT STDMETHODCALLTYPE _sws_WindowsSwitcher_IInputSwitchCallback_OnUpdateProfile(sws_IInputSwitchCallback* _this, IInputSwitchCallbackUpdateData *ud)
{
    // useful info: https://referencesource.microsoft.com/#system.windows.forms/winforms/Managed/System/WinForms/InputLanguage.cs,a01e59da9681988c

    wchar_t pwszKLID[9];

    uint16_t language = ud->dwID & 0xffff;
    uint16_t device = (ud->dwID >> 16) & 0x0fff;
    if (device == language)
    {
        swprintf_s(pwszKLID, 9, L"%08x", language);
        PostMessageW(FindWindowW(_T(SWS_WINDOWSWITCHER_CLASSNAME), NULL), WM_INPUTLANGCHANGE, 0, LoadKeyboardLayoutW(pwszKLID, KLF_ACTIVATE));
    }
    else
    {
        wchar_t pwszLanguage[5];
        swprintf_s(pwszLanguage, 5, L"%04x", language);
        wchar_t pwszDevice[5];
        swprintf_s(pwszDevice, 5, L"%04x", device);
        HKEY hKey = NULL;
        RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts", 0, KEY_READ, &hKey);
        if (hKey)
        {
            DWORD cSubKeys = 0;
            RegQueryInfoKeyW(hKey, NULL, NULL, NULL, &cSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
            if (cSubKeys)
            {
                for (unsigned int i = 0; i < cSubKeys; ++i)
                {
                    wchar_t name[9];
                    ZeroMemory(name, 9 * sizeof(wchar_t));
                    DWORD name_size = 9;
                    RegEnumKeyExW(hKey, i, name, &name_size, NULL, NULL, NULL, NULL);
                    if (name[0] && name_size == 8)
                    {
                        if (!wcsncmp(name + 4, pwszLanguage, 4))
                        {
                            wchar_t layoutId[5];
                            ZeroMemory(layoutId, 5 * sizeof(wchar_t));
                            DWORD layoutId_size = 5 * sizeof(wchar_t);
                            RegGetValueW(hKey, name, L"Layout Id", RRF_RT_REG_SZ, NULL, layoutId, &layoutId_size);
                            if (layoutId[0] && layoutId_size == 5 * sizeof(wchar_t))
                            {
                                if (!wcsncmp(layoutId, pwszDevice, 4))
                                {
                                    PostMessageW(FindWindowW(_T(SWS_WINDOWSWITCHER_CLASSNAME), NULL), WM_INPUTLANGCHANGE, 0, LoadKeyboardLayoutW(name, KLF_ACTIVATE));
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            RegCloseKey(hKey);
        }
    }
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE _sws_WindowsSwitcher_IInputSwitchCallback_QueryInterface(sws_IInputSwitchCallback* _this, REFIID riid, void** ppvObject)
{
    if (!IsEqualIID(riid, &sws_IID_IInputSwitchCallback) && !IsEqualIID(riid, &IID_IUnknown))
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }
    *ppvObject = _this;
    return S_OK;
}

static ULONG STDMETHODCALLTYPE _sws_WindowsSwitcher_IInputSwitchCallback_AddRefRelease(sws_IInputSwitchCallback* _this)
{
    return 1;
}

static HRESULT STDMETHODCALLTYPE _sws_WindowsSwitcher_IInputSwitchCallback_Stub(sws_IInputSwitchCallback* _this)
{
    return S_OK;
}

static const sws_IInputSwitchCallbackVtbl _sws_WindowSwitcher_InputSwitchCallbackVtbl = {
    _sws_WindowsSwitcher_IInputSwitchCallback_QueryInterface,
    _sws_WindowsSwitcher_IInputSwitchCallback_AddRefRelease,
    _sws_WindowsSwitcher_IInputSwitchCallback_AddRefRelease,
    _sws_WindowsSwitcher_IInputSwitchCallback_OnUpdateProfile,
    _sws_WindowsSwitcher_IInputSwitchCallback_Stub,
    _sws_WindowsSwitcher_IInputSwitchCallback_Stub,
    _sws_WindowsSwitcher_IInputSwitchCallback_Stub,
    _sws_WindowsSwitcher_IInputSwitchCallback_Stub,
    _sws_WindowsSwitcher_IInputSwitchCallback_Stub,
    _sws_WindowsSwitcher_IInputSwitchCallback_Stub,
    _sws_WindowsSwitcher_IInputSwitchCallback_Stub
};

void _sws_WindowSwitcher_Wineventproc(
    HWINEVENTHOOK hWinEventHook,
    DWORD event,
    HWND hwnd,
    LONG idObject,
    LONG idChild,
    DWORD idEventThread,
    DWORD dwmsEventTime
)
{
    if ((event == EVENT_OBJECT_CREATE) && hwnd && idObject == OBJID_WINDOW)
    {
        PostMessageW(FindWindowW(_T(SWS_WINDOWSWITCHER_CLASSNAME), NULL), RegisterWindowMessageW(L"SHELLHOOK"), HSHELL_WINDOWCREATED, hwnd);
    }
    else if ((event == EVENT_OBJECT_DESTROY) && hwnd && idObject == OBJID_WINDOW)
    {
        PostMessageW(FindWindowW(_T(SWS_WINDOWSWITCHER_CLASSNAME), NULL), RegisterWindowMessageW(L"SHELLHOOK"), HSHELL_WINDOWDESTROYED, hwnd);
    }
    else if ((event == EVENT_SYSTEM_FOREGROUND) && hwnd && (idObject == OBJID_WINDOW) && _sws_IsTopLevelWindow(hwnd))
    {
        PostMessageW(FindWindowW(_T(SWS_WINDOWSWITCHER_CLASSNAME), NULL), RegisterWindowMessageW(L"SHELLHOOK"), HSHELL_RUDEAPPACTIVATED, hwnd);
    }
}

static DWORD WINAPI _sws_WindowSwitcher_Calculate(sws_WindowSwitcher* _this)
{
    long long start = sws_milliseconds_now();
    HWND hFw = GetForegroundWindow();
    if (!_this->lastMiniModehWnd)
    {
        HWND hOwner = GetWindow(hFw, GW_OWNER);
        _this->lastMiniModehWnd = hOwner ? hOwner : hFw;
    }
    sws_WindowSwitcherLayout_Initialize(
        &(_this->layout), 
        _this->hMonitor, 
        _this->hWnd, 
        &(_this->dwRowHeight), 
        &(_this->pHWNDList), 
        (_this->mode ? _this->lastMiniModehWnd: NULL),
        _this->hWndWallpaper
    );
    long long init = sws_milliseconds_now();
    wchar_t wszClassName[100];
    ZeroMemory(wszClassName, 100);
    GetClassNameW(hFw, wszClassName, 100);
    if (_this->mode == SWS_WINDOWSWITCHER_LAYOUTMODE_FULL && _this->layout.bIncludeWallpaper && _this->layout.bWallpaperAlwaysLast && _sws_WindowHelpers_IsDesktopRaised())
    {
        sws_WindowSwitcherLayout_ComputeLayout(&(_this->layout), SWS_WINDOWSWITCHERLAYOUT_COMPUTE_DIRECTION_INITIAL, _this->layout.hWndWallpaper);
    }
    else
    {
        sws_WindowSwitcherLayout_ComputeLayout(&(_this->layout), SWS_WINDOWSWITCHERLAYOUT_COMPUTE_DIRECTION_INITIAL, NULL);
    }
    long long fin = sws_milliseconds_now();
    printf("[sws] CalculateHelper %d [[ %lld + %lld = %lld ]].\n", _this->mode, init - start, fin - init, fin - start);

    _this->layout.iIndex = _this->layout.pWindowList.cbSize == 1 ? 0 : _this->layout.iIndex - 1 - _this->layout.numTopMost;
    if (_this->mode == SWS_WINDOWSWITCHER_LAYOUTMODE_FULL && _this->layout.bIncludeWallpaper && _sws_WindowHelpers_IsDesktopRaised())
    {
        if (_this->layout.bWallpaperAlwaysLast)
        {
            if (!_this->layout.bWallpaperToggleBehavior)
            {
                // BEHAVIOR 1
                _this->layout.iIndex = _this->layout.pWindowList.cbSize - 1;
            }
            else
            {
                // BEHAVIOR 2
                _this->layout.iIndex = 0;
            }
        }
        else
        {
            if (!_this->layout.bWallpaperToggleBehavior)
            {
                // BEHAVIOR 1
            }
            else
            {
                // BEHAVIOR 2
                _this->layout.iIndex = _this->layout.pWindowList.cbSize - 1;
            }
        }
    }
    if (_this->layout.iIndex < 0)
    {
        _this->layout.iIndex = 0;
    }

    _this->cwIndex = -1;
    _this->cwMask = 0;
    _this->bPartialRedraw = FALSE;
}

static void _sws_WindowsSwitcher_DecideThumbnails(sws_WindowSwitcher* _this, DWORD dwMode)
{
    sws_WindowSwitcherLayoutWindow* pWindowList = _this->layout.pWindowList.pList;
    if (pWindowList)
    {
        for (unsigned int i = 0; i < _this->layout.pWindowList.cbSize; ++i)
        {
            if (pWindowList[i].hThumbnail)
            {
                DWM_THUMBNAIL_PROPERTIES dskThumbProps;
                ZeroMemory(&dskThumbProps, sizeof(DWM_THUMBNAIL_PROPERTIES));
                dskThumbProps.dwFlags = DWM_TNP_VISIBLE;
                dskThumbProps.fVisible = TRUE;
                DwmUpdateThumbnailProperties(pWindowList[i].hThumbnail, &dskThumbProps);
            }
        }
    }
}

static sws_error_t _sws_WindowSwitcher_GetCloseButtonRectFromIndex(sws_WindowSwitcher* _this, DWORD dwIndex, LPRECT lpRect)
{
    sws_WindowSwitcherLayoutWindow* pWindowList = _this->layout.pWindowList.pList;
    if (pWindowList)
    {
        RECT rc = pWindowList[dwIndex].rcWindow;
        lpRect->left = rc.right - _this->layout.cbRowTitleHeight;
        lpRect->top = rc.top + _this->layout.cbTopPadding;
        lpRect->right = rc.right - _this->layout.cbRightPadding;
        lpRect->bottom = rc.top + _this->layout.cbRowTitleHeight;
        return SWS_ERROR_SUCCESS;
    }
    return SWS_ERROR_GENERIC_ERROR;
}

void _sws_WindowSwitcher_SwitchToSelectedItemAndDismiss(sws_WindowSwitcher* _this)
{
    sws_WindowSwitcherLayoutWindow* pWindowList = _this->layout.pWindowList.pList;
    if (_this->layout.bIncludeWallpaper && pWindowList[_this->layout.iIndex].hWnd == _this->layout.hWndWallpaper)
    {
        _sws_WindowHelpers_ToggleDesktop();
    }
    else
    {
        sws_WindowSwitcherLayoutWindow* pWindowList = _this->layout.pWindowList.pList;
        if (pWindowList)
        {
            SwitchToThisWindow(GetLastActivePopup(pWindowList[_this->layout.iIndex].hWnd), TRUE);
        }
    }
    ShowWindow(_this->hWnd, SW_HIDE);
}

void sws_WindowSwitcher_RefreshTheme(sws_WindowSwitcher* _this)
{
    sws_WindowHelpers_RefreshImmersiveColorPolicyState();
    sws_error_Report(sws_error_GetFromInternalError(sws_WindowHelpers_ShouldSystemUseDarkMode(&(_this->bIsDarkMode))), NULL);
    if (_this->dwColorScheme)
    {
        _this->bIsDarkMode = _this->dwColorScheme - 1;
    }
    printf("[sws] Refreshing theme: %d\n", _this->dwTheme);
    INT preference = _this->dwCornerPreference;
    DwmSetWindowAttribute(_this->hWnd, DWMWA_WINDOW_CORNER_PREFERENCE, &preference, sizeof(preference));
    MARGINS marGlassInset = { 0, 0, 0, 0 };
    DwmExtendFrameIntoClientArea(_this->hWnd, &marGlassInset);
    sws_WindowHelpers_SetMicaMaterialForThisWindow(_this->hWnd, FALSE);
    int s = 0;
    if (sws_global_rovi.dwBuildNumber < 18985)
    {
        s = -1;
    }
    DwmSetWindowAttribute(_this->hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE + s, &_this->bIsDarkMode, sizeof(BOOL));
    sws_WindowHelpers_SetWindowBlur(
        _this->hWnd,
        0,
        0,
        0
    );
    DWORD dwOpacity = 95;
    DWORD dwSize = sizeof(DWORD);
    sws_SHRegGetValueFromHKCUHKLM(
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MultitaskingView\\AltTabViewHost",
        L"Grid_backgroundPercent",
        SRRF_RT_REG_DWORD,
        NULL,
        &(dwOpacity),
        (LPDWORD)(&dwSize)
    );
    _this->opacity = dwOpacity * 255 / 100;

    LONG_PTR exs = GetWindowLongPtrW(_this->hWnd, GWL_EXSTYLE);
    if (_this->dwTheme == SWS_WINDOWSWITCHER_THEME_NONE)
    {
        exs |= WS_EX_LAYERED;
    }
    else
    {
        exs &= ~WS_EX_LAYERED;
    }
    SetWindowLongPtrW(_this->hWnd, GWL_EXSTYLE, exs);

    if (_this->dwTheme == SWS_WINDOWSWITCHER_THEME_BACKDROP)
    {
        DWORD blur = (dwOpacity / 100.0) * 255;
        if (_this->hTheme) sws_WindowHelpers_SetWindowBlur(_this->hWnd, 4, _this->bIsDarkMode ? SWS_WINDOWSWITCHER_BACKGROUND_COLOR : SWS_WINDOWSWITCHER_BACKGROUND_COLOR_LIGHT, blur);
    }
    else if (_this->dwTheme == SWS_WINDOWSWITCHER_THEME_MICA)
    {
        sws_WindowHelpers_PermitDarkMode(_this->hWnd);
        MARGINS marGlassInset = { -1, -1, -1, -1 };
        DwmExtendFrameIntoClientArea(_this->hWnd, &marGlassInset);
        sws_WindowHelpers_SetMicaMaterialForThisWindow(_this->hWnd, TRUE);
    }
    if (_this->hBackgroundBrush)
    {
        DeleteObject(_this->hBackgroundBrush);
    }
    if (_this->dwTheme)
    {
        _this->hBackgroundBrush = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
    }
    else
    {
        _this->hBackgroundBrush = (HBRUSH)CreateSolidBrush(_this->bIsDarkMode ? SWS_WINDOWSWITCHER_BACKGROUND_COLOR : SWS_WINDOWSWITCHER_BACKGROUND_COLOR_LIGHT);
    }
    if (_this->hContourBrush)
    {
        DeleteObject(_this->hContourBrush);
    }
    _this->hContourBrush = (HBRUSH)CreateSolidBrush(_this->bIsDarkMode ? SWS_WINDOWSWITCHER_CONTOUR_COLOR : SWS_WINDOWSWITCHER_CONTOUR_COLOR_LIGHT);
}

static void _sws_WindowSwitcher_DrawContour(sws_WindowSwitcher* _this, HDC hdcPaint, RECT rc, int direction, int contour_size, RGBQUAD transparent)
{
    BYTE r = 0, g = 0, b = 0;
    if (_this->bIsDarkMode)
    {
        r = GetRValue(SWS_WINDOWSWITCHER_CONTOUR_COLOR);
        g = GetGValue(SWS_WINDOWSWITCHER_CONTOUR_COLOR);
        b = GetBValue(SWS_WINDOWSWITCHER_CONTOUR_COLOR);
    }
    else
    {
        r = GetRValue(SWS_WINDOWSWITCHER_CONTOUR_COLOR_LIGHT);
        g = GetGValue(SWS_WINDOWSWITCHER_CONTOUR_COLOR_LIGHT);
        b = GetBValue(SWS_WINDOWSWITCHER_CONTOUR_COLOR_LIGHT);
    }

    BITMAPINFO bi;
    ZeroMemory(&bi, sizeof(BITMAPINFO));
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = 1;
    bi.bmiHeader.biHeight = 1;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    RGBQUAD desiredColor = { b, g, r, 0xFF };

    int thickness = direction * (contour_size * (_this->layout.cbDpiX / DEFAULT_DPI_X));

    if (direction == SWS_CONTOUR_OUTER)
    {
        StretchDIBits(hdcPaint, rc.left + thickness, rc.top, thickness, rc.bottom - rc.top,
            0, 0, 1, 1, &desiredColor, &bi,
            DIB_RGB_COLORS, SRCCOPY);
        StretchDIBits(hdcPaint, rc.right, rc.top, thickness, rc.bottom - rc.top,
            0, 0, 1, 1, &desiredColor, &bi,
            DIB_RGB_COLORS, SRCCOPY);
        StretchDIBits(hdcPaint, rc.left + thickness, rc.top + thickness, rc.right - rc.left - thickness * 2, thickness,
            0, 0, 1, 1, &desiredColor, &bi,
            DIB_RGB_COLORS, SRCCOPY);
        StretchDIBits(hdcPaint, rc.left + thickness, rc.bottom, rc.right - rc.left - thickness * 2, thickness,
            0, 0, 1, 1, &desiredColor, &bi,
            DIB_RGB_COLORS, SRCCOPY);
    }
    else
    {
        StretchDIBits(hdcPaint, rc.left, rc.top, thickness, rc.bottom - rc.top,
            0, 0, 1, 1, &desiredColor, &bi,
            DIB_RGB_COLORS, SRCCOPY);
        StretchDIBits(hdcPaint, rc.right - thickness, rc.top, thickness, rc.bottom - rc.top,
            0, 0, 1, 1, &desiredColor, &bi,
            DIB_RGB_COLORS, SRCCOPY);
        StretchDIBits(hdcPaint, rc.left, rc.top, rc.right - rc.left, thickness,
            0, 0, 1, 1, &desiredColor, &bi,
            DIB_RGB_COLORS, SRCCOPY);
        StretchDIBits(hdcPaint, rc.left, rc.bottom - thickness, rc.right - rc.left, thickness,
            0, 0, 1, 1, &desiredColor, &bi,
            DIB_RGB_COLORS, SRCCOPY);
    }
}

sws_error_t sws_WindowSwitcher_RegisterHotkeys(sws_WindowSwitcher* _this, HKL hkl)
{
    sws_error_t rv = SWS_ERROR_SUCCESS;

    if (hkl)
    {
        _this->vkTilde = MapVirtualKeyExW(0x29, MAPVK_VSC_TO_VK_EX, hkl);
    }
    else
    {
        _this->vkTilde = MapVirtualKeyW(0x29, MAPVK_VSC_TO_VK_EX);
    }


    /*if (!rv)
    {
        if (!RegisterHotKey(_this->hWnd, 0, MOD_ALT, VK_ESCAPE))
        {
            rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
        }
    }*/
    if (!rv)
    {
        if (!RegisterHotKey(_this->hWnd, 1, MOD_ALT, VK_TAB))
        {
            rv = sws_error_GetFromWin32Error(GetLastError());
        }
    }
    if (!_this->bNoPerApplicationList)
    {
        if (!rv)
        {
            if (!RegisterHotKey(_this->hWnd, -1, MOD_ALT, _this->vkTilde))
            {
                rv = sws_error_GetFromWin32Error(GetLastError());
            }
        }
    }
    if (!rv)
    {
        if (!RegisterHotKey(_this->hWnd, 2, MOD_ALT | MOD_SHIFT, VK_TAB))
        {
            rv = sws_error_GetFromWin32Error(GetLastError());
        }
    }
    if (!_this->bNoPerApplicationList)
    {
        if (!rv)
        {
            if (!RegisterHotKey(_this->hWnd, -2, MOD_ALT | MOD_SHIFT, _this->vkTilde))
            {
                rv = sws_error_GetFromWin32Error(GetLastError());
            }
        }
    }
    if (!rv)
    {
        if (!RegisterHotKey(_this->hWnd, 3, MOD_ALT | MOD_CONTROL, VK_TAB))
        {
            rv = sws_error_GetFromWin32Error(GetLastError());
        }
    }
    if (!_this->bNoPerApplicationList)
    {
        if (!rv)
        {
            if (!RegisterHotKey(_this->hWnd, -3, MOD_ALT | MOD_CONTROL, _this->vkTilde))
            {
                rv = sws_error_GetFromWin32Error(GetLastError());
            }
        }
    }
    if (!rv)
    {
        if (!RegisterHotKey(_this->hWnd, 4, MOD_ALT | MOD_SHIFT | MOD_CONTROL, VK_TAB))
        {
            rv = sws_error_GetFromWin32Error(GetLastError());
        }
    }
    if (!_this->bNoPerApplicationList)
    {
        if (!rv)
        {
            if (!RegisterHotKey(_this->hWnd, -4, MOD_ALT | MOD_SHIFT | MOD_CONTROL, _this->vkTilde))
            {
                rv = sws_error_GetFromWin32Error(GetLastError());
            }
        }
    }
    _this->bNoPerApplicationListPrevious = _this->bNoPerApplicationList;

    return rv;
}

void sws_WindowSwitcher_UnregisterHotkeys(sws_WindowSwitcher* _this)
{
    //UnregisterHotKey(_this->hWnd, 0);
    UnregisterHotKey(_this->hWnd, 1);
    UnregisterHotKey(_this->hWnd, 2);
    UnregisterHotKey(_this->hWnd, 3);
    UnregisterHotKey(_this->hWnd, 4);
    if (!_this->bNoPerApplicationListPrevious)
    {
        UnregisterHotKey(_this->hWnd, -1);
        UnregisterHotKey(_this->hWnd, -2);
        UnregisterHotKey(_this->hWnd, -3);
        UnregisterHotKey(_this->hWnd, -4);
    }
}

void sws_WindowSwitcher_Paint(sws_WindowSwitcher* _this, DWORD dwFlags)
{
    HWND hWnd = _this->hWnd;
    DWORD dwTheme = _this->dwTheme;
    BOOL bIsWindowVisible = IsWindowVisible(_this->hWnd);

    PAINTSTRUCT ps;
    HDC hDC = NULL;
    if (dwTheme == SWS_WINDOWSWITCHER_THEME_NONE)
    {
        //hDC = GetDC(hWnd);
    }
    else
    {
        hDC = BeginPaint(hWnd, &ps);
    }

    sws_WindowSwitcherLayoutWindow* pWindowList = _this->layout.pWindowList.pList;

    RECT rc;
    GetClientRect(hWnd, &rc);
    POINT ptZero = { 0, 0 };
    SIZE siz = { rc.right - rc.left, rc.bottom - rc.top };
    BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

    HDC hdcPaint = _this->hdcPaint;
    HFONT hOldFont = NULL;
    if (hdcPaint)
    {
        long long a0 = sws_milliseconds_now();

        hOldFont = SelectObject(hdcPaint, _this->layout.hFontRegular);

        BYTE r = 0, g = 0, b = 0, a = _this->opacity;
        if (_this->bIsDarkMode)
        {
            r = GetRValue(SWS_WINDOWSWITCHER_BACKGROUND_COLOR) * a / 255;
            g = GetGValue(SWS_WINDOWSWITCHER_BACKGROUND_COLOR) * a / 255;
            b = GetBValue(SWS_WINDOWSWITCHER_BACKGROUND_COLOR) * a / 255;
        }
        else
        {
            r = GetRValue(SWS_WINDOWSWITCHER_BACKGROUND_COLOR_LIGHT) * a / 255;
            g = GetGValue(SWS_WINDOWSWITCHER_BACKGROUND_COLOR_LIGHT) * a / 255;
            b = GetBValue(SWS_WINDOWSWITCHER_BACKGROUND_COLOR_LIGHT) * a / 255;
        }
        if (_this->dwTheme)
        {
            r = 0;
            g = 0;
            b = 0;
            a = 0;
        }
        RGBQUAD bkcol = { b, g, r, a };

        // Draw background
        if ((dwFlags & SWS_WINDOWSWITCHER_PAINTFLAGS_REDRAWENTIRE))
        {
            BITMAPINFO bi;
            ZeroMemory(&bi, sizeof(BITMAPINFO));
            bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bi.bmiHeader.biWidth = 1;
            bi.bmiHeader.biHeight = 1;
            bi.bmiHeader.biPlanes = 1;
            bi.bmiHeader.biBitCount = 32;
            bi.bmiHeader.biCompression = BI_RGB;
            StretchDIBits(hdcPaint, 0, 0, siz.cx, siz.cy, 0, 0, 1, 1, &bkcol, &bi, DIB_RGB_COLORS, SRCCOPY);
        }

        void* pGdipGraphics = NULL;
        GdipCreateFromHDC(
            (HDC)hdcPaint,
            (void**)&pGdipGraphics
        );
        for (unsigned int i = 0; i < _this->layout.pWindowList.cbSize; ++i)
        {
            RGBQUAD rgbStart = bkcol;
            RGBQUAD rgbEnd = sws_GetFlashRGB(_this->bIsDarkMode);
            RGBQUAD rgbFinal;


            sws_tshwnd* tshWnd = NULL;
            if (pWindowList)
            {
                if (_this->bSwitcherIsPerApplication && _this->mode == SWS_WINDOWSWITCHER_LAYOUTMODE_FULL)
                {
                    tshWnd = pWindowList[i].last_flashing_tshwnd;
                    sws_window* pHWNDList = _this->pHWNDList.pList;
                    int k = -1;
                    if (pHWNDList)
                    {
                        for (unsigned int j = 0; j < _this->pHWNDList.cbSize; ++j)
                        {
                            if (pHWNDList[j].hWnd == pWindowList[i].hWnd)
                            {
                                k = j;
                                break;
                            }
                        }
                    }
                    if (pHWNDList && k >= 0)
                    {
                        for (unsigned int j = 0; j < _this->pHWNDList.cbSize; ++j)
                        {
                            if (pHWNDList[k].dwProcessId == pHWNDList[j].dwProcessId || !_wcsicmp(pHWNDList[k].wszPath, pHWNDList[j].wszPath))
                            {
                                if (pHWNDList[j].tshWnd && sws_tshwnd_GetFlashState(pHWNDList[j].tshWnd))
                                {
                                    tshWnd = pHWNDList[j].tshWnd;
                                    pWindowList[i].last_flashing_tshwnd = tshWnd;
                                    break;
                                }
                            }
                        }
                    }
                }
                else
                {
                    tshWnd = pWindowList[i].tshWnd;
                }
            }

            if (tshWnd)
            {
                if (sws_WindowHelpers_AreAnimationsAllowed())
                {
                    rgbFinal.rgbRed = sws_linear(sws_easing_easeOutQuad(tshWnd->cbFlashAnimationState), rgbStart.rgbRed, rgbEnd.rgbRed);
                    rgbFinal.rgbGreen = sws_linear(sws_easing_easeOutQuad(tshWnd->cbFlashAnimationState), rgbStart.rgbGreen, rgbEnd.rgbGreen);
                    rgbFinal.rgbBlue = sws_linear(sws_easing_easeOutQuad(tshWnd->cbFlashAnimationState), rgbStart.rgbBlue, rgbEnd.rgbBlue);
                    rgbFinal.rgbReserved = sws_linear(sws_easing_easeOutQuad(tshWnd->cbFlashAnimationState), rgbStart.rgbReserved, rgbEnd.rgbReserved);
                }
                else
                {
                    if (!sws_tshwnd_GetFlashState(tshWnd))
                    {
                        rgbFinal = rgbStart;
                    }
                    else
                    {
                        rgbFinal = rgbEnd;
                        /*if (!(tshWnd->dwFlashAnimationState % 2))
                        {
                            rgbFinal = rgbEnd;
                        }
                        else
                        {
                            rgbFinal = rgbStart;
                        }*/
                    }
                }
            }
            else
            {
                rgbFinal = rgbStart;
            }

            // Draw flash rectangle
            BOOL bShouldDrawFlashRectangle = FALSE;
            if ((dwFlags & SWS_WINDOWSWITCHER_PAINTFLAGS_ISFLASHANIMATION) && tshWnd)
            {
                if (sws_tshwnd_GetFlashState(tshWnd))
                {
                    if (tshWnd->dwFlashAnimationState != SWS_WINDOWSWITCHER_ANIMATOR_FLASH_MAXSTATE)
                    {
                        if (!(tshWnd->dwFlashAnimationState % 2))
                        {
                            tshWnd->cbFlashAnimationState += SWS_WINDOWSWITCHER_ANIMATOR_FLASH_STEP;
                        }
                        else
                        {
                            tshWnd->cbFlashAnimationState -= SWS_WINDOWSWITCHER_ANIMATOR_FLASH_STEP;
                        }
                        if (tshWnd->cbFlashAnimationState <= 0.0)
                        {
                            tshWnd->cbFlashAnimationState = 0.0;
                        }
                        if (tshWnd->cbFlashAnimationState >= 1.0)
                        {
                            tshWnd->cbFlashAnimationState = 1.0;
                        }
                        bShouldDrawFlashRectangle = TRUE;
                    }

                    if (tshWnd->cbFlashAnimationState == 1.0 && tshWnd->dwFlashAnimationState >= SWS_WINDOWSWITCHER_ANIMATOR_FLASH_MAXSTATE)
                    {
                    }
                    else if (tshWnd->cbFlashAnimationState == 1.0 && tshWnd->dwFlashAnimationState == SWS_WINDOWSWITCHER_ANIMATOR_FLASH_MAXSTATE - 1)
                    {
                        tshWnd->dwFlashAnimationState = SWS_WINDOWSWITCHER_ANIMATOR_FLASH_MAXSTATE;
                    }
                    else if (tshWnd->cbFlashAnimationState == 1.0 && !(tshWnd->dwFlashAnimationState % 2))
                    {
                        tshWnd->dwFlashAnimationState = tshWnd->dwFlashAnimationState + 1;
                        tshWnd->cbFlashAnimationState -= SWS_WINDOWSWITCHER_ANIMATOR_FLASH_STEP;
                    }
                    else if (tshWnd->cbFlashAnimationState == 0.0 && (tshWnd->dwFlashAnimationState % 2))
                    {
                        tshWnd->dwFlashAnimationState = tshWnd->dwFlashAnimationState + 1;
                        tshWnd->cbFlashAnimationState += SWS_WINDOWSWITCHER_ANIMATOR_FLASH_STEP;
                    }
                }
                else
                {
                    tshWnd->dwFlashAnimationState = 0;
                    tshWnd->cbFlashAnimationState -= SWS_WINDOWSWITCHER_ANIMATOR_FLASH_STEP;
                    if (tshWnd->cbFlashAnimationState <= 0.0)
                    {
                        tshWnd->cbFlashAnimationState = 0.0;
                    }
                    else
                    {
                        bShouldDrawFlashRectangle = TRUE;
                    }
                }
            }
            if (
                pWindowList &&
                bIsWindowVisible &&
                ((dwFlags & SWS_WINDOWSWITCHER_PAINTFLAGS_REDRAWENTIRE) ||
                ((dwFlags & SWS_WINDOWSWITCHER_PAINTFLAGS_ISFLASHANIMATION) && bShouldDrawFlashRectangle) ||
                ((dwFlags & SWS_WINDOWSWITCHER_PAINTFLAGS_ACTIVEMASKORINDEXCHANGED) && (i == _this->cwIndex || i == _this->cwOldIndex))
                ))
            {
                //printf("%d %d\n", dwFlags, i);

                RECT rc = pWindowList[i].rcWindow;
                rc.left += SWS_WINDOWSWITCHER_CONTOUR_SIZE;
                rc.top += SWS_WINDOWSWITCHER_CONTOUR_SIZE;
                rc.bottom -= SWS_WINDOWSWITCHER_CONTOUR_SIZE;
                rc.right -= SWS_WINDOWSWITCHER_CONTOUR_SIZE;

                BITMAPINFO bi;
                ZeroMemory(&bi, sizeof(BITMAPINFO));
                bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                bi.bmiHeader.biWidth = 1;
                bi.bmiHeader.biHeight = 1;
                bi.bmiHeader.biPlanes = 1;
                bi.bmiHeader.biBitCount = 32;
                bi.bmiHeader.biCompression = BI_RGB;
                StretchDIBits(hdcPaint, rc.left + 1, rc.top + 1, rc.right - rc.left - 2, rc.bottom - rc.top - 2,
                    0, 0, 1, 1, &rgbFinal, &bi,
                    DIB_RGB_COLORS, SRCCOPY);
            }

            // Draw highlight rectangle
            if (pWindowList && 
                bIsWindowVisible &&
                _this->layout.iIndex == i &&
                ((dwFlags & SWS_WINDOWSWITCHER_PAINTFLAGS_REDRAWENTIRE) ||
                    ((dwFlags & SWS_WINDOWSWITCHER_PAINTFLAGS_ACTIVEMASKORINDEXCHANGED) && (i == _this->cwIndex || i == _this->cwOldIndex))
                    )
                )
            {
                _sws_WindowSwitcher_DrawContour(
                    _this,
                    hdcPaint,
                    pWindowList[_this->layout.iIndex].rcWindow,
                    SWS_CONTOUR_INNER,
                    SWS_WINDOWSWITCHER_CONTOUR_SIZE,
                    rgbFinal
                );
            }

            // Draw hover rectangle
            if (pWindowList &&
                bIsWindowVisible &&
                _this->cwIndex == i &&
                ((dwFlags & SWS_WINDOWSWITCHER_PAINTFLAGS_REDRAWENTIRE) ||
                    ((dwFlags & SWS_WINDOWSWITCHER_PAINTFLAGS_ISFLASHANIMATION) && bShouldDrawFlashRectangle) ||
                    ((dwFlags & SWS_WINDOWSWITCHER_PAINTFLAGS_ACTIVEMASKORINDEXCHANGED))
                    ) &&
                _this->cwIndex != -1 &&
                _this->cwIndex < _this->layout.pWindowList.cbSize &&
                _this->cwMask & SWS_WINDOWFLAG_IS_ON_THUMBNAIL
                )
            {
                _sws_WindowSwitcher_DrawContour(
                    _this,
                    hdcPaint,
                    pWindowList[_this->cwIndex].rcThumbnail,
                    SWS_CONTOUR_OUTER,
                    SWS_WINDOWSWITCHER_HIGHLIGHT_SIZE,
                    rgbFinal
                );
            }

            // Draw fake window contents when window is empty
            if (pWindowList &&
                bIsWindowVisible &&
                ((dwFlags & SWS_WINDOWSWITCHER_PAINTFLAGS_REDRAWENTIRE) ||
                    ((dwFlags & SWS_WINDOWSWITCHER_PAINTFLAGS_ISFLASHANIMATION) && bShouldDrawFlashRectangle) ||
                    ((dwFlags & SWS_WINDOWSWITCHER_PAINTFLAGS_ACTIVEMASKORINDEXCHANGED) && (i == _this->cwIndex || i == _this->cwOldIndex))
                    ) &&
                pWindowList[i].iRowMax &&
                (pWindowList[i].dwWindowFlags & SWS_WINDOWSWITCHERLAYOUT_WINDOWFLAGS_ISEMPTY)
                )
            {
                COLORREF colorUp = SWS_WINDOWSWITCHERLAYOUT_EMPTYWINDOW_GRADIENT_UP;
                COLORREF colorDown = SWS_WINDOWSWITCHERLAYOUT_EMPTYWINDOW_GRADIENT_DOWN;
                int thPad = MulDiv(SWS_WINDOWSWITCHERLAYOUT_EMPTYWINDOW_THUMBNAILPADDING_LEFT, _this->layout.cbDpiY, DEFAULT_DPI_Y);
                for (unsigned int curr_line = SWS_WINDOWSWITCHERLAYOUT_EMPTYWINDOW_THUMBNAILPADDING_TOP; curr_line <= (pWindowList[i].rcThumbnail.bottom - pWindowList[i].rcThumbnail.top) - thPad; ++curr_line)
                {
                    double p = (curr_line * 1.0) / (pWindowList[i].rcThumbnail.bottom - pWindowList[i].rcThumbnail.top);
                    double r = GetRValue(colorUp) + p * (GetRValue(colorDown) - GetRValue(colorUp));
                    double g = GetGValue(colorUp) + p * (GetGValue(colorDown) - GetGValue(colorUp));
                    double b = GetBValue(colorUp) + p * (GetBValue(colorDown) - GetBValue(colorUp));

                    BITMAPINFO bi;
                    ZeroMemory(&bi, sizeof(BITMAPINFO));
                    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                    bi.bmiHeader.biWidth = 1;
                    bi.bmiHeader.biHeight = 1;
                    bi.bmiHeader.biPlanes = 1;
                    bi.bmiHeader.biBitCount = 32;
                    bi.bmiHeader.biCompression = BI_RGB;
                    RGBQUAD desiredColor = { b, g, r, 0xFF };
                    StretchDIBits(hdcPaint, pWindowList[i].rcThumbnail.left + thPad, pWindowList[i].rcThumbnail.top + curr_line, pWindowList[i].rcThumbnail.right - pWindowList[i].rcThumbnail.left - (2 * thPad), 1,
                        0, 0, 1, 1, &desiredColor, &bi,
                        DIB_RGB_COLORS, SRCCOPY);
                }
            }

            // Draw title
            if (pWindowList && 
                bIsWindowVisible &&
                ((dwFlags & SWS_WINDOWSWITCHER_PAINTFLAGS_REDRAWENTIRE) ||
                    ((dwFlags & SWS_WINDOWSWITCHER_PAINTFLAGS_ISFLASHANIMATION) && bShouldDrawFlashRectangle) ||
                    ((dwFlags & SWS_WINDOWSWITCHER_PAINTFLAGS_ACTIVEMASKORINDEXCHANGED) && (i == _this->cwIndex || i == _this->cwOldIndex))
                    ) &&
                pWindowList[i].iRowMax
                )
            {
                rc = pWindowList[i].rcWindow;
                DTTOPTS DttOpts, DttOpts2;
                DttOpts.dwSize = sizeof(DTTOPTS);
                DttOpts.dwFlags = DTT_COMPOSITED | DTT_TEXTCOLOR;
                DttOpts.crText = _this->bIsDarkMode ? SWS_WINDOWSWITCHER_TEXT_COLOR : SWS_WINDOWSWITCHER_TEXT_COLOR_LIGHT;
                DWORD dwTextFlags = DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_HIDEPREFIX;
                RECT rcText, rcText2;
                rcText.left = rc.left + _this->layout.cbLeftPadding + pWindowList[i].szIcon + _this->layout.cbRightPadding;
                rcText.top = rc.top + _this->layout.cbTopPadding;
                rcText.right = rc.right - _this->layout.cbRowTitleHeight - _this->layout.cbRightPadding;
                rcText.bottom = rc.top + _this->layout.cbRowTitleHeight;

                wchar_t wszTitle[MAX_PATH];
                memset(wszTitle, 0, MAX_PATH * sizeof(wchar_t));
                if (_this->layout.bIncludeWallpaper && pWindowList[i].hWnd == _this->layout.hWndWallpaper)
                {
                    sws_WindowHelpers_GetDesktopText(wszTitle);
                }
                else
                {
                    WCHAR wszRundll32Path[MAX_PATH];
                    GetSystemDirectoryW(wszRundll32Path, MAX_PATH);
                    wcscat_s(wszRundll32Path, MAX_PATH, L"\\rundll32.exe");
                    if (_this->bAlwaysUseWindowTitleAndIcon || _this->mode != SWS_WINDOWSWITCHER_LAYOUTMODE_FULL || !_this->bSwitcherIsPerApplication || (pWindowList[i].dwWindowFlags & SWS_WINDOWSWITCHERLAYOUT_WINDOWFLAGS_ISUWP) || !pWindowList[i].wszPath || (pWindowList[i].wszPath && !_wcsicmp(pWindowList[i].wszPath, wszRundll32Path)))
                    {
                        sws_WindowHelpers_GetWindowText(pWindowList[i].hWnd, wszTitle, MAX_PATH);
                    }
                    else
                    {
                        if (pWindowList[i].dwCount > 1)
                        {
                            DWORD dwPrefixLen = 0;
                            swprintf_s(wszTitle, MAX_PATH, L"(%d) ", pWindowList[i].dwCount);
                            dwPrefixLen = wcslen(wszTitle);
                            WCHAR wszExplorerPath[MAX_PATH];
                            GetWindowsDirectoryW(wszExplorerPath, MAX_PATH);
                            wcscat_s(wszExplorerPath, MAX_PATH, L"\\explorer.exe");
                            if (pWindowList[i].wszPath && !_wcsicmp(wszExplorerPath, pWindowList[i].wszPath))
                            {
                                HANDLE hExplorer = GetModuleHandleW(NULL);
                                if (hExplorer)
                                {
                                    LoadStringW(hExplorer, 6020, wszTitle + dwPrefixLen, MAX_PATH - dwPrefixLen);
                                }
                                if (!hExplorer || !wszTitle)
                                {
                                    wcscat_s(wszTitle + dwPrefixLen, MAX_PATH - dwPrefixLen, L"File Explorer");
                                }
                            }
                            else
                            {
                                IShellItem2* pIShellItem2 = NULL;
                                if (SUCCEEDED(SHCreateItemFromParsingName(pWindowList[i].wszPath, NULL, &IID_IShellItem2, &pIShellItem2)))
                                {
                                    LPWSTR wszOutText = NULL;
                                    if (SUCCEEDED(pIShellItem2->lpVtbl->GetString(pIShellItem2, &PKEY_FileDescription, &wszOutText)))
                                    {
                                        int len = wcslen(wszOutText);
                                        if (len >= 4 && wszOutText[len - 1] == L'e' && wszOutText[len - 2] == L'x' && wszOutText[len - 3] == L'e' && wszOutText[len - 4] == L'.')
                                        {
                                            CoTaskMemFree(wszOutText);
                                            if (SUCCEEDED(pIShellItem2->lpVtbl->GetString(pIShellItem2, &PKEY_Software_ProductName, &wszOutText)))
                                            {
                                                wcscpy_s(wszTitle + dwPrefixLen, MAX_PATH - dwPrefixLen, wszOutText);
                                                CoTaskMemFree(wszOutText);
                                            }
                                            else
                                            {
                                                sws_WindowHelpers_GetWindowText(pWindowList[i].hWnd, wszTitle + dwPrefixLen, MAX_PATH - dwPrefixLen);
                                            }
                                        }
                                        else
                                        {
                                            wcscpy_s(wszTitle + dwPrefixLen, MAX_PATH - dwPrefixLen, wszOutText);
                                            CoTaskMemFree(wszOutText);
                                        }
                                    }
                                    else
                                    {
                                        sws_WindowHelpers_GetWindowText(pWindowList[i].hWnd, wszTitle + dwPrefixLen, MAX_PATH - dwPrefixLen);
                                    }
                                    pIShellItem2->lpVtbl->Release(pIShellItem2);
                                }
                            }
                        }
                        else
                        {
                            sws_WindowHelpers_GetWindowText(pWindowList[i].hWnd, wszTitle, MAX_PATH);
                        }
                    }
                }
                if ((rcText.right - rcText.left) > (_this->layout.cbDpiX / DEFAULT_DPI_X) * 10)
                {
                    if (pWindowList[i].dwWindowFlags & SWS_WINDOWSWITCHERLAYOUT_WINDOWFLAGS_ISEMPTY)
                    {
                        DWORD dwTitleHeight = _this->layout.cbRowTitleHeight;
                        BOOL bIsCompositionEnabled = FALSE;
                        DwmIsCompositionEnabled(&bIsCompositionEnabled);
                        if (bIsCompositionEnabled)
                        {
                            RECT rcTitle;
                            DwmGetWindowAttribute(pWindowList[i].hWnd, DWMWA_CAPTION_BUTTON_BOUNDS, &rcTitle, sizeof(RECT));
                            dwTitleHeight = rcTitle.bottom - rcTitle.top;
                        }
                        int thPad = MulDiv(SWS_WINDOWSWITCHERLAYOUT_EMPTYWINDOW_THUMBNAILPADDING_LEFT, _this->layout.cbDpiY, DEFAULT_DPI_Y);
                        rcText2.left = pWindowList[i].rcThumbnail.left + 2 * thPad;
                        rcText2.top = pWindowList[i].rcThumbnail.top + SWS_WINDOWSWITCHERLAYOUT_EMPTYWINDOW_THUMBNAILPADDING_TOP;
                        rcText2.right = pWindowList[i].rcThumbnail.right - thPad;
                        rcText2.bottom = pWindowList[i].rcThumbnail.top + dwTitleHeight;
                        DttOpts2 = DttOpts;
                        DttOpts2.crText = SWS_WINDOWSWITCHERLAYOUT_EMPTYWINDOW_TITLECOLOR;
                    }
                    if ((_this->hTheme && IsThemeActive()))
                    {
                        DrawThemeTextEx(
                            _this->hTheme,
                            hdcPaint,
                            0,
                            0,
                            wszTitle,
                            -1,
                            dwTextFlags,
                            &rcText,
                            &DttOpts
                        );
                        if (pWindowList[i].dwWindowFlags & SWS_WINDOWSWITCHERLAYOUT_WINDOWFLAGS_ISEMPTY)
                        {
                            DrawThemeTextEx(
                                _this->hTheme,
                                hdcPaint,
                                0,
                                0,
                                wszTitle,
                                -1,
                                dwTextFlags,
                                &rcText2,
                                &DttOpts2
                            );
                        }
                    }
                    else
                    {
                        SIZE size;
                        size.cx = rcText.right - rcText.left;
                        size.cy = rcText.bottom - rcText.top;
                        HBITMAP hBitmap = sws_WindowHelpers_CreateAlphaTextBitmap(
                            wszTitle,
                            _this->layout.hFontRegular,
                            dwTextFlags,
                            size,
                            _this->bIsDarkMode ? SWS_WINDOWSWITCHER_TEXT_COLOR : SWS_WINDOWSWITCHER_TEXT_COLOR_LIGHT
                        );
                        if (hBitmap)
                        {
                            HDC hTempDC = CreateCompatibleDC(hdcPaint);
                            HBITMAP hOldBMP = (HBITMAP)SelectObject(hTempDC, hBitmap);
                            if (hOldBMP)
                            {
                                BITMAP BMInf;
                                GetObjectW(hBitmap, sizeof(BITMAP), &BMInf);

                                BLENDFUNCTION bf;
                                bf.BlendOp = AC_SRC_OVER;
                                bf.BlendFlags = 0;
                                bf.SourceConstantAlpha = 0xFF;
                                bf.AlphaFormat = AC_SRC_ALPHA;
                                GdiAlphaBlend(hdcPaint, rcText.left, rcText.top, BMInf.bmWidth, BMInf.bmHeight, hTempDC, 0, 0, BMInf.bmWidth, BMInf.bmHeight, bf);

                                SelectObject(hTempDC, hOldBMP);
                                DeleteObject(hBitmap);
                                DeleteDC(hTempDC);
                            }
                        }
                        if (pWindowList[i].dwWindowFlags & SWS_WINDOWSWITCHERLAYOUT_WINDOWFLAGS_ISEMPTY)
                        {
                            SIZE size;
                            size.cx = rcText2.right - rcText2.left;
                            size.cy = rcText2.bottom - rcText2.top;
                            HBITMAP hBitmap = sws_WindowHelpers_CreateAlphaTextBitmap(
                                wszTitle,
                                _this->layout.hFontRegular,
                                dwTextFlags,
                                size,
                                DttOpts2.crText
                            );
                            if (hBitmap)
                            {
                                HDC hTempDC = CreateCompatibleDC(hdcPaint);
                                HBITMAP hOldBMP = (HBITMAP)SelectObject(hTempDC, hBitmap);
                                if (hOldBMP)
                                {
                                    BITMAP BMInf;
                                    GetObjectW(hBitmap, sizeof(BITMAP), &BMInf);

                                    BLENDFUNCTION bf;
                                    bf.BlendOp = AC_SRC_OVER;
                                    bf.BlendFlags = 0;
                                    bf.SourceConstantAlpha = 0xFF;
                                    bf.AlphaFormat = AC_SRC_ALPHA;
                                    GdiAlphaBlend(hdcPaint, rcText2.left, rcText2.top, BMInf.bmWidth, BMInf.bmHeight, hTempDC, 0, 0, BMInf.bmWidth, BMInf.bmHeight, bf);

                                    SelectObject(hTempDC, hOldBMP);
                                    DeleteObject(hBitmap);
                                    DeleteDC(hTempDC);
                                }
                            }
                        }
                    }
                }
            }

            // Draw icon
            if (pWindowList && 
                bIsWindowVisible &&
                ((dwFlags & SWS_WINDOWSWITCHER_PAINTFLAGS_REDRAWENTIRE) ||
                    ((dwFlags & SWS_WINDOWSWITCHER_PAINTFLAGS_ISFLASHANIMATION) && bShouldDrawFlashRectangle) ||
                    ((dwFlags & SWS_WINDOWSWITCHER_PAINTFLAGS_ACTIVEMASKORINDEXCHANGED) && (i == _this->cwIndex || i == _this->cwOldIndex))
                    ) &&
                pWindowList[i].hIcon && 
                pWindowList[i].iRowMax
                )
            {
                rc = pWindowList[i].rcWindow;
                INT x = rc.left + _this->layout.cbLeftPadding + ((_this->layout.cbRowTitleHeight - pWindowList[i].szIcon) / 4.0) - pWindowList[i].rcIcon.left;
                INT y = rc.top + _this->layout.cbTopPadding + ((_this->layout.cbRowTitleHeight - pWindowList[i].szIcon) / 4.0) - pWindowList[i].rcIcon.top;
                INT w = pWindowList[i].rcIcon.right;
                INT h = pWindowList[i].rcIcon.bottom;
                RGBQUAD bkcol2 = rgbFinal;
                // I don't understand why this is necessary, but otherwise icons
                // obtained from the file system have a black plate as background
                if (bkcol2.rgbReserved == 255) bkcol2.rgbReserved = 254;
                sws_IconPainter_DrawIcon(
                    pWindowList[i].hIcon,
                    hdcPaint,
                    (pWindowList[i].tshWnd && pWindowList[i].tshWnd->bFlash) ? _this->hFlashBrush : _this->hBackgroundBrush,
                    pGdipGraphics,
                    x, y, w, h,
                    bkcol2,
                    TRUE
                );
                if (pWindowList[i].dwWindowFlags & SWS_WINDOWSWITCHERLAYOUT_WINDOWFLAGS_ISEMPTY)
                {
                    ICONINFO ii;
                    GetIconInfo(pWindowList[i].hIcon, &ii);
                    if (ii.hbmColor)
                    {
                        BITMAP bm;
                        GetObjectW(ii.hbmColor, sizeof(BITMAP), &bm);
                        if (bm.bmWidth && bm.bmHeight)
                        {
                            w = bm.bmWidth;
                            h = bm.bmHeight;
                        }
                        DeleteBitmap(ii.hbmColor);
                    }
                    if (ii.hbmMask) DeleteBitmap(ii.hbmMask);
                    sws_IconPainter_DrawIcon(
                        pWindowList[i].hIcon,
                        hdcPaint,
                        NULL,
                        pGdipGraphics,
                        pWindowList[i].rcThumbnail.left + (pWindowList[i].rcThumbnail.right - pWindowList[i].rcThumbnail.left) / 2 - (w / 2),
                        pWindowList[i].rcThumbnail.top + (pWindowList[i].rcThumbnail.bottom - pWindowList[i].rcThumbnail.top) / 2,
                        w, h,
                        bkcol2,
                        FALSE
                    );
                }
            }

            // Draw close button
            if (pWindowList &&
                bIsWindowVisible &&
                _this->cwIndex == i &&
                ((dwFlags & SWS_WINDOWSWITCHER_PAINTFLAGS_REDRAWENTIRE) ||
                    ((dwFlags & SWS_WINDOWSWITCHER_PAINTFLAGS_ISFLASHANIMATION) && bShouldDrawFlashRectangle) ||
                    ((dwFlags & SWS_WINDOWSWITCHER_PAINTFLAGS_ACTIVEMASKORINDEXCHANGED))
                    ) &&
                _this->cwIndex != -1 &&
                _this->cwIndex < _this->layout.pWindowList.cbSize &&
                (_this->cwMask & SWS_WINDOWFLAG_IS_ON_CLOSE || _this->cwMask & SWS_WINDOWFLAG_IS_ON_WINDOW) &&
                !(_this->layout.bIncludeWallpaper && pWindowList[_this->cwIndex].hWnd == _this->layout.hWndWallpaper)
                )
            {
                rc = pWindowList[_this->cwIndex].rcWindow;
                DTTOPTS DttOpts;
                DttOpts.dwSize = sizeof(DTTOPTS);
                DttOpts.dwFlags = DTT_COMPOSITED | DTT_TEXTCOLOR;
                DWORD dwTextFlags = DT_SINGLELINE | DT_VCENTER | DT_CENTER;
                RECT rcText;
                _sws_WindowSwitcher_GetCloseButtonRectFromIndex(_this, _this->cwIndex, &rcText);
                if (_this->cwMask & SWS_WINDOWFLAG_IS_ON_CLOSE)
                {
                    HFONT hOldFont2 = SelectObject(hdcPaint, _this->layout.hFontRegular2);
                    DttOpts.crText = SWS_WINDOWSWITCHER_CLOSE_COLOR;
                    if (_this->hTheme && IsThemeActive())
                    {
                        DrawThemeTextEx(
                            _this->hTheme,
                            hdcPaint,
                            0,
                            0,
                            L"\u2B1B",
                            -1,
                            dwTextFlags,
                            &rcText,
                            &DttOpts
                        );
                    }
                    else
                    {
                        SIZE size;
                        size.cx = rcText.right - rcText.left;
                        size.cy = rcText.bottom - rcText.top;
                        HBITMAP hBitmap = sws_WindowHelpers_CreateAlphaTextBitmap(
                            L"\u2B1B",
                            _this->layout.hFontRegular2,
                            dwTextFlags,
                            size,
                            DttOpts.crText
                        );
                        if (hBitmap)
                        {
                            HDC hTempDC = CreateCompatibleDC(hdcPaint);
                            HBITMAP hOldBMP = (HBITMAP)SelectObject(hTempDC, hBitmap);
                            if (hOldBMP)
                            {
                                BITMAP BMInf;
                                GetObjectW(hBitmap, sizeof(BITMAP), &BMInf);

                                BLENDFUNCTION bf;
                                bf.BlendOp = AC_SRC_OVER;
                                bf.BlendFlags = 0;
                                bf.SourceConstantAlpha = 0xFF;
                                bf.AlphaFormat = AC_SRC_ALPHA;
                                GdiAlphaBlend(hdcPaint, rcText.left, rcText.top, BMInf.bmWidth, BMInf.bmHeight, hTempDC, 0, 0, BMInf.bmWidth, BMInf.bmHeight, bf);

                                SelectObject(hTempDC, hOldBMP);
                                DeleteObject(hBitmap);
                                DeleteDC(hTempDC);
                            }
                        }
                    }
                    SelectObject(hdcPaint, hOldFont2);
                }
                if (!sws_WindowHelpers_IsWindows11())
                {
                    rcText.top += MulDiv(1, _this->layout.cbDpiY, DEFAULT_DPI_Y);
                    rcText.bottom += MulDiv(1, _this->layout.cbDpiY, DEFAULT_DPI_Y);
                }
                DttOpts.crText = (_this->bIsDarkMode || (!_this->bIsDarkMode && (_this->cwMask & SWS_WINDOWFLAG_IS_ON_CLOSE))) ? SWS_WINDOWSWITCHER_TEXT_COLOR : SWS_WINDOWSWITCHER_TEXT_COLOR_LIGHT;
                if (_this->hTheme && IsThemeActive())
                {
                    DrawThemeTextEx(
                        _this->hTheme,
                        hdcPaint,
                        0,
                        0,
                        L"\u2715",
                        -1,
                        dwTextFlags,
                        &rcText,
                        &DttOpts
                    );
                }
                else
                {
                    SIZE size;
                    size.cx = rcText.right - rcText.left;
                    size.cy = rcText.bottom - rcText.top;
                    HBITMAP hBitmap = sws_WindowHelpers_CreateAlphaTextBitmap(
                        L"\u2715",
                        _this->layout.hFontRegular,
                        dwTextFlags,
                        size,
                        DttOpts.crText
                    );
                    if (hBitmap)
                    {
                        HDC hTempDC = CreateCompatibleDC(hdcPaint);
                        HBITMAP hOldBMP = (HBITMAP)SelectObject(hTempDC, hBitmap);
                        if (hOldBMP)
                        {
                            BITMAP BMInf;
                            GetObjectW(hBitmap, sizeof(BITMAP), &BMInf);

                            BLENDFUNCTION bf;
                            bf.BlendOp = AC_SRC_OVER;
                            bf.BlendFlags = 0;
                            bf.SourceConstantAlpha = 0xFF;
                            bf.AlphaFormat = AC_SRC_ALPHA;
                            GdiAlphaBlend(hdcPaint, rcText.left, rcText.top, BMInf.bmWidth, BMInf.bmHeight, hTempDC, 0, 0, BMInf.bmWidth, BMInf.bmHeight, bf);

                            SelectObject(hTempDC, hOldBMP);
                            DeleteObject(hBitmap);
                            DeleteDC(hTempDC);
                        }
                    }
                }
            }
        }
        if (pGdipGraphics)
        {
            GdipDeleteGraphics((void*)pGdipGraphics);
        }

        BOOL bShouldDisableFlashAnimationTimer = TRUE;
        for (unsigned int i = 0; i < _this->layout.pWindowList.cbSize; ++i)
        {
            sws_tshwnd* tshWnd = ((_this->bSwitcherIsPerApplication && _this->mode == SWS_WINDOWSWITCHER_LAYOUTMODE_FULL) ? pWindowList[i].last_flashing_tshwnd : pWindowList[i].tshWnd);

            if (pWindowList && tshWnd)
            {
                if (sws_tshwnd_GetFlashState(tshWnd))
                {
                    if (tshWnd->dwFlashAnimationState != SWS_WINDOWSWITCHER_ANIMATOR_FLASH_MAXSTATE)
                    {
                        bShouldDisableFlashAnimationTimer = FALSE;
                    }
                }
                else
                {
                    if (tshWnd->cbFlashAnimationState != 0.0)
                    {
                        bShouldDisableFlashAnimationTimer = FALSE;
                    }
                }
            }
        }
        if (bShouldDisableFlashAnimationTimer)
        {
            ResetEvent(_this->hFlashAnimationSignal);
        }

        if (bIsWindowVisible)
        {
            if (dwTheme == SWS_WINDOWSWITCHER_THEME_NONE)
            {
                UpdateLayeredWindow(hWnd, NULL, NULL, &siz, hdcPaint, &ptZero, 0, &bf, ULW_ALPHA);
            }
            else
            {
                BitBlt(hDC, 0, 0, siz.cx, siz.cy, hdcPaint, 0, 0, SRCCOPY);
            }
        }

        long long a1 = sws_milliseconds_now();
        //printf("[sws] WindowSwitcher::Paint [[ %lld ]]\n", a1 - _this->lastUpdateTime);
        _this->lastUpdateTime = a1;
    }

    if (dwTheme == SWS_WINDOWSWITCHER_THEME_NONE)
    {
        //ReleaseDC(hWnd, hDC);
    }
    else
    {
        EndPaint(hWnd, &ps);
    }
}

static void WINAPI _sws_WindowSwitcher_Show(sws_WindowSwitcher* _this)
{
    long long a1 = sws_milliseconds_now();
    if (_this->dwWallpaperSupport == SWS_WALLPAPERSUPPORT_EXPLORER)
    {
        LONG_PTR atom = 0;
        RECT rc;
        SetRect(&rc, 0, 0, 0, 0);
        BOOL bIsWindowWallpaperWindow = IsWindow(_this->hWndWallpaper);
        if (bIsWindowWallpaperWindow)
        {
            GetWindowRect(_this->hWndWallpaper, &rc);
            atom = GetClassWord(_this->hWndWallpaper, GCW_ATOM);
        }
        if (!bIsWindowWallpaperWindow || rc.right - rc.left == 0 || rc.bottom - rc.top == 0 || atom != RegisterWindowMessageW(L"WorkerW"))
        {
            //printf("[sws] Invalid wallpaper window detected, reobtaining correct window.\n");
            _this->hWndWallpaper = NULL;
            if (sws_WindowHelpers_EnsureWallpaperHWND())
            {
                _this->hWndWallpaper = sws_WindowHelpers_GetWallpaperHWND();
            }
            else
            {
                _this->dwWallpaperSupport = SWS_WALLPAPERSUPPORT_NONE;
            }
        }
    }
    if (_this->hLastClosedWnds)
    {
        DPA_Destroy(_this->hLastClosedWnds);
        _this->hLastClosedWnds = NULL;
    }
    POINT pt;
    if (_this->bPrimaryOnly)
    {
        pt.x = 0;
        pt.y = 0;
    }
    else
    {
        GetCursorPos(&pt);
    }
    _this->hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
    sws_WindowSwitcherLayout_Clear(&(_this->layout));
    sws_vector_Clear(&(_this->pHWNDList));
    sws_vector_Initialize(&(_this->pHWNDList), sizeof(sws_window));
    HDPA hdpa = DPA_Create(SWS_VECTOR_CAPACITY);
    EnumWindows(sws_WindowHelpers_AddAltTabWindowsToTimeStampedHWNDList, hdpa);
    long long a2 = sws_milliseconds_now();
    for (unsigned int i = 0; i < DPA_GetPtrCount(hdpa); ++i)
    {
        sws_tshwnd* tshWnd = DPA_FastGetPtr(hdpa, i);
        int rv = DPA_Search(_this->htshwnds, tshWnd, 0, sws_tshwnd_CompareHWND, 0, 0);
        if (rv != -1)
        {
            sws_tshwnd* found_tshwnd = DPA_FastGetPtr(_this->htshwnds, rv);
            sws_tshwnd_ModifyTimestamp(tshWnd, found_tshwnd->ft);
        }
    }
    long long a3 = sws_milliseconds_now();
    DPA_Sort(hdpa, sws_tshwnd_CompareTimestamp, SWS_SORT_DESCENDING);
    long long a4 = sws_milliseconds_now();
    for (unsigned int i = 0; i < DPA_GetPtrCount(hdpa); ++i)
    {
        sws_tshwnd* tshWnd = DPA_FastGetPtr(hdpa, i);
        sws_window window;
        sws_window_Initialize(&window, tshWnd->hWnd);
        sws_vector_PushBack(&(_this->pHWNDList), &window);
    }
    DPA_DestroyCallback(hdpa, _sws_WindowSwitcher_free_stub, 0);
    long long a5 = sws_milliseconds_now();
    printf("[sws] WindowSwitcher::Show %x [[ %lld + %lld + %lld + %lld = %lld ]]\n", _this->hWndWallpaper, a2 - a1, a3 - a2, a4 - a3, a5 - a4, a5 - a1);
    _sws_WindowSwitcher_Calculate(_this);
    if (_this->layout.pWindowList.cbSize == 0)
    {
        ShowWindow(_this->hWnd, SW_HIDE);
        return;
    }
    if (_this->hdcWindow)
    {
        EndBufferedPaint(_this->hBufferedPaint, FALSE);
        ReleaseDC(_this->hWnd, _this->hdcWindow);
        _this->hdcPaint = NULL;
    }
    _this->hdcWindow = GetDC(_this->hWnd);
    if (_this->hdcWindow)
    {
        BP_PAINTPARAMS params;
        ZeroMemory(&params, sizeof(BP_PAINTPARAMS));
        params.cbSize = sizeof(BP_PAINTPARAMS);
        params.dwFlags = BPPF_NOCLIP | (_this->dwTheme == SWS_WINDOWSWITCHER_THEME_NONE ? 0 : BPPF_ERASE);
        RECT rc;
        SetRect(&rc, 0, 0, _this->layout.iWidth, _this->layout.iHeight);
        _this->hBufferedPaint = BeginBufferedPaint(_this->hdcWindow, &rc, BPBF_TOPDOWNDIB, &params, &(_this->hdcPaint));
    }
    sws_WindowSwitcherLayoutWindow* pWindowList = _this->layout.pWindowList.pList;
    sws_tshwnd* tshWnd = malloc(sizeof(sws_tshwnd));
    if (pWindowList && tshWnd)
    {
        for (int iCurrentWindow = _this->layout.pWindowList.cbSize - 1; iCurrentWindow >= 0; iCurrentWindow--)
        {
            sws_tshwnd_Initialize(tshWnd, pWindowList[iCurrentWindow].hWnd);
            int rv = DPA_Search(_this->htshwnds, tshWnd, 0, sws_tshwnd_CompareHWND, 0, 0);
            if (rv != -1)
            {
                pWindowList[iCurrentWindow].tshWnd = DPA_FastGetPtr(_this->htshwnds, rv);
                /*if (pWindowList[iCurrentWindow].tshWnd->bFlash)
                {
                    pWindowList[iCurrentWindow].cbFlashAnimationState = 1.0;
                    pWindowList[iCurrentWindow].dwFlashAnimationState = SWS_WINDOWSWITCHER_ANIMATOR_FLASH_MAXSTATE;
                }*/
            }
        }
    }
    if (tshWnd)
    {
        free(tshWnd);
    }
    sws_window* pHWNDList = _this->pHWNDList.pList;
    sws_tshwnd* tshwnd2 = malloc(sizeof(sws_tshwnd));
    if (pHWNDList && tshwnd2)
    {
        for (unsigned int i = 0; i < _this->pHWNDList.cbSize; ++i)
        {
            sws_tshwnd_Initialize(tshwnd2, pHWNDList[i].hWnd);
            int rv = DPA_Search(_this->htshwnds, tshwnd2, 0, sws_tshwnd_CompareHWND, 0, 0);
            if (rv != -1)
            {
                pHWNDList[i].tshWnd = DPA_FastGetPtr(_this->htshwnds, rv);
            }
        }
    }
    if (tshwnd2)
    {
        free(tshwnd2);
    }
    _sws_WindowsSwitcher_DecideThumbnails(_this, _this->mode);
    if (!IsWindowVisible(_this->hWnd) && _this->dwShowDelay)
    {
        BOOL bCloak = TRUE;
        DwmSetWindowAttribute(_this->hWnd, DWMWA_CLOAK, &bCloak, sizeof(BOOL));
        SetEvent(_this->hShowSignal);
    }
    else
    {
        BOOL bCloak = FALSE;
        DwmSetWindowAttribute(_this->hWnd, DWMWA_CLOAK, &bCloak, sizeof(BOOL));
    }
    if (_this->bShouldStartFlashTimerWhenShowing)
    {
        _this->bShouldStartFlashTimerWhenShowing = FALSE;
        SetEvent(_this->hFlashAnimationSignal);
    }
    SetWindowPos(_this->hWnd, 0, _this->layout.iX, _this->layout.iY, _this->layout.iWidth, _this->layout.iHeight, SWP_NOZORDER);
    ShowWindow(_this->hWnd, SW_SHOW);
    SetForegroundWindow(_this->hWnd);
    if (_this->dwTheme == SWS_WINDOWSWITCHER_THEME_NONE)
    {
        sws_WindowSwitcher_Paint(_this, SWS_WINDOWSWITCHER_PAINTFLAGS_REDRAWENTIRE);
    }
    else
    {
        _this->dwPaintFlags |= SWS_WINDOWSWITCHER_PAINTFLAGS_REDRAWENTIRE;
        InvalidateRect(_this->hWnd, NULL, TRUE);
    }
    for (int iCurrentWindow = _this->layout.pWindowList.cbSize - 1; iCurrentWindow >= 0; iCurrentWindow--)
    {
        DWORD iPID;
        GetWindowThreadProcessId(pWindowList[iCurrentWindow].hWnd, &iPID);
        sws_WindowSwitcherLayout layout;
        sws_WindowSwitcherLayout_Initialize(
            &layout,
            _this->hMonitor,
            _this->hWnd,
            &(_this->dwRowHeight),
            &(_this->pHWNDList),
            pWindowList[iCurrentWindow].hWnd,
            _this->hWndWallpaper
        );
        sws_WindowSwitcherLayoutWindow* pTestList = layout.pWindowList.pList;
        for (unsigned int j = 0; j < layout.pWindowList.cbSize; ++j)
        {
            DWORD jPID;
            GetWindowThreadProcessId(pTestList[j].hWnd, &jPID);
            pWindowList[iCurrentWindow].dwCount += ((iPID == jPID || !_wcsicmp(pWindowList[iCurrentWindow].wszPath, pTestList[j].wszPath)) ? 1 : 0);
        }
        sws_WindowSwitcherLayout_Clear(&layout);

        if (pWindowList[iCurrentWindow].hIcon == sws_DefAppIcon)
        {
            sws_IconPainter_CallbackParams* params = malloc(sizeof(sws_IconPainter_CallbackParams));
            if (params)
            {
                WCHAR wszRundll32Path[MAX_PATH];
                GetSystemDirectoryW(wszRundll32Path, MAX_PATH);
                wcscat_s(wszRundll32Path, MAX_PATH, L"\\rundll32.exe");
                params->bUseApplicationIcon = FALSE;
                if (!_this->bAlwaysUseWindowTitleAndIcon &&
                    !(pWindowList[iCurrentWindow].wszPath && !_wcsicmp(pWindowList[iCurrentWindow].wszPath, wszRundll32Path)) &&
                    !(pWindowList[iCurrentWindow].dwWindowFlags & SWS_WINDOWSWITCHERLAYOUT_WINDOWFLAGS_ISUWP) &&
                    _this->bSwitcherIsPerApplication &&
                    _this->mode == SWS_WINDOWSWITCHER_LAYOUTMODE_FULL &&
                    pWindowList[iCurrentWindow].dwCount > 1)
                {
                    params->bUseApplicationIcon = TRUE;
                }
                params->hWnd = _this->hWnd;
                params->index = iCurrentWindow;
                if (!_this->layout.timestamp)
                {
                    _this->layout.timestamp = sws_milliseconds_now();
                }
                params->timestamp = _this->layout.timestamp;
                params->bIsDesktop = (_this->layout.bIncludeWallpaper && pWindowList[iCurrentWindow].hWnd == _this->hWndWallpaper);
                if (!sws_IconPainter_ExtractAndDrawIconAsync(pWindowList[iCurrentWindow].hWnd, params))
                {
                    pWindowList[iCurrentWindow].hIcon = sws_LegacyDefAppIcon;
                    free(params);
                }
            }
        }
    }
    if (!_this->bWasControl)
    {
        SetTimer(_this->hWnd, SWS_WINDOWSWITCHER_TIMER_ASYNCKEYCHECK, SWS_WINDOWSWITCHER_TIMER_ASYNCKEYCHECK_DELAY, NULL);
    }
    _sws_WindowSwitcher_UpdateAccessibleText(_this);
}

static DWORD _sws_WindowSwitcher_EndTaskThreadProc(sws_WindowSwitcher_EndTaskThreadParams* params)
{
    SetThreadDesktop(params->hDesktop);
    if (IsHungAppWindow(params->hWnd))
    {
        sws_tshwnd* tshWnd = malloc(sizeof(sws_tshwnd));
        if (tshWnd)
        {
            sws_tshwnd_Initialize(tshWnd, params->hWnd);
            int rv = DPA_Search(params->sws->htshwnds, tshWnd, 0, sws_tshwnd_CompareHWND, 0, 0);
            if (rv != -1)
            {
                EndTask(params->hWnd, FALSE, FALSE);
            }
            free(tshWnd);
        }
    }
    else
    {
        PostMessageW(params->hWnd, WM_SYSCOMMAND, SC_CLOSE, 0);
    }
    free(params);
}

static LRESULT _sws_WindowsSwitcher_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    sws_WindowSwitcher* _this = NULL;
    if (uMsg == WM_CREATE)
    {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)(lParam);
        _this = (struct sws_WindowSwitcher*)(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)_this);
        SetTimer(hWnd, SWS_WINDOWSWITCHER_TIMER_STARTUP, SWS_WINDOWSWITCHER_TIMER_STARTUP_DELAY, 0);
    }
    else
    {
        LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
        _this = (struct sws_WindowSwitcher*)(ptr);
    }

    //printf("%d %d %d\n", uMsg, wParam, lParam);
    if (uMsg == WM_TIMER && wParam == SWS_WINDOWSWITCHER_TIMER_ASYNCKEYCHECK)
    {
        if (!_this->bWasControl && !(GetAsyncKeyState(VK_MENU) & 0x8000))
        {
            _sws_WindowSwitcher_SwitchToSelectedItemAndDismiss(_this);
            KillTimer(hWnd, SWS_WINDOWSWITCHER_TIMER_ASYNCKEYCHECK);
            return 0;
        }
    }
    else if (uMsg == WM_TIMER && wParam == SWS_WINDOWSWITCHER_TIMER_UPDATEACCESSIBLETEXT)
    {
        _sws_WindowSwitcher_UpdateAccessibleText(_this);
        KillTimer(_this->hWnd, SWS_WINDOWSWITCHER_TIMER_UPDATEACCESSIBLETEXT);
    }
    else if (uMsg == WM_TIMER && wParam == SWS_WINDOWSWITCHER_TIMER_PAINT)
    {
        SendMessageW(_this->hWnd, SWS_WINDOWSWITCHER_PAINT_MSG, SWS_WINDOWSWITCHER_PAINTFLAGS_REDRAWENTIRE, 0);
        KillTimer(_this->hWnd, SWS_WINDOWSWITCHER_TIMER_PAINT);
    }
    else if (uMsg == WM_TIMER && wParam == SWS_WINDOWSWITCHER_TIMER_CLOSEHWND)
    {
        sws_tshwnd* tshWnd = malloc(sizeof(sws_tshwnd));
        if (tshWnd)
        {
            if (_this->hLastClosedWnds)
            {
                for (unsigned j = 0; j < DPA_GetPtrCount(_this->hLastClosedWnds); ++j)
                {
                    HWND hLastClosedWnd = DPA_FastGetPtr(_this->hLastClosedWnds, j);
                    if (hLastClosedWnd)
                    {
                        sws_tshwnd_Initialize(tshWnd, hLastClosedWnd);
                        int rv = DPA_Search(_this->htshwnds, tshWnd, 0, sws_tshwnd_CompareHWND, 0, 0);
                        if (rv != -1)
                        {
                            sws_tshwnd* found = DPA_FastGetPtr(_this->htshwnds, rv);
                            if (!found->bFlash)
                            {
                                if (GetLastActivePopup(hLastClosedWnd) != hLastClosedWnd)
                                {
                                    /*FLASHWINFO fi;
                                    fi.cbSize = sizeof(FLASHWINFO);
                                    fi.hwnd = _this->hLastClosedWnd;
                                    fi.dwFlags = FLASHW_TIMERNOFG | FLASHW_TRAY;
                                    fi.uCount = 1;
                                    fi.dwTimeout = 0;
                                    FlashWindowEx(&fi);*/
                                    sws_WindowSwitcherLayoutWindow* pWindowList = _this->layout.pWindowList.pList;
                                    if (pWindowList)
                                    {
                                        for (unsigned int i = 0; i < _this->layout.pWindowList.cbSize; ++i)
                                        {
                                            if (hLastClosedWnd == pWindowList[i].hWnd)
                                            {
                                                _this->layout.iIndex = i;
                                                _sws_WindowSwitcher_SwitchToSelectedItemAndDismiss(_this);
                                                DPA_SetPtr(_this->hLastClosedWnds, j, NULL);
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            free(tshWnd);
        }
        KillTimer(_this->hWnd, SWS_WINDOWSWITCHER_TIMER_CLOSEHWND);
    }
    else if (uMsg == SWS_WINDOWSWITCHER_PAINT_MSG)
    {
        if (_this->dwTheme == SWS_WINDOWSWITCHER_THEME_NONE)
        {
            sws_WindowSwitcher_Paint(_this, wParam);
        }
        else
        {
            _this->dwPaintFlags |= wParam;
            RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
        }
    }
    else if (uMsg == WM_ERASEBKGND)
    {
        return 1;
    }
    else if (_this && uMsg == _this->msgShellHook && lParam)
    {
        if (wParam == HSHELL_WINDOWCREATED || wParam == HSHELL_WINDOWACTIVATED || wParam == HSHELL_RUDEAPPACTIVATED || wParam == HSHELL_FLASH || wParam == HSHELL_REDRAW)
        {
            sws_tshwnd* tshWnd;
            for (unsigned int i = 0; i < 2; ++i)
            {
                HWND hWnd = lParam;
                if (!i)
                {
                    HWND hOwner = GetWindow(lParam, GW_OWNER);
                    if (hOwner)
                    {
                        hWnd = hOwner;
                    }
                    else
                    {
                        continue;
                    }
                }
                tshWnd = malloc(sizeof(sws_tshwnd));
                if (tshWnd)
                {
                    sws_tshwnd_Initialize(tshWnd, hWnd);
                    int rv = DPA_Search(_this->htshwnds, tshWnd, 0, sws_tshwnd_CompareHWND, 0, 0);
                    if (rv == -1)
                    {
                        // If this window is not in the window list and is not the foreground window, 
                        // make sure it will be last in the window list when the switcher will be presented
                        // https://github.com/valinet/ExplorerPatcher/issues/1084
                        if (hWnd != GetForegroundWindow())
                        {
                            sws_tshwnd_ModifyTimestamp(tshWnd, sws_WindowHelpers_GetStartTime());
                        }
                        DPA_InsertPtr(_this->htshwnds, 0, tshWnd);
                        sws_WindowSwitcherLayoutWindow* pWindowList = _this->layout.pWindowList.pList;
                        if (pWindowList)
                        {
                            for (unsigned int i = 0; i < _this->layout.pWindowList.cbSize; ++i)
                            {
                                if (hWnd == pWindowList[i].hWnd)
                                {
                                    pWindowList[i].tshWnd = tshWnd;
                                }
                            }
                        }
                        sws_window* pHWNDList = _this->pHWNDList.pList;
                        if (pHWNDList)
                        {
                            for (unsigned int i = 0; i < _this->pHWNDList.cbSize; ++i)
                            {
                                if (hWnd == pHWNDList[i].hWnd)
                                {
                                    pHWNDList[i].tshWnd = tshWnd;
                                }
                            }
                        }
                    }
                    else
                    {
                        free(tshWnd);
                        // Update flash status and have the window pop at the front of the list only
                        // when the window is the foreground window; otherwise, the OS (probably) denied
                        // the foreground request from the app and the window might still be flashing
                        // and not actually in the foreground
                        // https://github.com/valinet/ExplorerPatcher/issues/1084
                        if ((wParam == HSHELL_WINDOWCREATED || wParam == HSHELL_WINDOWACTIVATED || wParam == HSHELL_RUDEAPPACTIVATED) && (hWnd == GetForegroundWindow() || GetLastActivePopup(hWnd) == GetForegroundWindow()))
                        {
                            sws_tshwnd* found = DPA_FastGetPtr(_this->htshwnds, rv);
                            sws_tshwnd_UpdateTimestamp(found);
                            sws_tshwnd_SetFlashState(found, FALSE);
                            found->dwFlashAnimationState = 0;
                            found->cbFlashAnimationState = 0.0;
                        }
                    }
                }
            }
#if defined(DEBUG) | defined(_DEBUG)
            printf("[sws] tshwnd::insert: list count: %d\n", DPA_GetPtrCount(_this->htshwnds));
#endif
        }
        if (wParam == HSHELL_WINDOWDESTROYED)
        {
            sws_tshwnd* tshWnd = malloc(sizeof(sws_tshwnd));
            if (tshWnd)
            {
                sws_tshwnd_Initialize(tshWnd, lParam);
                int rv = DPA_Search(_this->htshwnds, tshWnd, 0, sws_tshwnd_CompareHWND, 0, 0);
                if (rv != -1)
                {
                    sws_WindowSwitcherLayoutWindow* pWindowList = _this->layout.pWindowList.pList;
                    if (pWindowList)
                    {
                        for (unsigned int i = 0; i < _this->layout.pWindowList.cbSize; ++i)
                        {
                            if (pWindowList[i].tshWnd == DPA_FastGetPtr(_this->htshwnds, rv))
                            {
                                pWindowList[i].tshWnd = NULL;
                            }
                            if (pWindowList[i].last_flashing_tshwnd == DPA_FastGetPtr(_this->htshwnds, rv))
                            {
                                pWindowList[i].last_flashing_tshwnd = NULL;
                            }
                        }
                    }
                    sws_window* pHWNDList = _this->pHWNDList.pList;
                    if (pHWNDList)
                    {
                        for (unsigned int i = 0; i < _this->pHWNDList.cbSize; ++i)
                        {
                            if (pHWNDList[i].tshWnd == DPA_FastGetPtr(_this->htshwnds, rv))
                            {
                                pHWNDList[i].tshWnd = NULL;
                            }
                        }
                    }

                    free(DPA_FastGetPtr(_this->htshwnds, rv));
                    DPA_DeletePtr(_this->htshwnds, rv);
                }
                free(tshWnd);
            }
            if (IsWindowVisible(_this->hWnd))
            {
                sws_window* pHWNDList = _this->pHWNDList.pList;
                int bContains = -1;
                for (int i = 0; i < _this->pHWNDList.cbSize; ++i)
                {
                    if (pHWNDList[i].hWnd == lParam)
                    {
                        bContains = i;
                        break;
                    }
                }
                if (bContains != -1)
                {
                    _sws_WindowSwitcher_Show(_this);
                }
            }
#if defined(DEBUG) | defined(_DEBUG)
            printf("[sws] tshwnd::remove: list count: %d\n", DPA_GetPtrCount(_this->htshwnds));
#endif
        }
        if (wParam == HSHELL_FLASH)
        {
            sws_tshwnd* tshWnd = malloc(sizeof(sws_tshwnd));
            if (tshWnd)
            {
                sws_tshwnd_Initialize(tshWnd, lParam);
                int rv = DPA_Search(_this->htshwnds, tshWnd, 0, sws_tshwnd_CompareHWND, 0, 0);
                if (rv != -1)
                {
                    sws_tshwnd* found = DPA_FastGetPtr(_this->htshwnds, rv);
                    if (!sws_tshwnd_GetFlashState(found))
                    {
                        sws_tshwnd_SetFlashState(found, TRUE);
                        if (IsWindowVisible(_this->hWnd))
                        {
                            SetEvent(_this->hFlashAnimationSignal);
                        }
                        else
                        {
                            _this->bShouldStartFlashTimerWhenShowing = TRUE;
                        }
                    }
                }
                free(tshWnd);
            }
            for (unsigned int i = 0; i < 2; ++i)
            {
                HWND hWnd = lParam;
                if (!i)
                {
                    HWND hOwner = GetWindow(lParam, GW_OWNER);
                    if (hOwner)
                    {
                        hWnd = hOwner;
                    }
                    else
                    {
                        continue;
                    }
                }

                if (_this->hLastClosedWnds)
                {
                    for (unsigned int j = 0; j < DPA_GetPtrCount(_this->hLastClosedWnds); ++j)
                    {
                        HWND hLastClosedWnd = DPA_FastGetPtr(_this->hLastClosedWnds, j);
                        if (hLastClosedWnd && hLastClosedWnd == hWnd)
                        {
                            ShowWindow(_this->hWnd, SW_HIDE);
                            SwitchToThisWindow(GetLastActivePopup(hLastClosedWnd), TRUE);

                            /*sws_WindowSwitcherLayoutWindow* pWindowList = _this->layout.pWindowList.pList;
                            if (pWindowList)
                            {
                                for (unsigned int i = 0; i < _this->layout.pWindowList.cbSize; ++i)
                                {
                                    if (hWnd == pWindowList[i].hWnd)
                                    {
                                        KillTimer(_this->hWnd, SWS_WINDOWSWITCHER_TIMER_CLOSEHWND);
                                        _this->layout.iIndex = i;
                                        _sws_WindowSwitcher_SwitchToSelectedItemAndDismiss(_this);
                                        DPA_SetPtr(_this->hLastClosedWnds, j, NULL);
                                        break;
                                    }
                                }
                            }*/
                        }
                    }
                }
            }
#if defined(DEBUG) | defined(_DEBUG)
            WCHAR wn[200];
            sws_InternalGetWindowText(lParam, wn, 200);
            wprintf(L"[sws] Flash [[ %s ]]\n", wn);
#endif
        }
        if (wParam == HSHELL_REDRAW)
        {
            for (unsigned int i = 0; i < 2; ++i)
            {
                HWND hWnd = lParam;
                if (!i)
                {
                    HWND hOwner = GetWindow(lParam, GW_OWNER);
                    if (hOwner)
                    {
                        hWnd = hOwner;
                    }
                    else
                    {
                        continue;
                    }
                }

                BOOL bWasForFlashingOff = FALSE;
                sws_tshwnd* tshWnd = malloc(sizeof(sws_tshwnd));
                if (tshWnd)
                {
                    sws_tshwnd_Initialize(tshWnd, hWnd);
                    int rv = DPA_Search(_this->htshwnds, tshWnd, 0, sws_tshwnd_CompareHWND, 0, 0);
                    if (rv != -1)
                    {
                        sws_tshwnd* found = DPA_FastGetPtr(_this->htshwnds, rv);
                        if (sws_tshwnd_GetFlashState(found))
                        {
                            sws_tshwnd_SetFlashState(found, FALSE);
                            if (IsWindowVisible(_this->hWnd))
                            {
                                SetEvent(_this->hFlashAnimationSignal);
                            }
                            else
                            {
                                _this->bShouldStartFlashTimerWhenShowing = TRUE;
                            }
                            found->dwFlashAnimationState = 0;
                            found->cbFlashAnimationState = 1.0;
                        }
                    }
                    free(tshWnd);
                }

                if (!bWasForFlashingOff)
                {
                    if (IsWindowVisible(_this->hWnd))
                    {
                        sws_WindowSwitcherLayoutWindow* pWindowList = _this->layout.pWindowList.pList;
                        if (pWindowList)
                        {
                            for (unsigned int i = 0; i < _this->layout.pWindowList.cbSize; ++i)
                            {
                                if (hWnd == pWindowList[i].hWnd)
                                {
                                    sws_IconPainter_CallbackParams* params = malloc(sizeof(sws_IconPainter_CallbackParams));
                                    if (params)
                                    {
                                        WCHAR wszRundll32Path[MAX_PATH];
                                        GetSystemDirectoryW(wszRundll32Path, MAX_PATH);
                                        wcscat_s(wszRundll32Path, MAX_PATH, L"\\rundll32.exe");
                                        params->bUseApplicationIcon = FALSE;
                                        if (!_this->bAlwaysUseWindowTitleAndIcon &&
                                            !(pWindowList[i].wszPath && !_wcsicmp(pWindowList[i].wszPath, wszRundll32Path)) &&
                                            !(pWindowList[i].dwWindowFlags & SWS_WINDOWSWITCHERLAYOUT_WINDOWFLAGS_ISUWP) &&
                                            _this->bSwitcherIsPerApplication && 
                                            _this->mode == SWS_WINDOWSWITCHER_LAYOUTMODE_FULL && 
                                            pWindowList[i].dwCount > 1)
                                        {
                                            params->bUseApplicationIcon = TRUE;
                                        }
                                        if (!params->bUseApplicationIcon)
                                        {
                                            params->hWnd = _this->hWnd;
                                            params->index = i;
                                            if (!_this->layout.timestamp)
                                            {
                                                _this->layout.timestamp = sws_milliseconds_now();
                                            }
                                            params->timestamp = _this->layout.timestamp;
                                            params->bIsDesktop = (_this->layout.bIncludeWallpaper && pWindowList[i].hWnd == _this->hWndWallpaper);
                                            if (!sws_IconPainter_ExtractAndDrawIconAsync(pWindowList[i].hWnd, params))
                                            {
                                                pWindowList[i].hIcon = sws_LegacyDefAppIcon;
                                                free(params);
                                                SendMessageW(_this->hWnd, SWS_WINDOWSWITCHER_PAINT_MSG, SWS_WINDOWSWITCHER_PAINTFLAGS_REDRAWENTIRE, 0);
                                            }
                                        }
                                        else
                                        {
                                            free(params);
                                            SendMessageW(_this->hWnd, SWS_WINDOWSWITCHER_PAINT_MSG, SWS_WINDOWSWITCHER_PAINTFLAGS_REDRAWENTIRE, 0);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
#if defined(DEBUG) | defined(_DEBUG)
            WCHAR wn[200];
            sws_InternalGetWindowText(lParam, wn, 200);
            wprintf(L"[sws] Don't flash [[ %s ]]\n", wn);
#endif
        }

        if (wParam == HSHELL_WINDOWCREATED || wParam == HSHELL_WINDOWACTIVATED || wParam == HSHELL_RUDEAPPACTIVATED)
        {
            if (IsWindowVisible(_this->hWnd) && lParam != _this->hWnd && sws_WindowHelpers_IsAltTabWindow(lParam))
            {
                HDPA hdpa = DPA_Create(SWS_VECTOR_CAPACITY);
                EnumWindows(sws_WindowHelpers_AddAltTabWindowsToTimeStampedHWNDList, hdpa);
                sws_WindowSwitcherLayoutWindow* pWindowList = _this->layout.pWindowList.pList;
                if (pWindowList)
                {
                    for (unsigned int i = 0; i < _this->layout.pWindowList.cbSize; ++i)
                    {
                        for (unsigned int j = 0; j < DPA_GetPtrCount(hdpa); ++j)
                        {
                            sws_tshwnd* tshWnd = DPA_FastGetPtr(hdpa, j);
                            if (tshWnd->hWnd == pWindowList[i].hWnd)
                            {
                                tshWnd->hWnd = NULL;
                            }
                        }
                    }
                    BOOL bShouldShow = FALSE;
                    for (unsigned j = 0; j < DPA_GetPtrCount(hdpa); ++j)
                    {
                        sws_tshwnd* tshWnd = DPA_FastGetPtr(hdpa, j);
                        if (tshWnd->hWnd)
                        {
                            bShouldShow = TRUE;
                        }
                        free(tshWnd);
                    }
                    if (bShouldShow)
                    {
                        _sws_WindowSwitcher_Show(_this);
                    }
                    DPA_Destroy(hdpa);
                }
            }
        }
    }
    else if (uMsg == WM_SETTINGCHANGE)
    {
        if (lParam && CompareStringOrdinal(lParam, -1, L"ImmersiveColorSet", -1, TRUE) == CSTR_EQUAL)
        {
            sws_WindowSwitcher_RefreshTheme(_this);
            return 0;
        }
    }
    else if (uMsg == WM_CLOSE)
    {
        DestroyWindow(hWnd);
        return 0;
    }
    else if (uMsg == WM_DESTROY)
    {
        PostQuitMessage(0);
        SetEvent(_this->hEvExit);
        return 0;
    }
    else if (0 && uMsg == WM_NCHITTEST)
    {
        return HTCAPTION;
    }
    else if (uMsg == WM_SHOWWINDOW)
    {
        if (wParam == FALSE)
        {
            //ResetEvent(_this->hFlashAnimationSignal);
            KillTimer(_this->hWnd, SWS_WINDOWSWITCHER_TIMER_CLOSEHWND);
            KillTimer(hWnd, SWS_WINDOWSWITCHER_TIMER_ASYNCKEYCHECK);
            _this->lastMiniModehWnd = NULL;
            //sws_WindowSwitcherLayout_Clear(&(_this->layout));
        }
        else
        {
            //SetWindowPos(_this->hWnd, 0, _this->layout.iX, _this->layout.iY, _this->layout.iWidth, _this->layout.iHeight, SWP_NOZORDER);
        }
        return 0;
    }
    else if (uMsg == WM_PAINT && _this->dwTheme != SWS_WINDOWSWITCHER_THEME_NONE)
    {
        sws_WindowSwitcher_Paint(_this, _this->dwPaintFlags);
        _this->dwPaintFlags = SWS_WINDOWSWITCHER_PAINTFLAGS_NONE;
        return 0;
    }
    else if (uMsg == WM_MOUSEMOVE)
    {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        sws_WindowSwitcherLayoutWindow* pWindowList = _this->layout.pWindowList.pList;
        if (pWindowList)
        {
            INT cwIndex = -1, cwMask = 0;
            for (unsigned int i = 0; i < _this->layout.pWindowList.cbSize; ++i)
            {
                RECT rc;

                rc = pWindowList[i].rcThumbnail;
                if (x > rc.left && x < rc.right && y > rc.top && y < rc.bottom)
                {
                    cwMask |= SWS_WINDOWFLAG_IS_ON_THUMBNAIL;
                }

                rc = pWindowList[i].rcWindow;
                RECT rcText;
                _sws_WindowSwitcher_GetCloseButtonRectFromIndex(_this, i, &rcText);
                if (x > rcText.left && x < rcText.right && y > rcText.top && y < rcText.bottom)
                {
                    cwMask |= SWS_WINDOWFLAG_IS_ON_CLOSE;
                }
                if (x > rc.left && x < rc.right && y > rc.top && y < rc.bottom)
                {
                    cwMask |= SWS_WINDOWFLAG_IS_ON_WINDOW;
                    cwIndex = i;
                }
            }
            if (_this->cwMask != cwMask || _this->cwIndex != cwIndex)
            {
                _this->cwOldIndex = _this->cwIndex;
                _this->cwOldMask = _this->cwOldMask;
                _this->cwMask = cwMask;
                _this->cwIndex = cwIndex;
                if (_this->dwTheme == SWS_WINDOWSWITCHER_THEME_NONE)
                {
                    sws_WindowSwitcher_Paint(_this, SWS_WINDOWSWITCHER_PAINTFLAGS_ACTIVEMASKORINDEXCHANGED);
                }
                else
                {
                    _this->dwPaintFlags |= SWS_WINDOWSWITCHER_PAINTFLAGS_ACTIVEMASKORINDEXCHANGED;
                    RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
                }
            }
        }
        return 0;
    }
    else if (uMsg == WM_LBUTTONDOWN || uMsg == WM_MBUTTONDOWN)
    {
        _this->bIsMouseClicking = TRUE;
        return 0;
    }
    else if (uMsg == WM_LBUTTONUP || uMsg == WM_MBUTTONUP)
    {
        if (_this->bIsMouseClicking)
        {
            WCHAR wszRundll32Path[MAX_PATH];
            GetSystemDirectoryW(wszRundll32Path, MAX_PATH);
            wcscat_s(wszRundll32Path, MAX_PATH, L"\\rundll32.exe");

            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            sws_WindowSwitcherLayoutWindow* pWindowList = _this->layout.pWindowList.pList;
            if (pWindowList)
            {
                for (unsigned int i = 0; i < _this->layout.pWindowList.cbSize; ++i)
                {
                    RECT rc = pWindowList[i].rcWindow;
                    if (x > rc.left && x < rc.right && y > rc.top && y < rc.bottom)
                    {
                        if (_this->cwIndex != -1 &&
                            _this->cwIndex < _this->layout.pWindowList.cbSize &&
                            (uMsg == WM_MBUTTONUP ? 1 : (_this->cwMask & SWS_WINDOWFLAG_IS_ON_CLOSE)) &&
                            !(_this->layout.bIncludeWallpaper && pWindowList[i].hWnd == _this->layout.hWndWallpaper)
                            )
                        {
                            DWORD iPID;
                            GetWindowThreadProcessId(pWindowList[i].hWnd, &iPID);
                            if (_this->hLastClosedWnds)
                            {
                                DPA_Destroy(_this->hLastClosedWnds);
                                _this->hLastClosedWnds = NULL;
                            }
                            _this->hLastClosedWnds = DPA_Create(SWS_VECTOR_CAPACITY);

                            if (_this->bSwitcherIsPerApplication && _this->mode == SWS_WINDOWSWITCHER_LAYOUTMODE_FULL)
                            {
                                sws_WindowSwitcherLayout layout;
                                sws_WindowSwitcherLayout_Initialize(
                                    &layout,
                                    _this->hMonitor,
                                    _this->hWnd,
                                    &(_this->dwRowHeight),
                                    &(_this->pHWNDList),
                                    pWindowList[i].hWnd,
                                    _this->hWndWallpaper
                                );
                                sws_WindowSwitcherLayoutWindow* pTestList = layout.pWindowList.pList;
                                for (unsigned int j = 0; j < layout.pWindowList.cbSize; ++j)
                                {
                                    DWORD jPID;
                                    GetWindowThreadProcessId(pTestList[j].hWnd, &jPID);
                                    BOOL bShouldInclude = FALSE;
                                    if (!_this->bSwitcherIsPerApplication)
                                    {
                                        bShouldInclude = (j == i);
                                    }
                                    else
                                    {
                                        bShouldInclude = (iPID == jPID || !_wcsicmp(pWindowList[i].wszPath, pTestList[j].wszPath));
                                    }
                                    if (bShouldInclude)
                                    {
                                        DPA_AppendPtr(_this->hLastClosedWnds, pTestList[j].hWnd);
                                    }
                                }
                                sws_WindowSwitcherLayout_Clear(&layout);
                            }
                            else
                            {
                                DPA_AppendPtr(_this->hLastClosedWnds, pWindowList[i].hWnd);
                            }
                            for (unsigned j = 0; j < DPA_GetPtrCount(_this->hLastClosedWnds); ++j)
                            {
                                HWND hWnd = DPA_FastGetPtr(_this->hLastClosedWnds, j);
                                if (IsHungAppWindow(hWnd))
                                {
                                    ShowWindow(_this->hWnd, SW_HIDE);
                                    sws_WindowSwitcher_EndTaskThreadParams* pEndTaskParams = malloc(sizeof(sws_WindowSwitcher_EndTaskThreadParams));
                                    if (pEndTaskParams)
                                    {
                                        pEndTaskParams->hWnd = hWnd;
                                        pEndTaskParams->sws = _this;
                                        pEndTaskParams->hDesktop = GetThreadDesktop(GetCurrentThreadId());
                                        if (!SHCreateThread(_sws_WindowSwitcher_EndTaskThreadProc, pEndTaskParams, CTF_NOADDREFLIB, NULL))
                                        {
                                            free(pEndTaskParams);
                                            EndTask(hWnd, FALSE, FALSE);
                                        }
                                    }
                                }
                                else
                                {
                                    PostMessageW(hWnd, WM_SYSCOMMAND, SC_CLOSE, 0);
                                }
                            }
                            SetTimer(_this->hWnd, SWS_WINDOWSWITCHER_TIMER_CLOSEHWND, SWS_WINDOWSWITCHER_TIMER_CLOSEHWND_DELAY, NULL);
                            //EndTask(pWindowList[i].hWnd, FALSE, FALSE);
                            //PostMessageW(pWindowList[i].hWnd, WM_CLOSE, 0, 0);
                            break;
                        }
                        if (uMsg == WM_MBUTTONUP)
                        {
                            break;
                        }
                        _this->layout.iIndex = i;
                        _sws_WindowSwitcher_SwitchToSelectedItemAndDismiss(_this);
                        break;
                    }
                }
            }
            _this->bIsMouseClicking = FALSE;
        }
        return 0;
    }
    else if ((uMsg == WM_KEYUP && wParam == VK_ESCAPE) || uMsg == WM_KILLFOCUS || (uMsg == WM_ACTIVATE && wParam == WA_INACTIVE))
    {
        _this->bWasControl = FALSE; 
        ShowWindow(_this->hWnd, SW_HIDE);
        return 0;
    }
    else if ((uMsg == WM_KEYUP && wParam == VK_MENU && !_this->bWasControl) ||
             ((uMsg == WM_KEYUP || uMsg == WM_SYSKEYUP) && wParam == VK_SPACE) ||
             ((uMsg == WM_KEYUP || uMsg == WM_SYSKEYUP) && wParam == VK_RETURN) ||
             (uMsg == WM_KEYUP && wParam == (_this->mode == SWS_WINDOWSWITCHER_LAYOUTMODE_MINI ? _this->vkTilde : VK_TAB) && !(GetKeyState(VK_MENU) & 0x8000) && !_this->bWasControl))
    {
        _sws_WindowSwitcher_SwitchToSelectedItemAndDismiss(_this);
        return 0;
    }
    else if (uMsg == WM_HOTKEY || uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN)
    {
        /*if (uMsg == WM_HOTKEY && (int)wParam == 0)
        {
            _this->bWasControl = FALSE;
            ShowWindow(_this->hWnd, SW_HIDE);
            return 0;
        }*/
        if (uMsg == WM_HOTKEY && (LOWORD(lParam) & MOD_CONTROL))
        {
            _this->bWasControl = TRUE;
        }
        if (((uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN) && wParam == _this->vkTilde) ||
            ((uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN) && wParam == VK_TAB) ||
            (uMsg == WM_HOTKEY && (LOWORD(lParam) & MOD_ALT)) || 
            ((uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN) && wParam == VK_LEFT) ||
            ((uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN) && wParam == VK_RIGHT) ||
            ((uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN) && wParam == VK_UP) ||
            ((uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN) && wParam == VK_DOWN)
            )
        {
            if (!IsWindowVisible(_this->hWnd))
            {
                if (uMsg == WM_HOTKEY && (int)wParam < 0)
                {
                    
                    _this->mode = SWS_WINDOWSWITCHER_LAYOUTMODE_MINI;
                }
                else
                {
                    _this->mode = SWS_WINDOWSWITCHER_LAYOUTMODE_FULL;
                }
                _sws_WindowSwitcher_Show(_this);

                /*
                BOOL bAltDepressed = FALSE;
                HANDLE hThread = CreateThread(0, 0, _sws_WindowSwitcher_PrepareShow, _this, 0, 0);
                while (GetKeyState(VK_MENU) & 0x8000)
                {
                    DWORD dwRet = WaitForSingleObject(
                        hThread,
                        0
                    );
                    if (dwRet == WAIT_OBJECT_0)
                    {
                        if (_this->layout.pWindowList.cbSize == 0)
                        {
                            return 0;
                        }
                        Sleep(1);
                        _this->bPartialRedraw = FALSE;
                        SetWindowPos(_this->hWnd, 0, _this->layout.iX, _this->layout.iY, _this->layout.iWidth, _this->layout.iHeight, SWP_NOZORDER);
                        ShowWindow(_this->hWnd, SW_SHOW);
                        SetForegroundWindow(_this->hWnd);
                        return 0;
                    }
                }
                int k = 0;
                sws_WindowHelpers_RealEnumWindows((WNDENUMPROC)_sws_WindowSwitcher_EnumWindowsCallback, (LPARAM)_this);
                if (_this->layout.bIncludeWallpaper && _this->hWndLast != 0)
                {
                    _sws_WindowSwitcher_ToggleDesktop(_this);
                }
                WaitForSingleObject(
                    hThread,
                    INFINITE
                );
                sws_WindowSwitcherLayout_Clear(&(_this->layout));
                */
                return 0;
            }
            else
            {
                int direction = SWS_WINDOWSWITCHERLAYOUT_COMPUTE_DIRECTION_INITIAL;
                sws_WindowSwitcherLayoutWindow* pWindowList = _this->layout.pWindowList.pList;

                RECT rcPrev = pWindowList[_this->layout.iIndex].rcWindow;

                int indexStart = _this->layout.iIndex; // avoids infinite cycles
                if (
                    ((uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN) && (GetKeyState(VK_SHIFT) & 0x8000)) ||
                    (uMsg == WM_HOTKEY && (LOWORD(lParam) & MOD_SHIFT)) ||
                    ((uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN) && wParam == VK_LEFT) ||
                    ((uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN) && wParam == VK_UP)
                    )
                {
                    direction = SWS_WINDOWSWITCHERLAYOUT_COMPUTE_DIRECTION_BACKWARD;

                    while (TRUE)
                    {
                        if (_this->layout.iIndex == _this->layout.pWindowList.cbSize - 1)
                        {
                            _this->layout.iIndex = 0;
                        }
                        else
                        {
                            _this->layout.iIndex++;
                        }

                        if ((uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN) && wParam == VK_UP)
                        {
                            if (indexStart == _this->layout.iIndex)
                            {
                                indexStart = -1;
                                break;
                            }
                            if (pWindowList[_this->layout.iIndex].rcWindow.top != rcPrev.top)
                            {
                                break;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                else
                {
                    direction = SWS_WINDOWSWITCHERLAYOUT_COMPUTE_DIRECTION_FORWARD;

                    while (TRUE)
                    {
                        if (_this->layout.iIndex == 0)
                        {
                            _this->layout.iIndex = _this->layout.pWindowList.cbSize - 1;
                        }
                        else
                        {
                            _this->layout.iIndex--;
                        }

                        if ((uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN) && wParam == VK_DOWN)
                        {
                            if (indexStart == _this->layout.iIndex)
                            {
                                indexStart = -1;
                                break;
                            }
                            if (pWindowList[_this->layout.iIndex].rcWindow.top != rcPrev.top)
                            {
                                break;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                }

                if (!pWindowList[_this->layout.iIndex].hThumbnail)
                {
                    sws_WindowSwitcherLayout_ComputeLayout(
                        &(_this->layout),
                        direction,
                        NULL
                    );
                    _this->cwIndex = -1;
                    _this->cwMask = 0;
                }

                // indexStart == -1 when there is a single row, so nothing to do on up/down
                if (indexStart != -1 && (((uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN) && wParam == VK_DOWN) || ((uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN) && wParam == VK_UP)))
                {
                    RECT rcCurrent = pWindowList[_this->layout.iIndex].rcWindow;

                    int i = _this->layout.iIndex;
                    int minIndex = _this->layout.iIndex;
                    int minDist = INT_MAX;
                    int xPrev = rcPrev.left + (rcPrev.right - rcPrev.left) / 2; // point of reference is middle of window rectangle

                    while (pWindowList[i].rcWindow.top == rcCurrent.top)
                    {
                        int xCurr = pWindowList[i].rcWindow.left + (pWindowList[i].rcWindow.right - pWindowList[i].rcWindow.left) / 2;
                        if (abs(xPrev - xCurr) < minDist)
                        {
                            minDist = abs(xPrev - xCurr);
                            minIndex = i;
                        }

                        i = ((uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN) && wParam == VK_DOWN) ? (i - 1) : (i + 1);
                    }

                    _this->layout.iIndex = minIndex;
                }

                if (_this->dwTheme == SWS_WINDOWSWITCHER_THEME_NONE)
                {
                    sws_WindowSwitcher_Paint(_this, SWS_WINDOWSWITCHER_PAINTFLAGS_REDRAWENTIRE);
                }
                else
                {
                    _this->dwPaintFlags |= SWS_WINDOWSWITCHER_PAINTFLAGS_REDRAWENTIRE;
                    RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
                }
                SetForegroundWindow(_this->hWnd);
                _sws_WindowSwitcher_UpdateAccessibleText(_this);
            }
        }
        return 0;
    }
    else if (uMsg == WM_INPUTLANGCHANGE)
    {
        sws_WindowSwitcher_UnregisterHotkeys(_this);
        sws_WindowSwitcher_RegisterHotkeys(_this, lParam);
        return 0;
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

static DWORD _sws_WindowSwitcher_FlashAnimationProcedure(sws_WindowSwitcher* _this)
{
    if (_this && _this->hFlashAnimationSignal)
    {
        while (WaitForSingleObject(_this->hFlashAnimationSignal, INFINITE) == WAIT_OBJECT_0)
        {
            if (!_this->hWnd)
            {
                break;
            }
            PostMessageW(_this->hWnd, SWS_WINDOWSWITCHER_PAINT_MSG, SWS_WINDOWSWITCHER_PAINTFLAGS_ISFLASHANIMATION, 0);
            sws_nanosleep((LONGLONG)SWS_WINDOWSWITCHER_ANIMATOR_FLASH_DELAY * (LONGLONG)10000);
        }
    }
}

static DWORD _sws_WindowSwitcher_ShowAsyncProcedure(sws_WindowSwitcher* _this)
{
    if (_this && _this->hShowSignal)
    {
        while (WaitForSingleObject(_this->hShowSignal, INFINITE) == WAIT_OBJECT_0)
        {
            if (!_this->hWnd)
            {
                break;
            }
            long long mulres = (LONGLONG)_this->dwShowDelay * (LONGLONG)10000;
            long long start = sws_milliseconds_now();
            sws_nanosleep(mulres);
            printf("[sws] Delayed showing by %lld ms due to: user configuration.\n", sws_milliseconds_now() - start);
            if (IsWindowVisible(_this->hWnd))
            {
                BOOL bCloak = FALSE;
                DwmSetWindowAttribute(_this->hWnd, DWMWA_CLOAK, &bCloak, sizeof(BOOL));
            }
        }
    }
}

static sws_error_t _sws_WindowSwitcher_RegisterWindowClass(sws_WindowSwitcher* _this)
{
    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = _sws_WindowsSwitcher_WndProc;
    wc.hbrBackground = _this->hBackgroundBrush;
    wc.hInstance = GetModuleHandleW(NULL);
    wc.lpszClassName = _T(SWS_WINDOWSWITCHER_CLASSNAME);
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    ATOM a = RegisterClassExW(&wc);
    if (!a)
    {
        return sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
    }
    return SWS_ERROR_SUCCESS;
}

__declspec(dllexport) sws_error_t sws_WindowSwitcher_RunMessageQueue(sws_WindowSwitcher* _this)
{
    if (_this->bWithRegMon)
    {
        return sws_RegistryMonitor_Notify(&(_this->rm), QS_ALLINPUT);
    }
    else
    {
        return SWS_ERROR_NOT_INITIALIZED;
    }
}

void sws_WindowSwitcher_InitializeDefaultSettings(sws_WindowSwitcher* _this)
{
    _this->dwShowDelay = SWS_WINDOWSWITCHER_SHOWDELAY;
    _this->dwMaxWP = SWS_WINDOWSWITCHERLAYOUT_PERCENTAGEWIDTH;
    _this->dwMaxHP = SWS_WINDOWSWITCHERLAYOUT_PERCENTAGEHEIGHT;
    _this->bIncludeWallpaper = SWS_WINDOWSWITCHERLAYOUT_INCLUDE_WALLPAPER;
    _this->dwRowHeight = SWS_WINDOWSWITCHERLAYOUT_ROWHEIGHT;
    _this->dwColorScheme = 0;
    _this->dwTheme = SWS_WINDOWSWITCHER_THEME_NONE;
    _this->dwCornerPreference = DWMWCP_ROUND;
    _this->bPrimaryOnly = FALSE;
    _this->bPerMonitor = FALSE;
    _this->dwMaxAbsoluteWP = 0;
    _this->dwMaxAbsoluteHP = 0;
    _this->bNoPerApplicationList = FALSE;
    _this->bNoPerApplicationListPrevious = FALSE;
    _this->dwMasterPadding = SWS_WINDOWSWITCHERLAYOUT_MASTER_PADDING_TOP;
    _this->dwWallpaperSupport = SWS_WALLPAPERSUPPORT_NONE;
    _this->bSwitcherIsPerApplication = FALSE;
    _this->bAlwaysUseWindowTitleAndIcon = FALSE;
}

__declspec(dllexport) void sws_WindowSwitcher_Clear(sws_WindowSwitcher* _this)
{
    if (_this)
    {
        if (_this->pAccPropServices != NULL)
        {
            MSAAPROPID props[] = { LiveSetting_Property_GUID };
            _this->pAccPropServices->lpVtbl->ClearHwndProps(
                _this->pAccPropServices,
                _this->hWndAccessible,
                OBJID_CLIENT,
                CHILDID_SELF,
                props,
                ARRAYSIZE(props));
            _this->pAccPropServices->lpVtbl->Release(_this->pAccPropServices);
            _this->pAccPropServices = NULL;
        }
        DestroyWindow(_this->hWndAccessible);
        CloseHandle(_this->hEvExit);
        if (_this->bWithRegMon)
        {
            sws_RegistryMonitor_Clear(&(_this->rm));
        }
        if (_this->pInputSwitchControl)
        {
            _this->pInputSwitchControl->lpVtbl->Release(_this->pInputSwitchControl);
        }
        sws_WindowSwitcherLayout_Clear(&(_this->layout));
        sws_vector_Clear(&(_this->pHWNDList));
#if defined(DEBUG) | defined(_DEBUG)
        printf("[sws] tshwnd::destroy: list count: %d\n", DPA_GetPtrCount(_this->htshwnds));
#endif
        DPA_DestroyCallback(_this->htshwnds, _sws_WindowSwitcher_free_stub, 0);
        sws_WindowSwitcher_UnregisterHotkeys(_this);
        UnhookWinEvent(_this->global_hook);
        DestroyWindow(_this->hWnd);
        _this->hWnd = NULL;
        SetEvent(_this->hShowSignal);
        WaitForSingleObject(_this->hShowThread, INFINITE);
        CloseHandle(_this->hShowSignal);
        CloseHandle(_this->hShowThread);
        SetEvent(_this->hFlashAnimationSignal);
        WaitForSingleObject(_this->hFlashAnimationThread, INFINITE);
        CloseHandle(_this->hFlashAnimationSignal);
        CloseHandle(_this->hFlashAnimationThread);
        BufferedPaintUnInit();
        UnregisterClassW(_T(SWS_WINDOWSWITCHER_CLASSNAME), GetModuleHandle(NULL));
        DeleteObject(_this->hBackgroundBrush);
        DeleteObject(_this->hFlashBrush);
        DeleteObject(_this->hContourBrush);
        DeleteObject(_this->hCloseButtonBrush);
        CloseThemeData(_this->hTheme);
        sws_WindowHelpers_Clear();
        if (_this->hrRo != S_FALSE)
        {
            RoUninitialize();
        }
        if (_this->hrCo != S_FALSE)
        {
            CoUninitialize();
        }
        if (_this->bIsDynamic)
        {
            free(_this);
        }
        else
        {
            memset(_this, 0, sizeof(sws_WindowSwitcher));
        }
#if defined(DEBUG) | defined(_DEBUG)
        _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
        _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
        _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
        _CrtDumpMemoryLeaks();
#endif
    }
}

__declspec(dllexport) sws_error_t sws_WindowSwitcher_Initialize(sws_WindowSwitcher** __this, BOOL bWithRegMon)
{
    sws_error_t rv = SWS_ERROR_SUCCESS;
    sws_WindowSwitcher* _this = NULL;
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
    _CrtDumpMemoryLeaks();
#endif

    if (!rv)
    {
        if (!*__this)
        {
            *__this = calloc(1, sizeof(sws_WindowSwitcher));
            if (!*__this)
            {
                rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_NO_MEMORY), NULL);
            }
            (*__this)->bIsDynamic = TRUE;
        }
        else
        {
            (*__this)->bIsDynamic = FALSE;
            //memset((*__this), 0, sizeof(sws_WindowSwitcher));
        }
        _this = *__this;
    }
    if (!rv)
    {
        _this->hrCo = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        if (_this->hrCo != S_OK && _this->hrCo != S_FALSE)
        {
            rv = sws_error_Report(sws_error_GetFromHRESULT(_this->hrCo), NULL);
        }
    }
    if (!rv)
    {
        _this->hrRo = RoInitialize(RO_INIT_MULTITHREADED);
        if (_this->hrRo != S_OK && _this->hrRo != S_FALSE)
        {
            rv = sws_error_Report(sws_error_GetFromHRESULT(_this->hrRo), NULL);
        }
    }
    if (!rv)
    {
        rv = sws_error_Report(sws_error_GetFromInternalError(sws_WindowHelpers_Initialize()), NULL);
    }
    if (!rv)
    {
        rv = sws_vector_Initialize(&(_this->pHWNDList), sizeof(sws_window));
    }
    if (!rv)
    {
        _this->hShowSignal = CreateEventW(NULL, FALSE, FALSE, NULL);
        if (!_this->hShowSignal)
        {
            rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
        }
    }
    if (!rv)
    {
        _this->hShowThread = CreateThread(NULL, 0, _sws_WindowSwitcher_ShowAsyncProcedure, _this, 0, NULL);
        if (!_this->hShowThread)
        {
            rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
        }
    }
    if (!rv)
    {
        _this->hFlashAnimationSignal = CreateEventW(NULL, TRUE, FALSE, NULL);
        if (!_this->hFlashAnimationSignal)
        {
            rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
        }
    }
    if (!rv)
    {
        _this->hFlashAnimationThread = CreateThread(NULL, 0, _sws_WindowSwitcher_FlashAnimationProcedure, _this, 0, NULL);
        if (!_this->hFlashAnimationThread)
        {
            rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
        }
    }
    if (!rv)
    {
        _this->htshwnds = DPA_Create(SWS_VECTOR_CAPACITY);
        if (!_this->htshwnds)
        {
            rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
        }
    }
    if (!rv)
    {
        sws_error_Report(sws_error_GetFromInternalError(sws_WindowHelpers_ShouldSystemUseDarkMode(&(_this->bIsDarkMode))), NULL);
        _this->hBackgroundBrush = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
        _this->hContourBrush = (HBRUSH)CreateSolidBrush(_this->bIsDarkMode ? SWS_WINDOWSWITCHER_CONTOUR_COLOR : SWS_WINDOWSWITCHER_CONTOUR_COLOR_LIGHT);
        _this->hFlashBrush = (HBRUSH)CreateSolidBrush(SWS_WINDOWSWITCHER_FLASH_COLOR);
        _this->hCloseButtonBrush = (HBRUSH)CreateSolidBrush(SWS_WINDOWSWITCHER_CLOSE_COLOR);
        _this->hTheme = OpenThemeData(NULL, _T(SWS_WINDOWSWITCHER_THEME_CLASS));
        _this->last_change = 0;
        _this->bWallpaperAlwaysLast = SWS_WINDOWSWITCHERLAYOUT_WALLPAPER_ALWAYS_LAST;
        _this->mode = SWS_WINDOWSWITCHER_LAYOUTMODE_FULL;
        _this->lastMiniModehWnd = NULL;
    }
    if (!rv)
    {
        _this->msgShellHook = RegisterWindowMessageW(L"SHELLHOOK");
        if (!_this->msgShellHook)
        {
            rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
        }
    }
    if (!rv)
    {
        rv = sws_error_Report(sws_error_GetFromInternalError(_sws_WindowSwitcher_RegisterWindowClass(_this)), NULL);
    }
    if (!rv)
    {
        sws_WindowHelpers_PermitDarkMode(NULL);
        BufferedPaintInit();
#ifndef _DEBUG
        _this->hWnd = _sws_CreateWindowInBand(
#else
        _this->hWnd = CreateWindowEx(
#endif
            WS_EX_TOOLWINDOW | WS_EX_LAYERED,
            _T(SWS_WINDOWSWITCHER_CLASSNAME),
            L"",
            WS_POPUP,
            0, 0, 0, 0,
            NULL, NULL, GetModuleHandle(NULL), _this
#ifndef _DEBUG
            , ZBID_UIACCESS
#endif
        );
        if (!_this->hWnd)
        {
            rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
        }
    }
    if (!rv)
    {
        if (!SetWinEventHook(
            EVENT_MIN,
            EVENT_MAX,
            NULL,
            _sws_WindowSwitcher_Wineventproc,
            0,
            0,
            WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS
        ))
        {
            rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
        }
    }
    if (!rv)
    {
        rv = sws_error_Report(sws_WindowSwitcher_RegisterHotkeys(_this, NULL), NULL);
    }
    if (!rv)
    {
        if (_this->hWnd && !RegisterShellHookWindow(_this->hWnd))
        {
            rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
        }
    }
    if (!rv)
    {
        _this->hEvExit = CreateEventW(NULL, FALSE, FALSE, NULL);
        if (!_this->hEvExit)
        {
            rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
        }
    }
    if (!rv)
    {
        rv = sws_error_GetFromHRESULT(CoCreateInstance(&sws_CLSID_InputSwitchControl, NULL, CLSCTX_INPROC_SERVER, &sws_IID_InputSwitchControl, &(_this->pInputSwitchControl)));
    }
    if (!rv)
    {
        rv = sws_error_GetFromHRESULT(_this->pInputSwitchControl->lpVtbl->Init(_this->pInputSwitchControl, 100));
    }
    if (!rv)
    {
        _this->InputSwitchCallback.lpVtbl = &_sws_WindowSwitcher_InputSwitchCallbackVtbl;
        rv = sws_error_GetFromHRESULT(_this->pInputSwitchControl->lpVtbl->SetCallback(_this->pInputSwitchControl, &(_this->InputSwitchCallback)));
    }
    if (!rv)
    {
        _this->hWndAccessible = CreateWindowExW(
            0,
            L"Static",
            L"",
            WS_CHILD,
            0, 0, 0, 0,
            _this->hWnd,
            NULL,
            (HINSTANCE)GetWindowLongPtrW(_this->hWnd, GWLP_HINSTANCE),
            NULL
        );
        if (!_this->hWndAccessible)
        {
            rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
        }
    }
    if (!rv)
    {
        rv = sws_error_GetFromHRESULT(CoCreateInstance(&CLSID_AccPropServices, NULL, CLSCTX_INPROC, &IID_IAccPropServices, &(_this->pAccPropServices)));
    }
    if (!rv)
    {
        VARIANT var;
        var.vt = VT_I4;
        var.lVal = 2; //Assertive;
        rv = sws_error_GetFromHRESULT(_this->pAccPropServices->lpVtbl->SetHwndProp(_this->pAccPropServices, _this->hWndAccessible, OBJID_CLIENT, CHILDID_SELF, LiveSetting_Property_GUID, var));
    }

    if (!rv)
    {
        _this->bWithRegMon = bWithRegMon;
        if (_this->bWithRegMon)
        {
            DWORD dwInitial = 95, dwSize = sizeof(DWORD);
            rv = sws_RegistryMonitor_Initialize(
                &(_this->rm),
                L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MultitaskingView\\AltTabViewHost",
                L"Grid_backgroundPercent",
                KEY_READ,
                REG_NOTIFY_CHANGE_LAST_SET,
                SRRF_RT_REG_DWORD,
                &dwInitial,
                dwSize,
                sws_WindowSwitcher_RefreshTheme,
                _this
            );
            DWORD blur = (dwInitial / 100.0) * 255;
            if (_this->hTheme) sws_WindowHelpers_SetWindowBlur(_this->hWnd, 4, _this->bIsDarkMode ? SWS_WINDOWSWITCHER_BACKGROUND_COLOR : SWS_WINDOWSWITCHER_BACKGROUND_COLOR_LIGHT, blur);
        }
        if (_this->bIsDynamic)
        {
            sws_WindowSwitcher_InitializeDefaultSettings(_this);
        }
        BOOL bExcludedFromPeek = TRUE;
        DwmSetWindowAttribute(_this->hWnd, DWMWA_EXCLUDED_FROM_PEEK, &bExcludedFromPeek, sizeof(BOOL));
    }
    if (!rv)
    {
        if (_this->dwWallpaperSupport == SWS_WALLPAPERSUPPORT_EXPLORER)
        {
            int k = 0;
            //Sleep(500);
            while (!sws_WindowHelpers_EnsureWallpaperHWND())
            {
                //printf("[sws] Ensuring wallpaper\n");
                k++;
                if (k == 100)
                {
                    break;
                }
                Sleep(100);
            }
            Sleep(100);
            _this->hWndWallpaper = sws_WindowHelpers_GetWallpaperHWND();
            if (_this->hWndWallpaper)
            {
                RECT rc;
                GetWindowRect(_this->hWndWallpaper, &rc);
                printf("[sws] Wallpaper RECT %d %d %d %d\n", rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
                sws_WindowSwitcher_RefreshTheme(_this);
            }
            else
            {
                _this->dwWallpaperSupport = SWS_WALLPAPERSUPPORT_NONE;
            }
        }
    }
    if (!rv)
    {
        _this->bIsInitialized = TRUE;
    }

    if (rv && (*__this) && (*__this)->bIsDynamic)
    {
        free((*__this));
        (*__this) = NULL;
    }

    return rv;
}