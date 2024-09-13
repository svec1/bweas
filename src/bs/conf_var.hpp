#ifndef _CONF_VAR__H
#define _CONF_VAR__H

#include "../mdef.hpp"

#include <string>
#include <vector>

#if defined(WIN)

#define RELEASE_FLAGS_COMPILER ""
#define DEBUG_FLAGS_COMPILER ""
#define RELEASE_FLAGS_LINKER ""
#define DEBUG_FLAGS_LINKER ""

#define DEFUALT_COMPILER_C "clang"

#define PATH_TO_COMPILER_WIN1 ""

#elif defined(UNIX)

#define RELEASE_FLAGS_COMPILER ""
#define DEBUG_FLAGS_COMPILER ""
#define RELEASE_FLAGS_LINKER ""
#define DEBUG_FLAGS_LINKER ""

#define DEFUALT_COMPILER_C "clang"

#define PATH_TO_COMPILER_UNIX_CL1 ""

#endif

namespace config_var {

extern const char *version;

extern bool logging;

// stop the build if the build of one of the targets fails
extern bool warning_prj;

// dynamic post-semantic command module(DPCM)
extern bool make_dpcm;

extern const char *logfile_name;
} // namespace config_var
#endif