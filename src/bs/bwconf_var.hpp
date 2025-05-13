//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWCONF_VAR__H
#define BWCONF_VAR__H

#include <bwmacros_platform.h>

#define DEFAULT_BWEAS_JSON_CONFIG                                                                                      \
    "{ \
        \"cache-gn \": \"fast_bwcache\", \
        \"packages\": [] \
    }"

#define DEFAULT_BWEAS_GENERATOR "bwgenerator"

#if defined(WIN)

#define RELEASE_FLAGS_COMPILER_CPP                                                                                     \
    "-Wall -Wextra -Waddress -Warray-bounds -Wbool-operation -Wunused-value -Wchar-subscripts -Wcomment "              \
    "-Wint-in-bool-context -Wuninitialized -Wvec-elem-size -Wmisleading-indentation -Wmisleading-indentation "         \
    "-Wparentheses -Wpessimizing-move -Wsign-compare -Wsizeof-pointer-div -Wstrict-aliasing -Wtautological-compare "   \
    "-Wuninitialized -Wunused-function -Wunused-variable -Wpedantic -Wsign-promo"
#define DEBUG_FLAGS_COMPILER_CPP                                                                                       \
    "-g -gno-inline-points -Wall -Wextra -Waddress -Warray-bounds -Wbool-operation -Wunused-value -Wchar-subscripts "  \
    "-Wcomment "                                                                                                       \
    "-Wint-in-bool-context -Wuninitialized -Wvec-elem-size -Wmisleading-indentation -Wmisleading-indentation "         \
    "-Wparentheses -Wpessimizing-move -Wsign-compare -Wsizeof-pointer-div -Wstrict-aliasing -Wtautological-compare "   \
    "-Wuninitialized -Wunused-function -Wunused-variable -Wpedantic -Wsign-promo"
#define RELEASE_FLAGS_LINKER_CXX "-fuse-ld=lld -s"
#define DEBUG_FLAGS_LINKER_CXX "-fuse-ld=lld"

#define DEFAULT_COMPILER_C "clang"

#define PATH_TO_COMPILER_WIN1 "null"

#elif defined(UNIX)

#define RELEASE_FLAGS_COMPILER_CPP                                                                                     \
    "-Wall -Wextra -Waddress -Warray-bounds -Wbool-operation -Wunused-value -Wchar-subscripts -Wcomment "              \
    "-Wint-in-bool-context -Wuninitialized -Wvec-elem-size -Wmisleading-indentation -Wmisleading-indentation "         \
    "-Wparentheses -Wpessimizing-move -Wsign-compare -Wsizeof-pointer-div -Wstrict-aliasing -Wtautological-compare "   \
    "-Wuninitialized -Wunused-function -Wunused-variable -Wpedantic -Wsign-promo"
#define DEBUG_FLAGS_COMPILER_CPP                                                                                       \
    "-g -gno-inline-points -Wall -Wextra -Waddress -Warray-bounds -Wbool-operation -Wunused-value -Wchar-subscripts "  \
    "-Wcomment "                                                                                                       \
    "-Wint-in-bool-context -Wuninitialized -Wvec-elem-size -Wmisleading-indentation -Wmisleading-indentation "         \
    "-Wparentheses -Wpessimizing-move -Wsign-compare -Wsizeof-pointer-div -Wstrict-aliasing -Wtautological-compare "   \
    "-Wuninitialized -Wunused-function -Wunused-variable -Wpedantic -Wsign-promo"
#define RELEASE_FLAGS_LINKER_CXX "-fuse-ld=lld -s"
#define DEBUG_FLAGS_LINKER_CXX "-fuse-ld=lld"

#define DEFAULT_COMPILER_C "clang"

#define PATH_TO_COMPILER_UNIX_CL1 "null"

#endif

#endif
