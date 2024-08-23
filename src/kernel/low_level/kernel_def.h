#ifndef KERNEL_DEF__H_
#define KERNEL_DEF__H_

#include "macros.h"

#define COUNT_KERN_ERR 256

#if defined(WIN)
#include <Windows.h>
#define KERNELF __stdcall
extern HANDLE hndl;
#elif defined(UNIX)
#define KERNELF
#endif

#endif