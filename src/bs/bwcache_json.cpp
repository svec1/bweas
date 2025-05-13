//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include "bwcache_api.hpp"

#include <nlohmann/json.hpp>

using namespace bweas;
using namespace cache_api;

static logger _log{"BWCACHE[JSON]"};

json_bwcache::json_bwcache(bw_context *const context) : base_bwcache(context) {
    if (!context)
        (_log << bwtools::fatal) << (log_message(log_type::fatal) << "Bweas the context is not defined");
}
void json_bwcache::delete_cache() {
    delete this;
}

std::string json_bwcache::create_cache() {
    nlohmann::json cache_data;

    for (const auto &target : context->out_targets)
        cache_data["targets"][target.name_target] = {{"type", var::struct_sb::target_t_str(target.target_t)},
                                                     {"configuration", var::struct_sb::cfg_str(target.target_cfg)},
                                                     {"version", target.version_target.get_str_version()},
                                                     {"generator", target.name_generator},
                                                     {"dependencies", target.target_vec_libs},
                                                     {"project",
                                                      {{"name", target.prj.name_project},
                                                       {"version", target.prj.version_project.get_str_version()},
                                                       {"lang", target.prj.language},
                                                       {"path_compiler", target.prj.path_compiler},
                                                       {"path_linker", target.prj.path_linker},
                                                       {"release_flags_compiler", target.prj.rflags_compiler},
                                                       {"release_flags_linker", target.prj.rflags_linker},
                                                       {"debug_flags_compiler", target.prj.dflags_compiler},
                                                       {"debug_flags_linker", target.prj.dflags_linker},
                                                       {"std_c", target.prj.standart_c},
                                                       {"std_cpp", target.prj.standart_cpp},
                                                       {"files", target.prj.src_files},
                                                       {"include_paths", target.prj.include_paths},
                                                       {"use_it_templates", target.prj.use_it_templates},
                                                       {"templates", target.prj.vec_templates},
                                                       {"custom_extension_fields", target.prj.custom_ext_fields}}}};

    for (const auto &_template : context->templates) {
        cache_data["templates"][_template.name] = {{"name_call_component", _template.name_call_component},
                                                   {"returnable", _template.returnable},
                                                   {"accept_params", _template.name_accept_params}};
        for (const auto &arg : _template.args)
            cache_data["templates"][_template.name]["args"].push_back({{"type", arg.arg_t}, {"str", arg.str_arg}});
    }

    for (const auto &call_component : context->call_components)
        cache_data["call_components"][call_component.name] = {{"name_program", call_component.name_program},
                                                              {"pattern_ret_files", call_component.pattern_ret_files}};

    for (const auto &global_external_arg : context->global_external_args)
        cache_data["global_external_args"].push_back(
            {{"name", global_external_arg.first}, {"value", global_external_arg.second}});

    return cache_data.dump(4);
}

void json_bwcache::extract_cache_data(std::string &&cache_str) {
    try {
        nlohmann::json cache_data = nlohmann::json::parse(cache_str);

        for (const auto &target : cache_data["targets"].items()) {
            var::struct_sb::target_out target_o_tmp;
            target_o_tmp.name_target = target.key();

            const auto &fields           = target.value();
            target_o_tmp.target_t        = var::struct_sb::to_type_target(fields["type"]);
            target_o_tmp.target_cfg      = var::struct_sb::to_cfg(fields["configuration"]);
            target_o_tmp.version_target  = (std::string)fields["version"];
            target_o_tmp.name_generator  = fields["generator"];
            target_o_tmp.target_vec_libs = fields["dependencies"];

            const auto &prj                    = fields["project"];
            target_o_tmp.prj.name_project      = prj["name"];
            target_o_tmp.prj.version_project   = (std::string)prj["version"];
            target_o_tmp.prj.language          = prj["lang"];
            target_o_tmp.prj.path_compiler     = prj["path_compiler"];
            target_o_tmp.prj.path_linker       = prj["path_linker"];
            target_o_tmp.prj.rflags_compiler   = prj["release_flags_compiler"];
            target_o_tmp.prj.rflags_linker     = prj["release_flags_linker"];
            target_o_tmp.prj.dflags_compiler   = prj["debug_flags_compiler"];
            target_o_tmp.prj.dflags_linker     = prj["debug_flags_linker"];
            target_o_tmp.prj.standart_c        = prj["std_c"];
            target_o_tmp.prj.standart_cpp      = prj["std_cpp"];
            target_o_tmp.prj.src_files         = prj["files"];
            target_o_tmp.prj.include_paths     = prj["include_paths"];
            target_o_tmp.prj.use_it_templates  = prj["use_it_templates"];
            target_o_tmp.prj.vec_templates     = prj["templates"];
            target_o_tmp.prj.custom_ext_fields = prj["custom_extension_fields"];

            context->out_targets.push_back(target_o_tmp);
        }

        for (const auto &_template : cache_data["templates"].items()) {
            var::struct_sb::template_command template_tmp;
            template_tmp.name = _template.key();

            const auto &fields               = _template.value();
            template_tmp.name_call_component = fields["name_call_component"];
            template_tmp.returnable          = fields["returnable"];
            template_tmp.name_accept_params  = fields["accept_params"];
            for (const auto &arg : fields["args"])
                template_tmp.args.push_back(var::struct_sb::template_command::arg(
                    arg["str"], (var::struct_sb::template_command::arg::type)arg["type"]));

            context->templates.push_back(template_tmp);
        }

        for (const auto &call_component : cache_data["call_components"].items()) {
            var::struct_sb::call_component call_component_tmp;
            call_component_tmp.name = call_component.key();

            const auto &fields                   = call_component.value();
            call_component_tmp.name_program      = fields["name_program"];
            call_component_tmp.pattern_ret_files = fields["pattern_ret_files"];

            context->call_components.push_back(call_component_tmp);
        }

        for (const auto &call_component : cache_data["global_external_args"].items())
            context->global_external_args.emplace_back(call_component.value()["name"], call_component.value()["value"]);
    }
    catch (std::exception &what) {
        (_log << bwtools::fatal) << (log_message(log_type::fatal)
                                     << "Invalid structure of the bweas cache file: " << what.what());
    }
}
