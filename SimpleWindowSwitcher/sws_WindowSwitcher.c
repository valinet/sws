#include "sws_WindowSwitcher.h"

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
    if ((event == EVENT_SYSTEM_MOVESIZEEND || event == EVENT_OBJECT_CLOAKED || event == EVENT_OBJECT_UNCLOAKED) && hwnd && idObject == OBJID_WINDOW)
    {
        static DWORD last_dwmsEventTime;
        if (last_dwmsEventTime != dwmsEventTime)
        {
            PostMessageW(FindWindowW(_T(SWS_WINDOWSWITCHER_CLASSNAME), NULL), RegisterWindowMessageW(L"SHELLHOOK"), HSHELL_RUDEAPPACTIVATED, hwnd);
        }
        last_dwmsEventTime = dwmsEventTime;
    }
}

static BOOL CALLBACK _sws_WindowSwitcher_EnumWindowsCallback(_In_ HWND hWnd, _In_ sws_WindowSwitcher* _this)
{
    if (sws_WindowHelpers_IsAltTabWindow(hWnd, _this->hWndWallpaper) || (_this->bIncludeWallpaper && !_this->bWallpaperAlwaysLast && hWnd == _this->hWndWallpaper))
    {
        sws_window window;
        sws_window_Initialize(&window, hWnd);
        if (sws_vector_PushBack(&_this->pHWNDList, &window) != SWS_ERROR_SUCCESS)
        {
            return FALSE;
        }
    }

    return TRUE;
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

inline long long milliseconds_now() {
    LARGE_INTEGER s_frequency;
    BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);
    if (s_use_qpc) {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        return (1000LL * now.QuadPart) / s_frequency.QuadPart;
    }
    else {
        return GetTickCount();
    }
}

static DWORD WINAPI _sws_WindowSwitcher_CalculateHelper(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, sws_WindowSwitcher* _this)
{
    for (int LayoutMode = 0; LayoutMode < 2; ++LayoutMode)
    {
        //Sleep(100);
        long long start = milliseconds_now();
        ////POINT ptCursor;
        ////GetCursorPos(&ptCursor);
        ////_this->hMonitor = MonitorFromPoint(ptCursor, MONITOR_DEFAULTTOPRIMARY);
        _this->hMonitor = hMonitor;
        if (!_this->lastMiniModehWnd)
        {
            _this->lastMiniModehWnd = GetForegroundWindow();
        }
        sws_WindowSwitcherLayout_Initialize(&(_this->layout), _this->hMonitor, _this->hWnd, &(_this->dwRowHeight), &(_this->pHWNDList), (LayoutMode ? _this->lastMiniModehWnd: NULL));
        wchar_t* wszClassName[100];
        GetClassNameW(GetForegroundWindow(), wszClassName, 100);
        if (LayoutMode == SWS_WINDOWSWITCHER_LAYOUTMODE_FULL && _this->layout.bIncludeWallpaper && _this->layout.bWallpaperAlwaysLast && !wcscmp(wszClassName, L"WorkerW"))
        {
            sws_WindowSwitcherLayout_ComputeLayout(&(_this->layout), SWS_WINDOWSWITCHERLAYOUT_COMPUTE_DIRECTION_INITIAL, sws_WindowHelpers_GetWallpaperHWND());
        }
        else
        {
            sws_WindowSwitcherLayout_ComputeLayout(&(_this->layout), SWS_WINDOWSWITCHERLAYOUT_COMPUTE_DIRECTION_INITIAL, NULL);
        }
        long long elapsed = milliseconds_now() - start;
        //printf("Window switcher completed in %lld.\n", elapsed);

        //sws_WindowSwitcherLayoutWindow* pWindowList = _this->layout.pWindowList.pList;
        ///for (int i = 0; i < _this->layout.pWindowList.cbSize; ++i)
        //{
        //    printf("%d %d\n", pWindowList[i].rcThumbnail.left, pWindowList[i].rcThumbnail.top);
        //}
        _this->layout.iIndex = _this->layout.pWindowList.cbSize == 1 ? 0 : _this->layout.iIndex - 1 - _this->layout.numTopMost;
        if (LayoutMode == SWS_WINDOWSWITCHER_LAYOUTMODE_FULL && _this->layout.bIncludeWallpaper && !wcscmp(wszClassName, L"WorkerW"))
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
        RECT rcZero;
        memset(&rcZero, 0, sizeof(RECT));
        _this->cwIndex = -1;
        _this->cwMask = 0;
        _this->bPartialRedraw = FALSE;
        if (!LayoutMode)
        {
            _this->layouts[_this->numLayouts] = _this->layout;
        }
        else
        {
            _this->minilayouts[_this->numLayouts] = _this->layout;
            _this->numLayouts++;
            if (_this->numLayouts > SWS_WINDOWSWITCHER_MAX_NUM_MONITORS - 1)
            {
                return FALSE;
            }
            return TRUE;
        }
    }



    SetWindowPos(_this->hWnd, 0, _this->layout.iX, _this->layout.iY, _this->layout.iWidth, _this->layout.iHeight, SWP_NOZORDER);
    ShowWindow(_this->hWnd, SW_SHOW);
    SetForegroundWindow(_this->hWnd);
    // If Alt is not pressed anymore, select first entry,
    // the user has already finished Alt-Tab quickly
    if (!(GetAsyncKeyState(VK_MENU) & 0x8000))
    {
        _sws_WindowSwitcher_SwitchToSelectedItemAndDismiss(_this);
    }
}

