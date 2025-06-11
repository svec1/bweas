//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses />
// ------------------------------------------
//

#ifndef STATIC_STRUCT_HPP
#define STATIC_STRUCT_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <bwtools.hpp>

#include <bwaliases.hpp>
#include <bwconf_var.hpp>

// The name of a variable that, when set to 1,
// will allow project functions and many target functions
// to create configuration variables for quick access to their internals.
static constexpr auto DECL_VAR_STRUCT = "DECL_CONFIG_VAR";

// enum of str postfix name var a project
static constexpr auto PRJ_VAR_NAME                   = "_NAME";
static constexpr auto PRJ_VAR_NAME_LANG              = "_LANG";
static constexpr auto PRJ_VAR_NAME_VER               = "_VERSION";
static constexpr auto PRJ_VAR_NAME_DFLAGS_C          = "_DFLAGS_COMPILER";
static constexpr auto PRJ_VAR_NAME_DFLAGS_L          = "_DFLAGS_LINKER";
static constexpr auto PRJ_VAR_NAME_RFLAGS_C          = "_RFLAGS_COMPILER";
static constexpr auto PRJ_VAR_NAME_RFLAGS_L          = "_RFLAGS_LINKER";
static constexpr auto PRJ_VAR_NAME_PTH_C             = "_PATH_COMPILER";
static constexpr auto PRJ_VAR_NAME_PTH_L             = "_PATH_LINKER";
static constexpr auto PRJ_VAR_NAME_STD_C             = "_STANDART_C";
static constexpr auto PRJ_VAR_NAME_STD_CPP           = "_STANDART_CPP";
static constexpr auto PRJ_VAR_NAME_SRC_FILES         = "_SRC_FILES";
static constexpr auto PRJ_VAR_NAME_UTEMPLATES        = "_USE_TEMPLATES";
static constexpr auto PRJ_VAR_NAME_CUSTOM_EXT_FIELDS = "_CUSTOM_EXTENSION_FIELDS";

// enum of str postfix name var a target
static constexpr auto TRG_VAR_NAME_NPROJECT   = "_NAME_PROJECT";
static constexpr auto TRG_VAR_NAME_VER        = "_VERSION";
static constexpr auto TRG_VAR_NAME_CFG        = "_CFG";
static constexpr auto TRG_VAR_NAME_TYPE_T     = "_TYPE_TARGET";
static constexpr auto TRG_VAR_NAME_NGENERATOR = "_GENERATOR_NAME";
static constexpr auto TRG_VAR_NAME_LLIBS      = "_LIBS";

// name of additional fields, which are also part of structures,
// but which cannot be changed by the user
static constexpr auto TRG_NAME_FIELD_PROJECT = "_PROJECT";
static constexpr auto TRG_NAME_FIELD_NTARGET = "_NAME_TARGET";

// enum of the name field of target struct
static constexpr auto NAME_FIELD_TARGET_NAME = "TARGET_NAME";
static constexpr auto NAME_FIELD_TARGET_LIBS = "TARGET_LIBS";
static constexpr auto NAME_FIELD_TARGET_TYPE = "TARGET_TYPE";
static constexpr auto NAME_FIELD_TARGET_CFG  = "TARGET_CFG";
static constexpr auto NAME_FIELD_TARGET_VER  = "TARGET_VER";

// enum of the name field of project struct
static constexpr auto NAME_FIELD_PROJECT_NAME       = "T_PROJECT_NAME";
static constexpr auto NAME_FIELD_PROJECT_VER        = "T_PROJECT_VER";
static constexpr auto NAME_FIELD_PROJECT_LANG       = "T_PROJECT_LANG";
static constexpr auto NAME_FIELD_PROJECT_PCOMPILER  = "T_PROJECT_PATH_COMPILER";
static constexpr auto NAME_FIELD_PROJECT_PLINKER    = "T_PROJECT_PATH_LINKER";
static constexpr auto NAME_FIELD_PROJECT_RFCOMPILER = "T_PROJECT_RFLAGS_COMPILER";
static constexpr auto NAME_FIELD_PROJECT_RFLINKER   = "T_PROJECT_RFLAGS_LINKER";
static constexpr auto NAME_FIELD_PROJECT_DFCOMPILER = "T_PROJECT_DFLAGS_COMPILER";
static constexpr auto NAME_FIELD_PROJECT_DFLINKER   = "T_PROJECT_DFLAGS_LINKER";
static constexpr auto NAME_FIELD_PROJECT_STD_C      = "T_PROJECT_STANDART_C";
static constexpr auto NAME_FIELD_PROJECT_STD_CPP    = "T_PROJECT_STANDART_CPP";
static constexpr auto NAME_FIELD_PROJECT_SRC_FILES  = "T_PROJECT_SRC_FILES";

