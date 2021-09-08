#include "sws_WindowSwitcher.h"

static void CALLBACK _sws_WindowSwitcher_NotifySettingsChange(sws_WindowSwitcher* _this, BOOL bIsInHKLM, DWORD* value, size_t size)
{
    DWORD blur = (*value / 100.0) * 255;
    if (blur == 255)
    {
        SetLayeredWindowAttributes(
            _this->hWnd,
            SWS_WINDOWSWITCHER_BACKGROUND_COLOR,
            blur,
            LWA_ALPHA
        );
    }
    else
    {
        SetLayeredWindowAttributes(
            _this->hWnd,
            SWS_WINDOWSWITCHER_BACKGROUND_COLOR,
            255,
            LWA_ALPHA
        );
        sws_WindowHelpers_SetWindowBlur(_this->hWnd, 4, SWS_WINDOWSWITCHER_BACKGROUND_COLOR, blur);
    }
}

static DWORD WINAPI _sws_WindowSwitcher_PrepareShow(sws_WindowSwitcher* _this)
{
    Sleep(100);
#ifndef COMPILE_AS_LIBRARY
    _this->pTaskList->lpVtbl->DeleteTab(_this->pTaskList, _this->hWnd);
#endif
    POINT ptCursor;
    GetCursorPos(&ptCursor);
    _this->hMonitor = MonitorFromPoint(ptCursor, MONITOR_DEFAULTTOPRIMARY);
    sws_WindowSwitcherLayout_Initialize(&(_this->layout), _this->hMonitor, _this->hWnd);
    sws_WindowSwitcherLayout_ComputeLayout(&(_this->layout), SWS_WINDOWSWITCHERLAYOUT_COMPUTE_DIRECTION_INITIAL);
    //sws_WindowSwitcherLayoutWindow* pWindowList = _this->layout.pWindowList.pList;
    ///for (int i = 0; i < _this->layout.pWindowList.cbSize; ++i)
    //{
    //    printf("%d %d\n", pWindowList[i].rcThumbnail.left, pWindowList[i].rcThumbnail.top);
    //}
    _this->layout.iIndex = _this->layout.pWindowList.cbSize == 1 ? 0 : _this->layout.iIndex - 1 - _this->layout.numTopMost;
    RECT rcZero;
    memset(&rcZero, 0, sizeof(RECT));
    _this->rcPrev = rcZero;
    //_this->bPartialRedraw = FALSE;
    //RedrawWindow(_this->hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
}

static BOOL CALLBACK _sws_WindowSwitcher_EnumWindowsCallback(_In_ HWND hWnd, _In_ sws_WindowSwitcher* _this)
{
    BOOL isCloaked;
    DwmGetWindowAttribute(hWnd, DWMWA_CLOAKED, &isCloaked, sizeof(BOOL));

    if (sws_WindowHelpers_IsAltTabWindow(hWnd) && !isCloaked)
    {
        if (hWnd == GetForegroundWindow())
        {
            SwitchToThisWindow(_this->hWndLast, TRUE);
            return FALSE;
        }
        _this->hWndLast = hWnd;
    }

    return TRUE;
}

static DWORD WINAPI _sws_WindowSwitcher_ToggleDesktop(sws_WindowSwitcher* _this)
{
    /*
    IShellDispatch4* pShellDisp = NULL;
    HRESULT hr = CoCreateInstance(&__CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, &__IID_IDispatch, (LPVOID*)&pShellDisp);
    if (SUCCEEDED(hr) && pShellDisp)
    {
        printf("da\n");
        pShellDisp->lpVtbl->ToggleDesktop(pShellDisp);
    }
    */
    // from CShowDesktopButton::_ToggleDesktop in explorer.exe:
    PostMessageW(FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL), 0x579, 3 - 1, 0); // 1 to restore
    PostMessageW(FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL), 0x579, 3 - 0, 0); // 0 to show
}

