#ifndef _H_SWS_UTILITY_H_
#define _H_SWS_UTILITY_H_
#include <Windows.h>
inline long long sws_milliseconds_now() {
    LARGE_INTEGER s_frequency;
    BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);
    if (s_use_qpc) {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        return (1000LL * now.QuadPart) / s_frequency.QuadPart;
    }
    else {
        return GetTickCount();
    }
}

// https://stackoverflow.com/questions/13397571/precise-thread-sleep-needed-max-1ms-error
inline BOOLEAN sws_nanosleep(LONGLONG ns) {
    /* Declarations */
    HANDLE timer;   /* Timer handle */
    LARGE_INTEGER li;   /* Time defintion */
    /* Create timer */
    if (!(timer = CreateWaitableTimer(NULL, TRUE, NULL)))
        return FALSE;
    /* Set timer properties */
    li.QuadPart = -ns;
    if (!SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE)) {
        CloseHandle(timer);
        return FALSE;
    }
    /* Start & wait for timer */
    WaitForSingleObject(timer, INFINITE);
    /* Clean resources */
    CloseHandle(timer);
    /* Slept without problems */
    return TRUE;
}
#endif