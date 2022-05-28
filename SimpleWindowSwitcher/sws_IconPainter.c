#include "sws_IconPainter.h"
#include "sws_WindowSwitcher.h"

void sws_IconPainter_DrawIcon(HICON hIcon, HDC hDC, HBRUSH hBrush, void* pGdipGraphics, INT x, INT y, INT w, INT h, RGBQUAD bkcol, BOOL bShouldFillBackground)
{
    if (hIcon == NULL || hDC == NULL || w == 0 || h == 0)
    {
        return;
    }

    BITMAPINFO bi;
    ZeroMemory(&bi, sizeof(BITMAPINFO));
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = 1;
    bi.bmiHeader.biHeight = 1;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    if (bShouldFillBackground)
    {
        StretchDIBits(hDC, x, y, w, h, 0, 0, 1, 1, &bkcol, &bi, DIB_RGB_COLORS, SRCCOPY);
    }

    // Not using GdipCreateBitmapFromHICON directly because some bug in GDI+
    // renders weird black lines in some transparent areas; this is the only
    // way I could properly get this to work
    // from: https://stackoverflow.com/questions/11338009/how-do-i-copy-an-hicon-from-gdi-to-gdi-with-transparency
    if (pGdipGraphics)
    {
        ICONINFO ii;
        if (GetIconInfo(hIcon, &ii))
        {
            void* pGdipBitmap = NULL;
            GdipCreateBitmapFromHBITMAP(
                (HBITMAP)ii.hbmColor,
                (HPALETTE)NULL,
                (void*)&pGdipBitmap
            );
            if (pGdipBitmap)
            {
                INT rct[4] = { 0, 0, 0, 0 };
                GdipGetImageWidth((void*)pGdipBitmap, (UINT*)(rct + 2));
                GdipGetImageHeight((void*)pGdipBitmap, (UINT*)(rct + 3));
                INT PixelFormat;
                GdipGetImagePixelFormat((void*)pGdipBitmap, &PixelFormat);
                INT LockedBitmapData[100]; // make sure this is large enough
                GdipBitmapLockBits(
                    (void*)pGdipBitmap,
                    (INT*)rct,
                    (INT)0, // Gdiplus::ImageLockModeRead
                    (INT)PixelFormat,
                    (void*)LockedBitmapData
                );
                if (*(BYTE**)(LockedBitmapData + 4)) // Scan0
                {
                    void* pGdipBitmap2 = NULL;
                    GdipCreateBitmapFromScan0(
                        (INT)LockedBitmapData[0], // Width
                        (INT)LockedBitmapData[1], // Height
                        (INT)LockedBitmapData[2], // Stride
                        (INT)((10 | (32 << 8) | 0x00040000 | 0x00020000 | 0x00200000)), // PixelFormat = PixelFormat32bppARGB
                        *(BYTE**)(LockedBitmapData + 4), // Scan0
                        (void*)&pGdipBitmap2
                    );
                    if (pGdipBitmap2)
                    {
                        GdipDrawImageRectI(
                            (void*)pGdipGraphics,
                            (void*)pGdipBitmap2,
                            (INT)x,
                            (INT)y,
                            (INT)w,
                            (INT)h
                        );
                        GdipDisposeImage((void*)pGdipBitmap2);
                        // We proceed to check if this worked by verifying if the
                        // written bitmap is all transparent; if it is, then it
                        // means nothing was drawn, so we fallback to creating the
                        // bitmap using GdipCreateBitmapFromHICON and drawing that 
                        HDC hDC_Test = NULL;
                        HBITMAP hBM_Test = NULL;
                        HBITMAP hBM_Test_Old = NULL;
                        if (hDC_Test = CreateCompatibleDC(hDC))
                        {
                            if (hBM_Test = CreateCompatibleBitmap(hDC, w, h))
                            {
                                if (hBM_Test_Old = SelectObject(hDC_Test, hBM_Test))
                                {
                                    if (BitBlt(hDC_Test, 0, 0, w, h, hDC, x, y, SRCCOPY))
                                    {
                                        BITMAPINFO info;
                                        info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                                        info.bmiHeader.biWidth = w;
                                        info.bmiHeader.biHeight = -h;
                                        info.bmiHeader.biPlanes = 1;
                                        info.bmiHeader.biBitCount = 32;
                                        info.bmiHeader.biCompression = BI_RGB;
                                        info.bmiHeader.biSizeImage = w * h * 4;
                                        info.bmiHeader.biXPelsPerMeter = 0;
                                        info.bmiHeader.biYPelsPerMeter = 0;
                                        info.bmiHeader.biClrUsed = 0;
                                        info.bmiHeader.biClrImportant = 0;
                                        void* bits = malloc(info.bmiHeader.biSizeImage);
                                        if (bits)
                                        {
                                            if (GetDIBits(hDC_Test, hBM_Test, 0, h, bits, &info, DIB_RGB_COLORS))
                                            {
                                                BYTE* ptr;
                                                int ii;
                                                BOOL bUsed = FALSE;
                                                for (ii = 0, ptr = bits; ii < w * h; ii++, ptr += 4)
                                                {
                                                    if (ptr[3] != bkcol.rgbReserved)
                                                    {
                                                        bUsed = TRUE;
                                                        break;
                                                    }
                                                }
                                                if (!bUsed)
                                                {
                                                    if (bShouldFillBackground)
                                                    {
                                                        StretchDIBits(hDC, x, y, w, h, 0, 0, 1, 1, &bkcol, &bi, DIB_RGB_COLORS, SRCCOPY);
                                                    }
                                                    void* pGdipBitmap3 = NULL;
                                                    GdipCreateBitmapFromHICON(
                                                        (HICON)hIcon,
                                                        (void**)&pGdipBitmap3
                                                    );
                                                    if (pGdipBitmap3)
                                                    {
                                                        GdipDrawImageRectI(
                                                            (void*)pGdipGraphics,
                                                            (void*)pGdipBitmap3,
                                                            (INT)x,
                                                            (INT)y,
                                                            (INT)w,
                                                            (INT)h
                                                        );
                                                        GdipDisposeImage((void*)pGdipBitmap3);
                                                    }
                                                }
                                            }
                                            free(bits);
                                        }
                                    }
                                    SelectObject(hDC_Test, hBM_Test_Old);
                                }
                                DeleteObject(hBM_Test);
                            }
                            DeleteDC(hDC_Test);
                        }
                    }
                    GdipBitmapUnlockBits(
                        (void*)pGdipBitmap,
                        (void*)&LockedBitmapData
                    );
                }
                GdipDisposeImage((void*)pGdipBitmap);
            }
            DeleteObject(ii.hbmColor);
            DeleteObject(ii.hbmMask);
        }
    }
    else
    {
        // Fallback to crappier drawing using GDI if GDI+ is unavailable
        if (bShouldFillBackground)
        {
            DrawIconEx(hDC, x, y, hIcon, w, h, 0, NULL, hBrush, DI_NORMAL);
        }
        else
        {
            DrawIcon(hDC, x, y, hIcon);
        }
    }
}

