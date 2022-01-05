#ifndef _H_SWS_DEF_H_
#define _H_SWS_DEF_H_
#define UNICODE
#include <Windows.h>

#define SWS_GUID_TEXTUAL "{BEA057BB-66C7-4758-A610-FAE6013E9F98}"

DEFINE_GUID(LiveSetting_Property_GUID, 0xc12bcd8e, 0x2a8e, 0x4950, 0x8a, 0xe7, 0x36, 0x25, 0x11, 0x1d, 0x58, 0xeb);

#ifndef NTDDI_WIN10_CO
#define DWMWA_USE_HOSTBACKDROPBRUSH 17            // [set] BOOL, Allows the use of host backdrop brushes for the window.
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20          // [set] BOOL, Allows a window to either use the accent color, or dark, according to the user Color Mode preferences.
#define DWMWA_WINDOW_CORNER_PREFERENCE 33         // [set] WINDOW_CORNER_PREFERENCE, Controls the policy that rounds top-level window corners
#define DWMWA_BORDER_COLOR 34                     // [set] COLORREF, The color of the thin border around a top-level window
#define DWMWA_CAPTION_COLOR 35                    // [set] COLORREF, The color of the caption
#define DWMWA_TEXT_COLOR 36                       // [set] COLORREF, The color of the caption text
#define DWMWA_VISIBLE_FRAME_BORDER_THICKNESS 37   // [get] UINT, width of the visible border around a thick frame window

#define DWMWCP_DEFAULT 0
#define DWMWCP_DONOTROUND 1
#define DWMWCP_ROUND 2
#define DWMWCP_ROUNDSMALL 3
#endif

#define DWMWA_MICA_EFFFECT 1029

#define DEFAULT_DPI_X 96.0
#define DEFAULT_DPI_Y 96.0

#define SWS_UWP_ICON_SCALE_FACTOR 0.7

#define SWS_WINDOWSWITCHER_THEME_NONE 0
#define SWS_WINDOWSWITCHER_THEME_BACKDROP 1
#define SWS_WINDOWSWITCHER_THEME_MICA 2

#define SWS_WINDOWSWITCHER_CLASSNAME "SimpleWindowSwitcher_" SWS_GUID_TEXTUAL
#define SWS_WINDOWSWITCHER_BACKGROUND_COLOR RGB(32, 32, 32)
#define SWS_WINDOWSWITCHER_BACKGROUND_COLOR_LIGHT RGB(243, 243, 243)
#define SWS_WINDOWSWITCHER_CONTOUR_COLOR RGB(255, 255, 255)
#define SWS_WINDOWSWITCHER_CONTOUR_COLOR_LIGHT RGB(0, 0, 0)
#define SWS_WINDOWSWITCHER_TEXT_COLOR RGB(255, 255, 255)
#define SWS_WINDOWSWITCHER_TEXT_COLOR_LIGHT RGB(0, 0, 0)
#define SWS_WINDOWSWITCHER_CLOSE_COLOR RGB(196, 43, 28)
#define SWS_WINDOWSWITCHER_CONTOUR_SIZE 2
#define SWS_WINDOWSWITCHER_HIGHLIGHT_SIZE 1
#define SWS_WINDOWSWITCHER_THEME_CLASS "ControlPanelStyle"
#define SWS_WINDOWSWITCHER_THEME_INDEX 8
#define SWS_FONT_NAME "Segoe UI"
#define SWS_FONT_SIZE -12
#define SWS_FONT_SIZE2 -24

#define SWS_WINDOWFLAG_IS_ON_WINDOW    0b001
#define SWS_WINDOWFLAG_IS_ON_THUMBNAIL 0b010
#define SWS_WINDOWFLAG_IS_ON_CLOSE     0b100

#define SWS_CONTOUR_INNER 1
#define SWS_CONTOUR_OUTER -1

#define SWS_WINDOWSWITCHERLAYOUT_INCLUDE_WALLPAPER TRUE
#define SWS_WINDOWSWITCHERLAYOUT_WALLPAPER_ALWAYS_LAST TRUE
#define SWS_WINDOWSWITCHERLAYOUT_WALLPAPER_TOGGLE TRUE
#define SWS_WINDOWSWITCHERLAYOUT_PERCENTAGEWIDTH 80
#define SWS_WINDOWSWITCHERLAYOUT_PERCENTAGEHEIGHT 80
#define SWS_WINDOWSWITCHERLAYOUT_ROWHEIGHT 230
#define SWS_WINDOWSWITCHERLAYOUT_MAX_TILE_WIDTH 2.0

