//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses />
// ------------------------------------------
//

#ifndef STATIC_STRUCT_HPP
#define STATIC_STRUCT_HPP

#include <bwtools.hpp>

#include <bwaliases.hpp>
#include <bwconf_var.hpp>

namespace bweas {

// The name of a variable that, when set to 1,
// will allow project functions and many target functions
// to create configuration variables for quick access to their internals.
static constexpr auto DECL_VAR_STRUCT = "DECL_CONFIG_VAR";

// enum of str postfix name var a project
static constexpr auto PRJ_VAR_NAME                   = "_NAME";
static constexpr auto PRJ_VAR_NAME_LANG              = "_LANG";
static constexpr auto PRJ_VAR_NAME_DFLAGS_C          = "_DFLAGS_COMPILER";
static constexpr auto PRJ_VAR_NAME_DFLAGS_L          = "_DFLAGS_LINKER";
static constexpr auto PRJ_VAR_NAME_RFLAGS_C          = "_RFLAGS_COMPILER";
static constexpr auto PRJ_VAR_NAME_RFLAGS_L          = "_RFLAGS_LINKER";
static constexpr auto PRJ_VAR_NAME_PTH_C             = "_PATH_COMPILER";
static constexpr auto PRJ_VAR_NAME_PTH_L             = "_PATH_LINKER";
static constexpr auto PRJ_VAR_NAME_STD_C             = "_STANDART_C";
static constexpr auto PRJ_VAR_NAME_STD_CPP           = "_STANDART_CPP";
static constexpr auto PRJ_VAR_NAME_SRC_FILES         = "_SRC_FILES";
static constexpr auto PRJ_VAR_NAME_LIBS              = "_LIBS";
static constexpr auto PRJ_VAR_NAME_INCLUDE_PATHS     = "_INCLUDE_PATHS";
static constexpr auto PRJ_VAR_NAME_CUSTOM_EXT_FIELDS = "_CUSTOM_EXTENSION_FIELDS";

// enum of str postfix name var a target
static constexpr auto TRG_VAR_NAME              = "_NAME";
static constexpr auto TRG_VAR_NAME_VER          = "_VERSION";
static constexpr auto TRG_VAR_NAME_CFG          = "_CFG";
static constexpr auto TRG_VAR_NAME_TYPE         = "_TYPE_TARGET";
static constexpr auto TRG_VAR_NAME_GENERATOR    = "_GENERATOR_NAME";
static constexpr auto TRG_VAR_NAME_TEMPLATES    = "_TEMPLATES";
static constexpr auto TRG_VAR_NAME_DEPENDENCIES = "_DEPENDENCIES";

// name of additional fields, which are also part of structures,
// but which cannot be changed by the user
static constexpr auto TRG_NAME_FIELD_PROJECT = "_PROJECT";
static constexpr auto TRG_NAME_FIELD_NTARGET = "_NAME_TARGET";

// enum of the name field of target struct
static constexpr auto NAME_FIELD_TARGET_NAME         = "TARGET_NAME";
static constexpr auto NAME_FIELD_TARGET_VER          = "TARGET_VER";
static constexpr auto NAME_FIELD_TARGET_CFG          = "TARGET_CFG";
static constexpr auto NAME_FIELD_TARGET_TYPE         = "TARGET_TYPE";
static constexpr auto NAME_FIELD_TARGET_GENERATOR    = "_GENERATOR_NAME";
static constexpr auto NAME_FIELD_TARGET_TEMPLATES    = "TARGET_TEMPLATES";
static constexpr auto NAME_FIELD_TARGET_DEPENDENCIES = "TARGET_DEPENDENCIES";

// enum of the name field of project struct
static constexpr auto NAME_FIELD_PROJECT_LANG          = "T_PROJECT_LANG";
static constexpr auto NAME_FIELD_PROJECT_PCOMPILER     = "T_PROJECT_PATH_COMPILER";
static constexpr auto NAME_FIELD_PROJECT_PLINKER       = "T_PROJECT_PATH_LINKER";
static constexpr auto NAME_FIELD_PROJECT_RFCOMPILER    = "T_PROJECT_RFLAGS_COMPILER";
static constexpr auto NAME_FIELD_PROJECT_RFLINKER      = "T_PROJECT_RFLAGS_LINKER";
static constexpr auto NAME_FIELD_PROJECT_DFCOMPILER    = "T_PROJECT_DFLAGS_COMPILER";
static constexpr auto NAME_FIELD_PROJECT_DFLINKER      = "T_PROJECT_DFLAGS_LINKER";
static constexpr auto NAME_FIELD_PROJECT_STD_C         = "T_PROJECT_STANDART_C";
static constexpr auto NAME_FIELD_PROJECT_STD_CPP       = "T_PROJECT_STANDART_CPP";
static constexpr auto NAME_FIELD_PROJECT_SRC_FILES     = "T_PROJECT_SRC_FILES";
static constexpr auto NAME_FIELD_PROJECT_LIBS          = "T_PROJECT_LIBS";
static constexpr auto NAME_FIELD_PROJECT_INCLUDE_PATHS = "T_PROJECT_INCLUDE_PATHS";

static constexpr auto FEATURE_FIELD_BS_CURRENT_IF = "FBS_CURRENT_INPUT_FILE";
static constexpr auto FEATURE_FIELD_BS_CURRENT_OF = "FBS_CURRENT_OUTPUT_FILE";

static constexpr auto NAME_FIELD_TEMPLATE_COMMAND_NAME              = "_NAME";
static constexpr auto NAME_FIELD_TEMPLATE_COMMAND_NAME_CCMP         = "_NCALL_C";
static constexpr auto NAME_FIELD_TEMPLATE_COMMAND_NAME_ACCEPTS_ARGS = "_ACP_ARGS";
static constexpr auto NAME_FIELD_TEMPLATE_COMMAND_RET               = "_RETURN";
static constexpr auto NAME_FIELD_TEMPLATE_COMMAND_NAME_ARGS         = "_ARGS";

static constexpr auto NAME_FIELD_TEMPLATE_COMMAND_IFILES           = "_IFILES";
static constexpr auto NAME_FIELD_TEMPLATE_COMMAND_SINGLE_GENERATES = "_SINGLE_GENERATE";

static constexpr auto NAME_FIELD_CALL_COMPONENT_NAME          = "_NAME";
static constexpr auto NAME_FIELD_CALL_COMPONENT_NAME_PROGRAM  = "_NAME_PROGRAM";
static constexpr auto NAME_FIELD_CALL_COMPONENT_PATTERN_FILES = "_PATTERN_FILES";

namespace structs_context {

struct template_command;

enum class target_type {
    exe = 0,
    lib,
    interpret
};
enum class target_cfg {
    RELEASE = 0,
    DEBUG
};

inline string target_type_str(const target_type &target_t) {
    if (target_t == target_type::exe)
        return "EXECUTABLE";
    else if (target_t == target_type::lib)
        return "LIBRARY";
    else if (target_t == target_type::interpret)
        return "RUN-TIME";
    return "null";
}
inline string target_cfg_str(const target_cfg &target_t) {
    if (target_t == target_cfg::RELEASE)
        return "RELEASE";
    else if (target_t == target_cfg::DEBUG)
        return "DEBUG";
    return "null";
}

inline target_type to_target_type(string target_t) {
    if (target_t == "EXECUTABLE")
        return target_type::exe;
    else if (target_t == "LIBRARY")
        return target_type::lib;
    else if (target_t == "RUN-TIME")
        return target_type::interpret;
    return target_type::exe;
}
inline target_cfg to_target_cfg(string target_t) {
    if (target_t == "RELEASE")
        return target_cfg::RELEASE;
    else if (target_t == "DEBUG")
        return target_cfg::DEBUG;
    return target_cfg::RELEASE;
}

// structure for naming versions in a style MinorMajorPatch
// --------------------------------------------------------
// default(0.0.0)
struct version {
    version() = default;
    version(string version_str) {
        std::regex version_syntax(R"(^(\d)(?:\.(\d))?(?:\.(\d))?$)");

        std::smatch args_match;
        if (!std::regex_match(version_str, args_match, version_syntax))
            return;

        size_t i = 0;
        for (const auto &arg : args_match) {
            if (arg == args_match[0])
                continue;

            const auto arg_str = arg.str();

            switch (i) {
            case 0:
                major = std::stoi(arg_str);
                break;
            case 1:
                minor = std::stoi(arg_str);
                break;
            case 2:
                patch = std::stoi(arg_str);
                break;
            default:
                std::unreachable();
            }

            ++i;
        }
    }
    version(size_t mj, size_t mn, size_t ptch) : major{mj}, minor{mn}, patch{ptch} {
    }

