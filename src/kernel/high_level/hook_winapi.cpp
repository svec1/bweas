#include "hook_winapi.hpp"
#include <stdlib.h>

#include <detours.h>

BOOL HOOK_PROCCES{0};
BOOL WAS_HOOK{0};

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
(WINAPI *realVirtualAllocEx)(
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

BOOL 
WINAPI bwhookWriteConsoleW(
    _In_ HANDLE hConsoleOutput,
    _In_reads_(nNumberOfCharsToWrite) CONST VOID* lpBuffer,
    _In_ DWORD nNumberOfCharsToWrite,
    _Out_opt_ LPDWORD lpNumberOfCharsWritten,
    _Reserved_ LPVOID lpReserved
    ){
        hook_handleExW(lpBuffer, nNumberOfCharsToWrite)
        return DETECTED_HOOK_N;
    }
BOOL 
WINAPI bwhookWriteConsoleA(
    _In_ HANDLE hConsoleOutput,
    _In_reads_(nNumberOfCharsToWrite) CONST VOID* lpBuffer,
    _In_ DWORD nNumberOfCharsToWrite,
    _Out_opt_ LPDWORD lpNumberOfCharsWritten,
    _Reserved_ LPVOID lpReserved
    ){
        hook_handleExA(lpBuffer, nNumberOfCharsToWrite)
        return DETECTED_HOOK_N;
    }
BOOL
WINAPI bwhookCreateProcessW(
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
    ){
        hook_handleExA("CreateProcessW call", 20)
        return DETECTED_HOOK_N;
    }
BOOL
WINAPI bwhookCreateProcessA(
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
    ){
        hook_handleExA("CreateProcessA call", 20)
        return DETECTED_HOOK_N;
    }
HANDLE
WINAPI bwhookCreateRemoteThread(
    _In_ HANDLE hProcess,
    _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
    _In_ SIZE_T dwStackSize,
    _In_ LPTHREAD_START_ROUTINE lpStartAddress,
    _In_opt_ LPVOID lpParameter,
    _In_ DWORD dwCreationFlags,
    _Out_opt_ LPDWORD lpThreadId
    ){
        hook_handleExA("CreateRemoteThread call", 24)
        return DETECTED_HOOK_H;
    }
LPVOID
WINAPI bwhookVirtualAlloc(
    _In_opt_ LPVOID lpAddress,
    _In_ SIZE_T dwSize,
    _In_ DWORD flAllocationType,
    _In_ DWORD flProtect
    ) {
        hook_handleExA("VirtualAlloc call", 18)
        return DETECTED_HOOK_H;
    }
LPVOID
WINAPI bwhookVirtualAllocEx(
    _In_ HANDLE hProcess,
    _In_opt_ LPVOID lpAddress,
    _In_ SIZE_T dwSize,
    _In_ DWORD flAllocationType,
    _In_ DWORD flProtect
    ){
        hook_handleExA("VirtualAllocEx call", 20)
        return DETECTED_HOOK_H;
    }
BOOL
WINAPI bwhookWriteProcessMemory(
    _In_ HANDLE hProcess,
    _In_ LPVOID lpBaseAddress,
    _In_reads_bytes_(nSize) LPCVOID lpBuffer,
    _In_ SIZE_T nSize,
    _Out_opt_ SIZE_T* lpNumberOfBytesWritten
    ) {
        hook_handleExA("WriteProcessMemory call", 24)
        return DETECTED_HOOK_N;
    }
BOOL
WINAPI bwhookReadProcessMemory(
    _In_ HANDLE hProcess,
    _In_ LPCVOID lpBaseAddress,
    _Out_writes_bytes_to_(nSize,*lpNumberOfBytesRead) LPVOID lpBuffer,
    _In_ SIZE_T nSize,
    _Out_opt_ SIZE_T* lpNumberOfBytesRead
    ) {
        hook_handleExA("ReadProcessMemory call", 23)
        return DETECTED_HOOK_N;
    }
LSTATUS
APIENTRY bwhookRegOpenKeyExW(
    _In_ HKEY hKey,
    _In_opt_ LPCWSTR lpSubKey,
    _In_opt_ DWORD ulOptions,
    _In_ REGSAM samDesired,
    _Out_ PHKEY phkResult
    ) {
        hook_handleExA("RegOpenKeyExW call", 19)
        return DETECTED_HOOK_N;
    }
LSTATUS
APIENTRY bwhookRegOpenKeyExA(
    _In_ HKEY hKey,
    _In_opt_ LPCSTR lpSubKey,
    _In_opt_ DWORD ulOptions,
    _In_ REGSAM samDesired,
    _Out_ PHKEY phkResult
    ) {
        hook_handleExA("RegOpenKeyExA call", 19)
        return DETECTED_HOOK_N;
    }
void _winapi_call_hook(){
    if(HOOK_PROCCES) return;
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)realWriteConsoleW, bwhookWriteConsoleW);
    DetourAttach(&(PVOID&)realWriteConsoleA, bwhookWriteConsoleA);
    DetourAttach(&(PVOID&)realCreateProcessW, bwhookCreateProcessW);
    DetourAttach(&(PVOID&)realCreateProcessA, bwhookCreateProcessA);
    DetourAttach(&(PVOID&)realCreateRemoteThread, bwhookCreateRemoteThread);
    DetourAttach(&(PVOID&)realVirtualAlloc, bwhookVirtualAlloc);
    DetourAttach(&(PVOID&)realVirtualAllocEx, bwhookVirtualAllocEx);
    DetourAttach(&(PVOID&)realWriteProcessMemory, bwhookWriteProcessMemory);
    DetourAttach(&(PVOID&)realReadProcessMemory, bwhookReadProcessMemory);
    DetourAttach(&(PVOID&)realRegOpenKeyExW, bwhookRegOpenKeyExW);
    DetourAttach(&(PVOID&)realRegOpenKeyExA, bwhookRegOpenKeyExA);
    DetourTransactionCommit();
    HOOK_PROCCES = 1;
}
void _winapi_call_remove(){
    if(!HOOK_PROCCES) return;
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(PVOID&)realWriteConsoleW, bwhookWriteConsoleW);
    DetourDetach(&(PVOID&)realWriteConsoleA, bwhookWriteConsoleA);
    DetourDetach(&(PVOID&)realCreateProcessW, bwhookCreateProcessW);
    DetourDetach(&(PVOID&)realCreateProcessA, bwhookCreateProcessA);
    DetourDetach(&(PVOID&)realCreateRemoteThread, bwhookCreateRemoteThread);
    DetourDetach(&(PVOID&)realVirtualAlloc, bwhookVirtualAlloc);
    DetourDetach(&(PVOID&)realVirtualAllocEx, bwhookVirtualAllocEx);
    DetourDetach(&(PVOID&)realWriteProcessMemory, bwhookWriteProcessMemory);
    DetourDetach(&(PVOID&)realReadProcessMemory, bwhookReadProcessMemory);
    DetourDetach(&(PVOID&)realRegOpenKeyExW, bwhookRegOpenKeyExW);
    DetourDetach(&(PVOID&)realRegOpenKeyExA, bwhookRegOpenKeyExA);
    DetourTransactionCommit();
    HOOK_PROCCES = 0;
}

BOOL _winapi_call_knw_was_hook(){
    return WAS_HOOK;
}