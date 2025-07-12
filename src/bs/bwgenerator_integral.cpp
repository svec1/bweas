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

static logger _log{"BWGENERATOR[INTERNAL]"};

generator_api::integral_generator::integral_generator(func_build_graph_depends_file _build_graph_depends_file_p,
                                                      func_get_input_files _get_input_files_p,
                                                      func_generator _generate_p) {
    build_graph_depends_file_p = _build_graph_depends_file_p;
    get_input_files_p          = _get_input_files_p;
    generate_p                 = _generate_p;
}

uset<string> generator_api::integral_generator::build_graph_depends_file(string_v language, string_v name_file,
                                                                         string_v work_directory,
                                                                         vec<string> include_paths) {
    return build_graph_depends_file_p(language, name_file, work_directory, include_paths);
}

void generator_api::integral_generator::get_input_files() {
    try {
        get_input_files_p(_context);
    }
    catch (std::exception &excp) {
        _log << bwtools::fatal
             << (log_message(log_type::fatal) << "Couldn't get the input files for the current target's templates("
                                              << _context->current_target->name << "):\n"
                                              << excp.what());
    }
}

generator_api::commands generator_api::integral_generator::generate_commands() {
    try {
        generator_tools::parse_basic_args(*_context->current_target, _context->current_target->queue_templates,
                                          _context->global_external_args);
        return generate_p(_context);
    }
    catch (std::exception &excp) {
        _log << bwtools::fatal
             << (log_message(log_type::fatal) << "Failed to generate a template command for the current target("
                                              << _context->current_target->name << "):\n"
                                              << excp.what());
    }
}

