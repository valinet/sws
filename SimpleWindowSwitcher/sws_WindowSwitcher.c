#include "sws_WindowSwitcher.h"

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
    if (_this->hTheme && IsThemeActive())
    {
        sws_WindowHelpers_SetWindowBlur(
            _this->hWnd,
            4,
            _this->bIsDarkMode ? SWS_WINDOWSWITCHER_BACKGROUND_COLOR : SWS_WINDOWSWITCHER_BACKGROUND_COLOR_LIGHT,
            blur
        );
    }
}

static BOOL CALLBACK _sws_WindowSwitcher_EnumWindowsCallback(_In_ HWND hWnd, _In_ sws_WindowSwitcher* _this)
{
    if (sws_WindowHelpers_IsAltTabWindow(hWnd))
    {
        if (hWnd == GetForegroundWindow())
        {
            ShowWindow(_this->hWnd, SW_SHOW);
            SwitchToThisWindow(_this->hWndLast, TRUE);
            ShowWindow(_this->hWnd, SW_HIDE);
            _this->hWndLast = 0;
            return FALSE;
        }
        _this->hWndLast = hWnd;
    }

    return TRUE;
}

static DWORD WINAPI _sws_WindowSwitcher_Show(sws_WindowSwitcher* _this)
{
    //Sleep(100);
    POINT ptCursor;
    GetCursorPos(&ptCursor);
    _this->hMonitor = MonitorFromPoint(ptCursor, MONITOR_DEFAULTTOPRIMARY);
    sws_WindowSwitcherLayout_Initialize(&(_this->layout), _this->hMonitor, _this->hWnd, &(_this->dwRowHeight));
    wchar_t* wszClassName[100];
    GetClassNameW(GetForegroundWindow(), wszClassName, 100);
    if (_this->layout.bIncludeWallpaper && _this->layout.bWallpaperAlwaysLast && !wcscmp(wszClassName, L"WorkerW"))
    {
        sws_WindowSwitcherLayout_ComputeLayout(&(_this->layout), SWS_WINDOWSWITCHERLAYOUT_COMPUTE_DIRECTION_INITIAL, sws_WindowHelpers_GetWallpaperHWND(_this->hMonitor));
    }
    else
    {
        sws_WindowSwitcherLayout_ComputeLayout(&(_this->layout), SWS_WINDOWSWITCHERLAYOUT_COMPUTE_DIRECTION_INITIAL, NULL);
    }

    //sws_WindowSwitcherLayoutWindow* pWindowList = _this->layout.pWindowList.pList;
    ///for (int i = 0; i < _this->layout.pWindowList.cbSize; ++i)
    //{
    //    printf("%d %d\n", pWindowList[i].rcThumbnail.left, pWindowList[i].rcThumbnail.top);
    //}
    _this->layout.iIndex = _this->layout.pWindowList.cbSize == 1 ? 0 : _this->layout.iIndex - 1 - _this->layout.numTopMost;
    if (_this->layout.bIncludeWallpaper && !wcscmp(wszClassName, L"WorkerW"))
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
    sws_WindowSwitcher_SetTransparencyFromRegistry(_this, HKEY_CURRENT_USER);
    if (_this->hContourBrush)
    {
        DeleteObject(_this->hContourBrush);
    }
    _this->hContourBrush = (HBRUSH)CreateSolidBrush(_this->bIsDarkMode ? SWS_WINDOWSWITCHER_CONTOUR_COLOR : SWS_WINDOWSWITCHER_CONTOUR_COLOR_LIGHT);
}

