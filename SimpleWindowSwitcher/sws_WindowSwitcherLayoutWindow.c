#include "sws_WindowSwitcherLayoutWindow.h"

void sws_WindowSwitcherLayoutWindow_Erase(sws_WindowSwitcherLayoutWindow* _this)
{
    if (_this->hThumbnail)
    {
        sws_error_Report(sws_error_GetFromHRESULT(DwmUnregisterThumbnail(_this->hThumbnail)), NULL);
        _this->hThumbnail = 0;
    }
    SIZE siz;
    siz.cx = 0;
    siz.cy = 0;
    _this->sizWindow = siz;
    RECT rc;
    rc.left = 0;
    rc.top = 0;
    rc.bottom = 0;
    rc.right = 0;
    _this->rcThumbnail = rc;
    _this->rcWindow = rc;
    _this->iRowMax = 0;
    ZeroMemory(_this->wszPath, MAX_PATH);
}

void sws_WindowSwitcherLayoutWindow_Clear(sws_WindowSwitcherLayoutWindow* _this)
{
    if (_this->hThumbnail)
    {
        DwmUnregisterThumbnail(_this->hThumbnail);
        _this->hThumbnail = 0;
    }
    //if (_this->hIcon && !_this->bOwnProcess)
    if (_this->hIcon && sws_DefAppIcon && _this->hIcon != sws_DefAppIcon && sws_LegacyDefAppIcon && _this->hIcon != sws_LegacyDefAppIcon)
    {
        DestroyIcon(_this->hIcon);
    }
    sws_WindowSwitcherLayoutWindow_Initialize(_this, 0, NULL);
}

sws_error_t sws_WindowSwitcherLayoutWindow_Initialize(sws_WindowSwitcherLayoutWindow* _this, HWND hWnd, WCHAR* wszPath)
{
    sws_error_t rv = SWS_ERROR_SUCCESS;

    if (!rv)
    {
        if (!_this)
        {
            rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_NO_MEMORY), NULL);
        }
    }
    if (!rv)
    {
        memset(_this, 0, sizeof(sws_WindowSwitcherLayoutWindow));
    }
    if (!rv)
    {
        _this->hWnd = hWnd;
    }
    if (!rv && wszPath)
    {
        wcscpy_s(_this->wszPath, MAX_PATH, wszPath);
    }

    return rv;
}