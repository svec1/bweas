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

// enum of str postfix name var a target
inline constexpr auto TRG_VAR_NAME              = "_NAME";
inline constexpr auto TRG_VAR_NAME_VER          = "_VERSION";
inline constexpr auto TRG_VAR_NAME_TEMPLATES    = "_TEMPLATES";
inline constexpr auto TRG_VAR_NAME_DEPENDENCIES = "_DEPENDENCIES";

// name of additional fields, which are also part of structures,
// but which cannot be changed by the user
inline constexpr auto TRG_NAME_FIELD_EXTENSION = "_EXTENSION";
inline constexpr auto TRG_NAME_FIELD_NTARGET   = "_NAME";

inline constexpr auto NAME_FIELD_TEMPLATE_COMMAND_NAME              = "_NAME";
inline constexpr auto NAME_FIELD_TEMPLATE_COMMAND_NAME_CCMP         = "_NCALL_C";
inline constexpr auto NAME_FIELD_TEMPLATE_COMMAND_NAME_ACCEPTS_ARGS = "_ACP_ARGS";
inline constexpr auto NAME_FIELD_TEMPLATE_COMMAND_RET               = "_RETURN";
inline constexpr auto NAME_FIELD_TEMPLATE_COMMAND_NAME_ARGS         = "_ARGS";

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
    size_t major = 0, minor = 0, patch = 0;
};

struct language {
    struct dependency_finder {
        dependency_finder() = default;
        dependency_finder(string_v _search_regex, char _char_global_search)
            : search_regex(_search_regex), char_global_search(_char_global_search) {
        }

      public:
        string search_regex;
        char char_global_search;
    };

  public:
    language() = default;
    language(string _name, dependency_finder &&_dfinder_data = {}) : name(_name), dfinder_data(_dfinder_data) {
    }

  public:
    string name;
    dependency_finder dfinder_data;
};

struct profile {
    using fields = umap<string, std::variant<pdiff, string, vec<string>>>;

    profile() = default;
    profile(language _lang, fields _release_fields, std::optional<fields> _debug_fields = std::nullopt)
        : lang(_lang), release_fields(_release_fields), debug_fields(_debug_fields) {
    }

  public:
    void merge(const profile &prf) {
        cfg  = prf.cfg;
        lang = prf.lang;
        for (const auto &[key, value] : prf.global_fields)
            if (!global_fields.contains(key))
                global_fields[key] = value;
        for (const auto &[key, value] : prf.release_fields)
            if (!release_fields.contains(key))
                release_fields[key] = value;

        if (prf.debug_fields) {
            if (!debug_fields)
                debug_fields = prf.debug_fields;
            else
                for (const auto &[key, value] : *prf.debug_fields)
                    if (!debug_fields->contains(key))
                        (*debug_fields)[key] = value;
        }
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
              typename = std::enable_if<
                  std::is_same_v<T, pdiff> || std::is_same_v<T, string> || std::is_same_v<T, vec<string>>, void>::type>
    T &get(string_v key) {
        return std::get<T>(this->operator[](key.data()));
    }
    template <typename T,
              typename = std::enable_if<
                  std::is_same_v<T, pdiff> || std::is_same_v<T, string> || std::is_same_v<T, vec<string>>, void>::type>
    const T &get(string_v key) const {
        return std::get<T>(this->operator[](key.data()));
    }

    bool contains(string_v key) const {
        return get_fields().contains(key.data());
    }

    template <typename T> T *get_if(string_v key) {
        if (contains(key) && std::holds_alternative<T>(this->operator[](key.data())))
            return &get<T>(key);
        return nullptr;
    }
    template <typename T> const T *get_if(string_v key) const {
        if (contains(key) && std::holds_alternative<T>(this->operator[](key.data())))
            return &get<T>(key);
        return nullptr;
    }

  public:
    size_t cfg = 0;
    language lang;

    fields release_fields;
    std::optional<fields> debug_fields;

  public:
    fields global_fields; // in package impl

  public:
    static constexpr auto FIELD_SOURCE_FILES  = "source_files";
    static constexpr auto FIELD_INCLUDE_PATHS = "include_paths";
};
// target structure for build system
// ---------------------------------
struct target {
    target() = default;

  public:
    profile ext;

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

      public:
        static constexpr auto RETURN_VALUE_TARGET = "target";
    };
    struct arg {
        enum class e_type {
            trgfield = 0,
            internal,
            string
        };

        arg() = default;
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
                                                               const vec<string> &templates_target);

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

  public:
    static constexpr auto FEATURE_INPUT_FILE   = "INPUT_FILE";
    static constexpr auto FEATURE_OUTPUT_FILE  = "OUTPUT_FILE";
    static constexpr auto FEATURE_DEPENDENCIES = "DEPENDENCIES";
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
} // namespace structs_context

namespace sc = structs_context;
} // namespace bweas

#endif
