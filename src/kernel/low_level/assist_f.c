#include "assist_f.h"
#include "sys_file.h"
#include "err.h"

#include "macros.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
HANDLE hndl;
#endif

static _err KERNEL_ERR[COUNT_KERN_ERR];
static size32t CNT_KERN_ERR = 0;

static u32t kl_output_warning = KL_NO_DEBUG_OUTPUT;

#ifdef _WIN32

__declspec(noinline)
void init_assistf(u32t output_warning){
    hndl = GetStdHandle(STD_OUTPUT_HANDLE);
    kl_output_warning = output_warning;
    
    init_track_file(128);

    KERNEL_ERR[0] = make_err("KNL001:Undefined behavior", "The kernel can no longer serve the program.", 1);
    KERNEL_ERR[1] = make_err("KNL002:Out of range", "The kernel noticed that the value was out of range.", 2);
    KERNEL_ERR[2] = make_err("KNL003:Danger Found", "Security is not guaranteed. The kernel can no longer support the application.", 3);
}
__declspec(noinline)
void dump_assistf(){
    if(kl_output_warning && kl_get_size_list_tf()){
        char* buf_f = kl_get_track_list();
        c_wout_kernel("Was not closed:\n[\n%s]\n", FOREGROUND_RED, buf_f);
        free(buf_f);
    }
    kl_reset_track_file();
    free_track_f_list();
}

__declspec(noinline)
i8t KERNELF c_wout(str str_, u32t clr){
    
    if(hndl == NULL) return 0;
    
    SetConsoleTextAttribute(hndl, clr);
    puts(str_);
    SetConsoleTextAttribute(hndl, 15);
    
    return 1;
}
__declspec(noinline)
i8t KERNELF c_wout_kernel(str fmt, u32t clr, ...){
    if(hndl == NULL) return 0;

    va_list param;
    va_start(param, clr);
    
    SetConsoleTextAttribute(hndl, clr);
    vprintf(fmt, param);
    SetConsoleTextAttribute(hndl, 15);
    va_end(param);

    return 1;
}
__declspec(noinline)
const pfile KERNELF open_file_inp(str path, str inf_open){
    if(kl_find_file(path) != -1)
        return NULL;
    FILE* file = fopen(path, "rb");
    if(file == NULL)
        return NULL;
    kl_track_file(file, path, inf_open);
    return kl_get_track_file(kl_get_size_list_tf()-1);
}
__declspec(noinline)
const pfile KERNELF open_file_out(str path, str inf_open){
    if(kl_find_file(path) != -1)
        return NULL;
    FILE* file = fopen(path, "a");
    if(file == NULL)
        return NULL;

    kl_track_file(file, path, inf_open);
    return kl_get_track_file(kl_get_size_list_tf()-1);
}
__declspec(noinline)
str KERNELF read_file_inp(const pfile file){
    if(*file == NULL) return NULL;
    fseek(*file, 0, SEEK_END);
    size32t size_f = ftell(*file);
    fseek(*file, 0, SEEK_SET);
    char* buf = malloc(sizeof(char)*(size_f+1));
    size32t read_size = fread(buf, sizeof(char), size_f, *file);
    if (read_size != size_f)
        return NULL;
    buf[size_f] = '\0';
    return buf;
}
__declspec(noinline)
void KERNELF write_file_out(const pfile file, str src){
    if(file == NULL) return;
    fwrite(src, sizeof(char), strlen(src), *file);
}
__declspec(noinline)
void close_file_assist(str path){
    if(kl_find_file(path) == -1) return;
    kl_strack_file(kl_find_file(path));
}
__declspec(noinline)
const pfile get_pfile_ind(u32t ind){
    return kl_get_track_file(ind);
}
__declspec(noinline)
const pfile get_pfile(str path){
    return kl_get_track_file(kl_find_file(path));
}
__declspec(noinline)
i32t get_index_file(const pfile file){
    if(file == NULL) return NULL;
    return kl_find_file_p(file);
}
__declspec(noinline)
str get_path_file(const pfile file){
    if(file == NULL) return NULL;
    return kl_get_info_file(kl_find_file_p(file))->name;
}
__declspec(noinline)
str get_desc_file(const pfile file){
    if(file == NULL) return NULL;
    return kl_get_desc_track_file(kl_find_file_p(file));
}
__declspec(noinline)
str get_list_open_files(){
    return kl_get_track_list();
}

