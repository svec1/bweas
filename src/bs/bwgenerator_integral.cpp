//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include <bwgenerator_integral.hpp>
#include <bwgntools.hpp>
#include <tools/bwfile.hpp>

using namespace bweas;

static logger log{"BWGENERATOR[INTERNAL]"};

generator_api::integral_generator::integral_generator(func_build_graph_depends_file _build_graph_depends_file_p,
                                                      func_get_input_files _get_input_files_p,
                                                      func_generator _generate_p) {
    build_graph_depends_file_p = _build_graph_depends_file_p;
    get_input_files_p          = _get_input_files_p;
    generate_p                 = _generate_p;
}

void generator_api::integral_generator::init() {
}

uset<string> generator_api::integral_generator::build_graph_depends_file(string_v language, string_v name_file,
                                                                         string_v work_directory,
                                                                         vec<string> include_paths) {
    return build_graph_depends_file_p(language, name_file, work_directory, include_paths);
}

void generator_api::integral_generator::get_input_files(generator_api::data_transfer &data_t) {
    get_input_files_p(data_t);
}

generator_api::commands generator_api::integral_generator::generate_commands(generator_api::data_transfer &data_t) {
    generator_tools::parse_basic_args(*data_t.context->current_target, data_t.context->templates,
                                      data_t.context->global_external_args);
    return generate_p(data_t);
}