static void _sws_IconPainter_Callback(
	HWND hWnd,
	UINT uMsg,
	sws_IconPainter_CallbackParams* params,
	HICON hIcon
)
{
    LONG_PTR ptr = GetWindowLongPtr(params->hWnd, GWLP_USERDATA);
    sws_WindowSwitcher* _this = (struct sws_WindowSwitcher*)(ptr);

	if (_this->layout.timestamp == params->timestamp)
	{
        DWORD dwProcessId;
        GetWindowThreadProcessId(hWnd, &dwProcessId);
        sws_WindowSwitcherLayoutWindow* pWindowList = _this->layout.pWindowList.pList;

        if (!hIcon || (params->bUseApplicationIcon && !pWindowList[params->index].dwIconSource))
        {
            if (params->bUseApplicationIcon && !pWindowList[params->index].dwIconSource)
            {
                if (hIcon && dwProcessId != GetCurrentProcessId())
                {
                    DestroyIcon(hIcon);
                }
                pWindowList[params->index].dwIconSource = 1;
            }
            switch (pWindowList[params->index].dwIconSource)
            {
            case 0:
            {
                pWindowList[params->index].dwIconSource++;
                if (SendMessageCallbackW(hWnd, WM_GETICON, ICON_SMALL2, 0, _sws_IconPainter_Callback, params))
                {
                    return;
                }
                else
                {
                    pWindowList[params->index].hIcon = sws_LegacyDefAppIcon;
                }
                break;
            }
            case 1:
            {
                pWindowList[params->index].dwIconSource++;
                wchar_t wszPath[MAX_PATH];
                ZeroMemory(wszPath, MAX_PATH * sizeof(wchar_t));
                if (!params->bIsDesktop)
                {
                    if (!sws_WindowHelpers_IsWindowUWP(hWnd))
                    {
                        HANDLE hProcess;
                        hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwProcessId);
                        if (hProcess)
                        {
                            GetModuleFileNameExW((HMODULE)hProcess, NULL, wszPath, MAX_PATH);
                            CharLowerW(wszPath);
                            SHFILEINFOW shinfo;
                            ZeroMemory(&shinfo, sizeof(SHFILEINFOW));
                            SHGetFileInfoW(
                                wszPath,
                                FILE_ATTRIBUTE_NORMAL,
                                &shinfo,
                                sizeof(SHFILEINFOW),
                                SHGFI_ICON
                            );
                            if (shinfo.hIcon)
                            {
                                _sws_IconPainter_Callback(hWnd, uMsg, params, shinfo.hIcon);
                                return;
                            }
                        }
                    }
                    else
                    {
                        HRESULT hr = S_OK;
                        IShellItemImageFactory* imageFactory = NULL;
                        SIIGBF flags = SIIGBF_RESIZETOFIT | SIIGBF_ICONBACKGROUND;

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
                                if (imageFactory)
                                {
                                    double factor = SWS_UWP_ICON_SCALE_FACTOR;
                                    int szIcon = pWindowList[params->index].szIcon / factor;

                                    SIZE size;
                                    size.cx = szIcon;
                                    size.cy = szIcon;
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
                                            HICON hExIcon = ImageList_GetIcon(hImageList, 0, 0);
                                            ImageList_Destroy(hImageList);

                                            DeleteObject(hBitmap);
                                            imageFactory->lpVtbl->Release(imageFactory);

                                            pWindowList[params->index].dwWindowFlags |= SWS_WINDOWSWITCHERLAYOUT_WINDOWFLAGS_ISUWP;
                                            szIcon = pWindowList[params->index].rcIcon.right;
                                            szIcon = szIcon * factor;
                                            szIcon = pWindowList[params->index].rcIcon.right - szIcon;
                                            pWindowList[params->index].rcIcon.left = szIcon / 2;
                                            pWindowList[params->index].rcIcon.top = szIcon / 2;
                                            pWindowList[params->index].rcIcon.right = pWindowList[params->index].rcIcon.right + szIcon;
                                            pWindowList[params->index].rcIcon.bottom = pWindowList[params->index].rcIcon.bottom + szIcon;

                                            _sws_IconPainter_Callback(hWnd, uMsg, params, hExIcon);
                                            return;
                                        }
                                        DeleteObject(hBitmap);
                                    }
                                    imageFactory->lpVtbl->Release(imageFactory);
                                }
                            }
                        }
                    }
                }
                else
                {
                    if (GetSystemDirectoryW(wszPath, MAX_PATH))
                    {
                        wcscat_s(wszPath, MAX_PATH, L"\\imageres.dll");
                        HICON hExIcon = ExtractIconW(
                            GetModuleHandleW(NULL),
                            wszPath,
                            -110
                        );
                        if (hExIcon)
                        {
                            _sws_IconPainter_Callback(hWnd, uMsg, params, hExIcon);
                            return;
                        }
                    }
                }
                pWindowList[params->index].hIcon = sws_LegacyDefAppIcon;
            }
            }
        }
        else
        {
            if (pWindowList[params->index].hIcon && sws_DefAppIcon && pWindowList[params->index].hIcon != sws_DefAppIcon && sws_LegacyDefAppIcon && pWindowList[params->index].hIcon != sws_LegacyDefAppIcon)
            {
                DestroyIcon(pWindowList[params->index].hIcon);
                pWindowList[params->index].hIcon = NULL;
            }
            if (dwProcessId == GetCurrentProcessId() && pWindowList[params->index].dwIconSource <= 1)
            {
                pWindowList[params->index].hIcon = CopyIcon(hIcon);
            }
            else
            {
                pWindowList[params->index].hIcon = hIcon;
            }
        }

        BOOL bShouldPaint = TRUE;
        for (unsigned int i = 0; i < _this->layout.pWindowList.cbSize; ++i)
        {
            if (pWindowList[i].hIcon == sws_DefAppIcon && !IsHungAppWindow(pWindowList[i].hWnd))
            {
                bShouldPaint = FALSE;
                break;
            }
        }
        if (bShouldPaint)
        {
            //printf("[sws] Asynchronously obtained application icons in %lld ms.\n", sws_milliseconds_now() - _this->layout.timestamp);
            KillTimer(_this->hWnd, SWS_WINDOWSWITCHER_TIMER_PAINT);
            SendMessageW(_this->hWnd, SWS_WINDOWSWITCHER_PAINT_MSG, SWS_WINDOWSWITCHER_PAINTFLAGS_REDRAWENTIRE, 0);
        }
	}

    free(params);
}

BOOL sws_IconPainter_ExtractAndDrawIconAsync(HWND hWnd, sws_IconPainter_CallbackParams* params)
{
    if (IsHungAppWindow(hWnd))
    {
        return FALSE;
    }
    SetTimer(params->hWnd, SWS_WINDOWSWITCHER_TIMER_PAINT, SWS_WINDOWSWITCHER_TIMER_PAINT_GETICONASYNC_DELAY, NULL);
	return SendMessageCallbackW(hWnd, WM_GETICON, ICON_BIG, 0, _sws_IconPainter_Callback, params);
}