  public:
    string get_str_version() const {
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

    bool operator==(const string ver2) {
        if (std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch) == ver2)
            return 1;
        return 0;
    }

  private:
    size_t major{0};
    size_t minor{0};
    size_t patch{0};
};

// project structure
// -----------------
// includes settings that are needed
// to generate compilation commands, linking, etc.
struct project {
    project() {
        custom_ext_fields.merge(preset_ext_fields);
    }

  public:
    string language;

    string path_compiler{DEFAULT_COMPILER_CPP}, path_linker{DEFAULT_COMPILER_CPP};
    string rflags_compiler{RELEASE_FLAGS_COMPILER_CPP}, rflags_linker{RELEASE_FLAGS_LINKER_CXX};
    string dflags_compiler{DEBUG_FLAGS_COMPILER_CPP}, dflags_linker{DEBUG_FLAGS_LINKER_CXX};
    pdiff standart_c = 11, standart_cpp = 17;

    vec<string> src_files;
    vec<string> libs;
    vec<string> include_paths{"/usr/include", "/usr/local/include"};

    map<string, string> custom_ext_fields;

    static map<string, string> preset_ext_fields;
};

// target structure for build system
// ---------------------------------
struct target {
    target() = default;

  public:
    project prj;

    target_type type;
    target_cfg cfg;

