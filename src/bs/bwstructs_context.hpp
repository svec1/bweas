//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses />
// ------------------------------------------
//

#ifndef STATIC_STRUCT_HPP
#define STATIC_STRUCT_HPP

#include <bwaliases.hpp>

namespace bweas {

// The name of a variable that, when set to 1,
// will allow project functions and many target functions
// to create configuration variables for quick access to their internals.
inline constexpr auto DECL_VAR_STRUCT = "DECL_CONFIG_VAR";

// enum of str postfix name var a project
inline constexpr auto PRJ_VAR_NAME                   = "_NAME";
inline constexpr auto PRJ_VAR_NAME_DFLAGS_C          = "_DFLAGS_COMPILER";
inline constexpr auto PRJ_VAR_NAME_DFLAGS_L          = "_DFLAGS_LINKER";
inline constexpr auto PRJ_VAR_NAME_RFLAGS_C          = "_RFLAGS_COMPILER";
inline constexpr auto PRJ_VAR_NAME_RFLAGS_L          = "_RFLAGS_LINKER";
inline constexpr auto PRJ_VAR_NAME_PTH_C             = "_PATH_COMPILER";
inline constexpr auto PRJ_VAR_NAME_PTH_L             = "_PATH_LINKER";
inline constexpr auto PRJ_VAR_NAME_STD_C             = "_STANDART_C";
inline constexpr auto PRJ_VAR_NAME_STD_CPP           = "_STANDART_CPP";
inline constexpr auto PRJ_VAR_NAME_SRC_FILES         = "_SRC_FILES";
inline constexpr auto PRJ_VAR_NAME_LIBS              = "_LIBS";
inline constexpr auto PRJ_VAR_NAME_INCLUDE_PATHS     = "_INCLUDE_PATHS";
inline constexpr auto PRJ_VAR_NAME_CUSTOM_EXT_FIELDS = "_CUSTOM_EXTENSION_FIELDS";

// enum of str postfix name var a target
inline constexpr auto TRG_VAR_NAME              = "_NAME";
inline constexpr auto TRG_VAR_NAME_VER          = "_VERSION";
inline constexpr auto TRG_VAR_NAME_CFG          = "_CFG";
inline constexpr auto TRG_VAR_NAME_TYPE         = "_TYPE_TARGET";
inline constexpr auto TRG_VAR_NAME_GENERATOR    = "_GENERATOR_NAME";
inline constexpr auto TRG_VAR_NAME_TEMPLATES    = "_TEMPLATES";
inline constexpr auto TRG_VAR_NAME_DEPENDENCIES = "_DEPENDENCIES";

// name of additional fields, which are also part of structures,
// but which cannot be changed by the user
inline constexpr auto TRG_NAME_FIELD_EXTENSION = "_EXTENSION";
inline constexpr auto TRG_NAME_FIELD_NTARGET   = "_NAME_TARGET";

// enum of the name field of target struct
inline constexpr auto NAME_FIELD_TARGET_NAME         = "TARGET_NAME";
inline constexpr auto NAME_FIELD_TARGET_VER          = "TARGET_VER";
inline constexpr auto NAME_FIELD_TARGET_CFG          = "TARGET_CFG";
inline constexpr auto NAME_FIELD_TARGET_TYPE         = "TARGET_TYPE";
inline constexpr auto NAME_FIELD_TARGET_GENERATOR    = "_GENERATOR_NAME";
inline constexpr auto NAME_FIELD_TARGET_TEMPLATES    = "TARGET_TEMPLATES";
inline constexpr auto NAME_FIELD_TARGET_DEPENDENCIES = "TARGET_DEPENDENCIES";

// enum of the name field of project struct
inline constexpr auto NAME_FIELD_PROJECT_LANG          = "T_PROJECT_LANG";
inline constexpr auto NAME_FIELD_PROJECT_PCOMPILER     = "T_PROJECT_PATH_COMPILER";
inline constexpr auto NAME_FIELD_PROJECT_PLINKER       = "T_PROJECT_PATH_LINKER";
inline constexpr auto NAME_FIELD_PROJECT_RFCOMPILER    = "T_PROJECT_RFLAGS_COMPILER";
inline constexpr auto NAME_FIELD_PROJECT_RFLINKER      = "T_PROJECT_RFLAGS_LINKER";
inline constexpr auto NAME_FIELD_PROJECT_DFCOMPILER    = "T_PROJECT_DFLAGS_COMPILER";
inline constexpr auto NAME_FIELD_PROJECT_DFLINKER      = "T_PROJECT_DFLAGS_LINKER";
inline constexpr auto NAME_FIELD_PROJECT_STD_C         = "T_PROJECT_STANDART_C";
inline constexpr auto NAME_FIELD_PROJECT_STD_CPP       = "T_PROJECT_STANDART_CPP";
inline constexpr auto NAME_FIELD_PROJECT_SRC_FILES     = "T_PROJECT_SRC_FILES";
inline constexpr auto NAME_FIELD_PROJECT_LIBS          = "T_PROJECT_LIBS";
inline constexpr auto NAME_FIELD_PROJECT_INCLUDE_PATHS = "T_PROJECT_INCLUDE_PATHS";

inline constexpr auto FEATURE_ARG_IF = "INPUT_FILE";
inline constexpr auto FEATURE_ARG_OF = "OUTPUT_FILE";

inline constexpr auto NAME_FIELD_TEMPLATE_COMMAND_NAME              = "_NAME";
inline constexpr auto NAME_FIELD_TEMPLATE_COMMAND_NAME_CCMP         = "_NCALL_C";
inline constexpr auto NAME_FIELD_TEMPLATE_COMMAND_NAME_ACCEPTS_ARGS = "_ACP_ARGS";
inline constexpr auto NAME_FIELD_TEMPLATE_COMMAND_RET               = "_RETURN";
inline constexpr auto NAME_FIELD_TEMPLATE_COMMAND_NAME_ARGS         = "_ARGS";

inline constexpr auto NAME_FIELD_TEMPLATE_COMMAND_IFILES           = "_IFILES";
inline constexpr auto NAME_FIELD_TEMPLATE_COMMAND_SINGLE_GENERATES = "_SINGLE_GENERATE";

inline constexpr auto NAME_FIELD_CALL_COMPONENT_NAME          = "_NAME";
inline constexpr auto NAME_FIELD_CALL_COMPONENT_NAME_PROGRAM  = "_NAME_PROGRAM";
inline constexpr auto NAME_FIELD_CALL_COMPONENT_PATTERN_FILES = "_PATTERN_FILES";

namespace structs_context {

struct target;
struct template_command;

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

