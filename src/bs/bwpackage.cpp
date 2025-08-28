//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include <bwpackage.hpp>

#include <nlohmann/json.hpp>
#include <utils/lz4.hpp>

using namespace bweas;
using namespace bweas::utils;

// The first bytes in the bweas package file are the signature
static const string PACKAGE_PREFIX_BYTE = "sbw";
// Size in bytes of bweas packet signature
static constexpr size_t PACKAGE_PREFIX_BYTE_LENGHT = 3;

/* The bweas version, which is indicated after the signature, for subsequent verification of the package for version
 * compliance*/
static const string PACKAGE_VERSION = {VERSION_MAJOR_C, '.', VERSION_MINOR_C};
// Bweas version length for package
static constexpr size_t PACKAGE_VERSION_BWEAS_VERSION_LENGHT = 3;
/* Length in bytes of the entire signature (including version) of the bweas package */
static constexpr size_t PACKAGE_START_BYTES_LENGHT = PACKAGE_PREFIX_BYTE_LENGHT + PACKAGE_VERSION_BWEAS_VERSION_LENGHT;

// Bytes separating the json configuration data and the lua script (indicates the end of the json configuration of the
// bweas package)
static constexpr auto PACKAGE_SEPARATE_JSON_BYTES = "ejc";
// Source code separator bytes of cache
static constexpr auto PACKAGE_SEPARATE_LUA_CACHE = "elc";
// Source code separator bytes of generators
static constexpr auto PACKAGE_SEPARATE_LUA_DEPENDENCY_FINDER = "elf";

// Length of separating bytes json configuration of the bweas package
static constexpr size_t PACKAGE_SEPARATE_JSON_BYTES_LENGHT = 3;
// Length of separating bytes between cache source codes
static constexpr size_t PACKAGE_SEPARATE_LUA_CACHE_LENGHT = 3;
// Length of separating bytes between generator source codes
static constexpr size_t PACKAGE_SEPARATE_LUA_DEPENDENCY_FINDER_LENGHT = 3;

// Maximum bweas packet size(5 mb)
static constexpr size_t MAX_SIZE_PACKAGE = 5 MB;

static logger _log{"BWPACKAGE"};

string package::create_data_package(data_bw_package _data) {
    string data_str = PACKAGE_PREFIX_BYTE + PACKAGE_VERSION + _data.json_config + PACKAGE_SEPARATE_JSON_BYTES +
                      _data.src_lua_cache + PACKAGE_SEPARATE_LUA_CACHE;

    for (const auto &src_lua_generator : _data.src_lua_finders)
        data_str += src_lua_generator + PACKAGE_SEPARATE_LUA_DEPENDENCY_FINDER;

    return lz4::compress_data(data_str);
}

