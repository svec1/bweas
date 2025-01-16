#include "bwcache_api.hpp"

#include <nlohmann/json.hpp>

using namespace bweas;
using namespace cache_api;
using namespace bweas::bwexception;

json_bwcache::json_bwcache() {
    if (!init_glob_chjson) {
        assist.add_err("BWS-CACHE000", "Incorrect json cache structure");

        init_glob_chjson = 1;
    }
}

void json_bwcache::delete_cache() {
    delete this;
}

std::string json_bwcache::create_cache() {
    nlohmann::json cache_data;
    const auto &out_targets = _cache_data.targets_o_p != nullptr ? *_cache_data.targets_o_p : _cache_data.targets_o;

    for (const auto &target : out_targets)
        cache_data["targets"][target.name_target] = {{"type", var::struct_sb::target_t_str(target.target_t)},
                                                     {"configuration", var::struct_sb::cfg_str(target.target_cfg)},
                                                     {"version", target.version_target.get_str_version()},
                                                     {"generator", target.name_generator},
                                                     {"dependencies", target.target_vec_libs},
                                                     {"project",
                                                      {{"name", target.prj.name_project},
                                                       {"version", target.prj.version_project.get_str_version()},
                                                       {"lang", var::struct_sb::lang_str(target.prj.lang)},
                                                       {"path_compiler", target.prj.path_compiler},
                                                       {"path_linker", target.prj.path_linker},
                                                       {"release_flags_compiler", target.prj.rflags_compiler},
                                                       {"release_flags_linker", target.prj.rflags_linker},
                                                       {"debug_flags_compiler", target.prj.dflags_compiler},
                                                       {"debug_flags_linker", target.prj.dflags_linker},
                                                       {"std_c", target.prj.standart_c},
                                                       {"std_cpp", target.prj.standart_cpp},
                                                       {"files", target.prj.src_files},
                                                       {"use_it_templates", target.prj.use_it_templates},
                                                       {"templates", target.prj.vec_templates}}}};

    for (const auto &_template : _cache_data.templates) {
        cache_data["templates"][_template.name] = {{"name_call_component", _template.name_call_component},
                                                   {"returnable", _template.returnable},
                                                   {"accept_params", _template.name_accept_params}};
        for (const auto &arg : _template.args)
            cache_data["templates"][_template.name]["args"].push_back({{"type", arg.arg_t}, {"str", arg.str_arg}});
    }

    for (const auto &call_component : _cache_data.call_components)
        cache_data["call_components"][call_component.name] = {{"name_program", call_component.name_program},
                                                              {"pattern_ret_files", call_component.pattern_ret_files}};

    for (const auto &global_external_arg : _cache_data.global_external_args)
        cache_data["global_external_args"].push_back(
            {{"name", global_external_arg.first}, {"value", global_external_arg.second}});

    return cache_data.dump(4);
}

const base_bwcache::cache_data &json_bwcache::get_cache_data(std::string cache_str) {
    try {
        nlohmann::json cache_data = nlohmann::json::parse(cache_str);

        for (const auto &target : cache_data["targets"].items()) {
            var::struct_sb::target_out target_o_tmp;
            target_o_tmp.name_target = target.key();

            const auto &fields = target.value();
            target_o_tmp.target_t = var::struct_sb::to_type_target(fields["type"]);
            target_o_tmp.target_cfg = var::struct_sb::to_cfg(fields["configuration"]);
            target_o_tmp.version_target = (std::string)fields["version"];
            target_o_tmp.name_generator = fields["generator"];
            target_o_tmp.target_vec_libs = fields["dependencies"];

            const auto &prj = fields["project"];
            target_o_tmp.prj.name_project = prj["name"];
            target_o_tmp.prj.version_project = (std::string)prj["version"];
            target_o_tmp.prj.lang = var::struct_sb::to_lang(prj["lang"]);
            target_o_tmp.prj.path_compiler = prj["path_compiler"];
            target_o_tmp.prj.path_linker = prj["path_linker"];
            target_o_tmp.prj.rflags_compiler = prj["release_flags_compiler"];
            target_o_tmp.prj.rflags_linker = prj["release_flags_linker"];
            target_o_tmp.prj.dflags_compiler = prj["debug_flags_compiler"];
            target_o_tmp.prj.dflags_linker = prj["debug_flags_linker"];
            target_o_tmp.prj.standart_c = prj["std_c"];
            target_o_tmp.prj.standart_cpp = prj["std_cpp"];
            target_o_tmp.prj.src_files = prj["files"];
            target_o_tmp.prj.use_it_templates = prj["use_it_templates"];
            target_o_tmp.prj.vec_templates = prj["templates"];

            _cache_data.targets_o.push_back(target_o_tmp);
        }

        for (const auto &_template : cache_data["templates"].items()) {
            var::struct_sb::template_command template_tmp;
            template_tmp.name = _template.key();

            const auto &fields = _template.value();
            template_tmp.name_call_component = fields["name_call_component"];
            template_tmp.returnable = fields["returnable"];
            template_tmp.name_accept_params = fields["accept_params"];
            for (const auto &arg : fields["args"])
                template_tmp.args.push_back(var::struct_sb::template_command::arg(
                    arg["str"], (var::struct_sb::template_command::arg::type)arg["type"]));

            _cache_data.templates.push_back(template_tmp);
        }

        for (const auto &call_component : cache_data["call_components"].items()) {
            var::struct_sb::call_component call_component_tmp;
            call_component_tmp.name = call_component.key();

            const auto &fields = call_component.value();
            call_component_tmp.name_program = fields["name_program"];
            call_component_tmp.pattern_ret_files = fields["pattern_ret_files"];

            _cache_data.call_components.push_back(call_component_tmp);
        }

        for (const auto &call_component : cache_data["global_external_args"].items())
            _cache_data.global_external_args.emplace_back(call_component.value()["name"],
                                                          call_component.value()["value"]);

        return _cache_data;
    }
    catch (std::exception &what) {
        throw bwcache_excp(what.what(), "000");
    }
}