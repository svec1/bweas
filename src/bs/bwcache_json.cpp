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
} // namespace nlohmann

using namespace bweas;
using namespace cache_api;

static logger _log{"BWCACHE[JSON]"};

string json_cache::create_cache() {
    nlohmann::json cache_data;

    cache_data["config_file"] = _context->path_bweas_config;

    for (const auto &target : _context->targets)
        cache_data["targets"][target.name] = {
            {"type", sc::target_type_str(target.type)}, {"configuration", sc::target_cfg_str(target.cfg)},
            {"version", target.ver.get_str_version()},  {"templates", target.templates},
            {"dependencies", target.dependencies},      {"extension", target.ext}};

    for (const auto &_template : _context->templates) {
        cache_data["templates"][_template.name] = {{"name_call_component", _template.name_call_component},
                                                   {"returnable", _template.returnable},
                                                   {"accept_params", _template.name_accept_params}};
        for (const auto &arg : _template.args)
            cache_data["templates"][_template.name]["args"].push_back({{"type", arg.type}, {"str", arg.value}});
    }

    for (const auto &call_component : _context->call_components)
        cache_data["call_components"][call_component.name] = {{"name_program", call_component.name_program},
                                                              {"pattern_ret_files", call_component.pattern_ret_files}};

    for (const auto &global_external_arg : _context->global_external_args)
        cache_data["global_external_args"].push_back(
            {{"name", global_external_arg.first}, {"value", global_external_arg.second}});

    return cache_data.dump(4);
}
string json_cache::get_path_config(const string &cache_str) {
    try {
        nlohmann::json cache_data = nlohmann::json::parse(cache_str);
        return cache_data["config_file"];
    }
    catch (std::exception &what) {
        _log << (log_message(log_type::fatal) << "Invalid structure of the bweas cache file: " << what.what());
    }

    std::unreachable();
}

void json_cache::extract_cache_data(const string &cache_str) {
    try {
        nlohmann::json cache_data = nlohmann::json::parse(cache_str);

        _context->path_bweas_config = cache_data["config_file"];

        for (const auto &target : cache_data["targets"].items()) {
            sc::target target_o_tmp;
            target_o_tmp.name = target.key();

            const auto &fields        = target.value();
            target_o_tmp.type         = sc::to_target_type(fields["type"]);
            target_o_tmp.cfg          = sc::to_target_cfg(fields["configuration"]);
            target_o_tmp.ver          = (string)fields["version"];
            target_o_tmp.templates    = fields["templates"];
            target_o_tmp.dependencies = fields["dependencies"];

            target_o_tmp.ext = fields["extension"];

            _context->targets.push_back(target_o_tmp);
        }

        for (const auto &_template : cache_data["templates"].items()) {
            sc::template_command template_tmp;
            template_tmp.name = _template.key();

            const auto &fields               = _template.value();
            template_tmp.name_call_component = fields["name_call_component"];
            template_tmp.returnable          = fields["returnable"];
            template_tmp.name_accept_params  = fields["accept_params"];
            for (const auto &arg : fields["args"])
                template_tmp.args.push_back(
                    sc::template_command::arg(arg["str"], (sc::template_command::arg::e_type)arg["type"]));

            _context->templates.push_back(template_tmp);
        }

        for (const auto &call_component : cache_data["call_components"].items()) {
            sc::call_component call_component_tmp;
            call_component_tmp.name = call_component.key();

            const auto &fields                   = call_component.value();
            call_component_tmp.name_program      = fields["name_program"];
            call_component_tmp.pattern_ret_files = fields["pattern_ret_files"];

            _context->call_components.push_back(call_component_tmp);
        }

        for (const auto &call_component : cache_data["global_external_args"].items())
            _context->global_external_args.emplace_back(call_component.value()["name"],
                                                        call_component.value()["value"]);
    }
    catch (std::exception &what) {
        _log << (log_message(log_type::fatal) << "Invalid structure of the bweas cache file: " << what.what());
    }
}
