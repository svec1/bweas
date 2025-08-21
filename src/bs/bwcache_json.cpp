//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include <bwcache_api.hpp>

#include <nlohmann/json.hpp>

namespace nlohmann {
template <> struct adl_serializer<bweas::sc::profile::fields::mapped_type> {
    static void to_json(json &j, const bweas::sc::profile::fields::mapped_type &value) {
        std::visit([&](auto &&_value) { j = std::forward<decltype(_value)>(_value); }, value);
    }
    static void from_json(const json &j, bweas::sc::profile::fields::mapped_type &value) {
        if (j.is_string())
            value = static_cast<string>(j);
        else if (j.is_array())
            value = static_cast<vec<string>>(j);
        else
            assert("Invalid type for convert to std::variant");
    }
};
template <> struct adl_serializer<bweas::sc::profile> {
    static void to_json(json &j, const bweas::sc::profile &value) {
        j = value.get_fields();
    }
    static void from_json(const json &j, bweas::sc::profile &value) {
        value.get_fields() = j;
    }
};
template <> struct adl_serializer<bweas::sc::target> {
    static void to_json(json &j, const bweas::sc::target &value) {
        j = {{"type", bweas::sc::target_type_str(value.type)},
             {"cfg", bweas::sc::target_cfg_str(value.cfg)},
             {"version", value.ver.get_str_version()},
             {"templates", value.templates},
             {"dependencies", value.dependencies},
             {"extension", value.ext}};
    }
    static void from_json(const json &j, bweas::sc::target &value) {
        try {
            value.type = bweas::sc::to_target_type(j["type"]);
            value.cfg  = bweas::sc::to_target_cfg(j["cfg"]);
            value.ver  = j["version"].template get<string>();

            j.at("templates").get_to(value.templates);
            j.at("dependencies").get_to(value.dependencies);
            j.at("extension").get_to(value.ext);
        }
        catch (const json::exception &) {
            throw std::runtime_error(j.dump(4));
        }
    }
};
template <> struct adl_serializer<bweas::sc::template_command::return_value> {
    static void to_json(json &j, const bweas::sc::template_command::return_value &value) {
        j["value"] = value.value;
        j["type"]  = (pdiff)value.type;
    }
    static void from_json(const json &j, bweas::sc::template_command::return_value &value) {
        try {
            j.at("value").get_to(value.value);
            j.at("type").get_to(value.type);
        }
        catch (const json::exception &) {
            throw std::runtime_error(j.dump(4));
        }
    }
};
template <> struct adl_serializer<bweas::sc::template_command::arg> {
    static void to_json(json &j, const bweas::sc::template_command::arg &value) {
        j["prefix"] = value.prefix;
        j["value"]  = value.value;
        j["type"]   = (pdiff)value.type;
    }
    static void from_json(const json &j, bweas::sc::template_command::arg &value) {
        try {
            j.at("prefix").get_to(value.prefix);
            j.at("value").get_to(value.value);
            j.at("type").get_to(value.type);
        }
        catch (const json::exception &) {
            throw std::runtime_error(j.dump(4));
        }
    }
};
template <> struct adl_serializer<bweas::sc::template_command> {
    static void to_json(json &j, const bweas::sc::template_command &value) {
        j = {{"name_call_component", value.name_call_component},
             {"returnable", value.returnable},
             {"accept_params", value.name_accept_params},
             {"args", value.args}};
    }
    static void from_json(const json &j, bweas::sc::template_command &value) {
        try {
            j.at("name_call_component").get_to(value.name_call_component);
            j.at("accept_params").get_to(value.name_accept_params);
            value.returnable = j["returnable"].template get<bweas::sc::template_command::return_value>();
            value.args       = j["args"].template get<vec<bweas::sc::template_command::arg>>();
        }
        catch (const json::exception &) {
            throw std::runtime_error(j.dump(4));
        }
    }
};
template <> struct adl_serializer<bweas::sc::call_component> {
    static void to_json(json &j, const bweas::sc::call_component &value) {
        j = {{"name_program", value.name_program}, {"pattern_ret_files", value.pattern_ret_files}};
    }
    static void from_json(const json &j, bweas::sc::call_component &value) {
        try {
            j.at("name_program").get_to(value.name_program);
            j.at("pattern_ret_files").get_to(value.pattern_ret_files);
        }
        catch (const json::exception &) {
            throw std::runtime_error(j.dump(4));
        }
    }
};
} // namespace nlohmann

using namespace bweas;
using namespace cache_api;

static logger _log{"BWCACHE[JSON]"};

string json_cache::create_cache() {
    nlohmann::json cache_data;

    cache_data["config_file"]          = _context->path_bweas_config;
    cache_data["global_external_args"] = _context->global_external_args;

    for (const auto &target : _context->targets)
        cache_data["targets"][target.name] = target;
    for (const auto &_template : _context->templates)
        cache_data["templates"][_template.name] = _template;
    for (const auto &call_component : _context->call_components)
        cache_data["call_components"][call_component.name] = call_component;

    return cache_data.dump(4);
}
string json_cache::get_path_config(const string &cache_str) {
    try {
        nlohmann::json cache_data = nlohmann::json::parse(cache_str);
        return cache_data["config_file"];
    }
    catch (std::exception &what) {
        _log << (log_message(log_type::fatal) << "Invalid structure of the bweas cache file: \n" << what.what());
    }

    std::unreachable();
}

void json_cache::extract_cache_data(const string &cache_str) {
    try {
        nlohmann::json cache_data = nlohmann::json::parse(cache_str);

        _context->path_bweas_config = cache_data["config_file"];

        auto init_vector = [](const auto &j_items, auto &vec) {
            for (const auto &it : j_items) {
                auto value = it.value().template get<typename std::decay_t<decltype(vec)>::value_type>();
                value.name = it.key();
                vec.push_back(std::move(value));
            }
        };

        init_vector(cache_data["targets"].items(), _context->targets);
        init_vector(cache_data["templates"].items(), _context->templates);
        init_vector(cache_data["call_components"].items(), _context->call_components);
        _context->global_external_args = cache_data["global_external_args"].template get<vec<pair<string, string>>>();
    }
    catch (std::exception &what) {
        _log << (log_message(log_type::fatal) << "Invalid structure of the bweas cache file: \n" << what.what());
    }
}