static DWORD WINAPI _sws_WindowSwitcher_Calculate(sws_WindowSwitcher* _this)
{
    for (unsigned int i = 0; i < _this->numLayouts; ++i)
    {
        sws_WindowSwitcherLayout_Clear(&(_this->layouts[i]));
    }
    for (unsigned int i = 0; i < _this->numLayouts; ++i)
    {
        sws_WindowSwitcherLayout_Clear(&(_this->minilayouts[i]));
    }
    _this->numLayouts = 0;
    EnumDisplayMonitors(
        NULL,
        NULL,
        _sws_WindowSwitcher_CalculateHelper,
        _this
    );
    if (_this->numLayouts > _this->numLayoutsMax)
    {
        _this->numLayoutsMax = _this->numLayouts;
    }
}

static void _sws_WindowsSwitcher_DecideThumbnails(sws_WindowSwitcher* _this, DWORD dwMode)
{
    BOOL bHaveSelected = FALSE;
    for (unsigned int j = 0; j < _this->numLayouts; ++j)
    {
        if (dwMode == SWS_WINDOWSWITCHER_LAYOUTMODE_FULL && _this->hMonitor == _this->layouts[j].hMonitor)
        {
            _this->layout = _this->layouts[j];
            bHaveSelected = TRUE;
        }
        else if (dwMode == SWS_WINDOWSWITCHER_LAYOUTMODE_MINI && _this->hMonitor == _this->minilayouts[j].hMonitor)
        {
            _this->layout = _this->minilayouts[j];
            bHaveSelected = TRUE;
        }
        sws_WindowSwitcherLayoutWindow* pWindowList;
        pWindowList = _this->layouts[j].pWindowList.pList;
        if (pWindowList)
        {
            for (unsigned int i = 0; i < _this->layouts[j].pWindowList.cbSize; ++i)
            {
                if (pWindowList[i].hThumbnail)
                {
                    DWM_THUMBNAIL_PROPERTIES dskThumbProps;
                    ZeroMemory(&dskThumbProps, sizeof(DWM_THUMBNAIL_PROPERTIES));
                    dskThumbProps.dwFlags = DWM_TNP_VISIBLE;
                    dskThumbProps.fVisible = (dwMode == SWS_WINDOWSWITCHER_LAYOUTMODE_FULL && (_this->hMonitor == _this->layouts[j].hMonitor));
                    DwmUpdateThumbnailProperties(pWindowList[i].hThumbnail, &dskThumbProps);
                }
            }
        }
        pWindowList = _this->minilayouts[j].pWindowList.pList;
        if (pWindowList)
        {
            for (unsigned int i = 0; i < _this->minilayouts[j].pWindowList.cbSize; ++i)
            {
                if (pWindowList[i].hThumbnail)
                {
                    DWM_THUMBNAIL_PROPERTIES dskThumbProps;
                    ZeroMemory(&dskThumbProps, sizeof(DWM_THUMBNAIL_PROPERTIES));
                    dskThumbProps.dwFlags = DWM_TNP_VISIBLE;
                    dskThumbProps.fVisible = (dwMode == SWS_WINDOWSWITCHER_LAYOUTMODE_MINI && (_this->hMonitor == _this->minilayouts[j].hMonitor));
                    DwmUpdateThumbnailProperties(pWindowList[i].hThumbnail, &dskThumbProps);
                }
            }
        }
    }
    if (!bHaveSelected)
    {
        if (dwMode == SWS_WINDOWSWITCHER_LAYOUTMODE_FULL)
        {
            _this->layout = _this->layouts[0];
        }
        else if (dwMode == SWS_WINDOWSWITCHER_LAYOUTMODE_MINI)
        {
            _this->layout = _this->minilayouts[0];
        }
        sws_WindowSwitcherLayoutWindow* pWindowList;
        pWindowList = _this->layouts[0].pWindowList.pList;
        if (pWindowList)
        {
            for (unsigned int i = 0; i < _this->layouts[0].pWindowList.cbSize; ++i)
            {
                if (pWindowList[i].hThumbnail)
                {
                    DWM_THUMBNAIL_PROPERTIES dskThumbProps;
                    ZeroMemory(&dskThumbProps, sizeof(DWM_THUMBNAIL_PROPERTIES));
                    dskThumbProps.dwFlags = DWM_TNP_VISIBLE;
                    dskThumbProps.fVisible = (dwMode == SWS_WINDOWSWITCHER_LAYOUTMODE_FULL);
                    DwmUpdateThumbnailProperties(pWindowList[i].hThumbnail, &dskThumbProps);
                }
            }
        }
        pWindowList = _this->minilayouts[0].pWindowList.pList;
        if (pWindowList)
        {
            for (unsigned int i = 0; i < _this->minilayouts[0].pWindowList.cbSize; ++i)
            {
                if (pWindowList[i].hThumbnail)
                {
                    DWM_THUMBNAIL_PROPERTIES dskThumbProps;
                    ZeroMemory(&dskThumbProps, sizeof(DWM_THUMBNAIL_PROPERTIES));
                    dskThumbProps.dwFlags = DWM_TNP_VISIBLE;
                    dskThumbProps.fVisible = (dwMode == SWS_WINDOWSWITCHER_LAYOUTMODE_MINI);
                    DwmUpdateThumbnailProperties(pWindowList[i].hThumbnail, &dskThumbProps);
                }
            }
        }
    }
    //printf("[sws] Decided layout.\n");
}

