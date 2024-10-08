#include "assist_f.h"
#include "err.h"
#include "sys_file.h"

#include "macros.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN
HANDLE hndl;
#endif

static _err KERNEL_ERR[COUNT_KERN_ERR];
static size32t CNT_KERN_ERR = 0;

static u32t kl_output_warning = KL_NO_DEBUG_OUTPUT;

#if defined(WIN)

void
init_assistf(u32t output_warning) {
    hndl = GetStdHandle(STD_OUTPUT_HANDLE);
    kl_output_warning = output_warning;

    init_track_file(128);

    KERNEL_ERR[0] = make_err("KNL001:Undefined behavior", "The kernel can no longer serve the program.", 1);
    KERNEL_ERR[1] = make_err("KNL002:Out of range", "The kernel noticed that the value was out of range.", 2);
    KERNEL_ERR[2] =
        make_err("KNL003:Danger Found", "The kernel has noticed the danger. Security is not guaranteed.", 3);
}
void
dump_assistf() {
    if (kl_output_warning && kl_get_size_list_tf()) {
        char *buf_f = kl_get_track_list();
        c_wout_kernel("Was not closed:\n[\n%s]\n", FOREGROUND_RED, buf_f);
        free(buf_f);
    }
    hndl = NULL;
    kl_output_warning = KL_NO_DEBUG_OUTPUT;
    memset(&KERNEL_ERR, NULL, COUNT_KERN_ERR);

    kl_reset_track_file();
    free_track_f_list();
}

i8t KERNELF
c_wout(str str_, u32t clr) {

    if (hndl == NULL)
        return 0;

    SetConsoleTextAttribute(hndl, clr);
    puts(str_);
    SetConsoleTextAttribute(hndl, 15);

    return 1;
}
i8t KERNELF
c_wout_kernel(str fmt, u32t clr, ...) {
    if (hndl == NULL)
        return 0;

    va_list param;
    va_start(param, clr);

    SetConsoleTextAttribute(hndl, clr);
    vprintf(fmt, param);
    SetConsoleTextAttribute(hndl, 15);
    va_end(param);

    return 1;
}
void KERNELF
err_fatal_obj(_err obj_err) {
    c_wout_kernel("[KERNEL]: %s(%i)\n| %s\n", FOREGROUND_RED, obj_err.name_e, obj_err.ind, obj_err.desc_e);
    exit(EXIT_FAILURE);
}
void KERNELF
err_fatal_ref(const _err *obj_err) {
    if (obj_err == NULL)
        c_wout_kernel("[KERNEL]: uninitialized error object %x\n", FOREGROUND_RED, &(err_fatal_ref));
    else
        c_wout_kernel("[KERNEL]: %s(%i)\n| %s\n", FOREGROUND_RED, obj_err->name_e, obj_err->ind, obj_err->desc_e);
    exit(EXIT_FAILURE);
}

#elif defined(UNIX)

void
init_assistf(u32t output_warning) {
    kl_output_warning = output_warning;

    init_track_file(128);

    KERNEL_ERR[0] = make_err("KNL001:Undefined behavior", "The kernel can no longer serve the program.", 1);
    KERNEL_ERR[1] = make_err("KNL002:Out of range", "The kernel noticed that the value was out of range.", 2);
    KERNEL_ERR[2] =
        make_err("KNL003:Danger Found", "The kernel has noticed the danger. Security is not guaranteed.", 3);
}
void
dump_assistf() {
    if (kl_output_warning && kl_get_size_list_tf()) {
        char *buf_f = kl_get_track_list();
        c_wout_kernel("Was not closed:\n[\n%s]\n", "\033[31m\n", buf_f);
        free(buf_f);
    }
    kl_output_warning = KL_NO_DEBUG_OUTPUT;
    memset(&KERNEL_ERR, NULL, COUNT_KERN_ERR);

    kl_reset_track_file();
    free_track_f_list();
}