static LRESULT _sws_WindowsSwitcher_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    sws_WindowSwitcher* _this;
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

    if (uMsg == WM_CLOSE)
    {
        DestroyWindow(hWnd);
        return 0;
    }
    else if (uMsg == WM_DESTROY)
    {
        sws_WindowSwitcherLayout_Clear(&(_this->layout));
        PostQuitMessage(0);
        SetEvent(_this->rm.hEvEx);
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

        if (!_this->bPartialRedraw)
        {
            // Fill background
            FillRect(hDC, &rc, _this->hBackgroundBrush);

            // Draw title
            if (_this->hTheme)
            {
                for (unsigned int i = 0; i < _this->layout.pWindowList.cbSize; ++i)
                {
                    if (pWindowList && pWindowList[i].iRowMax)
                    {
                        unsigned int p = _this->layout.cbRowTitleHeight * (SWS_WINDOWSWITCHERLAYOUT_PERCENTAGEICON / 100.0);

                        rc = pWindowList[i].rcWindow;
                        DTTOPTS DttOpts;
                        DttOpts.dwSize = sizeof(DTTOPTS);
                        DttOpts.dwFlags = DTT_COMPOSITED | DTT_TEXTCOLOR;
                        DttOpts.crText = SWS_WINDOWSWITCHER_TEXT_COLOR;
                        DWORD dwTextFlags = DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS;
                        RECT rcText;
                        rcText.left = rc.left + _this->layout.cbLeftPadding + p + _this->layout.cbRightPadding;
                        rcText.top = rc.top + _this->layout.cbTopPadding;
                        rcText.right = rc.right - _this->layout.cbRightPadding;
                        rcText.bottom = rcText.top + _this->layout.cbRowTitleHeight - _this->layout.cbBottomPadding;

                        wchar_t wszTitle[MAX_PATH];
                        memset(wszTitle, 0, MAX_PATH * sizeof(wchar_t));
                        if (_this->layout.bIncludeWallpaper && i == 0)
                        {
                            wcscat_s(wszTitle, MAX_PATH, L"Desktop");
                        }
                        else
                        {
                            GetWindowText(pWindowList[i].hWnd, wszTitle, MAX_PATH);
                        }
                        DrawThemeTextEx(
                            _this->hTheme,
                            hDC,
                            SWS_WINDOWSWITCHER_THEME_INDEX,
                            0,
                            wszTitle,
                            -1,
                            dwTextFlags,
                            &rcText,
                            &DttOpts
                        );
                    }
                }
            }

            // Draw icon
            for (unsigned int i = 0; i < _this->layout.pWindowList.cbSize; ++i)
            {
                if (pWindowList && pWindowList[i].iRowMax)
                {
                    unsigned int p = _this->layout.cbRowTitleHeight * (SWS_WINDOWSWITCHERLAYOUT_PERCENTAGEICON / 100.0);
                    rc = pWindowList[i].rcWindow;
                    DrawIconEx(
                        hDC,
                        rc.left + _this->layout.cbLeftPadding + ((_this->layout.cbRowTitleHeight - p) / 4.0),
                        rc.top + _this->layout.cbTopPadding + ((_this->layout.cbRowTitleHeight - p) / 4.0),
                        pWindowList[i].hIcon,
                        p,
                        p,
                        0,
                        _this->hBackgroundBrush,
                        DI_NORMAL
                    );
                }
            }
        }

        // Draw highlight rectangle
        for (int j = (int)(_this->layout.iIndex) - 1; j < (int)_this->layout.iIndex + 2; ++j)
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
                    FrameRect(hDC, &rc, hBrush);
                    rc.left += 1;
                    rc.top += 1;
                    rc.right -= 1;
                    rc.bottom -= 1;
                }
            }
        }

        // Draw hover rectangle
        if (pWindowList)
        {
            for (unsigned int i = 0; i < _this->layout.pWindowList.cbSize; ++i)
            {
                RECT rc = pWindowList[i].rcThumbnail;
                rc.left -= 2;
                rc.top -= 2;
                rc.right += 2;
                rc.bottom += 2;
                FrameRect(hDC, &rc, _this->hBackgroundBrush);
                rc.left -= 1;
                rc.top -= 1;
                rc.right += 1;
                rc.bottom += 1;
                FrameRect(hDC, &rc, _this->hBackgroundBrush);
            }
            RECT rc = _this->rcPrev;
            if (rc.left != 0)
            {
                FrameRect(hDC, &rc, _this->hContourBrush);
                rc.left -= 1;
                rc.top -= 1;
                rc.right += 1;
                rc.bottom += 1;
                FrameRect(hDC, &rc, _this->hContourBrush);
            }
        }

        _this->bPartialRedraw = TRUE;
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
            BOOL bWasIn = FALSE;
            for (unsigned int i = 0; i < _this->layout.pWindowList.cbSize; ++i)
            {
                RECT rc = pWindowList[i].rcThumbnail;
                rc.left -= 2;
                rc.top -= 2;
                rc.right += 2;
                rc.bottom += 2;
                if (x > rc.left && x < rc.right && y > rc.top && y < rc.bottom)
                {
                    if (_this->rcPrev.left != rc.left && _this->rcPrev.right != rc.right)
                    {
                        _this->rcPrev = rc;
                        _this->bPartialRedraw = TRUE;
                        RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
                        return 0;
                    }
                    bWasIn = TRUE;
                }
            }
            if (!bWasIn)
            {
                RECT rcZero;
                memset(&rcZero, 0, sizeof(RECT));
                if (_this->rcPrev.left != rcZero.left && _this->rcPrev.right != rcZero.right)
                {
                    _this->rcPrev = rcZero;
                    RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
                }
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
                    if (_this->layout.bIncludeWallpaper && i == 0)
                    {
                        _sws_WindowSwitcher_ToggleDesktop(_this);
                    }
                    else
                    {
                        SwitchToThisWindow(pWindowList[i].hWnd, TRUE);
                    }
                }
            }
            ShowWindow(_this->hWnd, SW_HIDE);
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
        if (_this->layout.bIncludeWallpaper && _this->layout.iIndex == 0)
        {
            _sws_WindowSwitcher_ToggleDesktop(_this);
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
                WaitForSingleObject(
                    hThread,
                    INFINITE
                );
                sws_WindowSwitcherLayout_Clear(&(_this->layout));
                return 0;
            }
            else
            {
                _this->bPartialRedraw = TRUE;
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
                        (LOWORD(lParam) & MOD_SHIFT) ? SWS_WINDOWSWITCHERLAYOUT_COMPUTE_DIRECTION_BACKWARD : SWS_WINDOWSWITCHERLAYOUT_COMPUTE_DIRECTION_FORWARD
                    );
                    memset(&(_this->rcPrev), 0, sizeof(RECT));
                    _this->bPartialRedraw = FALSE;
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
    WNDCLASS wc = { 0 };
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = _sws_WindowsSwitcher_WndProc;
    wc.hbrBackground = _this->hBackgroundBrush;
    wc.hInstance = GetModuleHandleW(NULL);
    wc.lpszClassName = _T(SWS_WINDOWSWITCHER_CLASSNAME);
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    ATOM a = RegisterClassW(&wc);
    /*
    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = _sws_WindowsSwitcher_WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetModuleHandleW(NULL);
    wc.hIcon = LoadIconW(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(GetStockObject(BLACK_BRUSH));
    wc.lpszMenuName = NULL;
    wc.lpszClassName = _T(SWS_WINDOWSWITCHER_CLASSNAME);
    wc.hIconSm = LoadIconW(NULL, IDI_APPLICATION);
    ATOM a = RegisterClassExW(&wc);*/
    if (!a)
    {
        return sws_error_Report(sws_error_GetFromWin32Error(GetLastError()));
    }
    return SWS_ERROR_SUCCESS;
}