static constexpr auto FEATURE_FIELD_BS_CURRENT_IF = "FBS_CURRENT_INPUT_FILE";
static constexpr auto FEATURE_FIELD_BS_CURRENT_OF = "FBS_CURRENT_OUTPUT_FILE";

static constexpr auto NAME_FIELD_TEMPLATE_COMMAND_NAME              = "_NAME";
static constexpr auto NAME_FIELD_TEMPLATE_COMMAND_NAME_CCMP         = "_NCALL_C";
static constexpr auto NAME_FIELD_TEMPLATE_COMMAND_NAME_ACCEPTS_ARGS = "_ACP_ARGS";
static constexpr auto NAME_FIELD_TEMPLATE_COMMAND_RET               = "_RETURN";
static constexpr auto NAME_FIELD_TEMPLATE_COMMAND_NAME_ARGS         = "_ARGS";

static constexpr auto NAME_FIELD_CALL_COMPONENT_NAME          = "_NAME";
static constexpr auto NAME_FIELD_CALL_COMPONENT_NAME_PROGRAM  = "_NAME_PROGRAM";
static constexpr auto NAME_FIELD_CALL_COMPONENT_PATTERN_FILES = "_PATTERN_FILES";

namespace var {
namespace struct_sb {

enum class type_target {
    exe = 0,
    lib,
    interpret
};
enum class configuration {
    RELEASE = 0,
    DEBUG
};

inline string target_t_str(const type_target &target_t) {
    if (target_t == type_target::exe)
        return "EXECUTABLE";
    else if (target_t == type_target::lib)
        return "LIBRARY";
    else if (target_t == type_target::interpret)
        return "RUN-TIME";
    return "null";
}
inline string cfg_str(const configuration &target_t) {
    if (target_t == configuration::RELEASE)
        return "RELEASE";
    else if (target_t == configuration::DEBUG)
        return "DEBUG";
    return "null";
}

inline type_target to_type_target(string target_t) {
    if (target_t == "EXECUTABLE")
        return type_target::exe;
    else if (target_t == "LIBRARY")
        return type_target::lib;
    else if (target_t == "RUN-TIME")
        return type_target::interpret;
    return type_target::exe;
}
inline configuration to_cfg(string target_t) {
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
    version(string version_str) {
        std::regex version_syntax(R"(^(\d)(?:\.(\d))?(?:\.(\d))?$)");

        std::smatch args_match;
        if (!std::regex_match(version_str, args_match, version_syntax))
            return;

        size_t i = 0;
        for (const auto &arg : args_match) {
            if (arg == args_match[0])
                continue;

            const auto arg_str = arg.str().c_str();

            switch (i) {
            case 0:
                major = std::atoll(arg_str);
                break;
            case 1:
                minor = std::atoll(arg_str);
                break;
            case 2:
                patch = std::atoll(arg_str);
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
    string name_project;
    version version_project;

    string language;

    string path_compiler{DEFAULT_COMPILER_CPP}, path_linker{DEFAULT_COMPILER_CPP};
    string rflags_compiler{RELEASE_FLAGS_COMPILER_CPP}, rflags_linker{RELEASE_FLAGS_LINKER_CXX};
    string dflags_compiler{DEBUG_FLAGS_COMPILER_CPP}, dflags_linker{DEBUG_FLAGS_LINKER_CXX};
    pdiff standart_c{98}, standart_cpp{14};

    vec<string> src_files;
    vec<string> include_paths{"/usr/include", "/usr/local/include"};
    vec<string> vec_templates;

    map<string, string> custom_ext_fields;

    static map<string, string> preset_ext_fields;
};

// target structure
// ----------------
// includes the main project, the current configuration
// and additional dependencies
struct target {
    target() = default;

  public:
    std::shared_ptr<project> prj;
    type_target target_t;
    configuration target_cfg;

    string name_target;
    string name_generator{DEFAULT_BWEAS_GENERATOR};
    version version_target;

    vec<string> target_vec_libs;
};

// target structure for build system
// ---------------------------------
struct target_out {
    target_out() = default;

  public:
    project prj;
    type_target target_t;
    configuration target_cfg;

    string name_target;
    string name_generator;
    version version_target;

    vec<string> target_vec_libs;
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

  public:
    string name;

    string name_call_component;
    vec<string> name_accept_params;
    string returnable;

    vec<arg> args;
};

struct call_component {
    string name;
    string name_program;

    // file.txt
    string pattern_ret_files;
};

} // namespace struct_sb
} // namespace var

#endif
