#ifndef _H_SWS_ICONPAINTER_
#define _H_SWS_ICONPAINTER_
#include <Windows.h>

typedef struct _sws_IconPainter_CallbackParams
{
	long long timestamp;
	HWND hWnd;
	int index;
	BOOL bIsDesktop;
} sws_IconPainter_CallbackParams;

static void _sws_IconPainter_Callback(
	HWND hWnd,
	UINT uMsg,
	sws_IconPainter_CallbackParams* params,
	HICON hIcon
);

void sws_IconPainter_DrawIcon(HICON hIcon, HDC hDC, HBRUSH hBrush, void* pGdipGraphics, INT x, INT y, INT w, INT h, RGBQUAD bkcol);

BOOL sws_IconPainter_ExtractAndDrawIconAsync(HWND hWnd, sws_IconPainter_CallbackParams* params);
#endif