#ifndef MACROS__H_
#define MACROS__H_

// macro set

#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))
#define UNIX
#if defined(__linux__)
#define linux
#elif defined(__APPLE__) && defined(__MACH__)
#define APPLE_OS
#endif
#else
#if defined(_WIN32) || defined(_WIN64)
#define WIN
#endif
#endif

#if defined(__LP64__) || defined(_M_IA64) || defined(_WIN64)
#define X64
#else
#define X32
#endif

#define is_digit(ch) (ch >= '0' && ch <= '9')
#define indch(ind, size_src)                                                                                           \
    if (ind < 0 || ind >= size_src)                                                                                    \
    return NULL

#endif