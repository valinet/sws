#include "sws_WindowSwitcherLayout.h"

static BOOL CALLBACK _sws_WindowSwitcherLayout_EnumWindowsCallback(_In_ HWND hWnd, _In_ sws_WindowSwitcherLayout* _this)
{
	BOOL isCloaked;
	DwmGetWindowAttribute(hWnd, DWMWA_CLOAKED, &isCloaked, sizeof(BOOL));

	if (sws_WindowHelpers_IsAltTabWindow(hWnd) && !isCloaked)
	{
		sws_WindowSwitcherLayoutWindow swsLayoutWindow;
		sws_WindowSwitcherLayoutWindow_Initialize(&swsLayoutWindow, hWnd);
		sws_vector_PushBack(&_this->pWindowList, &swsLayoutWindow);
		DWORD band;
		_sws_GetWindowBand(hWnd, &band);
		if (band != ZBID_DESKTOP) _this->numTopMost++;
	}

	return TRUE;
}

sws_error_t sws_WindowSwitcherLayout_InvalidateLayout(sws_WindowSwitcherLayout* _this)
{
	sws_error_t rv = SWS_ERROR_SUCCESS;

	sws_WindowSwitcherLayoutWindow* pWindowList = _this->pWindowList.pList;
	for (int iCurrentWindow = _this->pWindowList.cbSize - 1; iCurrentWindow >= 0; iCurrentWindow--)
	{
		sws_WindowSwitcherLayoutWindow_Erase(&(pWindowList[iCurrentWindow]));
	}
}

