#include "sws_WindowSwitcherLayoutWindow.h"

void sws_WindowSwitcherLayoutWindow_Erase(sws_WindowSwitcherLayoutWindow* _this)
{
    if (_this->hThumbnail)
    {
        sws_error_Report(sws_error_GetFromHRESULT(DwmUnregisterThumbnail(_this->hThumbnail)));
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
}

void sws_WindowSwitcherLayoutWindow_Clear(sws_WindowSwitcherLayoutWindow* _this)
{
    if (_this->hThumbnail)
    {
        sws_error_Report(sws_error_GetFromHRESULT(DwmUnregisterThumbnail(_this->hThumbnail)));
        _this->hThumbnail = 0;
    }
    if (_this->hIcon && !_this->bOwnProcess)
    {
        DestroyIcon(_this->hIcon);
    }
    sws_WindowSwitcherLayoutWindow_Initialize(_this, 0);
}

sws_error_t sws_WindowSwitcherLayoutWindow_Initialize(sws_WindowSwitcherLayoutWindow* _this, HWND hWnd)
{
    sws_error_t rv = SWS_ERROR_SUCCESS;

    if (!rv)
    {
        if (!_this)
        {
            rv = sws_error_Report(SWS_ERROR_NO_MEMORY);
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

    return rv;
}