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

/** \brief Structures describing bweas concepts. */
namespace structs_context {

struct target;
struct template_command;

/** \brief Structure for naming versions in a style MinorMajorPatch. */
struct version final {
    /** \brief Constructor.
     * \param [in] _major Older part of the version.
     * \param [in] _minor Middle part of the version.
     * \param [in] _patch Junior part of the version.
     */
    version(size_t _major = 0, size_t _minor = 0, size_t _patch = 0) : major{_major}, minor{_minor}, patch{_patch} {
    }
    /** \brief Constructor.
     * \param [in] version_str The version presented in lowercase.
     */
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

  public:
    /** \brief Returns the version as a string.
     * \return string
     */
    string get_str_version() const {
        return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
    }

    /** \brief Checks for equality of the two versions.
     * \param [in] ver2 The version that the current version will be compared with.
     * \return bool
     */
    bool operator==(const version &ver2) {
        if (major == ver2.major && minor == ver2.minor && patch == ver2.patch)
            return 1;
        return 0;
    }
    /** \brief Checks whether the current version is younger than the transmitted one.
     * \param [in] ver2 The version that the source version will be compared with.
     * \return bool
     */
    bool operator<(const version &ver2) {
        if (major < ver2.major || (major <= ver2.major && minor < ver2.minor) ||
            (major <= ver2.major && minor <= ver2.minor && patch < ver2.patch))
            return 1;
        return 0;
    }

  private:
    size_t major = 0, minor = 0, patch = 0;
};

/** \brief The structure defining the language object. */
struct language final {
    /** \brief The structure defining the dependency finder. */
    struct dependency_finder final {
        /** \brief Constructor. */
        dependency_finder() = default;
        /** \brief Constructor.
         * \param [in] _search_regex A regular expression that will be used to search for strings that define
         * dependencies.
         * \param [in] _char_global_search A symbol indicating that the found dependency should be searched for using
         * global paths.
         */
        dependency_finder(string_v _search_regex, char _char_global_search)
            : search_regex(_search_regex), char_global_search(_char_global_search) {
        }

      public:
        string search_regex;     ///< A regular expression for searching strings defining dependencies.
        char char_global_search; ///< The symbol that defines the search for global paths.
    };

  public:
    /** \brief Constructor. */
    language() = default;
    /** \brief Constructor.
     * \param [in] _name The name of the language object.
     * \param [in] _dfinder_data The corresponding dependency finder.
     */
    language(string _name, dependency_finder &&_dfinder_data = {}) : name(_name), dfinder_data(_dfinder_data) {
    }

  public:
    string name;
    dependency_finder dfinder_data;
};

/** \brief A structure that is an extension for target objects. */
struct profile final {
    /** \brief The type of the fields container. */
    using fields = umap<string, std::variant<pdiff, string, vec<string>>>;

    /** \brief Constructor. */
    profile() = default;
    /** \brief Constructor.
     * \param [in] _lang The corresponding language object.
     * \param [in] _release_fields Fields corresponding to the release configuration.
     * \param [in] _debug_fields Fields corresponding to the debug configuration.
     */
    profile(language _lang, fields _release_fields, std::optional<fields> _debug_fields = std::nullopt)
        : lang(_lang), release_fields(_release_fields), debug_fields(_debug_fields) {
    }

  public:
    /** \brief Combines the current extension with the transmitted one.
     * \param [in] prf The extension that the merge will be played with.
     */
    void merge(const profile &prf) {
        cfg  = prf.cfg;
        lang = prf.lang;
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
    /** \brief Sets up the extension configuration.
     * \param [in] _cfg If it is greater than 0, it is a debug configuration, otherwise it is a release.
     */
    void set_fields(size_t _cfg) {
        cfg = _cfg;
    }
    /** @{ \name Returns the container of the fields of the current configuration. */
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
    /** @} */

  public:
    /** @{ \name Returns a reference to the value of the corresponding field.
     * \warning The field must exist.
     * \param [in] key The name of the field.
     */
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
    /** @} */

    /** @{ \name Returns a reference to the value of the corresponding field that has already been specified.
     * \warning The field must exist.
     * \param [in] key The name of the field.
     * \return The reduced value is of type T.
     */
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
    /** @} */

    /** \brief Checks the existence of the field.
     * \param [in] key The name of the field.
     * \return bool
     */
    bool contains(string_v key) const {
        return get_fields().contains(key.data());
    }

    /** @{ \name If the field exists, returns the field value reduced to T, otherwise nullptr.
     * \param [in] key The name of the field.
     * \return A pointer to the converted value to type T.
     */
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
    /** @} */

  public:
    size_t cfg = 0; ///< Defines the configuration of the extension.
    language lang;  ///< The corresponding language object.

    fields release_fields;              ///< A container of fields corresponding to the release configuration.
    std::optional<fields> debug_fields; ///< A container of fields corresponding to the debug configuration.

  public:
    static constexpr auto FIELD_SOURCE_FILES  = "source_files";  ///< A required field containing the source files.
    static constexpr auto FIELD_INCLUDE_PATHS = "include_paths"; ///< A required field containing global paths.
};
/** \brief Target structure for build system. */
struct target final {
    /** \brief Constructor. */
    target() = default;

