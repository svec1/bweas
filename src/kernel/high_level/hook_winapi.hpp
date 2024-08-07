#ifndef HOOK_WINAPI__H
#define HOOK_WINAPI__H

#ifdef _WIN32

#include "../../../external/detours/detours.h"

#include <Windows.h>
#include <processthreadsapi.h>
#include <memoryapi.h>
#include <wininet.h>

#define DETECTED_HOOK_H NULL
#define DETECTED_HOOK_N 0x0001

#define hook_handleW(info) realWriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), L"WARNING!!!A winapi call was detected, \
                                                    most likely the dynamically loaded dll library is hostile. INFO: ", \
                                                    149, NULL, NULL); \
                          realWriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), info, \
                                                    strlen((const char*)info), NULL, NULL); 
#define hook_handleA(info) realWriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), "WARNING!!!A winapi call was detected, \
                                                    most likely the dynamically loaded dll library is hostile. INFO: ", \
                                                    149, NULL, NULL); \
                          realWriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), info, \
                                                    strlen((const char*)info), NULL, NULL);
#define hook_handleExW(info, size_info) realWriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), L"WARNING!!!A winapi call was detected, \
                                                    most likely the dynamically loaded dll library is hostile. INFO: ", \
                                                    149, NULL, NULL); \
                          realWriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), info, \
                                                    size_info, NULL, NULL);  
#define hook_handleExA(info, size_info) realWriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), "WARNING!!!A winapi call was detected, \
                                                    most likely the dynamically loaded dll library is hostile. INFO: ", \
                                                    149, NULL, NULL); \
                          realWriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), info, \
                                                    size_info, NULL, NULL); 

BOOL
(WINAPI *realWriteConsoleW)(
    _In_ HANDLE hConsoleOutput,
    _In_reads_(nNumberOfCharsToWrite) CONST VOID* lpBuffer,
    _In_ DWORD nNumberOfCharsToWrite,
    _Out_opt_ LPDWORD lpNumberOfCharsWritten,
    _Reserved_ LPVOID lpReserved
    ) = WriteConsoleW;
BOOL
(WINAPI *realWriteConsoleA)(
    _In_ HANDLE hConsoleOutput,
    _In_reads_(nNumberOfCharsToWrite) CONST VOID* lpBuffer,
    _In_ DWORD nNumberOfCharsToWrite,
    _Out_opt_ LPDWORD lpNumberOfCharsWritten,
    _Reserved_ LPVOID lpReserved
    ) = WriteConsoleA;
BOOL
(WINAPI *realCreateProcessW)(
    _In_opt_ LPCWSTR lpApplicationName,
    _Inout_opt_ LPWSTR lpCommandLine,
    _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
    _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
    _In_ BOOL bInheritHandles,
    _In_ DWORD dwCreationFlags,
    _In_opt_ LPVOID lpEnvironment,
    _In_opt_ LPCWSTR lpCurrentDirectory,
    _In_ LPSTARTUPINFOW lpStartupInfo,
    _Out_ LPPROCESS_INFORMATION lpProcessInformation
    ) = CreateProcessW;
BOOL
(WINAPI *realCreateProcessA)(
    _In_opt_ LPCSTR lpApplicationName,
    _Inout_opt_ LPSTR lpCommandLine,
    _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
    _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
    _In_ BOOL bInheritHandles,
    _In_ DWORD dwCreationFlags,
    _In_opt_ LPVOID lpEnvironment,
    _In_opt_ LPCSTR lpCurrentDirectory,
    _In_ LPSTARTUPINFOA lpStartupInfo,
    _Out_ LPPROCESS_INFORMATION lpProcessInformation
    ) = CreateProcessA;
HANDLE
(WINAPI *realCreateRemoteThread)(
    _In_ HANDLE hProcess,
    _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
    _In_ SIZE_T dwStackSize,
    _In_ LPTHREAD_START_ROUTINE lpStartAddress,
    _In_opt_ LPVOID lpParameter,
    _In_ DWORD dwCreationFlags,
    _Out_opt_ LPDWORD lpThreadId
    ) = CreateRemoteThread;
LPVOID
(WINAPI *realVirtualAlloc)(
    _In_opt_ LPVOID lpAddress,
    _In_ SIZE_T dwSize,
    _In_ DWORD flAllocationType,
    _In_ DWORD flProtect
    ) = VirtualAlloc;
LPVOID
(WINAPI *VirtualAllocEx)(
    _In_ HANDLE hProcess,
    _In_opt_ LPVOID lpAddress,
    _In_ SIZE_T dwSize,
    _In_ DWORD flAllocationType,
    _In_ DWORD flProtect
    ) = VirtualAllocEx;
BOOL
(WINAPI *realWriteProcessMemory)(
    _In_ HANDLE hProcess,
    _In_ LPVOID lpBaseAddress,
    _In_reads_bytes_(nSize) LPCVOID lpBuffer,
    _In_ SIZE_T nSize,
    _Out_opt_ SIZE_T* lpNumberOfBytesWritten
    ) = WriteProcessMemory;
BOOL
(WINAPI *realReadProcessMemory)(
    _In_ HANDLE hProcess,
    _In_ LPCVOID lpBaseAddress,
    _Out_writes_bytes_to_(nSize,*lpNumberOfBytesRead) LPVOID lpBuffer,
    _In_ SIZE_T nSize,
    _Out_opt_ SIZE_T* lpNumberOfBytesRead
    ) = ReadProcessMemory;
LSTATUS
(APIENTRY *realRegOpenKeyExW)(
    _In_ HKEY hKey,
    _In_opt_ LPCWSTR lpSubKey,
    _In_opt_ DWORD ulOptions,
    _In_ REGSAM samDesired,
    _Out_ PHKEY phkResult
    ) = RegOpenKeyExW;
LSTATUS
(APIENTRY *realRegOpenKeyExA)(
    _In_ HKEY hKey,
    _In_opt_ LPCSTR lpSubKey,
    _In_opt_ DWORD ulOptions,
    _In_ REGSAM samDesired,
    _Out_ PHKEY phkResult
    ) = RegOpenKeyExA;

extern "C"{

    void _winapi_call_hook();
    void _winapi_call_remove();

}

#endif

#endif