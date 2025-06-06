//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWMACROS_HPP
#define BWMACROS_HPP

// macro set

#define _DEBUG

#if defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
#define UNIX
#if defined(__linux__)
#define _linux
#elif defined(__APPLE__) && defined(__MACH__)
#define APPLE_OS
#endif
#elif defined(_WIN32) || defined(_WIN64)
#define WIN
#include <Windows.h>
#endif

#if defined(__LP64__) || defined(_M_IA64) || defined(_WIN64)
#define X64
#else
#define X32
#endif

#endif
