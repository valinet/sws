#include "sws_WindowHelpers_PackageInfo.hpp"

HICON sws_WindowHelpers_PackageInfo_GetForAumid(wchar_t* aumid)
{
    wchar_t* excl = wcschr(aumid, L'!');
    if (!excl)
    {
        return NULL;
    }
    excl[0] = 0;
    std::wstring packageFamilyName{ aumid };
    excl[0] = '!';
    std::wstring appId{ excl + 1 };

    for (auto const& package : PackageManager{}.FindPackages(packageFamilyName))
    {
        std::wstring wszPath{ package.Logo().RawUri() };
        if (wszPath.data())
        {
            Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromFile(wszPath.data());
            if (bitmap)
            {
                HICON hIcon = NULL;
                bitmap->GetHICON(&hIcon);
                delete bitmap;
                return hIcon;
            }
        }
    }

    return NULL;
}