    string name;
    string name_generator{DEFAULT_BWEAS_GENERATOR};
    version ver;

    vec<string> templates;
    vec<string> dependencies;

  public:
    vec<template_command> queue_templates;

    bool built_success = 0;
};

struct template_command {
    template_command() = default;

  public:
    struct arg {
      public:
        enum class type {
            extglobal = 0,
            trgfield,
            string,
            internal,
            features
        };

      public:
        arg()            = default;
        arg(const arg &) = default;
        arg(string _arg, type _arg_t) : str_arg(_arg), arg_t(_arg_t) {
        }

      public:
        string str_arg;
        type arg_t;
    };

  public:
    static template_command create_template_command(string_v template_name, const string &template_str);

    // Creates a stack of templates for the correct sequential generation of commands(for every targets)
    static vec<template_command> create_queue_target_templates(const vec<template_command> &templates,
                                                               const vec<string> &templates_target,
                                                               target_type target_t);

  private:
    // Recursive function, for create_stack_target_templates
    static void recovery_queue_target_templates(vec<template_command> &vec_templates,
                                                vec<template_command> &queue_target_templates,
                                                const string &name_internal_param);

  public:
    string name;

    string name_call_component;
    string returnable;

    vec<string> name_accept_params;
    vec<arg> args;

  public:
    vec<string> ifiles;
    bool single_generates = 0;
    bool returns_target   = 0;
};

struct call_component {
    string name;
    string name_program;

    // file.txt
    string pattern_ret_files;
};

} // namespace structs_context

namespace sc = structs_context;
} // namespace bweas

#endif