static void WINAPI _sws_WindowSwitcher_Show(sws_WindowSwitcher* _this)
{
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
    _sws_WindowsSwitcher_DecideThumbnails(_this, _this->mode);
    if (_this->layout.pWindowList.cbSize == 0)
    {
        ShowWindow(_this->hWnd, SW_HIDE);
        return;
    }
    if (!IsWindowVisible(_this->hWnd) && _this->dwShowDelay)
    {
        _this->bRudeChangesAllowed = FALSE;
        BOOL bCloak = TRUE;
        DwmSetWindowAttribute(_this->hWnd, DWMWA_CLOAK, &bCloak, sizeof(BOOL));
        SetTimer(_this->hWnd, SWS_WINDOWSWITCHER_TIMER_SHOW, _this->dwShowDelay, NULL);
    }
    else
    {
        BOOL bCloak = FALSE;
        DwmSetWindowAttribute(_this->hWnd, DWMWA_CLOAK, &bCloak, sizeof(BOOL));
    }
    SetWindowPos(_this->hWnd, 0, _this->layout.iX, _this->layout.iY, _this->layout.iWidth, _this->layout.iHeight, SWP_NOZORDER);
    ShowWindow(_this->hWnd, SW_SHOW);
    SetForegroundWindow(_this->hWnd);
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

static void _sws_WindowSwitcher_WindowList_Remove(sws_WindowSwitcher* _this, HWND hWnd, BOOL* bOk)
{
    if (bOk) *bOk = FALSE;
    sws_window* pHWNDList = _this->pHWNDList.pList;
    sws_vector newVector;
    sws_vector_Initialize(&newVector, sizeof(sws_window));
    for (int i = 0; i < _this->pHWNDList.cbSize; ++i)
    {
        if (IsWindow(pHWNDList[i].hWnd) && pHWNDList[i].hWnd != hWnd)
        {
            sws_vector_PushBack(&newVector, &(pHWNDList[i]));
        }
        else
        {
            sws_window_Clear(&(pHWNDList[i]));
            if (bOk) *bOk = TRUE;
        }
    }
    sws_vector_Clear(&(_this->pHWNDList));
    _this->pHWNDList = newVector;
}

static sws_window _sws_WindowSwitcher_WindowList_PushToFront(sws_WindowSwitcher* _this, HWND hWnd, BOOL* bOk)
{
    if (bOk) *bOk = FALSE;
    sws_window* pHWNDList = _this->pHWNDList.pList;
    int bContains = -1;
    for (int i = 0; i < _this->pHWNDList.cbSize; ++i)
    {
        if (pHWNDList[i].hWnd == hWnd)
        {
            bContains = i;
            break;
        }
    }
    if (bContains >= 0)
    {
        sws_window temp = pHWNDList[bContains];
        for (int i = bContains; i > 0; i--)
        {
            pHWNDList[i] = pHWNDList[i - 1];
        }
        pHWNDList[0] = temp;
        return temp;
    }
    else
    {
        sws_window zero;
        ZeroMemory(&zero, sizeof(sws_window));
        sws_vector_PushBack(&(_this->pHWNDList), &zero);
        for (int i = _this->pHWNDList.cbSize - 1; i > 0; i--)
        {
            pHWNDList[i] = pHWNDList[i - 1];
        }
        sws_window window;
        sws_window_Initialize(&window, hWnd);
        pHWNDList[0] = window;
        if (bOk) *bOk = TRUE;
        return window;
    }
}

void _sws_WindowSwitcher_SwitchToSelectedItemAndDismiss(sws_WindowSwitcher* _this)
{
    sws_WindowSwitcherLayoutWindow* pWindowList = _this->layout.pWindowList.pList;
    if (_this->layout.bIncludeWallpaper && pWindowList[_this->layout.iIndex].hWnd == _this->layout.hWndWallpaper)
    {
        BOOL bCloak = TRUE;
        DwmSetWindowAttribute(_this->hWnd, DWMWA_CLOAK, &bCloak, sizeof(BOOL));
        _sws_WindowHelpers_ToggleDesktop();
        _this->bEnabled = FALSE;
        _this->bRudeChangesAllowed = FALSE;
        SetTimer(
            _this->hWnd,
            SWS_WINDOWSWITCHER_TIMER_TOGGLE_DESKTOP,
            SWS_WINDOWSWITCHER_TIMER_TOGGLE_DESKTOP_DELAY,
            NULL
        );
    }
    else
    {
        sws_WindowSwitcherLayoutWindow* pWindowList = _this->layout.pWindowList.pList;
        if (pWindowList)
        {
            SwitchToThisWindow(pWindowList[_this->layout.iIndex].hWnd, TRUE);
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

    long long start = milliseconds_now();
    _this->lastMiniModehWnd = NULL;
    _sws_WindowSwitcher_Calculate(_this);
    long long elapsed = milliseconds_now() - start;
    printf(
        "[sws] Recomputed layouts in %lld ms because: Settings changed.\n",
        elapsed);
}

static void _sws_WindowSwitcher_DrawContour(sws_WindowSwitcher* _this, HDC hdcPaint, RECT rc, int direction, int contour_size)
{
    BYTE r = 0, g = 0, b = 0;
    if (_this->bIsDarkMode)
    {
        r = GetRValue(SWS_WINDOWSWITCHER_CONTOUR_COLOR);
        g = GetBValue(SWS_WINDOWSWITCHER_CONTOUR_COLOR);
        b = GetBValue(SWS_WINDOWSWITCHER_CONTOUR_COLOR);
    }
    else
    {
        r = GetRValue(SWS_WINDOWSWITCHER_CONTOUR_COLOR_LIGHT);
        g = GetBValue(SWS_WINDOWSWITCHER_CONTOUR_COLOR_LIGHT);
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
    RGBQUAD transparent = { 0, 0, 0, 0 };

    if (direction == SWS_CONTOUR_INNER)
    {
        StretchDIBits(hdcPaint, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
            0, 0, 1, 1, &desiredColor, &bi,
            DIB_RGB_COLORS, SRCPAINT);
    }

    int thickness = direction * (contour_size * (_this->layout.cbDpiX / DEFAULT_DPI_X));
    rc.left += thickness;
    rc.top += thickness;
    rc.right -= thickness;
    rc.bottom -= thickness;

    StretchDIBits(hdcPaint, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
        0, 0, 1, 1, (direction == SWS_CONTOUR_INNER ? &transparent : &desiredColor), &bi,
        DIB_RGB_COLORS, (direction == SWS_CONTOUR_INNER ? SRCAND : SRCPAINT));

    if (direction == SWS_CONTOUR_OUTER)
    {
        rc.left -= thickness;
        rc.top -= thickness;
        rc.right += thickness;
        rc.bottom += thickness;

        StretchDIBits(hdcPaint, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
            0, 0, 1, 1, &transparent, &bi,
            DIB_RGB_COLORS, SRCAND);
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
    if (uMsg == WM_TIMER && wParam == SWS_WINDOWSWITCHER_TIMER_STARTUP)
    {
        _this->hWndWallpaper = sws_WindowHelpers_GetWallpaperHWND();
        sws_WindowHelpers_RealEnumWindows((WNDENUMPROC)_sws_WindowSwitcher_EnumWindowsCallback, (LPARAM)_this);
        sws_window* pHWNDList = _this->pHWNDList.pList;
        sws_vector newVector;
        sws_vector_Initialize(&newVector, sizeof(sws_window));
        for (int i = _this->pHWNDList.cbSize - 1; i >= 0; i--)
        {
            sws_vector_PushBack(&newVector, &(pHWNDList[i]));
        }
        sws_vector_Clear(&(_this->pHWNDList));
        _this->pHWNDList = newVector;

        sws_WindowSwitcher_RefreshTheme(_this);

        _this->bEnabled = TRUE;

        KillTimer(hWnd, SWS_WINDOWSWITCHER_TIMER_STARTUP);
        return 0;
    }
    else if (uMsg == WM_TIMER && wParam == SWS_WINDOWSWITCHER_TIMER_TOGGLE_DESKTOP)
    {
        if (!_this->layout.bWallpaperAlwaysLast)
        {
            _sws_WindowSwitcher_WindowList_PushToFront(_this, _this->layout.hWndWallpaper, NULL);
        }
        long long start = milliseconds_now();
        _sws_WindowSwitcher_Calculate(_this);
        long long elapsed = milliseconds_now() - start;
        printf(
            "[sws] Recomputed layouts in %lld ms because: Toggle desktop.\n",
            elapsed);
        BOOL bCloak = FALSE;
        DwmSetWindowAttribute(_this->hWnd, DWMWA_CLOAK, &bCloak, sizeof(BOOL));
        KillTimer(hWnd, SWS_WINDOWSWITCHER_TIMER_TOGGLE_DESKTOP);
        _this->bRudeChangesAllowed = TRUE;
        _this->bEnabled = TRUE;
        return 0;
    }
    else if (uMsg == WM_TIMER && wParam == SWS_WINDOWSWITCHER_TIMER_SHOW)
    {
        BOOL bCloak = FALSE;
        DwmSetWindowAttribute(_this->hWnd, DWMWA_CLOAK, &bCloak, sizeof(BOOL));
        KillTimer(hWnd, SWS_WINDOWSWITCHER_TIMER_SHOW);
        _this->bRudeChangesAllowed = TRUE;
        return 0;
    }
    else if (uMsg == WM_TIMER && wParam == SWS_WINDOWSWITCHER_TIMER_PEEKATDESKTOP)
    {
        PostMessageW(FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL), WM_HOTKEY, 516, 0);
        KillTimer(hWnd, SWS_WINDOWSWITCHER_TIMER_PEEKATDESKTOP);
        return 0;
    }
    else if (_this && uMsg == _this->msgShellHook)
    {
        if (wParam == HSHELL_WINDOWCREATED || wParam == HSHELL_WINDOWDESTROYED || wParam == HSHELL_WINDOWACTIVATED || wParam == HSHELL_RUDEAPPACTIVATED)
        {
            sws_window window;
            window.hWnd = 0;
            BOOL bOk = FALSE;
            if (wParam == HSHELL_WINDOWDESTROYED)
            {
                if (lParam != hWnd)
                {
                    /*wchar_t text[200];
                    GetWindowTextW(lParam, text, 200);
                    wprintf(L"[-] [%d] %d : %s\n", _this->pHWNDList.cbSize, lParam, text);*/
                    _sws_WindowSwitcher_WindowList_Remove(_this, lParam, &bOk);
                }
            }
            else if (wParam == HSHELL_WINDOWCREATED || wParam == HSHELL_WINDOWACTIVATED || wParam == HSHELL_RUDEAPPACTIVATED)
            {
                if (lParam && lParam != hWnd && sws_WindowHelpers_IsAltTabWindow(lParam, NULL))
                {
                    /*if (wParam == HSHELL_WINDOWCREATED)
                    {
                        wchar_t text[200];
                        GetWindowTextW(lParam, text, 200);
                        wprintf(L"[+] [%d] %d : %s\n", _this->pHWNDList.cbSize, lParam, text);
                    }*/
                    window = _sws_WindowSwitcher_WindowList_PushToFront(_this, lParam, NULL);
                    bOk = TRUE;
                }
                if (!lParam)
                {
                    bOk = TRUE;
                }
            }
            if (bOk && _this->bRudeChangesAllowed)
            {
                BOOL isCloaked = FALSE;
                DwmGetWindowAttribute(lParam, DWMWA_CLOAKED, &isCloaked, sizeof(BOOL));
                if (!isCloaked || !lParam) // || (window.hWnd && window.bIsApplicationFrameHost))
                {
                    long long start = milliseconds_now();
                    if (!IsWindowVisible(_this->hWnd))
                    {
                        _this->lastMiniModehWnd = NULL;
                    }
                    _sws_WindowSwitcher_Calculate(_this);
                    long long elapsed = milliseconds_now() - start;
                    printf(
                        wParam == HSHELL_WINDOWCREATED ?
                        "[sws] Recomputed layouts in %lld ms because: Window created, mode %d.\n" :
                        (wParam == HSHELL_WINDOWDESTROYED ?
                            "[sws] Recomputed layouts in %lld ms because: Window destroyed, mode %d.\n" :
                            "[sws] Recomputed layouts in %lld ms because: Foreground window changed, mode %d.\n"),
                        elapsed, _this->mode);
                    _sws_WindowsSwitcher_DecideThumbnails(_this, _this->mode);
                    if (IsWindowVisible(_this->hWnd))
                    {
                        RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
                        _sws_WindowSwitcher_Show(_this);
                    }
                }
                return 0;
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
            _this->bRudeChangesAllowed = TRUE;
            _this->lastMiniModehWnd = NULL;
            //sws_WindowSwitcherLayout_Clear(&(_this->layout));
        }
        else
        {
            //SetWindowPos(_this->hWnd, 0, _this->layout.iX, _this->layout.iY, _this->layout.iWidth, _this->layout.iHeight, SWP_NOZORDER);
        }
        return 0;
    }
    else if (uMsg == WM_PAINT)
    {
        PAINTSTRUCT ps;
        HDC hDC = BeginPaint(hWnd, &ps);

        sws_WindowSwitcherLayoutWindow* pWindowList = _this->layout.pWindowList.pList;

        RECT rc;
        GetClientRect(hWnd, &rc);

        HDC hdcPaint = NULL;
        BP_PAINTPARAMS params = { sizeof(BP_PAINTPARAMS) };
        params.dwFlags = BPPF_ERASE;
        HPAINTBUFFER hBufferedPaint = BeginBufferedPaint(hDC, &rc, BPBF_TOPDOWNDIB, &params, &hdcPaint);
        HFONT hOldFont = NULL;
        if (hdcPaint)
        {
            // Draw background when the themeing engine is disabled
            if (!IsThemeActive() || _this->dwTheme == SWS_WINDOWSWITCHER_THEME_NONE)
            {
                COLORREF oldcr = SetBkColor(hdcPaint, _this->bIsDarkMode ? SWS_WINDOWSWITCHER_BACKGROUND_COLOR : SWS_WINDOWSWITCHER_BACKGROUND_COLOR_LIGHT);
                ExtTextOutW(hdcPaint, 0, 0, ETO_OPAQUE, &rc, L"", 0, 0);
                SetBkColor(hdcPaint, oldcr);
                SetTextColor(hdcPaint, GetSysColor(COLOR_WINDOWTEXT));
            }

            hOldFont = SelectObject(hdcPaint, _this->layout.hFontRegular);

            // Draw highlight rectangle
            if (pWindowList)
            {
                if (!IsThemeActive() || _this->dwTheme == SWS_WINDOWSWITCHER_THEME_NONE)
                {
                    RECT rc = pWindowList[_this->layout.iIndex].rcWindow;
                    rc.left += 1;
                    rc.top += 1;
                    rc.right -= 1;
                    rc.bottom -= 1;
                    for (unsigned int i = 0; i < SWS_WINDOWSWITCHER_CONTOUR_SIZE * (_this->layout.cbDpiX / DEFAULT_DPI_X); ++i)
                    {
                        FrameRect(hdcPaint, &rc, _this->hContourBrush);
                        rc.left += 1;
                        rc.top += 1;
                        rc.right -= 1;
                        rc.bottom -= 1;
                    }
                }
                else
                {
                    _sws_WindowSwitcher_DrawContour(_this, hdcPaint, pWindowList[_this->layout.iIndex].rcWindow, SWS_CONTOUR_INNER, SWS_WINDOWSWITCHER_CONTOUR_SIZE);
                }
            }

            // Draw hover rectangle
            if (pWindowList &&
                _this->cwIndex != -1 &&
                _this->cwIndex < _this->layout.pWindowList.cbSize &&
                _this->cwMask & SWS_WINDOWFLAG_IS_ON_THUMBNAIL
                )
            {
                if (!IsThemeActive() || _this->dwTheme == SWS_WINDOWSWITCHER_THEME_NONE)
                {
                    RECT rc = pWindowList[_this->cwIndex].rcThumbnail;
                    rc.left -= 1;
                    rc.top -= 1;
                    rc.right += 1;
                    rc.bottom += 1;
                    for (unsigned int i = 0; i < SWS_WINDOWSWITCHER_HIGHLIGHT_SIZE * (_this->layout.cbDpiX / DEFAULT_DPI_X); ++i)
                    {
                        FrameRect(hdcPaint, &rc, _this->hContourBrush);
                        rc.left -= 1;
                        rc.top -= 1;
                        rc.right += 1;
                        rc.bottom += 1;
                    }
                }
                else
                {
                    _sws_WindowSwitcher_DrawContour(_this, hdcPaint, pWindowList[_this->cwIndex].rcThumbnail, SWS_CONTOUR_OUTER, SWS_WINDOWSWITCHER_HIGHLIGHT_SIZE);
                }
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
                        GetWindowTextW(pWindowList[i].hWnd, wszTitle, MAX_PATH);
                    }
                    if ((rcText.right - rcText.left) > (_this->layout.cbDpiX / DEFAULT_DPI_X) * 10)
                    {
                        if (_this->hTheme && IsThemeActive())
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
                            SetTextColor(hdcPaint, _this->bIsDarkMode ? SWS_WINDOWSWITCHER_TEXT_COLOR : SWS_WINDOWSWITCHER_TEXT_COLOR_LIGHT);
                            SetBkColor(hdcPaint, _this->bIsDarkMode ? SWS_WINDOWSWITCHER_BACKGROUND_COLOR : SWS_WINDOWSWITCHER_BACKGROUND_COLOR_LIGHT);
                            DrawTextW(
                                hdcPaint,
                                wszTitle,
                                -1,
                                &rcText,
                                dwTextFlags
                            );
                        }
                    }
                }
            }

            // Draw icon
            for (unsigned int i = 0; i < _this->layout.pWindowList.cbSize; ++i)
            {
                if (pWindowList && pWindowList[i].iRowMax)
                {
                    rc = pWindowList[i].rcWindow;
                    DrawIconEx(
                        hdcPaint,
                        rc.left + _this->layout.cbLeftPadding + ((_this->layout.cbRowTitleHeight - pWindowList[i].szIcon) / 4.0) - pWindowList[i].rcIcon.left,
                        rc.top + _this->layout.cbTopPadding + ((_this->layout.cbRowTitleHeight - pWindowList[i].szIcon) / 4.0) - pWindowList[i].rcIcon.top,
                        pWindowList[i].hIcon,
                        pWindowList[i].rcIcon.right,
                        pWindowList[i].rcIcon.bottom,
                        0,
                        _this->hBackgroundBrush,
                        DI_NORMAL
                    );
                }
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
                        SetTextColor(hdcPaint, DttOpts.crText);
                        SetBkColor(hdcPaint, _this->bIsDarkMode ? SWS_WINDOWSWITCHER_BACKGROUND_COLOR : SWS_WINDOWSWITCHER_BACKGROUND_COLOR_LIGHT);
                        DrawTextW(
                            hdcPaint,
                            L"\u2B1B",
                            -1,
                            &rcText,
                            dwTextFlags
                        );
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
                        L"\u274C",
                        -1,
                        dwTextFlags,
                        &rcText,
                        &DttOpts
                    );
                }
                else
                {
                    SetTextColor(hdcPaint, DttOpts.crText);
                    SetBkColor(hdcPaint, _this->cwMask & SWS_WINDOWFLAG_IS_ON_CLOSE ? SWS_WINDOWSWITCHER_CLOSE_COLOR : (_this->bIsDarkMode ? SWS_WINDOWSWITCHER_BACKGROUND_COLOR : SWS_WINDOWSWITCHER_BACKGROUND_COLOR_LIGHT));
                    DrawTextW(
                        hdcPaint,
                        L"\u274C",
                        -1,
                        &rcText,
                        dwTextFlags
                    );
                }
            }
            SelectObject(hdcPaint, hOldFont);
            EndBufferedPaint(hBufferedPaint, TRUE);
        }

        EndPaint(hWnd, &ps);
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
                RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
            }
        }
        _this->bIsMouseClicking = FALSE;
        return 0;
    }
    else if (uMsg == WM_LBUTTONDOWN)
    {
        _this->bIsMouseClicking = TRUE;
        return 0;
    }
    else if (uMsg == WM_LBUTTONUP)
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
                            _this->cwMask & SWS_WINDOWFLAG_IS_ON_CLOSE &&
                            !(_this->layout.bIncludeWallpaper && pWindowList[i].hWnd == _this->layout.hWndWallpaper)
                            )
                        {
                            PostMessageW(pWindowList[i].hWnd, WM_SYSCOMMAND, SC_CLOSE, 0);
                            //EndTask(pWindowList[i].hWnd, FALSE, FALSE);
                            //PostMessageW(pWindowList[i].hWnd, WM_CLOSE, 0, 0);
                            return 0;
                        }
                        _this->layout.iIndex = i;
                        _sws_WindowSwitcher_SwitchToSelectedItemAndDismiss(_this);
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
             (uMsg == WM_KEYUP && wParam == VK_SPACE) ||
             (uMsg == WM_KEYUP && wParam == VK_RETURN) ||
             (uMsg == WM_KEYUP && wParam == (_this->mode == SWS_WINDOWSWITCHER_LAYOUTMODE_MINI ? VK_OEM_3 : VK_TAB) && !(GetKeyState(VK_MENU) & 0x8000) && !_this->bWasControl))
    {
        _sws_WindowSwitcher_SwitchToSelectedItemAndDismiss(_this);
        return 0;
    }
    else if (uMsg == WM_HOTKEY || uMsg == WM_KEYDOWN)
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
        if ((uMsg == WM_KEYDOWN && wParam == VK_OEM_3) || 
            (uMsg == WM_KEYDOWN && wParam == VK_TAB) || 
            (uMsg == WM_HOTKEY && (LOWORD(lParam) & MOD_ALT)) || 
            (uMsg == WM_KEYDOWN && wParam == VK_LEFT) ||
            (uMsg == WM_KEYDOWN && wParam == VK_RIGHT) ||
            (uMsg == WM_KEYDOWN && wParam == VK_UP) ||
            (uMsg == WM_KEYDOWN && wParam == VK_DOWN)
            )
        {
            if (_this->bEnabled && !IsWindowVisible(_this->hWnd))
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
                    (uMsg == WM_KEYDOWN && (GetKeyState(VK_SHIFT) & 0x8000)) ||
                    (uMsg == WM_HOTKEY && (LOWORD(lParam) & MOD_SHIFT)) ||
                    (uMsg == WM_KEYDOWN && wParam == VK_LEFT) ||
                    (uMsg == WM_KEYDOWN && wParam == VK_UP)
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

                        if (uMsg == WM_KEYDOWN && wParam == VK_UP)
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

                        if (uMsg == WM_KEYDOWN && wParam == VK_DOWN)
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
                if (indexStart != -1 && ((uMsg == WM_KEYDOWN && wParam == VK_DOWN) || (uMsg == WM_KEYDOWN && wParam == VK_UP)))
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

                        i = (uMsg == WM_KEYDOWN && wParam == VK_DOWN) ? (i - 1) : (i + 1);
                    }

                    _this->layout.iIndex = minIndex;
                }

                RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
                SetForegroundWindow(_this->hWnd);
            }
        }
        return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
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