  public:
    /** @{ \name Returns the specified value of the passed field.
     * \param key The name of the field.
     */
    template <typename T> T &fields(string key) {
        return ext.get<T>(key);
    }
    template <typename T> const T &fields(string key) const {
        return ext.get<T>(key);
    }
    /** @} */

  public:
    string name;

    profile ext; ///< Extension for the target object.
    version ver; ///< The version of the target object.

    vec<string> templates;    ///< An array of names command templates that will assemble the target object.
    vec<string> dependencies; ///< An array of names of the target objects on which this target object depends.

  public:
    vec<template_command> queue_templates; ///< A sorted array of templates.
    bool built_success = 0;                ///< Determines the success of building the target object.
};

/** \brief A command template that defines the rules for building a command line. */
struct template_command final {
    /** \brief Constructor. */
    template_command() = default;

  public:
    /** \brief The return value of the template. */
    struct return_value {
        /** \brief The type of the returned value. */
        enum class e_type {
            object = 0,
            extension_field
        };

        /** \brief Constructor. */
        return_value() = default;
        /** \brief Constructor.
         * \param [in] _value The return value.
         * \param [in] _type The type of the returned value.
         */
        return_value(string _value, e_type _type) : value(_value), type(_type) {
        }

      public:
        string value; ///< The return value.
        e_type type;  ///< The type of the returned value.

      public:
        static constexpr auto RETURN_VALUE_TARGET = "target";
    };
    /** \brief The argument of the template command. */
    struct arg {
        /** The type of the template command. */
        enum class e_type {
            trgfield = 0,
            internal,
            string
        };

        /** \brief Constructor. */
        arg() = default;
        /** \brief Constructor.
         * \param [in] _value The argument value.
         * \param [in] _type The type of the argument.
         * \param [in] _prefix The prefix of the argument.
         */
        arg(string _value, e_type _type, string _prefix = "") : value(_value), type(_type), prefix(_prefix) {
        }

      public:
        string value; ///< The argument value.
        e_type type;  ///< The type of the argument.

        string prefix; ///< The prefix that is added to the argument or to each of its elements when opening it.
    };

  public:
    /** \brief Constructs a command template.
     * \detail Constructs a command template based on the string describing the template.
     * \param [in] template_name The name of the template.
     * \param [in] template_str The string that the template will be based on.
     * \return bweas::template_command
     *
     * \example smth_program:{name_target}() -> target: -smth_prefix[smth_array:1] -o OUTPUT_FILE
     *
     */
    static template_command create_template_command(string_v template_name, const string &template_str);

    /** \brief Creates a stack of templates.
     * \detail Creates a stack of templates for the correct sequential generation of commands(for every targets).
     * \param [in] templates An array of all existing templates.
     * \param [in] templates_target An array of template names used by the target object.
     * \return vec<bweas::template_command>
     */
    static vec<template_command> create_queue_target_templates(const vec<template_command> &templates,
                                                               const vec<string> &templates_target);

  private:
    static void recovery_queue_target_templates(vec<template_command> &vec_templates,
                                                vec<template_command> &queue_target_templates,
                                                const string &name_internal_param);

  public:
    string name;

    string name_call_component; ///< Name of the calling component.
    return_value returnable;    ///< The returning value.

    vec<string> name_accept_params; ///< An array of values that the template accepts.
    vec<arg> args;                  ///< An array of arguments that make up the command.

  public:
    vec<string> ifiles;        ///< The source file of the current template.
    bool single_generates = 0; ///< If it is true, ifiles.size() commands will be generated for this template.
    bool returns_target   = 0; ///< If it is true, then this template is the target one - it returns the target.

  public:
    /** @{ \name Features
     * \detail Features are lowercase template arguments that the command generator will process separately.*/
    static constexpr auto FEATURE_INPUT_FILE   = "INPUT_FILE";
    static constexpr auto FEATURE_OUTPUT_FILE  = "OUTPUT_FILE";
    static constexpr auto FEATURE_DEPENDENCIES = "DEPENDENCIES";
    /** @} */
};

/** \brief The call component describes the program being run. */
struct call_component {
    /** \brief Constructor. */
    call_component() = default;
    /** \brief Constructor.
     * \param [in] _name The name of the call component.
     * \param [in] _name The name of the existing program.
     * \param [in] _pattern_ret_files The pattern of the output file.
     */
    call_component(string _name, string _name_program, string _pattern_ret_files)
        : name(_name), name_program(_name_program), pattern_ret_files(_pattern_ret_files) {
    }

  public:
    string name;
    string name_program;
    string pattern_ret_files;
};
} // namespace structs_context

namespace sc = structs_context;
} // namespace bweas

#endif