string package::init(data_bw_package _data, bool is_create_pckg) {
    nlohmann::json config_json = nlohmann::json::parse(_data.json_config);
    if (!config_json.contains("package-name") || (name = config_json["package-name"]) == "")
        _log << (log_message(log_type::fatal) << "Bweas package name field is empty");
    else if (!config_json.contains("bweas-version") ||
             ((bw_version = sc::version(config_json["bweas-version"])) == "0.0.0"))
        _log << (log_message(log_type::fatal) << "Build system version field is empty");

    if (config_json.contains("cache")) {
        nlohmann::json metainf_ch = config_json["cache"];
        if (!metainf_ch.is_structured())
            _log << (log_message(log_type::fatal) << "The \"cache\" field must be a structure");

        else if (!metainf_ch.contains("name") || !metainf_ch["name"].is_string())
            _log << (log_message(log_type::fatal) << "Cache metadata must include its name");
        else if (!metainf_ch.contains("lua-file") || !metainf_ch["lua-file"].is_string())
            _log << (log_message(log_type::fatal) << "Cache metadata must include the path to the lua source file");

        cfg.cache.name = metainf_ch["name"];

        if (is_create_pckg) {
            cfg.cache.src_lua =
                file_utils::read_file(file_utils::get_ref_file(file_utils::open_file((string)metainf_ch["lua-file"])));
            _data.src_lua_cache = cfg.cache.src_lua;
        }
        else
            cfg.cache.src_lua = _data.src_lua_cache;
    }

    // reading lua script file
    if (config_json.contains("dependency-finder")) {
        if (!config_json["dependency-finder"].is_structured())
            _log << (log_message(log_type::fatal) << "The \"dependency-finder\" field must be of type json structure");

        size_t i = 0;

        for (const auto &finder : config_json["dependency-finder"].items()) {
            nlohmann::json metainf_fn = finder.value();

            if (!metainf_fn.is_object())
                _log << (log_message(log_type::fatal)
                         << "The dependency-finder meta information unit must be a json object");
            else if (!metainf_fn.contains("lua-file") || !metainf_fn["lua-file"].is_string())
                _log << (log_message(log_type::fatal) << "dependency-finder metadata must include the path to the lua "
                                                         "(finder) source code file");

            auto lua_file_finder_source = file_utils::open_file((string)metainf_fn["lua-file"]);
            if (is_create_pckg) {
                cfg.finders.emplace_back(finder.key(),
                                         file_utils::read_file(file_utils::get_ref_file(lua_file_finder_source)));
                _data.src_lua_finders.push_back(cfg.finders[cfg.finders.size() - 1].src_lua);
            }
            else {
                cfg.finders.emplace_back(finder.key(), _data.src_lua_finders[i]);
                ++i;
            }
        }
    }
    if (config_json.contains("modules")) {
        if (!config_json["modules"].is_structured())
            _log << (log_message(log_type::fatal) << "The \"modules\" field must be of type json structure");

        for (const auto &_module : config_json["modules"].items()) {
            nlohmann::json metainf_md = _module.value();

            umap<string, sc::profile> profiles;
            uset<string> no_export_profiles;
            if (metainf_md.contains("profiles")) {
                if (!metainf_md["profiles"].is_structured())
                    _log << (log_message(log_type::fatal) << "The \"profiles\" field must be a structure");

                for (const auto &profile : metainf_md["profiles"].items()) {
                    nlohmann::json profile_info = profile.value();

                    sc::profile profile_tmp;

                    if (!profile_info.is_object())
                        _log << (log_message(log_type::fatal)
                                 << "The profile[" << profile.key() << "] meta information unit must be a json object");

                    static auto get_field = [&](const nlohmann::json &value) -> sc::profile::fields::mapped_type {
                        if (value.is_array()) {
                            return static_cast<vec<string>>(value);
                        }
                        else if (value.is_string())
                            return static_cast<string>(value);
                        else
                            _log << (log_message(log_type::fatal)
                                     << "Profile[" << profile.key()
                                     << "] fields must be of type string or array: " << value);

                        std::unreachable();
                    };

                    for (const auto &[key, value] : profile_info.items()) {
                        if (value.is_object()) {
                            if (key == "RELEASE")
                                for (const auto &[release_key, release_value] : value.items())
                                    profile_tmp.release_fields[release_key] = get_field(release_value);
                            else if (key == "DEBUG") {
                                profile_tmp.debug_fields = sc::profile::fields{};
                                for (const auto &[debug_key, debug_value] : value.items())
                                    profile_tmp.debug_fields.value()[debug_key] = get_field(debug_value);
                            }
                            else
                                _log << (log_message(log_type::fatal)
                                         << "A profile[" << profile.key()
                                         << "] can only have RELEASE, DEBUG and global fields.");
                        }
                        else {
                            if (key == "derive") {
                                sc::profile::fields::mapped_type variant_value = get_field(value);
                                vec<string> derive_profiles = std::holds_alternative<string>(variant_value)
                                                                  ? vec<string>{std::get<string>(variant_value)}
                                                                  : std::get<vec<string>>(variant_value);
                                for (const auto &derive_profile : derive_profiles)
                                    if (profiles.contains(derive_profile)) {
                                        for (const auto &it : profiles[derive_profile].global_fields)
                                            profile_tmp.global_fields.insert_or_assign(it.first, it.second);
                                        for (const auto &it : profiles[derive_profile].release_fields)
                                            profile_tmp.release_fields.insert_or_assign(it.first, it.second);
                                        if (profile_tmp.debug_fields)
                                            for (const auto &it : *profiles[derive_profile].debug_fields)
                                                profile_tmp.debug_fields->insert_or_assign(it.first, it.second);
                                        else
                                            profile_tmp.debug_fields = profiles[derive_profile].debug_fields;
                                    }
                            }
                            else if (key == "export" && value.is_boolean()) {
                                if (!static_cast<bool>(value))
                                    no_export_profiles.insert(profile.key());
                            }
                            else
                                profile_tmp.global_fields[key] = get_field(value);
                        }
                    }
                    if (!profile_tmp.global_fields.contains(sc::profile::FIELD_SOURCE_FILES) ||
                        !std::holds_alternative<vec<string>>(
                            profile_tmp.global_fields[sc::profile::FIELD_SOURCE_FILES]))
                        _log << (log_message(log_type::fatal) << "The profile[" << profile.key()
                                                              << "] must have source files field of type string.");
                    else if (!profile_tmp.global_fields.contains(sc::profile::FIELD_INCLUDE_PATHS) ||
                             !std::holds_alternative<vec<string>>(
                                 profile_tmp.global_fields[sc::profile::FIELD_INCLUDE_PATHS]))
                        _log << (log_message(log_type::fatal) << "The profile[" << profile.key()
                                                              << "] must have include paths field of type string.");
                    else if (!profile_tmp.global_fields.contains(sc::profile::FIELD_TARGET_TYPE) ||
                             !std::holds_alternative<string>(profile_tmp.global_fields[sc::profile::FIELD_TARGET_TYPE]))
                        _log << (log_message(log_type::fatal) << "The profile[" << profile.key()
                                                              << "] must have type of target field of type string.");
                    else if (!profile_tmp.global_fields.contains(sc::profile::FIELD_LANGUAGE) ||
                             !std::holds_alternative<string>(profile_tmp.global_fields[sc::profile::FIELD_LANGUAGE]))
                        _log << (log_message(log_type::fatal)
                                 << "The profile[" << profile.key() << "] must have language field of type string.");

                    for (const auto &it : profile_tmp.global_fields)
                        profile_tmp.release_fields.insert_or_assign(it.first, it.second);

                    if (profile_tmp.debug_fields)
                        for (const auto &it : *profile_tmp.debug_fields)
                            profile_tmp.debug_fields->insert_or_assign(it.first, it.second);

                    profiles[profile.key()] = std::move(profile_tmp);
                }
            }

            umap<string, decl_func> funcs;
            string lua_file;
            if (metainf_md.contains("functions")) {
                if (!metainf_md.contains("lua-file") || !metainf_md["lua-file"].is_string())
                    _log << (log_message(log_type::fatal)
                             << "Module metadata must include the name of the lua source file");

                lua_file = metainf_md["lua-file"];
                for (const auto &func : metainf_md["functions"].items()) {
                    auto it_func = func.value();
                    decl_func def_func_tmp;
                    def_func_tmp.name = func.key();
                    for (const auto &field : it_func.items()) {
                        if (field.key() == "accepted") {
                            if (!field.value().is_array())
                                _log << (log_message(log_type::fatal)
                                         << "The field for listing the types of function parameters must be an array");
                            for (size_t i = 0; i < field.value().size(); ++i) {
                                if (field.value()[i].is_string())
                                    def_func_tmp.expected_params.push_back(
                                        param{get_string_param_type((string)field.value()[i])});
                                else
                                    def_func_tmp.expected_params.push_back(param{field.value()[i]});
                            }
                        }
                    }
                    funcs[func.key()] = def_func_tmp;
                }
            }

            for (const auto &name_profile : no_export_profiles)
                profiles.extract(name_profile);

            cfg.modules.emplace_back(_module.key(), lua_file, std::move(funcs), std::move(profiles));
        }
    }

    if (_log.error_status())
        return "";
    return create_data_package(_data);
}