void integral_generator::get_input_files(context *const _context) {
    auto &target = *_context->current_target;
    for (auto &current_template : target.queue_templates) {
        const auto &call_component =
            std::find_if(_context->call_components.begin(), _context->call_components.end(),
                         [current_template](const sc::call_component &call_component) {
                             return call_component.name == current_template.name_call_component;
                         });
        size_t count_param_use_src_files = 0;
        for (auto &arg : current_template.args) {
            if (arg.arg_t == sc::template_command::arg::type::trgfield &&
                arg.str_arg.find(NAME_FIELD_PROJECT_SRC_FILES) == 0) {
                string mask;
                size_t it_str = arg.str_arg.find(":");
                if (it_str != arg.str_arg.npos) {
                    mask = arg.str_arg;
                    mask.erase(0, it_str + 1);
                }

                if (std::atoll(mask.c_str()) != 0) {
                    if (std::atoll(mask.c_str()) == 1) {
                        for (size_t i = 0; i < target.prj.src_files.size() &&
                                           std::find(current_template.ifiles.begin(), current_template.ifiles.end(),
                                                     target.prj.src_files[i]) == current_template.ifiles.end();
                             ++i)
                            current_template.ifiles.push_back(target.prj.src_files[i]);
                        current_template.single_generates = 1;
                    }
                    else {
                        for (size_t k = 0; k < target.prj.src_files.size() && k < std::atoll(mask.c_str()) &&
                                           std::find(current_template.ifiles.begin(), current_template.ifiles.end(),
                                                     target.prj.src_files[k]) == current_template.ifiles.end();
                             ++k)
                            current_template.ifiles.push_back(target.prj.src_files[k]);
                    }
                }
                else {
                    if (!mask.empty()) {
                        vec<string> slc_files = bwfile::file_slc_mask(mask, target.prj.src_files);
                        for (size_t i = 0; i < slc_files.size() &&
                                           std::find(current_template.ifiles.begin(), current_template.ifiles.end(),
                                                     slc_files[i]) == current_template.ifiles.end();
                             ++i)
                            current_template.ifiles.push_back(slc_files[i]);

                        current_template.single_generates = 1;
                    }
                    else
                        current_template.ifiles = target.prj.src_files;
                }

                ++count_param_use_src_files;
            }
            else if (arg.arg_t == sc::template_command::arg::type::features &&
                     arg.str_arg == FEATURE_FIELD_BS_CURRENT_IF) {
                for (size_t i = 0; i < target.prj.src_files.size() &&
                                   std::find(current_template.ifiles.begin(), current_template.ifiles.end(),
                                             target.prj.src_files[i]) == current_template.ifiles.end();
                     ++i)
                    current_template.ifiles.push_back(target.prj.src_files[i]);

                current_template.single_generates = 1;
                ++count_param_use_src_files;
            }

            if (count_param_use_src_files > 1)
                throw std::runtime_error("A template cannot have more than one use-source-files parameter.");
        }

        if (current_template.returnable == NAME_FIELD_PROJECT_SRC_FILES) {
            for (size_t i = 0; i < current_template.ifiles.size(); ++i)
                target.prj.src_files.push_back(generator_tools::get_name_output_file(
                    call_component->pattern_ret_files, i, _context->current_work_directory));
        }
        else if (current_template.returnable == target_type_str(target.type))
            current_template.returns_target = 1;
    }
}
generator_api::commands integral_generator::generate(context *const _context) {
    generator_api::commands cmd_s;

    static umap<string, vec<string>> returnable_target;

    umap<string, vec<string>> internal_args_stack_tmp;
    umap<string, vec<string>> commands_execute_template;

    size_t count_use_ifiles = 0;

    const auto &target = *_context->current_target;
    for (size_t j = 0; j < target.queue_templates.size();) {
        const auto &current_template = target.queue_templates[j];
        const auto &call_component =
            std::find_if(_context->call_components.begin(), _context->call_components.end(),
                         [current_template](const sc::call_component &call_component) {
                             return call_component.name == current_template.name_call_component;
                         });

        generator_api::command cmd;
        cmd.name = current_template.name + std::to_string(count_use_ifiles);

        string output_file = generator_tools::get_name_output_file(call_component->pattern_ret_files, count_use_ifiles,
                                                                   _context->current_work_directory);

        auto add_depends_cmd = [&](string_v _returnable) {
            for (const auto &_template : _context->templates) {
                if (_template.name == current_template.name)
                    break;

                if (_template.returnable == _returnable)
                    cmd.depends_command.insert(commands_execute_template[_template.name].begin(),
                                               commands_execute_template[_template.name].end());
            }
        };

        size_t real_count_use_ifiles = 0;

        for (auto &arg : current_template.args) {
            if ((arg.arg_t == sc::template_command::arg::type::features &&
                 arg.str_arg == FEATURE_FIELD_BS_CURRENT_IF) ||
                arg.arg_t == sc::template_command::arg::type::trgfield) {
                if (current_template.single_generates) {
                    if (generator_tools::should_uses_src_file(
                            current_template.ifiles[count_use_ifiles], output_file,
                            _context->dfiles[current_template.ifiles[count_use_ifiles]]) ||
                        current_template.returns_target) {
                        cmd.args.push_back(current_template.ifiles[count_use_ifiles++]);
                        ++real_count_use_ifiles;
                    }
                    else
                        ++count_use_ifiles;
                }
                else
                    for (; count_use_ifiles < current_template.ifiles.size(); ++count_use_ifiles)
                        if (generator_tools::should_uses_src_file(
                                current_template.ifiles[count_use_ifiles],
                                generator_tools::get_name_output_file(call_component->pattern_ret_files,
                                                                      count_use_ifiles,
                                                                      _context->current_work_directory),
                                _context->dfiles[current_template.ifiles[count_use_ifiles]]) ||
                            current_template.returns_target) {
                            cmd.args.push_back(current_template.ifiles[count_use_ifiles]);
                            ++real_count_use_ifiles;
                        }

                add_depends_cmd(NAME_FIELD_PROJECT_SRC_FILES);
            }
            else if (arg.arg_t == sc::template_command::arg::type::features &&
                     arg.str_arg == FEATURE_FIELD_BS_CURRENT_OF) {
                if (current_template.returns_target) {
                    returnable_target[target.name].push_back(output_file);
                    cmd.args.push_back(output_file);
                }
                else {
                    if (current_template.single_generates) {
                        if (current_template.returnable != NAME_FIELD_PROJECT_SRC_FILES)
                            internal_args_stack_tmp[current_template.returnable].push_back(output_file);

                        cmd.args.push_back(output_file);
                    }
                    else
                        for (size_t k = 0; k < count_use_ifiles; ++k) {
                            output_file = generator_tools::get_name_output_file(call_component->pattern_ret_files, k,
                                                                                _context->current_work_directory);
                            if (current_template.returnable != NAME_FIELD_PROJECT_SRC_FILES)
                                internal_args_stack_tmp[current_template.returnable].push_back(output_file);

                            cmd.args.push_back(output_file);
                        }
                }
            }
            else if (arg.arg_t == sc::template_command::arg::type::internal) {
                if (auto it = returnable_target.find(arg.str_arg); it != returnable_target.end())
                    cmd.args.insert(cmd.args.end(), it->second.begin(), it->second.end());
                else if (auto it = internal_args_stack_tmp.find(arg.str_arg); it != internal_args_stack_tmp.end())
                    cmd.args.insert(cmd.args.end(), it->second.begin(), it->second.end());

                add_depends_cmd(arg.str_arg);
            }
            else if (arg.arg_t == sc::template_command::arg::type::string)
                cmd.args.push_back(arg.str_arg);
        }
        if (!real_count_use_ifiles && !current_template.returns_target)
            _log << (log_message(log_type::msg) << "Skipped command generation for the file: " << output_file);
        else {
            cmd.name_output_file = output_file;
            cmd.name_program     = call_component->name_program;
            cmd_s.push_back(cmd);

            commands_execute_template[current_template.name].push_back(cmd.name);

            _log << (log_message(log_type::msg)
                     << "The command has been generated: " << generator_tools::build_string_command(cmd));
        }

        if (current_template.single_generates && count_use_ifiles < current_template.ifiles.size())
            continue;

        count_use_ifiles = 0;

        ++j;
    }
    return cmd_s;
}