sws_error_t sws_WindowSwitcher_RunMessageQueue(sws_WindowSwitcher* _this)
{
    return sws_RegistryMonitor_Notify(&(_this->rm), QS_ALLINPUT);
}

void sws_WindowSwitcher_Clear(sws_WindowSwitcher* _this)
{
    if (_this)
    {
        sws_RegistryMonitor_Clear(&(_this->rm));
        sws_WindowSwitcherLayout_Clear(&(_this->layout));
#ifndef COMPILE_AS_LIBRARY
        _this->pTaskList->lpVtbl->Release(_this->pTaskList);
#endif
        UnregisterHotKey(_this->hWnd, 0);
        UnregisterHotKey(_this->hWnd, 1);
        UnregisterHotKey(_this->hWnd, 2);
        UnregisterHotKey(_this->hWnd, 3);
        DestroyWindow(_this->hWnd);
        DeleteObject(_this->hBackgroundBrush);
        DeleteObject(_this->hContourBrush);
        CloseThemeData(_this->hTheme);
        memset(_this, 0, sizeof(sws_WindowSwitcher));
    }
}

sws_error_t sws_WindowSwitcher_Initialize(sws_WindowSwitcher* _this)
{
    sws_error_t rv = SWS_ERROR_SUCCESS;

    if (!rv)
    {
        if (!_this)
        {
            rv = sws_error_Report(SWS_ERROR_NO_MEMORY);
        }
        memset(_this, 0, sizeof(sws_WindowSwitcher));
    }
    if (!rv)
    {
        rv = sws_WindowHelpers_Initialize();
    }
    if (!rv)
    {
        _this->hBackgroundBrush = (HBRUSH)CreateSolidBrush(SWS_WINDOWSWITCHER_BACKGROUND_COLOR);
        _this->hContourBrush = (HBRUSH)CreateSolidBrush(SWS_WINDOWSWITCHER_CONTOUR_COLOR);
        _this->hTheme = OpenThemeData(NULL, _T(SWS_WINDOWSWITCHER_THEME_CLASS));
    }
    if (!rv)
    {
        rv = _sws_WindowSwitcher_RegisterWindowClass(_this);
    }
    if (!rv)
    {
#ifndef COMPILE_AS_LIBRARY
        HRESULT hr = CoCreateInstance(
            &__uuidof_TaskbarList,
            NULL,
            CLSCTX_ALL,
            &__uuidof_ITaskbarList,
            (void**)(&(_this->pTaskList))
        );
        rv = sws_error_Report(sws_error_GetFromHRESULT(_this->pTaskList->lpVtbl->HrInit(_this->pTaskList)));
#endif
    }
    if (!rv)
    {
#ifdef COMPILE_AS_LIBRARY
        _this->hWnd = _sws_CreateWindowInBand(
#else
        _this->hWnd = CreateWindowEx(
#endif
            WS_EX_LAYERED | WS_EX_TOOLWINDOW,
            _T(SWS_WINDOWSWITCHER_CLASSNAME),
            L"",
            WS_POPUP,
            0, 0, 0, 0,
            NULL, NULL, GetModuleHandle(NULL), _this
#ifdef COMPILE_AS_LIBRARY
            , ZBID_UIACCESS
#endif
        );
        if (!_this->hWnd)
        {
            rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()));
        }
    }
    if (!rv)
    {
        if (!RegisterHotKey(_this->hWnd, 0, MOD_ALT, VK_TAB))
        {
            rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()));
        }
    }
    if (!rv)
    {
        if (!RegisterHotKey(_this->hWnd, 1, MOD_ALT | MOD_SHIFT, VK_TAB))
        {
            rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()));
        }
    }
    if (!rv)
    {
        if (!RegisterHotKey(_this->hWnd, 2, MOD_ALT | MOD_CONTROL, VK_TAB))
        {
            rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()));
        }
    }
    if (!rv)
    {
        if (!RegisterHotKey(_this->hWnd, 3, MOD_ALT | MOD_SHIFT | MOD_CONTROL, VK_TAB))
        {
            rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()));
        }
    }
    if (!rv)
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
            _sws_WindowSwitcher_NotifySettingsChange,
            _this
        );
        _sws_WindowSwitcher_NotifySettingsChange(_this, FALSE, &dwInitial, dwSize);
    }

    return rv;
}