static LRESULT _sws_WindowsSwitcher_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    sws_WindowSwitcher* _this = NULL;
    if (uMsg == WM_CREATE)
    {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)(lParam);
        _this = (struct sws_WindowSwitcher*)(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)_this);
    }
    else
    {
        LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
        _this = (struct sws_WindowSwitcher*)(ptr);
    }

    if (_this && uMsg == _this->msgShellHook)
    {
        if (IsWindowVisible(_this->hWnd) && (wParam == HSHELL_WINDOWCREATED || wParam == HSHELL_WINDOWDESTROYED))
        {
            BOOL bOk = FALSE;
            if (wParam == HSHELL_WINDOWDESTROYED)
            {
                sws_WindowSwitcherLayoutWindow* pWindowList = _this->layout.pWindowList.pList;
                if (pWindowList)
                {
                    for (unsigned int i = 0; i < _this->layout.pWindowList.cbSize; ++i)
                    {
                        if (pWindowList[i].hWnd == lParam)
                        {
                            bOk = TRUE;
                        }
                    }
                }
            }
            else
            {
                if (_sws_IsTopLevelWindow(lParam))
                {
                    bOk = TRUE;
                }
            }
            if (bOk)
            {
                sws_WindowSwitcherLayout_Clear(&(_this->layout));
                _sws_WindowSwitcher_Show(_this);
                RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
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
        sws_WindowSwitcherLayout_Clear(&(_this->layout));
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
            sws_WindowSwitcherLayout_Clear(&(_this->layout));
        }
        else
        {
            SetWindowPos(_this->hWnd, 0, _this->layout.iX, _this->layout.iY, _this->layout.iWidth, _this->layout.iHeight, SWP_NOZORDER);
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
            if (!IsThemeActive())
            {
                COLORREF oldcr = SetBkColor(hdcPaint, _this->bIsDarkMode ? SWS_WINDOWSWITCHER_BACKGROUND_COLOR : SWS_WINDOWSWITCHER_BACKGROUND_COLOR_LIGHT);
                ExtTextOutW(hdcPaint, 0, 0, ETO_OPAQUE, &rc, L"", 0, 0);
                SetBkColor(hdcPaint, oldcr);
                SetTextColor(hdcPaint, GetSysColor(COLOR_WINDOWTEXT));
            }

            hOldFont = SelectObject(hdcPaint, _this->layout.hFontRegular);

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
                    rcText.right = rc.right - _this->layout.cbRowTitleHeight;
                    rcText.bottom = rc.top + _this->layout.cbRowTitleHeight;

                    wchar_t wszTitle[MAX_PATH];
                    memset(wszTitle, 0, MAX_PATH * sizeof(wchar_t));
                    if (_this->layout.bIncludeWallpaper && pWindowList[i].hWnd == _this->layout.hWndWallpaper)
                    {
                        sws_WindowHelpers_GetDesktopText(wszTitle);
                    }
                    else
                    {
                        GetWindowText(pWindowList[i].hWnd, wszTitle, MAX_PATH);
                    }
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

            // Draw highlight rectangle
            for (int j = (int)(_this->layout.iIndex); j < (int)_this->layout.iIndex + 1; ++j) // -1, +2
            {
                int k = j;
                if (j == -1)
                {
                    k = _this->layout.pWindowList.cbSize - 1;
                }
                else if (j == _this->layout.pWindowList.cbSize)
                {
                    k = 0;
                }
                HBRUSH hBrush = _this->hBackgroundBrush;
                if (pWindowList)
                {
                    rc = pWindowList[k].rcWindow;
                    if (k == _this->layout.iIndex)
                    {
                        hBrush = _this->hContourBrush;
                    }
                    rc.left += 1;
                    rc.top += 1;
                    rc.right -= 1;
                    rc.bottom -= 1;
                    for (unsigned int i = 0; i < SWS_WINDOWSWITCHER_CONTOUR_SIZE * (_this->layout.cbDpiX / DEFAULT_DPI_X); ++i)
                    {
                        FrameRect(hdcPaint, &rc, hBrush);
                        rc.left += 1;
                        rc.top += 1;
                        rc.right -= 1;
                        rc.bottom -= 1;
                    }
                }
            }

            // Draw hover rectangle
            if (pWindowList &&
                _this->cwIndex != -1 &&
                _this->cwIndex < _this->layout.pWindowList.cbSize &&
                _this->cwMask & SWS_WINDOWFLAG_IS_ON_THUMBNAIL
                )
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
                        SetTextColor(hdcPaint, SWS_WINDOWSWITCHER_CLOSE_COLOR);
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
                DttOpts.crText = _this->bIsDarkMode ? SWS_WINDOWSWITCHER_TEXT_COLOR : SWS_WINDOWSWITCHER_TEXT_COLOR_LIGHT;
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
                    SetTextColor(hdcPaint, _this->bIsDarkMode ? SWS_WINDOWSWITCHER_TEXT_COLOR : SWS_WINDOWSWITCHER_TEXT_COLOR_LIGHT);
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
        return 0;
    }
    else if (uMsg == WM_LBUTTONUP)
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
                        //CreateThread(0, 0, _sws_WindowHelpers_CloseWindow, pWindowList[i].hWnd, 0, 0);
                        EndTask(pWindowList[i].hWnd, FALSE, FALSE);
                        //PostMessageW(pWindowList[i].hWnd, WM_CLOSE, 0, 0);
                        return 0;
                    }
                    _this->layout.iIndex = i;
                    _sws_WindowSwitcher_SwitchToSelectedItemAndDismiss(_this);
                }
            }
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
             (uMsg == WM_KEYUP && wParam == VK_TAB && !(GetKeyState(VK_MENU) & 0x8000) && !_this->bWasControl))
    {
        _sws_WindowSwitcher_SwitchToSelectedItemAndDismiss(_this);
        return 0;
    }
    else if (uMsg == WM_HOTKEY || uMsg == WM_KEYDOWN)
    {
        if (uMsg == WM_HOTKEY && (LOWORD(lParam) & MOD_CONTROL))
        {
            _this->bWasControl = TRUE;
        }
        if ((uMsg == WM_KEYDOWN && wParam == VK_TAB) || (uMsg == WM_HOTKEY && (LOWORD(lParam) & MOD_ALT)))
        {
            if (!IsWindowVisible(_this->hWnd))
            {
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
                if ((uMsg == WM_KEYDOWN && (GetKeyState(VK_SHIFT) & 0x8000)) || (uMsg == WM_HOTKEY && (LOWORD(lParam) & MOD_SHIFT)))
                {
                    if (_this->layout.iIndex == _this->layout.pWindowList.cbSize - 1)
                    {
                        _this->layout.iIndex = 0;
                    }
                    else
                    {
                        _this->layout.iIndex++;
                    }
                }
                else
                {
                    if (_this->layout.iIndex == 0)
                    {
                        _this->layout.iIndex = _this->layout.pWindowList.cbSize - 1;
                    }
                    else
                    {
                        _this->layout.iIndex--;
                    }
                }
                sws_WindowSwitcherLayoutWindow* pWindowList = _this->layout.pWindowList.pList;
                if (!pWindowList[_this->layout.iIndex].hThumbnail)
                {
                    sws_WindowSwitcherLayout_ComputeLayout(
                        &(_this->layout),
                        (LOWORD(lParam) & MOD_SHIFT) ? SWS_WINDOWSWITCHERLAYOUT_COMPUTE_DIRECTION_BACKWARD : SWS_WINDOWSWITCHERLAYOUT_COMPUTE_DIRECTION_FORWARD,
                        NULL
                    );
                    _this->cwIndex = -1;
                    _this->cwMask = 0;
                }
                RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
                SetForegroundWindow(_this->hWnd);
            }
            return 0;
        }
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
    wc.hInstance = GetModuleHandle(NULL);
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
        sws_WindowSwitcherLayout_Clear(&(_this->layout));
        UnregisterHotKey(_this->hWnd, 0);
        UnregisterHotKey(_this->hWnd, 1);
        UnregisterHotKey(_this->hWnd, 2);
        UnregisterHotKey(_this->hWnd, 3);
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
        sws_error_Report(sws_error_GetFromInternalError(sws_WindowHelpers_ShouldSystemUseDarkMode(&(_this->bIsDarkMode))), NULL);
        _this->hBackgroundBrush = (HBRUSH)CreateSolidBrush(SWS_WINDOWSWITCHER_BACKGROUND_COLOR);
        _this->hContourBrush = (HBRUSH)CreateSolidBrush(_this->bIsDarkMode ? SWS_WINDOWSWITCHER_CONTOUR_COLOR : SWS_WINDOWSWITCHER_CONTOUR_COLOR_LIGHT);
        _this->hCloseButtonBrush = (HBRUSH)CreateSolidBrush(SWS_WINDOWSWITCHER_CLOSE_COLOR);
        _this->hTheme = OpenThemeData(NULL, _T(SWS_WINDOWSWITCHER_THEME_CLASS));
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
#ifndef DEBUG
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
#ifndef DEBUG
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
        if (!RegisterHotKey(_this->hWnd, 0, MOD_ALT, VK_TAB))
        {
            rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
        }
    }
    if (!rv)
    {
        if (!RegisterHotKey(_this->hWnd, 1, MOD_ALT | MOD_SHIFT, VK_TAB))
        {
            rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
        }
    }
    if (!rv)
    {
        if (!RegisterHotKey(_this->hWnd, 2, MOD_ALT | MOD_CONTROL, VK_TAB))
        {
            rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()), NULL);
        }
    }
    if (!rv)
    {
        if (!RegisterHotKey(_this->hWnd, 3, MOD_ALT | MOD_SHIFT | MOD_CONTROL, VK_TAB))
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
                _sws_WindowSwitcher_NotifyTransparencyChange,
                _this
            );
            _sws_WindowSwitcher_NotifyTransparencyChange(_this, FALSE, &dwInitial, dwSize);
        }
        if (_this->hTheme && IsThemeActive())
        {
            INT preference = DWMWCP_ROUND;
            DwmSetWindowAttribute(_this->hWnd, DWMWA_WINDOW_CORNER_PREFERENCE, &preference, sizeof(preference));
        }
        _this->dwMaxWP = SWS_WINDOWSWITCHERLAYOUT_PERCENTAGEWIDTH;
        _this->dwMaxHP = SWS_WINDOWSWITCHERLAYOUT_PERCENTAGEHEIGHT;
        _this->bIncludeWallpaper = SWS_WINDOWSWITCHERLAYOUT_INCLUDE_WALLPAPER;
        _this->dwRowHeight = SWS_WINDOWSWITCHERLAYOUT_ROWHEIGHT;
        _this->dwColorScheme = 0;
    }
    if (rv && (*__this) && (*__this)->bIsDynamic)
    {
        free((*__this));
        (*__this) = NULL;
    }

    return rv;
}