#define SWS_WINDOWSWITCHERLAYOUT_MASTER_PADDING_TOP 40
#define SWS_WINDOWSWITCHERLAYOUT_MASTER_PADDING_LEFT SWS_WINDOWSWITCHERLAYOUT_MASTER_PADDING_TOP
#define SWS_WINDOWSWITCHERLAYOUT_MASTER_PADDING_BOTTOM SWS_WINDOWSWITCHERLAYOUT_MASTER_PADDING_TOP
#define SWS_WINDOWSWITCHERLAYOUT_MASTER_PADDING_RIGHT SWS_WINDOWSWITCHERLAYOUT_MASTER_PADDING_TOP

#define SWS_WINDOWSWITCHERLAYOUT_ELEMENT_PADDING_TOP 5
#define SWS_WINDOWSWITCHERLAYOUT_ELEMENT_PADDING_LEFT 2
#define SWS_WINDOWSWITCHERLAYOUT_ELEMENT_PADDING_BOTTOM 5
#define SWS_WINDOWSWITCHERLAYOUT_ELEMENT_PADDING_RIGHT 2

#define SWS_WINDOWSWITCHERLAYOUT_PADDING_TOP 7
#define SWS_WINDOWSWITCHERLAYOUT_PADDING_LEFT SWS_WINDOWSWITCHERLAYOUT_PADDING_TOP
#define SWS_WINDOWSWITCHERLAYOUT_PADDING_BOTTOM SWS_WINDOWSWITCHERLAYOUT_PADDING_TOP
#define SWS_WINDOWSWITCHERLAYOUT_PADDING_RIGHT SWS_WINDOWSWITCHERLAYOUT_PADDING_TOP

#define SWS_WINDOWSWITCHERLAYOUT_PADDING_DIVIDER_HORIZONTAL SWS_WINDOWSWITCHERLAYOUT_PADDING_TOP
#define SWS_WINDOWSWITCHERLAYOUT_PADDING_DIVIDER_VERTICAL SWS_WINDOWSWITCHERLAYOUT_PADDING_LEFT

#define SWS_WINDOWSWITCHERLAYOUT_ROWTITLEHEIGHT 30
#define SWS_WINDOWSWITCHERLAYOUT_PERCENTAGEICON 70
#define SWS_WINDOWSWITCHERLAYOUT_COMPUTE_DIRECTION_INITIAL 0
#define SWS_WINDOWSWITCHERLAYOUT_COMPUTE_DIRECTION_FORWARD 1
#define SWS_WINDOWSWITCHERLAYOUT_COMPUTE_DIRECTION_BACKWARD -1

#define SWS_WINDOWSWITCHER_LAYOUTMODE_FULL 0
#define SWS_WINDOWSWITCHER_LAYOUTMODE_MINI 1

#define SWS_WINDOWSWITCHER_TIMER_SHOW 10
#define SWS_WINDOWSWITCHER_TIMER_TOGGLE_DESKTOP 11
#define SWS_WINDOWSWITCHER_TIMER_TOGGLE_DESKTOP_DELAY 500
#define SWS_WINDOWSWITCHER_TIMER_STARTUP 12
#define SWS_WINDOWSWITCHER_TIMER_STARTUP_DELAY 3000
#define SWS_WINDOWSWITCHER_TIMER_PEEKATDESKTOP 13
#define SWS_WINDOWSWITCHER_TIMER_PEEKATDESKTOP_DELAY 500
#define SWS_WINDOWSWITCHER_TIMER_ASYNCKEYCHECK 14
#define SWS_WINDOWSWITCHER_TIMER_ASYNCKEYCHECK_DELAY 100
#define SWS_WINDOWSWITCHER_TIMER_UPDATEACCESSIBLETEXT 15
#define SWS_WINDOWSWITCHER_TIMER_UPDATEACCESSIBLETEXT_DELAY 100

#define SWS_WINDOWSWITCHER_SHOWDELAY 100
#define SWS_WINDOWSWITCHER_MAX_NUM_MONITORS 20
#define SWS_WINDOWSWITCHER_MAX_NUM_WINDOWS 100

#define SWS_VECTOR_CAPACITY 50

#define SWS_SORT_DESCENDING 0b1
#define SWS_SORT_ASCENDING  0b0

#define SWS_WALLPAPERSUPPORT_NONE 0
#define SWS_WALLPAPERSUPPORT_EXPLORER 1

DEFINE_GUID(sws_CLSID_InputSwitchControl,
    0xB9BC2A50,
    0x43C3, 0x41AA, 0xa0, 0x86,
    0x5D, 0xB1, 0x4e, 0x18, 0x4b, 0xae
);

