#ifndef SYS_KL_FILE__H_
#define SYS_KL_FILE__H_

#include "type.h"
#include <time.h>

typedef struct inf_file {
    str name;
    str descr;
    struct tm *strack_file; // file tracking start time
} inf_file;

// _kl_file krnlFiles; <- all files

// init_track_file(NULL) <- setting null value for all of *system_track_file*,
// this reset to zero his
void
init_track_file(u32t size_buf_inf_f);

// it's better than init_track_file(NULL)
// - this takes into account the malloc implementation
void
free_track_f_list();

void
kl_track_file(FILE *file, str name, str open_inf);
void
kl_strack_file(u32t ind);

// Urgently closes (stops monitoring) of the file.
// Must be used before free_track_f_list()
void
kl_reset_track_file();

const pfile
kl_get_track_file(u32t ind);
const str *
kl_get_desc_track_file(u32t ind);
size32t
kl_get_size_list_tf();

char *
kl_get_track_list();

i32t
kl_find_file(str name);
i32t
kl_find_file_p(const pfile file);

i32t
kl_file_ind_valid(u32t ind);

const inf_file *
kl_get_info_file(u32t ind);
char *
kl_build_inf_file(const inf_file *file);

#endif