#include "sws_WindowSwitcher.h"

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
    else if ((event == EVENT_SYSTEM_FOREGROUND) && hwnd && idObject == OBJID_WINDOW)
    {
        PostMessageW(FindWindowW(_T(SWS_WINDOWSWITCHER_CLASSNAME), NULL), RegisterWindowMessageW(L"SHELLHOOK"), HSHELL_RUDEAPPACTIVATED, hwnd);
    }
}

void sws_WindowSwitcher_SetTransparencyFromRegistry(sws_WindowSwitcher * _this, HKEY hOrigin)
{
    HKEY hKey = NULL;
    DWORD dwSize = 0;
    DWORD dwOpacity = 0;

    RegCreateKeyExW(
        hOrigin,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MultitaskingView\\AltTabViewHost",
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ,
        NULL,
        &hKey,
        NULL
    );
    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = NULL;
    }
    if (hKey)
    {
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("Grid_backgroundPercent"),
            0,
            NULL,
            &dwOpacity,
            &dwSize
        );
        _sws_WindowSwitcher_NotifyTransparencyChange(_this, FALSE, &dwOpacity, dwSize);
        RegCloseKey(hKey);
    }
}

static void CALLBACK _sws_WindowSwitcher_NotifyTransparencyChange(sws_WindowSwitcher* _this, BOOL bIsInHKLM, DWORD* value, size_t size)
{
    DWORD blur = (*value / 100.0) * 255;
    if (_this->hTheme)
    {
        sws_WindowHelpers_SetWindowBlur(
            _this->hWnd,
            4,
            _this->bIsDarkMode ? SWS_WINDOWSWITCHER_BACKGROUND_COLOR : SWS_WINDOWSWITCHER_BACKGROUND_COLOR_LIGHT,
            blur
        );
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
    BOOL bIsWallpaperInForeground = !wcscmp(wszClassName, L"WorkerW") && (GetParent(hFw) == FindWindowW(L"Progman", NULL));
    if (_this->mode == SWS_WINDOWSWITCHER_LAYOUTMODE_FULL && _this->layout.bIncludeWallpaper && _this->layout.bWallpaperAlwaysLast && bIsWallpaperInForeground)
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
    if (_this->mode == SWS_WINDOWSWITCHER_LAYOUTMODE_FULL && _this->layout.bIncludeWallpaper && !wcscmp(wszClassName, L"WorkerW"))
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
    BOOL bMica = FALSE;
    DwmSetWindowAttribute(_this->hWnd, DWMWA_MICA_EFFFECT, &bMica, sizeof(BOOL));
    DwmSetWindowAttribute(_this->hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &_this->bIsDarkMode, sizeof(BOOL));
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
        sws_WindowSwitcher_SetTransparencyFromRegistry(_this, HKEY_CURRENT_USER);
    }
    else if (_this->dwTheme == SWS_WINDOWSWITCHER_THEME_MICA)
    {
        sws_WindowHelpers_PermitDarkMode(_this->hWnd);
        MARGINS marGlassInset = { -1, -1, -1, -1 };
        DwmExtendFrameIntoClientArea(_this->hWnd, &marGlassInset);
        bMica = TRUE;
        DwmSetWindowAttribute(_this->hWnd, DWMWA_MICA_EFFFECT, &bMica, sizeof(BOOL));
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

    if (direction == SWS_CONTOUR_INNER)
    {
        StretchDIBits(hdcPaint, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
            0, 0, 1, 1, &desiredColor, &bi,
            DIB_RGB_COLORS, SRCCOPY);
    }

    int thickness = direction * (contour_size * (_this->layout.cbDpiX / DEFAULT_DPI_X));
    rc.left += thickness;
    rc.top += thickness;
    rc.right -= thickness;
    rc.bottom -= thickness;

    StretchDIBits(hdcPaint, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
        0, 0, 1, 1, (direction == SWS_CONTOUR_INNER ? &transparent : &desiredColor), &bi,
        DIB_RGB_COLORS, SRCCOPY);

    if (direction == SWS_CONTOUR_OUTER)
    {
        rc.left -= thickness;
        rc.top -= thickness;
        rc.right += thickness;
        rc.bottom += thickness;

        StretchDIBits(hdcPaint, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
            0, 0, 1, 1, &transparent, &bi,
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

void sws_WindowSwitcher_Paint(sws_WindowSwitcher* _this)
{
    HWND hWnd = _this->hWnd;
    DWORD dwTheme = _this->dwTheme;

    PAINTSTRUCT ps;
    HDC hDC;
    if (dwTheme == SWS_WINDOWSWITCHER_THEME_NONE)
    {
        hDC = GetDC(hWnd);
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

    HDC hdcPaint = NULL;
    BP_PAINTPARAMS params;
    ZeroMemory(&params, sizeof(BP_PAINTPARAMS));
    params.cbSize = sizeof(BP_PAINTPARAMS);
    params.dwFlags = (dwTheme == SWS_WINDOWSWITCHER_THEME_NONE ? BPPF_NOCLIP : BPPF_ERASE);
    HPAINTBUFFER hBufferedPaint = BeginBufferedPaint(hDC, &rc, BPBF_TOPDOWNDIB, &params, &hdcPaint);
    HFONT hOldFont = NULL;
    if (hdcPaint)
    {
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
        if (dwTheme == SWS_WINDOWSWITCHER_THEME_NONE)
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

        // Draw highlight rectangle
        if (pWindowList)
        {
            _sws_WindowSwitcher_DrawContour(_this, hdcPaint, pWindowList[_this->layout.iIndex].rcWindow, SWS_CONTOUR_INNER, SWS_WINDOWSWITCHER_CONTOUR_SIZE, bkcol);
        }

        // Draw hover rectangle
        if (pWindowList &&
            _this->cwIndex != -1 &&
            _this->cwIndex < _this->layout.pWindowList.cbSize &&
            _this->cwMask & SWS_WINDOWFLAG_IS_ON_THUMBNAIL
            )
        {
            _sws_WindowSwitcher_DrawContour(_this, hdcPaint, pWindowList[_this->cwIndex].rcThumbnail, SWS_CONTOUR_OUTER, SWS_WINDOWSWITCHER_HIGHLIGHT_SIZE, bkcol);
        }

        // Draw title
        for (unsigned int i = 0; i < _this->layout.pWindowList.cbSize; ++i)
        {
            if (pWindowList && pWindowList[i].iRowMax)
            {
                rc = pWindowList[i].rcWindow;
                DTTOPTS DttOpts;
                DttOpts.dwSize = sizeof(DTTOPTS);
                DttOpts.dwFlags = DTT_COMPOSITED | DTT_TEXTCOLOR;
                DttOpts.crText = _this->bIsDarkMode ? SWS_WINDOWSWITCHER_TEXT_COLOR : SWS_WINDOWSWITCHER_TEXT_COLOR_LIGHT;
                DWORD dwTextFlags = DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_HIDEPREFIX;
                RECT rcText;
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
                    HWND hWndGhost = _sws_GhostWindowFromHungWindow(pWindowList[i].hWnd);
                    if (hWndGhost)
                    {
                        sws_InternalGetWindowText(hWndGhost, wszTitle, MAX_PATH);
                    }
                    else
                    {
                        sws_InternalGetWindowText(pWindowList[i].hWnd, wszTitle, MAX_PATH);
                    }
                }
                if ((rcText.right - rcText.left) > (_this->layout.cbDpiX / DEFAULT_DPI_X) * 10)
                {
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
                    }
                }
            }
        }

        // Draw icon
        void* pGdipGraphics = NULL;
        GdipCreateFromHDC(
            (HDC)hdcPaint,
            (void**)&pGdipGraphics
        );
        for (unsigned int i = 0; i < _this->layout.pWindowList.cbSize; ++i)
        {
            if (pWindowList && pWindowList[i].hIcon && pWindowList[i].iRowMax)
            {
                rc = pWindowList[i].rcWindow;
                INT x = rc.left + _this->layout.cbLeftPadding + ((_this->layout.cbRowTitleHeight - pWindowList[i].szIcon) / 4.0) - pWindowList[i].rcIcon.left;
                INT y = rc.top + _this->layout.cbTopPadding + ((_this->layout.cbRowTitleHeight - pWindowList[i].szIcon) / 4.0) - pWindowList[i].rcIcon.top;
                INT w = pWindowList[i].rcIcon.right;
                INT h = pWindowList[i].rcIcon.bottom;
                // I don't understand why this is necessary, but otherwise icons
                // obtained from the file system have a black plate as background
                RGBQUAD bkcol2 = bkcol;
                if (bkcol2.rgbReserved == 255) bkcol2.rgbReserved = 254;
                sws_IconPainter_DrawIcon(pWindowList[i].hIcon, hdcPaint, _this->hBackgroundBrush, pGdipGraphics, x, y, w, h, bkcol2);
            }
        }
        if (pGdipGraphics)
        {
            GdipDeleteGraphics((void*)pGdipGraphics);
        }

        // Draw close button
        if (pWindowList &&
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
        SelectObject(hdcPaint, hOldFont);

        UpdateLayeredWindow(hWnd, NULL, NULL, &siz, hdcPaint, &ptZero, 0, &bf, ULW_ALPHA);

        EndBufferedPaint(hBufferedPaint, dwTheme != SWS_WINDOWSWITCHER_THEME_NONE);
    }

    if (dwTheme == SWS_WINDOWSWITCHER_THEME_NONE)
    {
        ReleaseDC(hWnd, hDC);
    }
    else
    {
        EndPaint(hWnd, &ps);
    }
}

static void WINAPI _sws_WindowSwitcher_Show(sws_WindowSwitcher* _this)
{
    long long a1 = sws_milliseconds_now();
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
        sws_tshwnd* tshwnd = DPA_FastGetPtr(hdpa, i);
        int rv = DPA_Search(_this->htshwnds, tshwnd, 0, sws_tshwnd_CompareHWND, 0, 0);
        if (rv != -1)
        {
            sws_tshwnd* found_tshwnd = DPA_FastGetPtr(_this->htshwnds, rv);
            sws_tshwnd_ModifyTimestamp(tshwnd, found_tshwnd->ft);
        }
    }
    long long a3 = sws_milliseconds_now();
    DPA_Sort(hdpa, sws_tshwnd_CompareTimestamp, SWS_SORT_DESCENDING);
    long long a4 = sws_milliseconds_now();
    for (unsigned int i = 0; i < DPA_GetPtrCount(hdpa); ++i)
    {
        sws_tshwnd* tshwnd = DPA_FastGetPtr(hdpa, i);
        sws_window window;
        sws_window_Initialize(&window, tshwnd->hWnd);
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
    SetWindowPos(_this->hWnd, 0, _this->layout.iX, _this->layout.iY, _this->layout.iWidth, _this->layout.iHeight, SWP_NOZORDER);
    ShowWindow(_this->hWnd, SW_SHOW);
    SetForegroundWindow(_this->hWnd);
    if (_this->dwTheme == SWS_WINDOWSWITCHER_THEME_NONE)
    {
        sws_WindowSwitcher_Paint(_this);
    }
    else
    {
        InvalidateRect(_this->hWnd, NULL, TRUE);
    }
    sws_WindowSwitcherLayoutWindow* pWindowList = _this->layout.pWindowList.pList;
    for (int iCurrentWindow = _this->layout.pWindowList.cbSize - 1; iCurrentWindow >= 0; iCurrentWindow--)
    {
        if (pWindowList[iCurrentWindow].hIcon == sws_DefAppIcon)
        {
            sws_IconPainter_CallbackParams* params = malloc(sizeof(sws_IconPainter_CallbackParams));
            if (params)
            {
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
    else if (_this && uMsg == _this->msgShellHook && lParam)
    {
        if (wParam == HSHELL_WINDOWCREATED || wParam == HSHELL_WINDOWACTIVATED || wParam == HSHELL_RUDEAPPACTIVATED)
        {
            sws_tshwnd* tshwnd;

            HWND hOwner = GetWindow(lParam, GW_OWNER);
            if (hOwner)
            {
                tshwnd = malloc(sizeof(sws_tshwnd));
                if (tshwnd)
                {
                    sws_tshwnd_Initialize(tshwnd, hOwner);
                    int rv = DPA_Search(_this->htshwnds, tshwnd, 0, sws_tshwnd_CompareHWND, 0, 0);
                    if (rv == -1)
                    {
                        DPA_InsertPtr(_this->htshwnds, 0, tshwnd);
                    }
                    else
                    {
                        free(tshwnd);
                        sws_tshwnd_UpdateTimestamp(DPA_FastGetPtr(_this->htshwnds, rv));
                    }
                }
            }

            tshwnd = malloc(sizeof(sws_tshwnd));
            if (tshwnd)
            {
                sws_tshwnd_Initialize(tshwnd, lParam);
                int rv = DPA_Search(_this->htshwnds, tshwnd, 0, sws_tshwnd_CompareHWND, 0, 0);
                if (rv == -1)
                {
                    DPA_InsertPtr(_this->htshwnds, 0, tshwnd);
                }
                else
                {
                    free(tshwnd);
                    sws_tshwnd_UpdateTimestamp(DPA_FastGetPtr(_this->htshwnds, rv));
                }
            }

#if defined(DEBUG) | defined(_DEBUG)
            printf("[sws] tshwnd::insert: list count: %d\n", DPA_GetPtrCount(_this->htshwnds));
#endif
        }
        else if (wParam == HSHELL_WINDOWDESTROYED)
        {
            sws_tshwnd* tshwnd = malloc(sizeof(sws_tshwnd));
            if (tshwnd)
            {
                sws_tshwnd_Initialize(tshwnd, lParam);
                int rv = DPA_Search(_this->htshwnds, tshwnd, 0, sws_tshwnd_CompareHWND, 0, 0);
                if (rv != -1)
                {
                    free(DPA_FastGetPtr(_this->htshwnds, rv));
                    DPA_DeletePtr(_this->htshwnds, rv);
                }
                free(tshwnd);
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
        sws_WindowSwitcher_Paint(_this);
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
                _this->cwMask = cwMask;
                _this->cwIndex = cwIndex;
                if (_this->dwTheme == SWS_WINDOWSWITCHER_THEME_NONE)
                {
                    sws_WindowSwitcher_Paint(_this);
                }
                else
                {
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
                            PostMessageW(pWindowList[i].hWnd, WM_SYSCOMMAND, SC_CLOSE, 0);
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
                    sws_WindowSwitcher_Paint(_this);
                }
                else
                {
                    RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
                }
                SetForegroundWindow(_this->hWnd);
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
}

__declspec(dllexport) void sws_WindowSwitcher_Clear(sws_WindowSwitcher* _this)
{
    if (_this)
    {
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
        BufferedPaintUnInit();
        UnregisterClassW(_T(SWS_WINDOWSWITCHER_CLASSNAME), GetModuleHandle(NULL));
        DeleteObject(_this->hBackgroundBrush);
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
            _sws_WindowSwitcher_NotifyTransparencyChange(_this, FALSE, &dwInitial, dwSize);
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