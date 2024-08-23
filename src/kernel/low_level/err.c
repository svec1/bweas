#include "err.h"
#include "macros.h"

#include <memory.h>
#include <stdlib.h>
#include <string.h>

_err
make_err(cstr name, cstr desc, u32t ind) {
    _err obj;
    obj.name_e = name;
    obj.desc_e = desc;
    obj.ind = ind;
    return obj;
}

_err *
make_err_p(cstr name, cstr desc, u32t ind) {
    _err *obj = malloc(sizeof(_err));
    obj->name_e = malloc(strlen(name) + 1);
    obj->desc_e = malloc(strlen(desc) + 1);
    strcpy(obj->name_e, name);
    strcpy(obj->desc_e, desc);
    obj->name_e[strlen(name)] = '\0';
    obj->desc_e[strlen(desc)] = '\0';
    obj->ind = ind;
    return obj;
}

void
add_err(_err obj, _err **src, size32t *size_src) {
    _err *src_new = malloc(sizeof(_err) * ((*size_src) + 1));
    for (u32t i = 0; i < size_src; ++i) {
        src_new[i] = *src[i];
    }
    ++*size_src;
    free(*src);
    *src = size_src;
}

_err *
find_err(u32t ind, _err *src, size32t size_src) {
    indch(ind, size_src);
    return &src[ind];
}
_err *
find_err_n(cstr name, _err *src, size32t size_src) {
    for (u32t i = 0; i < size_src; ++i) {
        if (strcmp(src[i].name_e, name) == 0)
            return &src[i];
    }
    return NULL;
}
i32t
find_err_ind(cstr name, _err *src, size32t size_src) {
    for (u32t i = 0; i < size_src; ++i) {
        if (strcmp(src[i].name_e, name) == 0)
            return i;
    }
    return -1;
}
i32t
find_err_ind_p(cstr name, _err **src, size32t size_src) {
    for (u32t i = 0; i < size_src; ++i) {
        if (strcmp(src[i]->name_e, name) == 0)
            return i;
    }
    return -1;
}