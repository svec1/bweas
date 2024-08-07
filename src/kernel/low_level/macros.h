#ifndef MACROS__H_
#define MACROS__H_

    // macro set

    #define is_digit(ch) (ch >= '0' && ch <= '9')
    #define indch(ind, size_src) if(ind < 0 || ind >=size_src) return NULL

#endif