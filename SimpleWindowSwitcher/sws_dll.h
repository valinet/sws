#ifndef _H_SWS_DLL_
#define _H_SWS_DLL_
#include <Windows.h>
static HRESULT(*ApplyCompatResolutionQuirkingFunc)(void*, void*);
__declspec(dllexport) HRESULT ApplyCompatResolutionQuirking(void* p1, void* p2)
{
    return ApplyCompatResolutionQuirkingFunc(p1, p2);
}
static HRESULT(*CompatStringFunc)(void*, void*, void*, BOOL);
__declspec(dllexport) HRESULT CompatString(void* p1, void* p2, void* p3, BOOL p4)
{
    return CompatStringFunc(p1, p2, p3, p4);
}
static HRESULT(*CompatValueFunc)(void*, void*);
__declspec(dllexport) HRESULT CompatValue(void* p1, void* p2)
{
    return CompatValueFunc(p1, p2);
}
static HRESULT(*CreateDXGIFactoryFunc)(void*, void**);
__declspec(dllexport) HRESULT CreateDXGIFactory(void* p1, void** p2)
{
    return CreateDXGIFactoryFunc(p1, p2);
}
static HRESULT(*CreateDXGIFactory1Func)(void*, void**);
__declspec(dllexport) HRESULT CreateDXGIFactory1(void* p1, void** p2)
{
    return CreateDXGIFactory1Func(p1, p2);
}
static HRESULT(*CreateDXGIFactory2Func)(UINT, void*, void**);
__declspec(dllexport) HRESULT CreateDXGIFactory2(UINT p1, void* p2, void** p3)
{
    return CreateDXGIFactory2Func(p1, p2, p3);
}
static HRESULT(*DXGID3D10CreateDeviceFunc)();
__declspec(dllexport) HRESULT DXGID3D10CreateDevice() {
    return DXGID3D10CreateDeviceFunc();
}
static HRESULT(*DXGID3D10CreateLayeredDeviceFunc)();
__declspec(dllexport) HRESULT DXGID3D10CreateLayeredDevice()
{
    return DXGID3D10CreateLayeredDeviceFunc();
}
static HRESULT(*DXGID3D10GetLayeredDeviceSizeFunc)();
__declspec(dllexport) HRESULT DXGID3D10GetLayeredDeviceSize()
{
    return DXGID3D10GetLayeredDeviceSizeFunc();
}
static HRESULT(*DXGID3D10RegisterLayersFunc)();
__declspec(dllexport) HRESULT DXGID3D10RegisterLayers()
{
    return DXGID3D10RegisterLayersFunc();
}
static HRESULT(*DXGIDeclareAdapterRemovalSupportFunc)();
__declspec(dllexport) HRESULT DXGIDeclareAdapterRemovalSupport()
{
    return DXGIDeclareAdapterRemovalSupportFunc();
}
static HRESULT(*DXGIDumpJournalFunc)(void*);
__declspec(dllexport) HRESULT DXGIDumpJournal(void* p1)
{
    return DXGIDumpJournalFunc(p1);
}
static HRESULT(*DXGIGetDebugInterface1Func)(UINT, void*, void**);
__declspec(dllexport) HRESULT DXGIGetDebugInterface1(UINT p1, void* p2, void** p3)
{
    return DXGIGetDebugInterface1Func(p1, p2, p3);
}
static HRESULT(*DXGIReportAdapterConfigurationFunc)();
__declspec(dllexport) HRESULT DXGIReportAdapterConfiguration()
{
    return DXGIReportAdapterConfigurationFunc();
}
static HRESULT(*PIXBeginCaptureFunc)(INT64, void*);
__declspec(dllexport) HRESULT PIXBeginCapture(INT64 p1, void* p2)
{
    return PIXBeginCaptureFunc(p1, p2);
}
static HRESULT(*PIXEndCaptureFunc)();
__declspec(dllexport) HRESULT PIXEndCapture()
{
    return PIXEndCaptureFunc();
}
static HRESULT(*PIXGetCaptureStateFunc)();
__declspec(dllexport) HRESULT PIXGetCaptureState()
{
    return PIXGetCaptureStateFunc();
}
static HRESULT(*SetAppCompatStringPointerFunc)(SIZE_T, void*);
__declspec(dllexport) HRESULT SetAppCompatStringPointer(SIZE_T p1, void* p2)
{
    return SetAppCompatStringPointerFunc(p1, p2);
}
static HRESULT(*UpdateHMDEmulationStatusFunc)(char);
__declspec(dllexport) HRESULT UpdateHMDEmulationStatus(char p1)
{
    return UpdateHMDEmulationStatusFunc(p1);
}
#endif