__declspec(dllexport) void sws_WindowSwitcher_Clear(sws_WindowSwitcher* _this)
{
    if (_this)
    {
        CloseHandle(_this->hEvExit);
        if (_this->bWithRegMon)
        {
            sws_RegistryMonitor_Clear(&(_this->rm));
        }
        for (unsigned int i = 0; i < _this->numLayoutsMax; ++i)
        {
            sws_WindowSwitcherLayout_Clear(&(_this->layouts[i]));
        }
        for (unsigned int i = 0; i < _this->numLayoutsMax; ++i)
        {
            sws_WindowSwitcherLayout_Clear(&(_this->minilayouts[i]));
        }
        sws_vector_Clear(&(_this->pHWNDList));
        //UnregisterHotKey(_this->hWnd, 0);
        UnregisterHotKey(_this->hWnd, 1);
        UnregisterHotKey(_this->hWnd, 2);
        UnregisterHotKey(_this->hWnd, 3);
        UnregisterHotKey(_this->hWnd, 4);
        UnregisterHotKey(_this->hWnd, -1);
        UnregisterHotKey(_this->hWnd, -2);
        UnregisterHotKey(_this->hWnd, -3);
        UnregisterHotKey(_this->hWnd, -4);
        UnhookWinEvent(_this->global_hook);
        DestroyWindow(_this->hWnd);
        UnregisterClassW(_T(SWS_WINDOWSWITCHER_CLASSNAME), GetModuleHandle(NULL));
        DeleteObject(_this->hBackgroundBrush);
        DeleteObject(_this->hContourBrush);
        DeleteObject(_this->hCloseButtonBrush);
        CloseThemeData(_this->hTheme);
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
        _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
        _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
        _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
        _CrtDumpMemoryLeaks();
    }
}