void integral_generator::get_input_files(generator_api::data_transfer &data_t) {
    for (const auto &target : data_t.context->out_targets)
        for (const auto &current_template_name : target.prj.vec_templates) {
            const auto &current_template =
                std::find_if(data_t.context->templates.begin(), data_t.context->templates.end(),
                             [current_template_name](const var::struct_sb::template_command _template) {
                                 return _template.name == current_template_name;
                             });
            const auto &call_component =
                std::find_if(data_t.context->call_components.begin(), data_t.context->call_components.end(),
                             [current_template](const var::struct_sb::call_component &call_component) {
                                 return call_component.name == current_template->name_call_component;
                             });
            for (auto &arg : current_template->args) {
                if (arg.arg_t == var::struct_sb::template_command::arg::type::trgfield &&
                    arg.str_arg.find(NAME_FIELD_PROJECT_SRC_FILES) == 0) {
                    string mask;
                    size_t it_str = arg.str_arg.find(":");
                    if (it_str != arg.str_arg.npos) {
                        mask = arg.str_arg;
                        mask.erase(0, it_str + 1);
                    }

                    if (std::atoll(mask.c_str()) != 0) {
                        if (std::atoll(mask.c_str()) == 1) {
                            for (size_t i = 0;
                                 i < target.prj.src_files.size() &&
                                 std::find(data_t.ifiles[current_template->name].begin(),
                                           data_t.ifiles[current_template->name].end(),
                                           target.prj.src_files[i]) == data_t.ifiles[current_template->name].end();
                                 ++i)
                                data_t.ifiles[current_template->name].push_back(target.prj.src_files[i]);
                            arg.str_arg = "single";
                        }
                        else {
                            for (size_t k = 0;
                                 k < target.prj.src_files.size() && k < std::atoll(mask.c_str()) &&
                                 std::find(data_t.ifiles[current_template->name].begin(),
                                           data_t.ifiles[current_template->name].end(),
                                           target.prj.src_files[k]) == data_t.ifiles[current_template->name].end();
                                 ++k)
                                data_t.ifiles[current_template->name].push_back(target.prj.src_files[k]);
                            arg.str_arg = "all";
                        }
                    }
                    else {
                        vec<string> slc_files = bwfile::file_slc_mask(mask, target.prj.src_files);
                        for (size_t i = 0; i < slc_files.size() &&
                                           std::find(data_t.ifiles[current_template->name].begin(),
                                                     data_t.ifiles[current_template->name].end(),
                                                     slc_files[i]) == data_t.ifiles[current_template->name].end();
                             ++i)
                            data_t.ifiles[current_template->name].push_back(target.prj.src_files[i]);
                        arg.str_arg = "all";
                    }
                }
                else if (arg.arg_t == var::struct_sb::template_command::arg::type::features &&
                         arg.str_arg.find(FEATURE_FIELD_BS_CURRENT_IF) == 0) {
                    for (size_t i = 0;
                         i < target.prj.src_files.size() &&
                         std::find(data_t.ifiles[current_template->name].begin(),
                                   data_t.ifiles[current_template->name].end(),
                                   target.prj.src_files[i]) == data_t.ifiles[current_template->name].end();
                         ++i)

                        data_t.ifiles[current_template->name].push_back(target.prj.src_files[i]);
                }
            }
        }
}
generator_api::commands integral_generator::generate(generator_api::data_transfer &data_t) {
    generator_api::commands cmd_s;

    umap<string, vec<string>> internal_args_stack_tmp;
    umap<string, vec<string>> commands_execute_template;

    bool generate_for_single_file = 0;

    for (const auto &target : data_t.context->out_targets) {
        size_t count_use_ifiles = 0;
        size_t i                = 0;

        for (size_t j = 0; j < target.prj.vec_templates.size() || generate_for_single_file;) {
            const auto &current_template_name = target.prj.vec_templates[j];
            const auto &current_template =
                std::find_if(data_t.context->templates.begin(), data_t.context->templates.end(),
                             [current_template_name](const var::struct_sb::template_command _template) {
                                 return _template.name == current_template_name;
                             });
            const auto &call_component =
                std::find_if(data_t.context->call_components.begin(), data_t.context->call_components.end(),
                             [current_template](const var::struct_sb::call_component &call_component) {
                                 return call_component.name == current_template->name_call_component;
                             });

            generator_api::command cmd;
            string output_file =
                generator_tools::get_name_output_file(call_component->pattern_ret_files, i, data_t.work_directory);

            cmd.name = current_template_name + std::to_string(i);

            for (auto &arg : current_template->args) {
                if ((arg.arg_t == var::struct_sb::template_command::arg::type::features &&
                     arg.str_arg.find(FEATURE_FIELD_BS_CURRENT_IF) == 0) ||
                    arg.arg_t == var::struct_sb::template_command::arg::type::trgfield) {
                    if (arg.str_arg == "single" || arg.str_arg.find(FEATURE_FIELD_BS_CURRENT_IF) == 0) {
                        generate_for_single_file = 1;
                        output_file = generator_tools::get_name_output_file(call_component->pattern_ret_files,
                                                                            count_use_ifiles, data_t.work_directory);
                        if (generator_tools::should_uses_src_file(
                                data_t.ifiles[current_template_name][count_use_ifiles],
                                generator_tools::get_name_output_file(call_component->pattern_ret_files,
                                                                      count_use_ifiles, data_t.work_directory),
                                data_t.dfiles[data_t.ifiles[current_template_name][count_use_ifiles]]))
                            cmd.args.push_back(data_t.ifiles[current_template_name][count_use_ifiles++]);
                        else {
                            internal_args_stack_tmp[current_template->returnable].push_back(output_file);

                            ++count_use_ifiles;

                            cmd.args.clear();

                            break;
                        }
                    }
                    else
                        for (; count_use_ifiles < data_t.ifiles[current_template_name].size(); ++count_use_ifiles)
                            if (generator_tools::should_uses_src_file(
                                    data_t.ifiles[current_template_name][count_use_ifiles],
                                    generator_tools::get_name_output_file(call_component->pattern_ret_files,
                                                                          count_use_ifiles, data_t.work_directory),
                                    data_t.dfiles[data_t.ifiles[current_template_name][count_use_ifiles]]))
                                cmd.args.push_back(data_t.ifiles[current_template_name][count_use_ifiles]);
                }
                else if (arg.arg_t == var::struct_sb::template_command::arg::type::features &&
                         arg.str_arg.find(FEATURE_FIELD_BS_CURRENT_OF) == 0) {
                    if (current_template->returnable == target_t_str(target.target_t))
                        cmd.args.push_back(output_file);
                    else {
                        if (generate_for_single_file) {
                            internal_args_stack_tmp[current_template->returnable].push_back(output_file);

                            cmd.args.push_back(output_file);
                        }
                        else
                            for (size_t k = 0; k < count_use_ifiles; ++k) {
                                internal_args_stack_tmp[current_template->returnable].push_back(
                                    generator_tools::get_name_output_file(call_component->pattern_ret_files, k,
                                                                          data_t.work_directory));

                                cmd.args.push_back(
                                    internal_args_stack_tmp
                                        [current_template->returnable]
                                        [internal_args_stack_tmp[current_template->returnable].size() - 1]);
                            }
                    }
                    ++i;
                }
                else if (arg.arg_t == var::struct_sb::template_command::arg::type::internal) {
                    for (size_t k = 0; k < internal_args_stack_tmp[arg.str_arg].size(); ++k)
                        cmd.args.push_back(internal_args_stack_tmp[arg.str_arg][k]);

                    for (const auto &_template : data_t.context->templates) {
                        if (_template.name == current_template->name)
                            break;

                        if (_template.returnable == arg.str_arg)
                            cmd.depends_command.insert(cmd.depends_command.end(),
                                                       commands_execute_template[_template.name].begin(),
                                                       commands_execute_template[_template.name].end());
                    }
                }
                else if (arg.arg_t == var::struct_sb::template_command::arg::type::string)
                    cmd.args.push_back(arg.str_arg);
            }
            if (cmd.args.empty())
                log << (log_message(log_type::msg) << "Skipped command generation for the file: " << output_file);
            else {
                cmd.name_used_file = output_file;
                cmd.name_program   = call_component->name_program;
                cmd_s.push_back(cmd);

                commands_execute_template[current_template_name].push_back(cmd.name);

                log << (log_message(log_type::msg)
                        << "The command has been generated: " << generator_tools::build_string_command(cmd));
            }

            if (generate_for_single_file && count_use_ifiles < data_t.ifiles[current_template_name].size())
                continue;

            generate_for_single_file = 0;
            count_use_ifiles         = 0;
            i                        = 0;

            ++j;
        }
    }
    return cmd_s;
}

