#include "sws_WindowSwitcherLayoutWindow.h"

int sws_WindowSwitcherLayoutWindow_AddGroupedWnd(sws_WindowSwitcherLayoutWindow* _this, HWND hWnd)
{
    int rv = DPA_AppendPtr(_this->dpaGroupedWnds, hWnd);
    if (rv != -1) _this->dwCount++;
    return rv;
}

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
    if (_this->dpaGroupedWnds)
    {
        DPA_Destroy(_this->dpaGroupedWnds);
    }
    if (_this->wszAUMID)
    {
        CoTaskMemFree(_this->wszAUMID);
    }
    memset(_this, 0, sizeof(sws_WindowSwitcherLayoutWindow));
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
        _this->dwCount = 0;
    }
    if (!rv && wszPath)
    {
        wcscpy_s(_this->wszPath, MAX_PATH, wszPath);
    }
    if (!rv)
    {
        _this->wszAUMID = sws_WindowHelpers_GetAUMIDForHWND(_this->hWnd);
    }
    if (!rv)
    {
        _this->dpaGroupedWnds = DPA_Create(SWS_VECTOR_CAPACITY);
        if (!_this->dpaGroupedWnds)
        {
            rv = sws_error_Report(sws_error_GetFromInternalError(SWS_ERROR_NO_MEMORY), NULL);
        }
    }

    return rv;
}