        for (size_t i = 1; i < args_match.size(); ++i) {
            if (args_match[i].str().empty())
                break;
            const auto arg_str = args_match[i].str();

            switch (i) {
            case 1:
                major = std::stoi(arg_str);
                break;
            case 2:
                minor = std::stoi(arg_str);
                break;
            case 3:
                patch = std::stoi(arg_str);
                break;
            default:
                std::unreachable();
            }
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

struct profile {
    using fields = umap<string, std::variant<string, vec<string>>>;

    profile() = default;
    profile(fields _release_fields, std::optional<fields> _debug_fields = std::nullopt)
        : release_fields(_release_fields), debug_fields(_debug_fields) {
    }

  public:
    void set_fields(size_t _cfg) {
        cfg = _cfg;
    }
    fields &get_fields() {
        if (cfg)
            return debug_fields.value();
        return release_fields;
    }
    const fields &get_fields() const {
        if (cfg)
            return debug_fields.value();
        return release_fields;
    }

  public:
    fields::mapped_type &operator[](string key) {
        if (cfg)
            return debug_fields.value().at(key);
        return release_fields.at(key);
    }
    const fields::mapped_type &operator[](string key) const {
        if (cfg)
            return debug_fields.value().at(key);
        return release_fields.at(key);
    }

    template <typename T,
              typename = std::enable_if<std::is_same_v<T, string> || std::is_same_v<T, vec<string>>, void>::type>
    T &get(string key) {
        return std::get<T>(this->operator[](key));
    }
    template <typename T,
              typename = std::enable_if<std::is_same_v<T, string> || std::is_same_v<T, vec<string>>, void>::type>
    const T &get(string key) const {
        return std::get<T>(this->operator[](key));
    }

    bool contains(string key) const {
        return get_fields().contains(key);
    }

    bool is_string(string key) const {
        if (std::holds_alternative<string>(this->operator[](key)))
            return true;
        return false;
    }

  public:
    fields release_fields;
    std::optional<fields> debug_fields;

    fields global_fields; // in package impl

    size_t cfg = 0;

  public:
    static constexpr auto FIELD_SOURCE_FILES  = "source_files";
    static constexpr auto FIELD_INCLUDE_PATHS = "include_paths";
    static constexpr auto FIELD_LANGUAGE      = "language";
    static constexpr auto FIELD_TARGET_TYPE   = "target_type";
};
// target structure for build system
// ---------------------------------
struct target {
    target() = default;

    enum class e_type {
        exe = 0,
        lib,
        interpret
    };
    enum class e_cfg {
        release = 0,
        debug
    };

  public:
    profile ext;

    e_type type;
    e_cfg cfg;

    string name;
    version ver;

    vec<string> templates;
    vec<string> dependencies;

  public:
    vec<template_command> queue_templates;

    bool built_success = 0;

  public:
    template <typename T> T &fields(string key) {
        return ext.get<T>(key);
    }
    template <typename T> const T &fields(string key) const {
        return ext.get<T>(key);
    }
};

struct template_command {
    template_command() = default;

  public:
    struct return_value {
        enum class e_type {
            object = 0,
            extension_field
        };

        return_value()                     = default;
        return_value(const return_value &) = default;
        return_value(string _value, e_type _type) : value(_value), type(_type) {
        }

      public:
        string value;
        e_type type;
    };
    struct arg {
        enum class e_type {
            extglobal = 0,
            trgfield,
            string,
            internal,
            features
        };

        arg()            = default;
        arg(const arg &) = default;
        arg(string _value, e_type _type, string _prefix = "") : value(_value), type(_type), prefix(_prefix) {
        }

      public:
        string value;
        e_type type;

        string prefix;
    };

  public:
    static template_command create_template_command(string_v template_name, const string &template_str);

    // Creates a stack of templates for the correct sequential generation of commands(for every targets)
    static vec<template_command> create_queue_target_templates(const vec<template_command> &templates,
                                                               const vec<string> &templates_target,
                                                               target::e_type target_t);

  private:
    // Recursive function, for create_stack_target_templates
    static void recovery_queue_target_templates(vec<template_command> &vec_templates,
                                                vec<template_command> &queue_target_templates,
                                                const string &name_internal_param);

  public:
    string name;

    string name_call_component;
    return_value returnable;

    vec<string> name_accept_params;
    vec<arg> args;

  public:
    vec<string> ifiles;
    bool single_generates = 0;
    bool returns_target   = 0;
};

struct call_component {
    call_component() = default;
    call_component(string _name, string _name_program, string _pattern_ret_files)
        : name(_name), name_program(_name_program), pattern_ret_files(_pattern_ret_files) {
    }

  public:
    string name;
    string name_program;

    // file.txt
    string pattern_ret_files;
};

inline string target_type_str(target::e_type target_t) {
    if (target_t == target::e_type::lib)
        return "LIBRARY";
    else if (target_t == target::e_type::interpret)
        return "RUN-TIME";
    return "EXECUTABLE";
}
inline string target_cfg_str(target::e_cfg target_t) {
    if (target_t == target::e_cfg::debug)
        return "DEBUG";
    return "RELEASE";
}

inline target::e_type to_target_type(string target_t) {
    if (target_t == "LIBRARY")
        return target::e_type::lib;
    else if (target_t == "RUN-TIME")
        return target::e_type::interpret;
    return target::e_type::exe;
}
inline target::e_cfg to_target_cfg(string target_t) {
    if (target_t == "DEBUG")
        return target::e_cfg::debug;
    return target::e_cfg::release;
}

} // namespace structs_context

namespace sc = structs_context;
} // namespace bweas

#endif
