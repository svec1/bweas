//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include <bwcache_api.hpp>

#include <nlohmann/json.hpp>

namespace nlohmann {
template <> struct adl_serializer<bweas::sc::language::dependency_finder> {
    static void to_json(json &j, const bweas::sc::language::dependency_finder &value) {
        j = {{"search_regex", value.search_regex}, {"char_global_search", string(1, value.char_global_search)}};
    }
    static void from_json(const json &j, bweas::sc::language::dependency_finder &value) {
        j.at("search_regex").get_to(value.search_regex);
        value.char_global_search = static_cast<string>(j.at("char_global_search"))[0];
    }
};
template <> struct adl_serializer<bweas::sc::language> {
    static void to_json(json &j, const bweas::sc::language &value) {
        j = {{"name", value.name}, {"dependency_finder", value.dfinder_data}};
    }
    static void from_json(const json &j, bweas::sc::language &value) {
        j.at("name").get_to(value.name);
        j.at("dependency_finder").get_to(value.dfinder_data);
    }
};
template <> struct adl_serializer<bweas::sc::profile::fields::mapped_type> {
    static void to_json(json &j, const bweas::sc::profile::fields::mapped_type &value) {
        std::visit([&](auto &&_value) { j = std::forward<decltype(_value)>(_value); }, value);
    }
    static void from_json(const json &j, bweas::sc::profile::fields::mapped_type &value) {
        if (j.is_number())
            value = j.get<pdiff>();
        if (j.is_string())
            value = j.get<string>();
        else if (j.is_array())
            value = j.get<vec<string>>();
        else
            assert("Invalid type for convert to std::variant");
    }
};
template <> struct adl_serializer<bweas::sc::profile> {
    static void to_json(json &j, const bweas::sc::profile &value) {
        j["language"] = value.lang;
        j["fields"]   = value.get_fields();
    }
    static void from_json(const json &j, bweas::sc::profile &value) {
        j.at("language").get_to(value.lang);
        j.at("fields").get_to(value.get_fields());
    }
};
template <> struct adl_serializer<bweas::sc::target> {
    static void to_json(json &j, const bweas::sc::target &value) {
        j = {{"version", value.ver.get_str_version()},
             {"templates", value.templates},
             {"dependencies", value.dependencies},
             {"extension", value.ext}};
    }
    static void from_json(const json &j, bweas::sc::target &value) {
        try {
            value.ver = j["version"].template get<string>();

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

string json_cache::create_cache() const {
    nlohmann::json cache_data;

    cache_data["config_file"] = _context->path_bweas_config;

    for (const auto &target : _context->targets)
        cache_data["targets"][target.name] = target;
    for (const auto &_template : _context->templates)
        cache_data["templates"][_template.name] = _template;
    for (const auto &call_component : _context->call_components)
        cache_data["call_components"][call_component.name] = call_component;

    return cache_data.dump(4);
}
string json_cache::get_path_config(const string &cache_str) const {
    try {
        nlohmann::json cache_data = nlohmann::json::parse(cache_str);
        return cache_data["config_file"];
    }
    catch (std::exception &what) {
        _log << (log_message(log_type::fatal) << "Invalid structure of the bweas cache file: \n" << what.what());
    }

    std::unreachable();
}

void json_cache::extract_cache_data(const string &cache_str) const {
    try {
        nlohmann::json cache_data = nlohmann::json::parse(cache_str);

        cache_data.at("config_file").get_to(_context->path_bweas_config);

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
    }
    catch (std::exception &what) {
        _log << (log_message(log_type::fatal) << "Invalid structure of the bweas cache file: \n" << what.what());
    }
}