sws_error_t sws_WindowSwitcherLayout_ComputeLayout(sws_WindowSwitcherLayout* _this, int direction)
{
	sws_error_t rv = SWS_ERROR_SUCCESS;

	if (!rv)
	{
		unsigned int cbInitialLeft = _this->cbPadding + _this->cbLeftPadding, cbInitialTop = _this->cbPadding + _this->cbTopPadding + _this->cbRowTitleHeight + _this->cbBottomPadding;
		unsigned int cbCurrentLeft = cbInitialLeft, cbCurrentTop = cbInitialTop;
		unsigned int cbMaxWidthHit = 0, cbMaxWidth = 0;

		int iObtainedIndex = 0;

		sws_WindowSwitcherLayoutWindow* pWindowList = _this->pWindowList.pList;

		if (_this->iWidth)
		{
			cbMaxWidth = _this->iWidth - _this->cbPadding;
		}
		else
		{
			cbMaxWidth = _this->cbMaxWidth;
		}

		BOOL bHasTarget = FALSE;
		int iTarget = -1;

		if (direction != SWS_WINDOWSWITCHERLAYOUT_COMPUTE_DIRECTION_INITIAL)
		{
			if (direction == SWS_WINDOWSWITCHERLAYOUT_COMPUTE_DIRECTION_BACKWARD)
			{
				bHasTarget = TRUE;
				iObtainedIndex = _this->pWindowList.cbSize - 1;
			}
			else if (direction == SWS_WINDOWSWITCHERLAYOUT_COMPUTE_DIRECTION_FORWARD)
			{
				if (_this->iIndex == _this->pWindowList.cbSize - 1)
				{
					iObtainedIndex = _this->iIndex;
				}
				else
				{
					int i = 0, state = 0, h = 0;
					for (i = _this->pWindowList.cbSize - 1; i >= 0; i--)
					{
						if (state == 0)
						{
							if (pWindowList[i].hThumbnail)
							{
								h = pWindowList[i].rcThumbnail.top;
								state = 1;
							}
						}
						else if (state == 1)
						{
							if (h != pWindowList[i].rcThumbnail.top)
							{
								break;
							}
						}
					}
					iObtainedIndex = i;
				}
			}
			sws_WindowSwitcherLayout_InvalidateLayout(_this);
		}

		BOOL bIsWidthComputed = (_this->iWidth != 0);
		BOOL bFinishedLayout = FALSE;

		while (1)
		{
			int iCurrentCount = 0;

			cbInitialLeft = _this->cbPadding + _this->cbLeftPadding;
			cbInitialTop = _this->cbPadding + _this->cbTopPadding + _this->cbRowTitleHeight;
			cbCurrentLeft = cbInitialLeft;
			cbCurrentTop = cbInitialTop;
			cbMaxWidthHit = 0, cbMaxWidth = 0;

			for (int iCurrentWindow = iObtainedIndex ? iObtainedIndex : _this->iIndex; iCurrentWindow >= 0; iCurrentWindow--)
			{
				//TCHAR name[200];
				//GetWindowTextW(pWindowList[iCurrentWindow].hWnd, name, 200);
				//wprintf(L"%d %s ", pWindowList[iCurrentWindow].hWnd, name);

				if (pWindowList[iCurrentWindow].hWnd == _this->hWnd)
				{
					continue;
				}

				if (_this->iWidth)
				{
					cbMaxWidth = _this->iWidth + _this->cbPadding + _this->cbRightPadding;
				}
				else
				{
					cbMaxWidth = _this->cbMaxWidth;
				}

				if (!bFinishedLayout)
				{
					pWindowList[iCurrentWindow].iRowMax = -1;
				}

		        DwmRegisterThumbnail(
					_this->hWnd,
					pWindowList[iCurrentWindow].hWnd,
					&(pWindowList[iCurrentWindow].hThumbnail)
				);
				DwmQueryThumbnailSourceSize(pWindowList[iCurrentWindow].hThumbnail, &(pWindowList[iCurrentWindow].sizWindow));
				if ((pWindowList[iCurrentWindow].sizWindow.cy == 0) ||
					(pWindowList[iCurrentWindow].sizWindow.cx == 0))
				{
					DwmUnregisterThumbnail(pWindowList[iCurrentWindow].hThumbnail);
					pWindowList[iCurrentWindow].hThumbnail = 0;
					continue;
				}
				if (bFinishedLayout)
				{
					DwmUnregisterThumbnail(pWindowList[iCurrentWindow].hThumbnail);
					pWindowList[iCurrentWindow].hThumbnail = 0;
				}

				double width;
				
				if (_this->bIncludeWallpaper && iCurrentWindow == 0)
				{
					width = ((_this->mi.rcMonitor.right - _this->mi.rcMonitor.left) *
						_this->cbThumbnailAvailableHeight) /
						(_this->mi.rcMonitor.bottom - _this->mi.rcMonitor.top);
				}
				else
				{
					width = ((pWindowList[iCurrentWindow].sizWindow.cx) *
						_this->cbThumbnailAvailableHeight) /
						(pWindowList[iCurrentWindow].sizWindow.cy);
				}

				if (bFinishedLayout)
				{
					pWindowList[iCurrentWindow].sizWindow.cx = 0;
					pWindowList[iCurrentWindow].sizWindow.cy = 0;
				}

				//wchar_t name[200];
				//GetWindowTextW(pWindowList[iCurrentWindow].hWnd, name, 200);
				//wprintf(L"%s %d %f %d\n", name, cbCurrentLeft, width, cbMaxWidth);


				if (cbCurrentLeft + width + _this->cbRightPadding + _this->cbPadding > cbMaxWidth)
				{
					if (!bFinishedLayout)
					{
						int t = iCurrentWindow + 1;
						if (t == _this->pWindowList.cbSize) t = 0;
						for (int k = t; k < _this->pWindowList.cbSize; ++k)
						{
							if (pWindowList[k].iRowMax == -1)
							{
								pWindowList[k].iRowMax = cbCurrentLeft - _this->cbLeftPadding;
							}
							else
							{
								break;
							}
							//if (k == _this->pWindowList.cbSize - 1)
							//{
							//	k = -1;
							//}
						}
					}
					if (cbCurrentLeft - _this->cbLeftPadding > cbMaxWidthHit)
					{
						cbMaxWidthHit = cbCurrentLeft - _this->cbLeftPadding;
					}
					if (!bFinishedLayout)
					{
						if (cbCurrentTop + _this->cbThumbnailAvailableHeight + _this->cbBottomPadding > _this->cbMaxHeight)
						{
							HWND hWnd = pWindowList[iCurrentWindow].hWnd;
							sws_WindowSwitcherLayoutWindow_Erase(&(pWindowList[iCurrentWindow]));
							iTarget = iCurrentWindow + 1;

							if (bIsWidthComputed)
							{
								break;
							}
							else
							{
								bFinishedLayout = TRUE;
							}
						}
					}
					cbCurrentLeft = cbInitialLeft;
					if (!bFinishedLayout)
					{
						cbCurrentTop += _this->cbThumbnailAvailableHeight + _this->cbBottomPadding + cbInitialTop;
					}
				}

				//printf("%d %d\n", cbCurrentLeft, cbCurrentTop);

				if (!bFinishedLayout)
				{
					pWindowList[iCurrentWindow].rcThumbnail.left = cbCurrentLeft;
					pWindowList[iCurrentWindow].rcThumbnail.top = cbCurrentTop;
					pWindowList[iCurrentWindow].rcThumbnail.right = cbCurrentLeft + width;
					pWindowList[iCurrentWindow].rcThumbnail.bottom = cbCurrentTop + _this->cbThumbnailAvailableHeight;

					pWindowList[iCurrentWindow].rcWindow.left = cbCurrentLeft - _this->cbLeftPadding;
					pWindowList[iCurrentWindow].rcWindow.top = cbCurrentTop - _this->cbTopPadding - _this->cbRowTitleHeight;
					pWindowList[iCurrentWindow].rcWindow.right = cbCurrentLeft + width + _this->cbRightPadding;
					pWindowList[iCurrentWindow].rcWindow.bottom = cbCurrentTop + _this->cbThumbnailAvailableHeight + _this->cbBottomPadding;
				}

				cbCurrentLeft += width + _this->cbRightPadding + _this->cbLeftPadding;

				/*if (iCurrentWindow == 0)
				{
					iCurrentWindow = _this->pWindowList.cbSize;
				}*/

				iTarget = iCurrentWindow;
				iCurrentCount++;
				if (iCurrentCount == _this->pWindowList.cbSize)
				{
					break;
				}
			}

			if (!bHasTarget)
			{
				break;
			}
			else
			{
				int j, h = pWindowList[iObtainedIndex].rcThumbnail.top;
				if (!iTarget)
				{
					break;
				}
				for (j = iObtainedIndex; j >= 0; j--)
				{
					if (pWindowList[j].rcThumbnail.top != h)
					{
						j++;
						break;
					}
				}
				if (j == _this->iIndex)
				{
					break;
				}
				else
				{
					sws_WindowSwitcherLayout_InvalidateLayout(_this);
					iObtainedIndex = j - 1;
				}
			}
		}


		if (cbCurrentLeft - _this->cbLeftPadding > cbMaxWidthHit)
		{
			cbMaxWidthHit = cbCurrentLeft - _this->cbLeftPadding;
		}

		for (int k = 0; k < _this->pWindowList.cbSize; ++k)
		{
			if (pWindowList[k].iRowMax == -1)
			{
				pWindowList[k].iRowMax = cbCurrentLeft - _this->cbLeftPadding;
			}
			else
			{
				break;
			}
			//if (k == _this->pWindowList.cbSize - 1)
			//{
			//	k = -1;
			//}
		}

		if (_this->iWidth)
		{
			cbMaxWidthHit = _this->iWidth - _this->cbPadding;
		}

		if (!_this->iWidth)
		{
			_this->iHeight = cbCurrentTop + _this->cbThumbnailAvailableHeight + _this->cbBottomPadding + _this->cbPadding;
			_this->iWidth = cbMaxWidthHit + _this->cbPadding;
			_this->iX = ((_this->mi.rcWork.right - _this->mi.rcWork.left) - _this->iWidth) / 2 + _this->mi.rcWork.left;
			_this->iY = ((_this->mi.rcWork.bottom - _this->mi.rcWork.top) - _this->iHeight) / 2 + _this->mi.rcWork.top;
		}

		for (int iCurrentWindow = _this->pWindowList.cbSize - 1; iCurrentWindow >= 0; iCurrentWindow--)
		{
			if (pWindowList[iCurrentWindow].iRowMax)
			{
				if (pWindowList[iCurrentWindow].hThumbnail)
				{
					unsigned int diff = cbMaxWidthHit < pWindowList[iCurrentWindow].iRowMax ? 0 : cbMaxWidthHit - pWindowList[iCurrentWindow].iRowMax;
					pWindowList[iCurrentWindow].rcThumbnail.left += diff / 2;
					pWindowList[iCurrentWindow].rcThumbnail.right += diff / 2;
					////wchar_t name[200];
					////GetWindowTextW(pWindowList[iCurrentWindow].hWnd, name, 200);
					////wprintf(L"%s %d %d\n", name, pWindowList[iCurrentWindow].rcThumbnail.left, pWindowList[iCurrentWindow].rcThumbnail.right);

					pWindowList[iCurrentWindow].rcWindow.left += diff / 2;
					pWindowList[iCurrentWindow].rcWindow.right += diff / 2;

					DWM_THUMBNAIL_PROPERTIES dskThumbProps;
					dskThumbProps.dwFlags = DWM_TNP_SOURCECLIENTAREAONLY | DWM_TNP_VISIBLE | DWM_TNP_OPACITY | DWM_TNP_RECTDESTINATION;
					dskThumbProps.fSourceClientAreaOnly = FALSE;
					dskThumbProps.fVisible = TRUE;
					dskThumbProps.opacity = 255;
					dskThumbProps.rcDestination = pWindowList[iCurrentWindow].rcThumbnail;
					if (_this->bIncludeWallpaper && iCurrentWindow == 0)
					{
						dskThumbProps.dwFlags |= DWM_TNP_RECTSOURCE;
						dskThumbProps.rcSource.left = _this->mi.rcMonitor.left;
						dskThumbProps.rcSource.right = _this->mi.rcMonitor.right;
						dskThumbProps.rcSource.top = _this->mi.rcMonitor.top;
						dskThumbProps.rcSource.bottom = _this->mi.rcMonitor.bottom;
					}
					HRESULT hr = DwmUpdateThumbnailProperties(pWindowList[iCurrentWindow].hThumbnail, &dskThumbProps);
					if (FAILED(hr))
					{
						// error
					}
				}
			}
		}
	}

	if (!rv)
	{
		/*printf("\n");
		sws_WindowSwitcherLayoutWindow* pWindowList = _this->pWindowList.pList;
		for (UINT i = 0; i < _this->pWindowList.cbSize; ++i)
		{
			TCHAR name[200];
			GetWindowText(pWindowList[i].hWnd, name, 200);
			wprintf(L"%d %s\n", pWindowList[i].hWnd, name);
		}*/
	}

	return rv;
}

