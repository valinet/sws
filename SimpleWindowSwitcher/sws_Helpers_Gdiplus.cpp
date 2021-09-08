#include "sws_Helpers_Gdiplus.hpp"

void sws_Helpers_Gdiplus_Clear()
{
	Gdiplus::GdiplusShutdown(g_GdiplusToken);
}
sws_error_t sws_Helpers_Gdiplus_Initialize()
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	return sws_error_Report(sws_error_GetFromGdiplusStatus(Gdiplus::GdiplusStartup(&g_GdiplusToken, &gdiplusStartupInput, NULL)));
}
