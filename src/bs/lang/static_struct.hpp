#ifndef _STATIC_STRUCT__H
#define _STATIC_STRUCT__H

#include "../../kernel/low_level/type.h"
#include "../conf_var.hpp"

#include <memory>
#include <string>
#include <vector>

// The name of a variable that, when set to 1,
// will allow project functions and many target functions
// to create configuration variables for quick access to their internals.
#define DECL_VAR_STRUCT "DECL_CONFIG_VAR"

// enum of str postfix name var a project
#define PRJ_VAR_NAME_LANG "_LANG"
#define PRJ_VAR_NAME_VER "_VERSION"
#define PRJ_VAR_NAME_DFLAGS_C "_DFLAGS_COMPILER"
#define PRJ_VAR_NAME_DFLAGS_L "_DFLAGS_LINKER"
#define PRJ_VAR_NAME_RFLAGS_C "_RFLAGS_COMPILER"
#define PRJ_VAR_NAME_RFLAGS_L "_RFLAGS_LINKER"
#define PRJ_VAR_NAME_PTH_C "_PATH_COMPILER"
#define PRJ_VAR_NAME_PTH_L "_PATH_LINKER"
#define PRJ_VAR_NAME_STD_C "_STANDART_C"
#define PRJ_VAR_NAME_STD_CPP "_STANDART_CPP"
#define PRJ_VAR_NAME_SRC_FILES "_SRC_FILES"
#define PRJ_VAR_NAME_UTEMPLATES "_USE_TEMPLATES"
#define PRJ_VAR_NAME_UITTEMPLATES "_USE_IT_TEMPLATES"

// enum of str postfix name var a target
#define TRG_VAR_NAME_NPROJECT "_NAME_PROJECT"
#define TRG_VAR_NAME_VER "_VERSION"
#define TRG_VAR_NAME_CFG "_CFG"
#define TRG_VAR_NAME_TYPE_T "_TYPE_TARGET"
#define TRG_VAR_NAME_LLIBS "_LIBS"

namespace var {
namespace struct_sb {

enum class language {
    c = 0,
    cpp,
    csharp,
    asm_
};
enum class type_target {
    exe = 0,
    lib,
    interpret
};
enum class configuration {
    RELEASE,
    DEBUG
};

inline std::string
lang_str(const language &lang) {
    if (lang == language::c)
        return "C";
    else if (lang == language::cpp)
        return "C++";
    else if (lang == language::csharp)
        return "C#";
    else if (lang == language::asm_)
        return "ASM";
    return "null";
}
inline std::string
target_t_str(const type_target &target_t) {
    if (target_t == type_target::exe)
        return "EXECUTABLE";
    else if (target_t == type_target::lib)
        return "LIBRARY";
    else if (target_t == type_target::interpret)
        return "RUN-TIME";
    return "null";
}
inline std::string
cfg_str(const configuration &target_t) {
    if (target_t == configuration::RELEASE)
        return "RELEASE";
    else if (target_t == configuration::DEBUG)
        return "DEBUG";
    return "null";
}

inline language
to_lang(std::string lang) {
    if (lang == "C")
        return language::c;
    else if (lang == "C++")
        return language::cpp;
    else if (lang == "C#")
        return language::csharp;
    else if (lang == "ASM")
        return language::asm_;
    return language::c;
}
inline type_target
to_type_target(std::string target_t) {
    if (target_t == "EXECUTABLE")
        return type_target::exe;
    else if (target_t == "LIBRARY")
        return type_target::lib;
    else if (target_t == "RUN-TIME")
        return type_target::interpret;
    return type_target::exe;
}
inline configuration
to_cfg(std::string target_t) {
    if (target_t == "RELEASE")
        return configuration::RELEASE;
    else if (target_t == "DEBUG")
        return configuration::DEBUG;
    return configuration::RELEASE;
}

// structure for naming versions in a style MinorMajorPatch
// --------------------------------------------------------
// default(0.0.0)
struct version {
    version() = default;
    version(std::string version_str) {
        std::string major_str = version_str;
        std::string minor_str = version_str;
        std::string patch_str = version_str;

        major_str.erase(major_str.find('.'));
        minor_str.erase(0, minor_str.find('.') + 1);
        minor_str.erase(minor_str.find('.'));
        patch_str.erase(0, patch_str.find_last_of('.') + 1);

        major = std::stoi(major_str);
        minor = std::stoi(minor_str);
        patch = std::stoi(patch_str);
    }
    version(u32t mj, u32t mn, u32t ptch) : major{mj}, minor{mn}, patch{ptch} {
    }

    std::string
    get_str_version() {
        return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
    }

    u32t major{0};
    u32t minor{0};
    u32t patch{0};
};

// project structure
// -----------------
// includes settings that are needed
// to generate compilation commands, linking, etc.
struct project {
    project() = default;
    std::string name_project;
    version version_project;

    language lang;

    std::string path_compiler{DEFAULT_COMPILER_C}, path_linker{DEFAULT_COMPILER_C};
    std::string rflags_compiler{RELEASE_FLAGS_COMPILER}, rflags_linker{RELEASE_FLAGS_LINKER};
    std::string dflags_compiler{DEBUG_FLAGS_COMPILER}, dflags_linker{DEBUG_FLAGS_LINKER};
    u32t standart_c{98}, standart_cpp{14};

    bool use_it_templates{0};

    std::vector<std::string> src_files;
    std::vector<std::string> vec_templates{"null"};
};

// target structure
// ----------------
// includes the main project, the current configuration
// and additional dependencies
struct target {
    target() = default;
    std::shared_ptr<project> prj;
    type_target target_t;
    configuration target_cfg;

    std::string name_target;
    version version_target;

    std::vector<std::string> target_vec_libs{"null"};
};

struct target_out {
    target_out() = default;
    project prj;
    type_target target_t;
    configuration target_cfg;

    std::string name_target;
    version version_target;

    std::vector<std::string> target_vec_libs{"null"};
};

} // namespace struct_sb
} // namespace var

#endif