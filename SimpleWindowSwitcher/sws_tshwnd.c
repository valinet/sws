#include "sws_tshwnd.h"

void sws_tshwnd_ModifyTimestamp(sws_tshwnd* _this, FILETIME ft)
{
	_this->ft = ft;
}

void sws_tshwnd_UpdateTimestamp(sws_tshwnd* _this)
{
	GetSystemTimeAsFileTime(&(_this->ft));
}

int sws_tshwnd_CompareTimestamp(sws_tshwnd* p1, sws_tshwnd* p2, LPARAM flags)
{
	if (flags & SWS_SORT_DESCENDING)
	{
		return CompareFileTime(&(p2->ft), &(p1->ft));
	}
	return CompareFileTime(&(p1->ft), &(p2->ft));
}

int sws_tshwnd_CompareHWND(sws_tshwnd* p1, sws_tshwnd* p2)
{
	return !(p1 && p2 && p1->hWnd == p2->hWnd);
}

BOOL sws_tshwnd_GetFlashState(sws_tshwnd* _this)
{
	return _this->bFlash;
}

void sws_tshwnd_SetFlashState(sws_tshwnd* _this, BOOL bFlash)
{
	_this->bFlash = bFlash;
}

sws_error_t sws_tshwnd_Initialize(sws_tshwnd* _this, HWND hWnd)
{
	sws_error_t rv = SWS_ERROR_SUCCESS;

	if (!rv && _this)
	{
		_this->hWnd = hWnd;
		GetSystemTimeAsFileTime(&(_this->ft));
		_this->bFlash = FALSE;
		_this->cbFlashAnimationState = 0;
		_this->dwFlashAnimationState = 0;
	}

	return rv;
}