__declspec(dllexport) sws_error_t sws_WindowSwitcher_Initialize(sws_WindowSwitcher** __this, BOOL bWithRegMon)
{
    sws_error_t rv = SWS_ERROR_SUCCESS;
    sws_WindowSwitcher* _this = NULL;
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
    _CrtDumpMemoryLeaks();

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
            memset((*__this), 0, sizeof(sws_WindowSwitcher));
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
    }
    if (!rv)
    {
        sws_error_Report(sws_error_GetFromInternalError(sws_WindowHelpers_ShouldSystemUseDarkMode(&(_this->bIsDarkMode))), NULL);
        _this->hBackgroundBrush = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
        _this->hContourBrush = (HBRUSH)CreateSolidBrush(_this->bIsDarkMode ? SWS_WINDOWSWITCHER_CONTOUR_COLOR : SWS_WINDOWSWITCHER_CONTOUR_COLOR_LIGHT);
        _this->hCloseButtonBrush = (HBRUSH)CreateSolidBrush(SWS_WINDOWSWITCHER_CLOSE_COLOR);
        _this->hTheme = OpenThemeData(NULL, _T(SWS_WINDOWSWITCHER_THEME_CLASS));
        _this->numLayouts = 0;
        _this->numLayoutsMax = 0;
        _this->last_change = 0;
        _this->bRudeChangesAllowed = TRUE;
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
#ifndef _DEBUG
        _this->hWnd = _sws_CreateWindowInBand(
#else
        _this->hWnd = CreateWindowEx(
#endif
            WS_EX_TOOLWINDOW,
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
            EVENT_SYSTEM_MOVESIZEEND,
            EVENT_OBJECT_UNCLOAKED,
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
        if (!RegisterShellHookWindow(_this->hWnd))
        {
            rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
        }
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
            rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
        }
    }
    if (!rv)
    {
        if (!RegisterHotKey(_this->hWnd, -1, MOD_ALT, VK_OEM_3))
        {
            rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
        }
    }
    if (!rv)
    {
        if (!RegisterHotKey(_this->hWnd, 2, MOD_ALT | MOD_SHIFT, VK_TAB))
        {
            rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
        }
    }
    if (!rv)
    {
        if (!RegisterHotKey(_this->hWnd, -2, MOD_ALT | MOD_SHIFT, VK_OEM_3))
        {
            rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
        }
    }
    if (!rv)
    {
        if (!RegisterHotKey(_this->hWnd, 3, MOD_ALT | MOD_CONTROL, VK_TAB))
        {
            rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
        }
    }
    if (!rv)
    {
        if (!RegisterHotKey(_this->hWnd, -3, MOD_ALT | MOD_CONTROL, VK_OEM_3))
        {
            rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
        }
    }
    if (!rv)
    {
        if (!RegisterHotKey(_this->hWnd, 4, MOD_ALT | MOD_SHIFT | MOD_CONTROL, VK_TAB))
        {
            rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
        }
    }
    if (!rv)
    {
        if (!RegisterHotKey(_this->hWnd, -4, MOD_ALT | MOD_SHIFT | MOD_CONTROL, VK_OEM_3))
        {
            rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
        }
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
        _this->hEvExit = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (!_this->hEvExit)
        {
            rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
        }
    }
    if (!rv)
    {
        _this->bWithRegMon = bWithRegMon;
        if (_this->bWithRegMon)
        {
            DWORD dwInitial = 100, dwSize = sizeof(DWORD);
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
        _this->dwShowDelay = SWS_WINDOWSWITCHER_SHOWDELAY;
        _this->dwMaxWP = SWS_WINDOWSWITCHERLAYOUT_PERCENTAGEWIDTH;
        _this->dwMaxHP = SWS_WINDOWSWITCHERLAYOUT_PERCENTAGEHEIGHT;
        _this->bIncludeWallpaper = SWS_WINDOWSWITCHERLAYOUT_INCLUDE_WALLPAPER;
        _this->dwRowHeight = SWS_WINDOWSWITCHERLAYOUT_ROWHEIGHT;
        _this->dwColorScheme = 0;
        _this->dwTheme = SWS_WINDOWSWITCHER_THEME_MICA;
        _this->dwCornerPreference = DWMWCP_ROUND;
        BOOL bExcludedFromPeek = TRUE;
        DwmSetWindowAttribute(_this->hWnd, DWMWA_EXCLUDED_FROM_PEEK, &bExcludedFromPeek, sizeof(BOOL));
    }
    if (rv && (*__this) && (*__this)->bIsDynamic)
    {
        free((*__this));
        (*__this) = NULL;
    }

    return rv;
}