#include <bwmacros_platform.h>

#if defined(UNIX)

#include "/usr/include/unistd.h"

#else

#ifndef _UNISTD_H
#define _UNISTD_H

#include <direct.h>
#include <io.h>
#include <process.h>
#include <stdlib.h>

#define srandom srand
#define random rand

#define R_OK 4
#define W_OK 2
#define F_OK 0

#define access _access
#define dup2 _dup2
#define execve _execve
#define ftruncate _chsize
#define unlink _unlink
#define fileno _fileno
#define getcwd _getcwd
#define chdir _chdir
#define isatty _isatty
#define lseek _lseek

#define ssize_t __int64

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

typedef __int16 int16_t;
typedef __int32 int32_t;
typedef __int64 int64_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;

#endif

#endif
