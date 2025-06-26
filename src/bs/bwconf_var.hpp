//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWCONF_VAR_HPP
#define BWCONF_VAR_HPP

#include <bwmacros_platform.h>

#define DEFAULT_BWEAS_JSON_CONFIG                                                                                      \
    "{ \
        \"cache-gn \": \"fast_bwcache\", \
        \"packages\": [] \
    }"

#define DEFAULT_BWEAS_GENERATOR "bwgenerator"

#if defined(WIN)

#define RELEASE_FLAGS_COMPILER_CPP "-Wall"
#define DEBUG_FLAGS_COMPILER_CPP "-g -gno-inline-points -Wall"
#define RELEASE_FLAGS_LINKER_CXX "-fuse-ld=lld -s"
#define DEBUG_FLAGS_LINKER_CXX "-fuse-ld=lld"

#define DEFAULT_COMPILER_C "clang"
#define DEFAULT_COMPILER_CPP "clang++"

#elif defined(UNIX)

#define RELEASE_FLAGS_COMPILER_CPP "-Wall"
#define DEBUG_FLAGS_COMPILER_CPP "-g -gno-inline-points -Wall"
#define RELEASE_FLAGS_LINKER_CXX "-fuse-ld=lld -s"
#define DEBUG_FLAGS_LINKER_CXX "-fuse-ld=lld"

#define DEFAULT_COMPILER_C "clang"
#define DEFAULT_COMPILER_CPP "clang++"

#endif

#endif
