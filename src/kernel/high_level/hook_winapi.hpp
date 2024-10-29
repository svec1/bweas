#ifndef HOOK_WINAPI__H
#define HOOK_WINAPI__H

#include <Windows.h>
#include <memoryapi.h>
#include <processthreadsapi.h>
#include <wininet.h>

#define DETECTED_HOOK_H NULL
#define DETECTED_HOOK_N 0x0001

#define HOOK_TEXT_DETECTED                                                                                             \
    "WARNING!!!A winapi call was detected, most likely the dynamically loaded dll library is hostile.\n"
#define HOOK_TEXT_WARNING HOOK_TEXT_DETECTED "INFO: "

#define hook_handleExW(info, size_info)                                                                                \
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);                                                       \
    WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), (HOOK_TEXT_WARNING), 149, NULL, NULL);                              \
    WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), info, size_info, NULL, NULL);                                       \
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);                                                       \
    WAS_HOOK = 1;
#define hook_handleExA(info, size_info)                                                                                \
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);                                                       \
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), (HOOK_TEXT_WARNING), 149, NULL, NULL);                              \
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), info, size_info, NULL, NULL);                                       \
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);                                                       \
    WAS_HOOK = 1;

extern "C"
{
    void _winapi_call_hook();
    void _winapi_call_remove();

    BOOL _winapi_call_knw_was_hook();
}

#endif