DEFINE_GUID(sws_IID_InputSwitchControl,
    0xB9BC2A50,
    0x43C3, 0x41AA, 0xa0, 0x82,
    0x5D, 0xB1, 0x4e, 0x18, 0x4b, 0xae
);

DEFINE_GUID(sws_IID_IInputSwitchCallback,
    0xB9BC2A50,
    0x43C3, 0x41AA, 0xa0, 0x83,
    0x5D, 0xB1, 0x4e, 0x18, 0x4b, 0xae
);

typedef struct IInputSwitchCallbackUpdateData
{
    DWORD dwID; // OK
    DWORD dw0; // always 0
    LPCWSTR pwszLangShort; // OK ("ENG")
    LPCWSTR pwszLang; // OK ("English (United States)")
    LPCWSTR pwszKbShort; // OK ("US")
    LPCWSTR pwszKb; // OK ("US keyboard")
    LPCWSTR pwszUnknown5;
    LPCWSTR pwszUnknown6;
    LPCWSTR pwszLocale; // OK ("en-US")
    LPCWSTR pwszUnknown8;
    LPCWSTR pwszUnknown9;
    LPCWSTR pwszUnknown10;
    LPCWSTR pwszUnknown11;
    LPCWSTR pwszUnknown12;
    LPCWSTR pwszUnknown13;
    LPCWSTR pwszUnknown14;
    LPCWSTR pwszUnknown15;
    LPCWSTR pwszUnknown16;
    LPCWSTR pwszUnknown17;
    DWORD dwUnknown18;
    DWORD dwUnknown19;
    DWORD dwNumber; // ???
} IInputSwitchCallbackUpdateData;

typedef interface sws_IInputSwitchControl sws_IInputSwitchControl;

typedef interface sws_IInputSwitchCallback sws_IInputSwitchCallback;

typedef struct sws_IInputSwitchControlVtbl
{
    BEGIN_INTERFACE

    HRESULT(STDMETHODCALLTYPE* QueryInterface)(
        sws_IInputSwitchControl* This,
        /* [in] */ REFIID riid,
        /* [annotation][iid_is][out] */
        _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        sws_IInputSwitchControl* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        sws_IInputSwitchControl* This);

    HRESULT(STDMETHODCALLTYPE* Init)(
        sws_IInputSwitchControl* This,
        /* [in] */ unsigned int clientType);

    HRESULT(STDMETHODCALLTYPE* SetCallback)(
        sws_IInputSwitchControl* This,
        /* [in] */ sws_IInputSwitchCallback* pInputSwitchCallback);

    END_INTERFACE
} sws_IInputSwitchControlVtbl;

interface sws_IInputSwitchControl
{
    CONST_VTBL struct sws_IInputSwitchControlVtbl* lpVtbl;
};

typedef struct sws_IInputSwitchCallbackVtbl
{
    BEGIN_INTERFACE

    HRESULT(STDMETHODCALLTYPE* QueryInterface)(
        sws_IInputSwitchCallback* This,
        /* [in] */ REFIID riid,
        /* [annotation][iid_is][out] */
        _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        sws_IInputSwitchCallback* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        sws_IInputSwitchCallback* This);

    HRESULT(STDMETHODCALLTYPE* OnUpdateProfile)(
        sws_IInputSwitchCallback* This,
        /* [in] */ IInputSwitchCallbackUpdateData* ud);

    HRESULT(STDMETHODCALLTYPE* OnUpdateTsfFloatingFlags)(
        sws_IInputSwitchCallback* This);

    HRESULT(STDMETHODCALLTYPE* OnProfileCountChange)(
        sws_IInputSwitchCallback* This,
        /* [in] */ int a2,
        /* [in] */ int a3);

    HRESULT(STDMETHODCALLTYPE* OnShowHide)(
        sws_IInputSwitchCallback* This,
        /* [in] */ int dwShowStatus);

    HRESULT(STDMETHODCALLTYPE* OnImeModeItemUpdate)(
        sws_IInputSwitchCallback* This,
        /* [in] */ void* ime);

    HRESULT(STDMETHODCALLTYPE* OnModalitySelected)(
        sws_IInputSwitchCallback* This);

    HRESULT(STDMETHODCALLTYPE* OnContextFlagsChange)(
        sws_IInputSwitchCallback* This,
        /* [in] */ char flags);

    HRESULT(STDMETHODCALLTYPE* OnTouchKeyboardManualInvoke)(
        sws_IInputSwitchCallback* This);

    END_INTERFACE
} sws_IInputSwitchCallbackVtbl;

interface sws_IInputSwitchCallback
{
    CONST_VTBL struct sws_IInputSwitchCallbackVtbl* lpVtbl;
};
#endif
