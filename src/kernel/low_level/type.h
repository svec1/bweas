#ifndef TYPE__H_
#define TYPE__H_

#include <stdio.h>

typedef unsigned long long u64t;
typedef unsigned int u32t;
typedef unsigned short u16t;
typedef unsigned char u8t;

typedef long long i64t;
typedef int i32t;
typedef short i16t;
typedef char i8t;

typedef u32t size32t;
typedef u8t bool8;

typedef char *str;
typedef const char *cstr;

typedef const void *buf;

typedef FILE **pfile; // reference to pointer FILE

#endif