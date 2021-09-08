#ifndef _HPP_SWS_WINDOWHELPERS_PACKAGEINFO_
#define _HPP_SWS_WINDOWHELPERS_PACKAGEINFO_
#include <Windows.h>
#ifdef __cplusplus
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
#include <winrt/base.h>
namespace winrt::impl
{
    template <typename Async>
    auto wait_for(Async const& async, Windows::Foundation::TimeSpan const& timeout);
}
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.Management.Deployment.h>
#include <winrt/Windows.Storage.h>
#include <iostream>
#pragma comment(lib, "windowsapp")
#include "sws_def.h"
#include "sws_error.h"

using namespace winrt;
using namespace Windows::ApplicationModel;
using namespace Windows::Management::Deployment;
using namespace Windows::Storage;

//#include <winrt/Windows.UI.ViewManagement.h> // may need "Microsoft.Windows.CppWinRT" NuGet package

//using namespace winrt::Windows::UI::ViewManagement;
//winrt::Windows::UI::Color accent = UISettings{}.GetColorValue(UIColorType::Accent);
#endif

#ifdef __cplusplus
extern "C"
{
#endif
    HICON sws_WindowHelpers_PackageInfo_GetForAumid(wchar_t* aumid);
#ifdef __cplusplus
}
#endif
#endif