#ifndef KERNEL_DEF__H_
#define KERNEL_DEF__H_

#define COUNT_KERN_ERR 256

#ifdef _WIN32

    #include <Windows.h>

    #define KERNELF __stdcall
    extern HANDLE hndl;
#else
    #define KERNELF
#endif

#endif