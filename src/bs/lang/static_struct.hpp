#ifndef _STATIC_STRUCT__H
#define _STATIC_STRUCT__H

#include "../../kernel/high_level/bwtype.h"
#include "../conf_var.hpp"

#include <memory>
#include <string>
#include <vector>

// The name of a variable that, when set to 1,
// will allow project functions and many target functions
// to create configuration variables for quick access to their internals.
#define DECL_VAR_STRUCT "DECL_CONFIG_VAR"

// enum of str postfix name var a project
#define PRJ_VAR_NAME "_NAME"
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

// name of additional fields, which are also part of structures,
// but which cannot be changed by the user
#define TRG_NAME_FIELD_PROJECT "_PROJECT"
#define TRG_NAME_FIELD_NTARGET "_NAME_TARGET"

// enum of the name field of target struct
#define NAME_FIELD_TARGET_NAME "TARGET_NAME"
#define NAME_FIELD_TARGET_LIBS "TARGET_LIBS"
#define NAME_FIELD_TARGET_TYPE "TARGET_TYPE"
#define NAME_FIELD_TARGET_CFG "TARGET_CFG"
#define NAME_FIELD_TARGET_VER "TARGET_VER"

// enum of the name field of project struct
#define NAME_FIELD_PROJECT_NAME "T_PROJECT_NAME"
#define NAME_FIELD_PROJECT_VER "T_PROJECT_VER"
#define NAME_FIELD_PROJECT_LANG "T_PROJECT_LANG"
#define NAME_FIELD_PROJECT_PCOMPILER "T_PROJECT_PATH_COMPILER"
#define NAME_FIELD_PROJECT_PLINKER "T_PROJECT_PATH_LINKER"
#define NAME_FIELD_PROJECT_RFCOMPILER "T_PROJECT_RFLAGS_COMPILER"
#define NAME_FIELD_PROJECT_RFLINKER "T_PROJECT_RFLAGS_LINKER"
#define NAME_FIELD_PROJECT_DFCOMPILER "T_PROJECT_DFLAGS_COMPILER"
#define NAME_FIELD_PROJECT_DFLINKER "T_PROJECT_DFLAGS_LINKER"
#define NAME_FIELD_PROJECT_STD_C "T_PROJECT_STANDART_C"
#define NAME_FIELD_PROJECT_STD_CPP "T_PROJECT_STANDART_CPP"
#define NAME_FIELD_PROJECT_SRC_FILES "T_PROJECT_SRC_FILES"

#define FEATURE_FIELD_BS_CURRENT_IF "FBS_CURRENT_INPUT_FILE"
#define FEATURE_FIELD_BS_CURRENT_OF "FBS_CURRENT_OUTPUT_FILE"

#define NAME_FIELD_TEMPLATE_COMMAND_NAME "_NAME"
#define NAME_FIELD_TEMPLATE_COMMAND_NAME_CCMP "_NCALL_C"
#define NAME_FIELD_TEMPLATE_COMMAND_NAME_ACCEPTS_ARGS "_ACP_ARGS"
#define NAME_FIELD_TEMPLATE_COMMAND_RET "_RETURN"
#define NAME_FIELD_TEMPLATE_COMMAND_NAME_ARGS "_ARGS"

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

inline std::string lang_str(const language &lang) {
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
inline std::string target_t_str(const type_target &target_t) {
    if (target_t == type_target::exe)
        return "EXECUTABLE";
    else if (target_t == type_target::lib)
        return "LIBRARY";
    else if (target_t == type_target::interpret)
        return "RUN-TIME";
    return "null";
}
inline std::string cfg_str(const configuration &target_t) {
    if (target_t == configuration::RELEASE)
        return "RELEASE";
    else if (target_t == configuration::DEBUG)
        return "DEBUG";
    return "null";
}

inline language to_lang(std::string lang) {
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
inline type_target to_type_target(std::string target_t) {
    if (target_t == "EXECUTABLE")
        return type_target::exe;
    else if (target_t == "LIBRARY")
        return type_target::lib;
    else if (target_t == "RUN-TIME")
        return type_target::interpret;
    return type_target::exe;
}
inline configuration to_cfg(std::string target_t) {
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

        if (major_str.find('.') == major_str.npos)
            goto init_mmp;
        major_str.erase(major_str.find('.'));
        minor_str.erase(0, minor_str.find('.') + 1);
        if (minor_str.find('.') == minor_str.npos)
            goto init_mmp;
        minor_str.erase(minor_str.find('.'));
        patch_str.erase(0, patch_str.find_last_of('.') + 1);

    init_mmp:
        major = std::atoll(major_str.c_str());
        minor = std::atoll(minor_str.c_str());
        patch = std::atoll(patch_str.c_str());
    }
    version(u32t mj, u32t mn, u32t ptch) : major{mj}, minor{mn}, patch{ptch} {
    }

    std::string get_str_version() const {
        return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
    }

    bool operator==(const version &ver2) {
        if (major == ver2.major && minor == ver2.minor && patch == ver2.patch)
            return 1;
        return 0;
    }
    bool operator<(const version &ver2) {
        if (major < ver2.major || (major <= ver2.major && minor < ver2.minor) ||
            (major <= ver2.major && minor <= ver2.minor && patch < ver2.patch))
            return 1;
        return 0;
    }

    bool operator==(const std::string ver2) {
        if (std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch) == ver2)
            return 1;
        return 0;
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
    std::string rflags_compiler{RELEASE_FLAGS_COMPILER_CPP}, rflags_linker{RELEASE_FLAGS_LINKER_CXX};
    std::string dflags_compiler{DEBUG_FLAGS_COMPILER_CPP}, dflags_linker{DEBUG_FLAGS_LINKER_CXX};
    i32t standart_c{98}, standart_cpp{14};

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

// target structure for build system
// ---------------------------------
struct target_out {
    target_out() = default;
    project prj;
    type_target target_t;
    configuration target_cfg;

    std::string name_target;
    version version_target;

    std::vector<std::string> target_vec_libs{"null"};
};

struct template_command {
    struct arg {
        enum class type {
            extglobal = 0,
            trgfield,
            string,
            internal,
            features
        };

        arg() = default;
        arg(const arg&) = default;
        arg(std::string _arg, type _arg_t) : str_arg(_arg), arg_t(_arg_t) {
        }

        std::string str_arg;
        type arg_t;
    };
    std::string name;

    std::string name_call_component;
    std::vector<std::string> name_accept_params;
    std::string returnable;

    std::vector<arg> args;
};

struct call_component {
    std::string name;
    std::string name_program;

    // file.txt
    std::string pattern_ret_files;
};

} // namespace struct_sb
} // namespace var

#endif