void package::load(string_v raw_data_package) {
    string data_pckg = lz4::decompress_data(raw_data_package, MAX_SIZE_PACKAGE);
    if (data_pckg.size() == 0)
        _log << (log_message(log_type::fatal) << "Unsuccessful decompression of package bweas");

    if (data_pckg.find(PACKAGE_PREFIX_BYTE) == data_pckg.npos &&
        data_pckg.find(PACKAGE_SEPARATE_JSON_BYTES) == data_pckg.npos)
        _log << (log_message(log_type::fatal) << "Incorrect bweas package structure"
                                              << "[Package: " << data_pckg.size() << " bytes]");

    string prefix_package = data_pckg;
    prefix_package.erase(0, PACKAGE_PREFIX_BYTE_LENGHT);
    prefix_package.erase(PACKAGE_VERSION_BWEAS_VERSION_LENGHT);

    if (sc::version(prefix_package) < sc::version(PACKAGE_PREFIX_BYTE))
        _log << (log_message(log_type::fatal)
                 << "The package of this version is not supported by the build system"
                 << "[Package: " << data_pckg.size() << " bytes] Ver pckg: " << prefix_package);
    data_pckg.erase(0, PACKAGE_START_BYTES_LENGHT);

    data_bw_package data_package;
    data_package.json_config = data_pckg;
    data_package.json_config.erase(data_package.json_config.find(PACKAGE_SEPARATE_JSON_BYTES));

    data_package.src_lua_cache = data_pckg;
    data_package.src_lua_cache.erase(0, data_package.json_config.size() + PACKAGE_SEPARATE_JSON_BYTES_LENGHT);
    data_package.src_lua_cache.erase(data_package.src_lua_cache.find(PACKAGE_SEPARATE_LUA_CACHE));

    string src_finders = data_pckg;
    string current_src_finder;
    src_finders.erase(0, src_finders.find(PACKAGE_SEPARATE_LUA_CACHE) + PACKAGE_SEPARATE_LUA_CACHE_LENGHT);
    while (src_finders.size()) {
        current_src_finder = src_finders;
        current_src_finder.erase(current_src_finder.find(PACKAGE_SEPARATE_LUA_DEPENDENCY_FINDER));
        data_package.src_lua_finders.push_back(current_src_finder);

        src_finders.erase(0, current_src_finder.size() + PACKAGE_SEPARATE_LUA_DEPENDENCY_FINDER_LENGHT);
    }

    init(data_package);
}

bool package::is_init() {
    return name.size();
}
