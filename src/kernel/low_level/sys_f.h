#ifndef SYS_F__H_
#define SYS_F__H_

#include "type.h"

struct tm;

void
add_el(void **arr, size32t bit_size, size32t *size);
void
add_el_c(void **arr, size32t bit_size, size32t size);

void
del_el(void **arr, size32t bit_size, size32t *size, u32t ind);
void
del_el_c(void **arr, size32t bit_size, size32t size, u32t ind);
void
del_el_last(void **arr, size32t bit_size, size32t *size);
void
del_el_last_c(void **arr, size32t bit_size, size32t size);

str
get_time();
str
get_full_time();
str
get_time_stm(struct tm *_time);

#endif