i8t KERNELF
c_wout(str str_, str clr) {
    str str = malloc((strlen(str_) + strlen(clr) + 5) * sizeof(char));
    sprintf(str, "%s%s\033[0m", clr, str_);

    puts(str);
    free(str);
    return 1;
}
i8t KERNELF
c_wout_kernel(str fmt, str clr, ...) {
    str str = malloc((strlen(fmt) + strlen(clr) + sizeof("\033[0m\n") + 5) * sizeof(char));
    sprintf(str, "%s%s\033[0m\n", clr, fmt);

    va_list param;
    va_start(param, clr);

    vprintf(str, param);

    va_end(param);
    free(str);
    return 1;
}
void KERNELF
err_fatal_obj(_err obj_err) {
    c_wout_kernel("[KERNEL]: %s(%i)\n| %s", "\033[31m\n", obj_err.name_e, obj_err.ind, obj_err.desc_e);
    exit(EXIT_FAILURE);
}
void KERNELF
err_fatal_ref(const _err *obj_err) {
    if (obj_err == NULL) {
        c_wout_kernel("[KERNEL]: uninitialized error object %x", "\033[31m\n", &(err_fatal_ref));
        exit(EXIT_FAILURE);
    }
    c_wout_kernel("[KERNEL]: %s(%i)\n| %s", "\033[31m\n", obj_err->name_e, obj_err->ind, obj_err->desc_e);
    exit(EXIT_FAILURE);
}
#endif

// in sys_f.h
str
get_full_time();

const pfile KERNELF
open_file_inp(str path, str inf_open) {
    if (kl_find_file(path) != -1)
        return NULL;
    FILE *file = fopen(path, "rb");
    if (file == NULL)
        return NULL;
    kl_track_file(file, path, inf_open);
    return kl_get_track_file(kl_get_size_list_tf() - 1);
}
const pfile KERNELF
open_file_out(str path, str inf_open, u32t open_arg) {
    if (kl_find_file(path) != -1)
        return NULL;
    FILE *file;
    if (open_arg == OPEN_FILE_TO_WRITE)
        file = fopen(path, "w");
    else
        file = fopen(path, "a");
    if (file == NULL)
        return NULL;

    kl_track_file(file, path, inf_open);
    return kl_get_track_file(kl_get_size_list_tf() - 1);
}

str KERNELF
get_time_k() {
    return get_full_time();
}

str KERNELF
read_file_inp(const pfile file) {
    if (*file == NULL)
        return NULL;
    fseek(*file, 0, SEEK_END);
    size32t size_f = ftell(*file);
    fseek(*file, 0, SEEK_SET);
    char *buf = malloc(sizeof(char) * (size_f + 1));
    size32t read_size = fread(buf, sizeof(char), size_f, *file);
    if (read_size != size_f)
        return NULL;
    buf[size_f] = '\0';
    return buf;
}
void KERNELF
write_file_out(const pfile file, str src) {
    if (file == NULL)
        return;
    fwrite(src, sizeof(char), strlen(src), *file);
}
void
close_file_assist(str path) {
    if (kl_find_file(path) == -1)
        return;
    kl_strack_file(kl_find_file(path));
}
const pfile
get_pfile_ind(u32t ind) {
    return kl_get_track_file(ind);
}
const pfile
get_pfile(str path) {
    return kl_get_track_file(kl_find_file(path));
}
i32t
get_index_file(const pfile file) {
    if (file == NULL)
        return NULL;
    return kl_find_file_p(file);
}
str
get_path_file(const pfile file) {
    if (file == NULL)
        return NULL;
    return kl_get_info_file(kl_find_file_p(file))->name;
}
str
get_desc_file(const pfile file) {
    if (file == NULL)
        return NULL;
    return kl_get_desc_track_file(kl_find_file_p(file));
}
i32t
file_exist(u32t ind) {
    if (kl_file_ind_valid(ind))
        return 1;
    return 0;
}
str
get_list_open_files() {
    return kl_get_track_list();
}
const _err *
get_kernel_err(u32t ind) {
    indch(ind, COUNT_KERN_ERR);
    return KERNEL_ERR + ind;
}