__declspec(noinline)
void KERNELF err_fatal_obj(_err obj_err){
    c_wout_kernel("[KERNEL]: %s(%i)\n| %s\n", FOREGROUND_RED, obj_err.name_e, obj_err.ind, obj_err.desc_e);
    exit(EXIT_FAILURE);
}
__declspec(noinline)
void KERNELF err_fatal_ref(const _err *obj_err){
    if(obj_err == NULL)
        c_wout_kernel("[KERNEL]: uninitialized error object %x\n", FOREGROUND_RED, &(err_fatal_ref));
    else 
        c_wout_kernel("[KERNEL]: %s(%i)\n| %s\n", FOREGROUND_RED, obj_err->name_e, obj_err->ind, obj_err->desc_e);
    exit(EXIT_FAILURE);
}
__declspec(noinline)
const _err* get_kernel_err(u32t ind){
    indch(ind, COUNT_KERN_ERR);
    return KERNEL_ERR+ind;
}
#else

__attribute__ ((noinline))
void init_assistf(u32t output_warning){
    kl_output_warning = output_warning;

    init_track_file();

    KERNEL_ERR[0] = make_err("KNL001:Undefined behavior", "The kernel can no longer serve the program.", 1);
    KERNEL_ERR[1] = make_err("KNL002:Out of range", "The kernel noticed that the value was out of range.", 2);
}
__attribute__ ((noinline))
void dump_assistf(){
    #ifdef KL_DEBUG_OUTPUT

    #endif
    kl_reset_track_file();
}

__attribute__ ((noinline))
i8t KERNELF c_wout(str str_, str clr){
    str str = malloc((strlen(str_)+strlen(clr)+5)*sizeof(char));
    sprintf(str, "%s%s\033[0m", clr, str_);

    puts(str);
    free(str);
    return 1;
}
__attribute__ ((noinline))
i8t KERNELF c_wout_kernel(str fmt, u32t clr, ...){
    str str = malloc((strlen(fmt)+strlen(clr)+5)*sizeof(char));
    sprintf(str, "%s%s\033[0m", clr, fmt);

    va_list param;
    va_start(param, clr);

    vprintf(str, param);
    
    va_end(param);
    free(str);
    return 1;
}
__attribute__ ((noinline))
const pfile KERNELF open_file_inp(str path, str inf_open){
    if(kl_find_file(path) != -1)
        return NULL;
    FILE* file = fopen(path, "r");
    if(file == NULL)
        return NULL;
    kl_track_file(file, path, inf_open);
    return kl_get_track_file(kl_get_size_list_tf()-1);
}
__attribute__ ((noinline))
const pfile KERNELF open_file_out(str path, str inf_open){
    if(kl_find_file(path) != -1)
        return NULL;
    FILE* file = fopen(path, "a");
    if(file == NULL)
        return NULL;

    kl_track_file(file, path, inf_open);
    return kl_get_track_file(kl_get_size_list_tf()-1);
}
__attribute__ ((noinline))
str KERNELF read_file_inp(const pfile file){
    if(*file == NULL) return NULL;
    fseek(*file, 0, SEEK_END);
    size32t size_f = ftell(*file);
    fseek(*file, 0, SEEK_SET);
    char* buf = malloc(sizeof(char)*(size_f+1));
    fread(buf, sizeof(char), size_f, *file);
    buf[size_f] = '\0';

    return buf;
}
__attribute__ ((noinline))
void  KERNELF write_file_out(const pfile file, str src){
    if(file == NULL) return NULL;
    fwrite(src, sizeof(char), strlen(src), *file);
}
__attribute__ ((noinline))
void close_file(str path){
    if(kl_find_file(path) == -1) return;
    kl_strack_file(kl_find_file(path));
}
__attribute__ ((noinline))
const pfile get_pfile(str path){
    return kl_get_track_file(kl_find_file(path));
}
__attribute__ ((noinline))
str get_list_open_files(){
    return kl_get_track_list();
}
__attribute__ ((noinline))
void KERNELF err_fatal_obj(_err obj_err){
    c_wout_kernel("[KERNEL]: %s(%i)\n| %s", "\033[31m", obj_err.name_e, obj_err.ind, obj_err.desc_e);
    exit(EXIT_FAILURE);
}
__attribute__ ((noinline))
void KERNELF err_fatal_ref(_err *obj){
    if(obj_err == NULL){
        c_wout_kernel("[KERNEL]: uninitialized error object %x", "\033[31m", &(err_fatal_ref));
        exit(EXIT_FAILURE);
    }
    c_wout_kernel("[KERNEL]: %s(%i)\n| %s", "\033[31m", obj_err->name_e, obj_err->ind, obj_err->desc_e);
    exit(EXIT_FAILURE);
}
__attribute__ ((noinline))
const _err* get_kernel_err(u32t ind){
    indch(ind, COUNT_KERN_ERR);
    return &KERNEL_ERR[ind];
}
#endif