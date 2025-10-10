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

        for (const auto &_module : config_json["modules"].items()) {
            nlohmann::json metainf_md = _module.value();

            string name_src_file;
            if (metainf_md.contains("name-src-file")) {
                if (!metainf_md["name-src-file"].is_string())
                    _log << (log_message(log_type::fatal)
                             << "Module metadata must include the name of the source file of module");

                name_src_file =
                    (file_utils::get_path_program() / "packages" / static_cast<string>(metainf_md["name-src-file"]))
                        .string();
            }

            cfg.modules.emplace_back(_module.key(), name_src_file);
        }
    }
}
