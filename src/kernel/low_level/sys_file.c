#include "sys_f.h"
#include "sys_file.h"

#include "macros.h"

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _err _err;
typedef struct file {
    FILE **files;
    inf_file *desc;

    size32t count;
} _kl_file;

_kl_file krnlFiles;
static u32t _kl_size_buf_inf_f;
static u32t _kl_size_track_list_f;

const _err *
get_kernel_err(u32t ind);
void
err_fatal_obj(_err obj_err);
void
err_fatal_ref(const _err *obj_err);

void
init_track_file(u32t size_buf_inf_f) {
    krnlFiles.files = NULL;
    krnlFiles.desc = NULL;
    krnlFiles.count = NULL;

    _kl_size_buf_inf_f = size_buf_inf_f;
    _kl_size_track_list_f = _kl_size_buf_inf_f * _kl_size_buf_inf_f;
}
void
free_track_f_list() {
    free(krnlFiles.files);
    free(krnlFiles.desc);

    init_track_file(NULL);
}

void
kl_track_file(FILE *file, str name, str open_inf) {
    if (file == NULL)
        return;

    time_t _time = time(NULL);
    inf_file inf_n;

    inf_n.name = name;
    inf_n.descr = open_inf;
    inf_n.strack_file = localtime(&_time);

    add_el(&krnlFiles.files, sizeof(FILE *), &krnlFiles.count);
    krnlFiles.files[krnlFiles.count - 1] = file;

    add_el_c(&krnlFiles.desc, sizeof(inf_file), krnlFiles.count - 1);
    krnlFiles.desc[krnlFiles.count - 1] = inf_n;
}
void
kl_strack_file(u32t ind) {
    if (ind >= krnlFiles.count)
        err_fatal_ref(get_kernel_err(1));

    if (krnlFiles.files[ind] != NULL)
        fclose(krnlFiles.files[ind]);

    if (ind == krnlFiles.count - 1) {
        del_el_last_c(&krnlFiles.desc, sizeof(inf_file), krnlFiles.count);
        del_el_last(&krnlFiles.files, sizeof(FILE *), &krnlFiles.count);
        return;
    }

    del_el_c(&krnlFiles.desc, sizeof(inf_file), krnlFiles.count, ind);
    del_el(&krnlFiles.files, sizeof(FILE *), &krnlFiles.count, ind);
}
void
kl_reset_track_file() {
    while (krnlFiles.count)
        kl_strack_file(krnlFiles.count - 1);
}

const pfile
kl_get_track_file(u32t ind) {
    if (ind >= krnlFiles.count)
        err_fatal_ref(get_kernel_err(1));
    return &krnlFiles.files[ind];
}
const str *
kl_get_desc_track_file(u32t ind) {
    if (ind >= krnlFiles.count)
        err_fatal_ref(get_kernel_err(1));
    return &krnlFiles.desc[ind];
}

size32t
kl_get_size_list_tf() {
    return krnlFiles.count;
}

str
kl_get_track_list() {
    char *buf = malloc(sizeof(char) * _kl_size_track_list_f);
    memset(buf, NULL, sizeof(char) * _kl_size_track_list_f);
    for (u32t i = 0; i < krnlFiles.count; ++i) {
        char *p_buf_inf = kl_build_inf_file(&krnlFiles.desc[i]);
        strcat(buf, p_buf_inf);
        free(p_buf_inf);
    }
    return buf;
}

i32t
kl_find_file(str name) {
    for (u32t i = 0; i < krnlFiles.count; ++i) {
        if (!strcmp(krnlFiles.desc[i].name, name))
            return i;
    }
    return -1;
}
i32t
kl_find_file_p(const pfile file) {
    for (u32t i = 0; i < krnlFiles.count; ++i) {
        if (kl_get_track_file(i) == file)
            return i;
    }
    return -1;
}

i32t
kl_file_ind_valid(u32t ind) {
    if (ind < krnlFiles.count)
        return 1;
    return 0;
}

const inf_file *
kl_get_info_file(u32t ind) {
    if (ind >= krnlFiles.count)
        err_fatal_ref(get_kernel_err(1));
    return &krnlFiles.desc[ind];
}
str
kl_build_inf_file(const inf_file *file) {
    char *buf_t = malloc(sizeof(char) * _kl_size_buf_inf_f);
    memset(buf_t, NULL, sizeof(char) * _kl_size_buf_inf_f);
    strcat(buf_t, file->name);
    strcat(buf_t, get_time_stm(file->strack_file));
    strcat(buf_t, ": ");
    strcat(buf_t, file->descr);
    strcat(buf_t, "\n");
    return buf_t;
}