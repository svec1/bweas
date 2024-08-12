#ifndef _ASIST_F__H
#define _ASIST_F__H

/* ^^^ Here is the definition of assistant functions */

#include "type.h"
#include "kernel_def.h"
#include "macros.h"

typedef struct err _err;

#define KL_DEBUG_OUTPUT 1
#define KL_NO_DEBUG_OUTPUT 0

#ifdef __cplusplus

#ifdef _WIN32
__declspec(noinline)
#else
__attribute__ ((noinline))
#endif
extern "C"
{
    void init_assistf(u32t output_warning);
    void dump_assistf();
    
    /* writes and output some information to a output stream; */
    #ifdef _WIN32
    i8t  KERNELF c_wout         (str str_, u32t clr);
    #else
    i8t  KERNELF c_wout         (str str_, str  clr);
    #endif

    /* open, read/write and close files */

    // these functions return the address to a pointer that points to the FILE structure pointer,
    // and this pointer is valid until another pointer is added.
    const pfile KERNELF open_file_inp(str path, str inf_open);
    const pfile KERNELF open_file_out(str path, str inf_open);

    // the function returns the buffer for which memory was allocated via malloc
    str KERNELF read_file_inp(const pfile file);

    void KERNELF write_file_out(const pfile file, str src);

    void close_file_assist(str path);

    const pfile get_pfile_ind(u32t ind);
    const pfile get_pfile(str path);

    i32t get_index_file(const pfile file);
    str get_path_file(const pfile file);
    str get_desc_file(const pfile file);

    i32t file_exist(u32t ind);

    str get_list_open_files();


    /* functions which print in a output stream error of type _err*/

    void  KERNELF  err_fatal_obj     (_err obj_err);
    void  KERNELF  err_fatal_ref     (const _err *obj_err);

    const _err* get_kernel_err(u32t ind);
}
#else

#ifdef _WIN32

__declspec(noinline) extern void  KERNELF  err_fatal_obj     (_err obj_err);
__declspec(noinline) extern void  KERNELF  err_fatal_ref     (const _err *obj_err);

__declspec(noinline) extern const _err* get_kernel_err(u32t ind);

__declspec(noinline) extern i8t  KERNELF   c_wout_kernel (str fmt, u32t clr, ...);

#else

__attribute__ ((noinline)) extern void  KERNELF  err_fatal_obj     (_err obj_err);
__attribute__ ((noinline)) extern void  KERNELF  err_fatal_ref     (const _err *obj_err);

__attribute__ ((noinline)) extern i8t  KERNELF   c_wout_kernel (str fmt, str clr, ...);

#endif
#endif

#endif