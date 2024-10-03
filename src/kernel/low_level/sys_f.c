#include "assist_f.h"
#include "sys_f.h"

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void
add_el(void **arr, size32t bit_size, size32t *size) {
    void *arr_new = malloc(bit_size * ((*size) + 1));
    for (u32t i = 0; i < bit_size * (*size); ++i) {
        ((u8t *)arr_new)[i] = ((u8t *)(*arr))[i];
    }
    ++*size;
    free(*arr);
    *arr = arr_new;
}
void
add_el_c(void **arr, size32t bit_size, size32t size) {
    void *arr_new = malloc(bit_size * (size + 1));
    for (u32t i = 0; i < bit_size * size; ++i) {
        ((u8t *)arr_new)[i] = ((u8t *)(*arr))[i];
    }
    free(*arr);
    *arr = arr_new;
}
void
del_el(void **arr, size32t bit_size, size32t *size, u32t ind) {
    if (ind >= *size)
        err_fatal_ref(get_kernel_err(1));

    u32t beg_mempos = bit_size * ind;
    u32t end_mempos = (bit_size * ind) + bit_size;

    void *arr_new = malloc(bit_size * ((*size) - 1));
    for (u32t i = 0, b = 0; i < bit_size * (*size); ++i) {
        if (i >= beg_mempos && i < end_mempos)
            continue;
        ((u8t *)arr_new)[b] = ((u8t *)(*arr))[i];
        ++b;
    }
    --*size;
    free(*arr);
    *arr = arr_new;
}
void
del_el_c(void **arr, size32t bit_size, size32t size, u32t ind) {
    if (ind >= size)
        err_fatal_ref(get_kernel_err(1));

    u32t beg_mempos = bit_size * ind;
    u32t end_mempos = (bit_size * ind) + bit_size;

    void *arr_new = malloc(bit_size * (size - 1));
    for (u32t i = 0, b = 0; i < bit_size * size; ++i) {
        if (i >= beg_mempos && i < end_mempos)
            continue;
        ((u8t *)arr_new)[b] = ((u8t *)(*arr))[i];
        ++b;
    }
    free(*arr);
    *arr = arr_new;
}
void
del_el_last(void **arr, size32t bit_size, size32t *size) {
    void *arr_new = malloc(bit_size * (*size - 1));
    for (u32t i = 0; i < bit_size * (*size - 1); ++i) {
        ((u8t *)arr_new)[i] = ((u8t *)(*arr))[i];
    }
    --*size;
    free(*arr);
    *arr = arr_new;
}

void
del_el_last_c(void **arr, size32t bit_size, size32t size) {
    void *arr_new = malloc(bit_size * (size - 1));
    for (u32t i = 0; i < bit_size * (size - 1); ++i) {
        ((u8t *)arr_new)[i] = ((u8t *)(*arr))[i];
    }
    free(*arr);
    *arr = arr_new;
}

str
get_time() {
    time_t _time = time(NULL);
    struct tm *local = localtime(&_time);
    char buf_tm[256];
    sprintf(buf_tm, "[Min: %i; Sec: %i]", local->tm_min, local->tm_sec);
    return buf_tm;
}
str
get_full_time() {
    time_t _time = time(NULL);
    struct tm *local = localtime(&_time);
    char *buf_tm = malloc(256);
    sprintf(buf_tm, "%im : %id - [Min: %i; Sec: %i]", local->tm_mon, local->tm_mday, local->tm_min, local->tm_sec);
    return buf_tm;
}
str
get_time_stm(struct tm *_time) {
    char buf_tm[256];
    sprintf(buf_tm, "[Min: %i; Sec: %i]", _time->tm_min, _time->tm_sec);
    return buf_tm;
}