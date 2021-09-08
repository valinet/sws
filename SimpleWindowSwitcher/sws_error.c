#include "sws_error.h"

void sws_error_PrintStackTrace()
{
#if _WIN64
    DWORD machine = IMAGE_FILE_MACHINE_AMD64;
#else
    DWORD machine = IMAGE_FILE_MACHINE_I386;
#endif
    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
    HANDLE thread = GetCurrentThread();

    if (SymInitialize(process, NULL, TRUE) == FALSE)
    {
        return 0;
    }

    SymSetOptions(SYMOPT_LOAD_LINES);

    CONTEXT context;
    ZeroMemory(&context, sizeof(CONTEXT));
    context.ContextFlags = CONTEXT_FULL;
    RtlCaptureContext(&context);

#if _WIN64
    STACKFRAME frame;
    ZeroMemory(&frame, sizeof(STACKFRAME));
    frame.AddrPC.Offset = context.Rip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = context.Rbp;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Offset = context.Rsp;
    frame.AddrStack.Mode = AddrModeFlat;
#else
    STACKFRAME frame = {};
    frame.AddrPC.Offset = context.Eip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = context.Ebp;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Offset = context.Esp;
    frame.AddrStack.Mode = AddrModeFlat;
#endif

    UINT i = 0;
    while (StackWalk(machine, process, thread, &frame, &context, NULL, SymFunctionTableAccess, SymGetModuleBase, NULL))
    {
        printf("[%3d] = [0x%p] :: ", i, frame.AddrPC.Offset);

#if _WIN64
        DWORD64 moduleBase = 0;
#else
        DWORD moduleBase = 0;
#endif

        moduleBase = SymGetModuleBase(process, frame.AddrPC.Offset);

        char moduelBuff[MAX_PATH];
        if (moduleBase && GetModuleFileNameA((HINSTANCE)moduleBase, moduelBuff, MAX_PATH))
        {
        }
#if _WIN64
        DWORD64 offset = 0;
#else
        DWORD offset = 0;
#endif
        char symbolBuffer[sizeof(IMAGEHLP_SYMBOL) + 255];
        PIMAGEHLP_SYMBOL symbol = (PIMAGEHLP_SYMBOL)symbolBuffer;
        symbol->SizeOfStruct = (sizeof(IMAGEHLP_SYMBOL)) + 255;
        symbol->MaxNameLength = 254;

        if (SymGetSymFromAddr(process, frame.AddrPC.Offset, &offset, symbol))
        {
            printf("%s:", symbol->Name);
        }

        IMAGEHLP_LINE line;
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE);

        DWORD offset_ln = 0;
        if (SymGetLineFromAddr(process, frame.AddrPC.Offset, &offset_ln, &line))
        {
            wprintf(L"%d in file \"%s\"", line.LineNumber, line.FileName);
        }
        else
        {
        }

        printf("\n");
        ++i;
    }

    SymCleanup(process);

    CloseHandle(process);
}

char* sws_error_NumToDescription(sws_error_t errnum)
{
    return NULL;
}

sws_error_t sws_error_Report(sws_error_t errnum)
{
    if (errnum == SWS_ERROR_SUCCESS)
    {
        return errnum;
    }
    char* errdesc = NULL;
    printf("The following error occured in the application: ");
    if (errdesc = sws_error_NumToDescription(errnum))
    {
        printf("%s. ", errdesc);
    }
    else
    {
        printf("%d (0x%x). ", errnum, errnum);
    }
    printf("Here is the stack trace:\n");
    sws_error_PrintStackTrace();
    _getch();
    return errnum;
}

sws_error_t sws_error_GetFromHRESULT(HRESULT hResult)
{
    // NOT IMPLEMENTED
    if (SUCCEEDED(hResult))
    {
        return SWS_ERROR_SUCCESS;
    }
    return hResult;
}

sws_error_t sws_error_GetFromWin32Error(DWORD win32err)
{
    // NOT IMPLEMENTED
    if (!win32err)
    {
        return SWS_ERROR_SUCCESS;
    }
    return win32err;
}

sws_error_t sws_error_GetFromGdiplusStatus(int status)
{
    // NOT IMPLEMENTED
    if (!status)
    {
        return SWS_ERROR_SUCCESS;
    }
    return status;
}