#ifndef ERR__H_
#define ERR__H_

#include "type.h"

#ifdef __cplusplus
extern "C"
{
typedef struct err{
    str name_e;
    str desc_e;
    u32t ind;
} _err;

_err make_err(cstr name, cstr desc, u32t ind);
_err* make_err_p(cstr name, cstr desc, u32t ind);

void add_err(_err obj, _err **src, size32t *size_src);

_err* find_err(u32t ind, _err *src, size32t size_src);
_err* find_err_n(cstr name, _err *src, size32t size_src);
i32t find_err_ind(cstr name, _err *src, size32t size_src);
i32t find_err_ind_p(cstr name, _err **src, size32t size_src);
}
#else
typedef struct err{
    str name_e;
    str desc_e;
    u32t ind;
} _err;

_err make_err(cstr name, cstr desc, u32t ind);
_err* make_err_p(cstr name, cstr desc, u32t ind);

void add_err(_err obj, _err **src, size32t *size_src);

_err* find_err(u32t ind, _err *src, size32t size_src);
_err* find_err_n(cstr name, _err *src, size32t size_src);
i32t find_err_ind(cstr name, _err *src, size32t size_src);
i32t find_err_ind_p(cstr name, _err **src, size32t size_src);
#endif

#endif