void sws_WindowSwitcherLayout_Clear(sws_WindowSwitcherLayout* _this)
{
	if (_this)
	{
		sws_WindowSwitcherLayoutWindow* pWindowList = _this->pWindowList.pList;
		if (pWindowList)
		{
			for (int iCurrentWindow = 0; iCurrentWindow < _this->pWindowList.cbSize; ++iCurrentWindow)
			{
				sws_WindowSwitcherLayoutWindow_Clear(&(pWindowList[iCurrentWindow]));
			}
			sws_vector_Clear(&(_this->pWindowList));
		}
		memset(_this, 0, sizeof(sws_WindowSwitcherLayout));
	}
}

sws_error_t sws_WindowSwitcherLayout_Initialize(sws_WindowSwitcherLayout* _this, HMONITOR hMonitor, HWND hWnd)
{
	sws_error_t rv = SWS_ERROR_SUCCESS;

	if (!rv)
	{
		if (!_this)
		{
			rv = sws_error_Report(SWS_ERROR_NO_MEMORY);
		}
		memset(_this, 0, sizeof(sws_WindowSwitcherLayout));
	}
	if (!rv)
	{
		rv = sws_WindowHelpers_Initialize();
	}
	if (!rv)
	{
		rv = sws_vector_Initialize(&(_this->pWindowList), sizeof(sws_WindowSwitcherLayoutWindow));
	}
	_this->mi.cbSize = sizeof(MONITORINFO);
	if (!rv)
	{
		if (!GetMonitorInfoW(
			hMonitor,
			&(_this->mi)
		))
		{
			rv = sws_error_Report(sws_error_GetFromWin32Error(GetLastError()));
		}
	}
	if (!rv)
	{
		_this->bIncludeWallpaper = SWS_WINDOWSWITCHERLAYOUT_INCLUDE_WALLPAPER;
		if (_this->bIncludeWallpaper)
		{
			sws_WindowSwitcherLayoutWindow swsLayoutWindow;
			sws_WindowSwitcherLayoutWindow_Initialize(&swsLayoutWindow, sws_WindowHelpers_GetWallpaperHWND(hMonitor));
			sws_vector_PushBack(&_this->pWindowList, &swsLayoutWindow);
		}
	}
	if (!rv)
	{
		rv = sws_WindowHelpers_RealEnumWindows((WNDENUMPROC)_sws_WindowSwitcherLayout_EnumWindowsCallback, (LPARAM)_this);
	}

	_this->cbMaxHeight = 0;
	_this->cbMaxWidth = 0;
	_this->cbRowWidth = 0;
	_this->cbRowHeight = SWS_WINDOWSWITCHERLAYOUT_ROWHEIGHT;
	_this->cbRowTitleHeight = SWS_WINDOWSWITCHERLAYOUT_ROWTITLEHEIGHT;
	_this->cbPadding = SWS_WINDOWSWITCHERLAYOUT_PADDING;
	_this->cbTopPadding = SWS_WINDOWSWITCHERLAYOUT_PADDING_TOP;
	_this->cbBottomPadding = SWS_WINDOWSWITCHERLAYOUT_PADDING_BOTTOM;
	_this->cbLeftPadding = SWS_WINDOWSWITCHERLAYOUT_PADDING_LEFT;
	_this->cbRightPadding = SWS_WINDOWSWITCHERLAYOUT_PADDING_RIGHT;
	_this->cbThumbnailAvailableHeight = 0;
	_this->hWnd = hWnd;
	_this->hMonitor = hMonitor;
	_this->iIndex = _this->pWindowList.cbSize - 1;

	if (!rv)
	{
		_this->cbMaxWidth = (unsigned int)((double)(_this->mi.rcWork.right - _this->mi.rcWork.left) * (SWS_WINDOWSWITCHERLAYOUT_PERCENTAGEWIDTH / 100.0));
		_this->cbMaxHeight = (unsigned int)((double)(_this->mi.rcWork.bottom - _this->mi.rcWork.top) * (SWS_WINDOWSWITCHERLAYOUT_PERCENTAGEHEIGHT / 100.0));

		HRESULT hr = GetDpiForMonitor(
			hMonitor,
			MDT_DEFAULT,
			&(_this->cbDpiX),
			&(_this->cbDpiY)
		);
		rv = sws_error_Report(sws_error_GetFromHRESULT(hr));

		_this->cbRowHeight *= (_this->cbDpiY / DEFAULT_DPI_Y);
		_this->cbRowTitleHeight *= (_this->cbDpiY / DEFAULT_DPI_X);
		_this->cbTopPadding *= (_this->cbDpiY / DEFAULT_DPI_Y);
		_this->cbLeftPadding *= (_this->cbDpiX / DEFAULT_DPI_X);
		_this->cbBottomPadding *= (_this->cbDpiY / DEFAULT_DPI_Y);
		_this->cbRightPadding *= (_this->cbDpiX / DEFAULT_DPI_X);
		_this->cbPadding *= (_this->cbDpiX / DEFAULT_DPI_X);

		_this->cbThumbnailAvailableHeight = _this->cbRowHeight - _this->cbRowTitleHeight - _this->cbTopPadding - 2 * _this->cbBottomPadding;
	}
	if (!rv)
	{
		sws_WindowSwitcherLayoutWindow* pWindowList = _this->pWindowList.pList;
		for (int iCurrentWindow = _this->pWindowList.cbSize - 1; iCurrentWindow >= 0; iCurrentWindow--)
		{
			if (!pWindowList[iCurrentWindow].hIcon)
			{
				pWindowList[iCurrentWindow].hIcon = sws_WindowHelpers_GetIconFromHWND(pWindowList[iCurrentWindow].hWnd, &(pWindowList[iCurrentWindow].bOwnProcess), iCurrentWindow == 0);
			}
		}
	}

	return rv;
}
