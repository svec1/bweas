//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include <bwpackage.hpp>

#include <nlohmann/json.hpp>

using namespace bweas;
using namespace bweas::utils;

static logger _log{"BWPACKAGE"};

package::package(string_v json_config) {
    nlohmann::json config_json = nlohmann::json::parse(json_config);
    if (!config_json.contains("package-name") || (name = config_json["package-name"]) == "")
        _log << (log_message(log_type::fatal) << "Bweas package name field is empty");
    else if (!config_json.contains("bweas-version") || !config_json["bweas-version"].is_string())
        _log << (log_message(log_type::fatal) << "Build system version field is empty");

    if (sc::version(config_json["bweas-version"]) < sc::version(VERSION_FULL_STR))
        _log << (log_message(log_type::fatal) << "The Bweas package '" +
                                                     static_cast<string>(config_json.at("bweas-version")) +
                                                     "' is not supported");

    if (config_json.contains("cache")) {
        nlohmann::json metainf_ch = config_json["cache"];
        if (!metainf_ch.is_structured())
            _log << (log_message(log_type::fatal) << "The \"cache\" field must be a structure");

        else if (!metainf_ch.contains("name") || !metainf_ch["name"].is_string())
            _log << (log_message(log_type::fatal) << "Cache metadata must include its name");
        else if (!metainf_ch.contains("lua-file") || !metainf_ch["lua-file"].is_string())
            _log << (log_message(log_type::fatal) << "Cache metadata must include the path to the lua source file");

        cfg.cache.name = metainf_ch["name"];

        cfg.cache.src_lua = file_utils::read_file(file_utils::open_file((string)metainf_ch["lua-file"]));
    }

    // reading lua script file

    if (config_json.contains("modules")) {
        if (!config_json["modules"].is_structured())
            _log << (log_message(log_type::fatal) << "The \"modules\" field must be of type json structure");

        umap<string, sc::language> languages;
        for (const auto &_module : config_json["modules"].items()) {
            nlohmann::json metainf_md = _module.value();

            umap<string, sc::profile> profiles;
            uset<string> no_export_profiles;

            if (metainf_md.contains("languages")) {
                if (!metainf_md["languages"].is_structured())
                    _log << (log_message(log_type::fatal)
                             << "The \"dependency-finder\" field must be of type json structure");

                for (const auto &finder : metainf_md["languages"].items()) {
                    nlohmann::json metainf_fn = finder.value();

                    if (!metainf_fn.is_object())
                        _log << (log_message(log_type::fatal)
                                 << "The dependency-finder meta information unit must be a json object");

                    languages[finder.key()] = finder.key();

                    if (metainf_fn.contains("search_regex")) {
                        if (!metainf_fn.at("search_regex").is_string())
                            _log << (log_message(log_type::fatal) << "The search_regex field must be a string.");

                        languages[finder.key()].dfinder_data.search_regex = metainf_fn.at("search_regex");
                    }
                    if (metainf_fn.contains("char_global_search")) {
                        if (!metainf_fn.at("char_global_search").is_string())
                            _log << (log_message(log_type::fatal) << "The search_regex field must be a string.");

                        languages[finder.key()].dfinder_data.char_global_search =
                            static_cast<string>(metainf_fn.at("char_global_search"))[0];
                    }
                }
            }
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
                            return value.get<vec<string>>();
                        }
                        else if (value.is_string())
                            return value.get<string>();
                        else if (value.is_number())
                            return value.get<pdiff>();
                        else
                            _log << (log_message(log_type::fatal)
                                     << "Profile[" << profile.key()
                                     << "] fields must be of type number, string or array: " << value);

                        std::unreachable();
                    };

                    for (const auto &[key, value] : profile_info.items()) {
                        if (value.is_object()) {
                            if (key == "release")
                                for (const auto &[release_key, release_value] : value.items())
                                    profile_tmp.release_fields[release_key] = get_field(release_value);
                            else if (key == "debug") {
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
                                    else
                                        _log << (log_message(log_type::fatal)
                                                 << "It is impossible to inherit from a non-existent profile[" +
                                                        derive_profile + "].");
                            }
                            else if (key == "export" && value.is_boolean()) {
                                if (!static_cast<bool>(value))
                                    no_export_profiles.insert(profile.key());
                            }
                            else
                                profile_tmp.global_fields[key] = get_field(value);
                        }
                    }

                    if (profile_tmp.global_fields.contains("language") &&
                        std::holds_alternative<string>(profile_tmp.global_fields["language"])) {
                        auto &name_lang = std::get<string>(profile_tmp.global_fields.at("language"));
                        if (!languages.contains(name_lang))
                            _log << (log_message(log_type::fatal) << "Undefined language '" + name_lang + "'.");
                        profile_tmp.lang = languages.at(name_lang);
                    }

                    for (const auto &it : profile_tmp.global_fields)
                        profile_tmp.release_fields.insert_or_assign(it.first, it.second);

                    if (profile_tmp.debug_fields)
                        for (const auto &it : *profile_tmp.debug_fields)
                            profile_tmp.debug_fields->insert_or_assign(it.first, it.second);

                    profiles[profile.key()] = std::move(profile_tmp);
                }
            }

            string name_src_file;
            if (metainf_md.contains("name-src-file")) {
                if (!metainf_md["name-src-file"].is_string())
                    _log << (log_message(log_type::fatal)
                             << "Module metadata must include the name of the source file of module");

                name_src_file =
                    file_utils::get_path_program() + "/packages/" + static_cast<string>(metainf_md["name-src-file"]);
            }

            for (const auto &name_profile : no_export_profiles)
                profiles.extract(name_profile);

            cfg.modules.emplace_back(_module.key(), name_src_file, profiles);
        }
    }
}

bool package::is_init() {
    return name.size();
}
