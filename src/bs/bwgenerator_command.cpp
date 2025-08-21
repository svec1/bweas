//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include <bwgenerator_command.hpp>
#include <bwgntools.hpp>
#include <tools/bwfile.hpp>

using namespace bweas;

static logger _log{"BWGENERATOR_COMMAND"};

void command_generator::get_input_files() {
    auto &target              = *_context->current_target;
    vec<string> &source_files = target.fields<vec<string>>(sc::profile::FIELD_SOURCE_FILES);
    for (auto &current_template : target.queue_templates) {
        const auto &call_component =
            std::find_if(_context->call_components.begin(), _context->call_components.end(),
                         [current_template](const sc::call_component &call_component) {
                             return call_component.name == current_template.name_call_component;
                         });
        size_t count_param_use_src_files = 0;
        for (auto &arg : current_template.args) {
            if (arg.type == sc::template_command::arg::e_type::trgfield &&
                arg.value.find(sc::profile::FIELD_SOURCE_FILES) == 0) {
                string mask;
                size_t it_str = arg.value.find(":");
                if (it_str != arg.value.npos) {
                    mask = arg.value;
                    mask.erase(0, it_str + 1);
                }

                if (std::atoll(mask.c_str()) != 0) {
                    if (std::atoll(mask.c_str()) == 1) {
                        for (size_t i = 0; i < source_files.size() &&
                                           std::find(current_template.ifiles.begin(), current_template.ifiles.end(),
                                                     source_files[i]) == current_template.ifiles.end();
                             ++i)
                            current_template.ifiles.push_back(source_files[i]);
                        current_template.single_generates = 1;
                    }
                    else {
                        for (size_t k = 0; k < source_files.size() && k < std::atoll(mask.c_str()) &&
                                           std::find(current_template.ifiles.begin(), current_template.ifiles.end(),
                                                     source_files[k]) == current_template.ifiles.end();
                             ++k)
                            current_template.ifiles.push_back(source_files[k]);
                    }
                }
                else {
                    if (!mask.empty()) {
                        vec<string> slc_files = bwfile::file_slc_mask(mask, source_files);
                        for (size_t i = 0; i < slc_files.size() &&
                                           std::find(current_template.ifiles.begin(), current_template.ifiles.end(),
                                                     slc_files[i]) == current_template.ifiles.end();
                             ++i)
                            current_template.ifiles.push_back(slc_files[i]);

                        current_template.single_generates = 1;
                    }
                    else
                        current_template.ifiles = source_files;
                }

                ++count_param_use_src_files;

                arg.value = FEATURE_ARG_IF;
                arg.type  = sc::template_command::arg::e_type::features;
            }
            else if (arg.type == sc::template_command::arg::e_type::features && arg.value == FEATURE_ARG_IF) {
                for (size_t i = 0; i < source_files.size() &&
                                   std::find(current_template.ifiles.begin(), current_template.ifiles.end(),
                                             source_files[i]) == current_template.ifiles.end();
                     ++i)
                    current_template.ifiles.push_back(source_files[i]);

                current_template.single_generates = 1;
                ++count_param_use_src_files;
            }

            if (count_param_use_src_files > 1)
                _log << (log_message(log_type::fatal)
                         << "A template cannot have more than one use-source-files parameter.");
        }

        if (current_template.returnable.type == sc::template_command::return_value::e_type::object &&
            current_template.returnable.value == target_type_str(target.type))
            current_template.returns_target = 1;
        else if (current_template.returnable.type == sc::template_command::return_value::e_type::extension_field) {
            if (!target.ext.contains(current_template.returnable.value) ||
                !std::holds_alternative<vec<string>>(target.ext[current_template.returnable.value]))
                _log << (log_message(log_type::fatal)
                         << "The template's return value referring to the goal extension field does not exist: "
                         << current_template.returnable.value);

            vec<string> &str_s = target.fields<vec<string>>(current_template.returnable.value);
            for (size_t i = 0; i < current_template.ifiles.size(); ++i)
                str_s.push_back(generator_tools::get_name_output_file(_context->current_work_directory + "/" +
                                                                          call_component->pattern_ret_files,
                                                                      current_template.ifiles[i]));
        }
    }
}
commands command_generator::generate() {
    commands cmd_s;

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

        string pattern_output_file = _context->current_work_directory + "/" + call_component->pattern_ret_files;

        command cmd;
        cmd.name = current_template.name + std::to_string(count_use_ifiles);

        string output_file = generator_tools::get_name_output_file(
            pattern_output_file,
            count_use_ifiles < current_template.ifiles.size() ? current_template.ifiles[count_use_ifiles] : "",
            count_use_ifiles);

        auto add_depends_cmd = [&](string_v _returnable) {
            for (const auto &_template : target.queue_templates) {
                if (_template.name == current_template.name)
                    break;

                if (_template.returnable.value == _returnable)
                    cmd.depends_command.insert(commands_execute_template[_template.name].begin(),
                                               commands_execute_template[_template.name].end());
            }
        };

        size_t real_count_use_ifiles = 0;

        for (auto &arg : current_template.args) {
            if (arg.type == sc::template_command::arg::e_type::string)
                cmd.args.push_back(arg.value);
            else if (arg.type == sc::template_command::arg::e_type::extglobal) {
                const auto &extern_arg = std::find_if(
                    _context->global_external_args.begin(), _context->global_external_args.end(),
                    [&arg](const std::pair<string, string> extern_typemp) { return extern_typemp.first == arg.value; });
                if (extern_arg == _context->global_external_args.end())
                    _log << (log_message(log_type::fatal)
                             << "The specified external parameter does not exist: " << arg.value);

                cmd.args.push_back(extern_arg->second);
            }
            else if (arg.type == sc::template_command::arg::e_type::trgfield) {
                if (target.ext.contains(arg.value)) {
                    if (target.ext.is_string(arg.value))
                        cmd.args.push_back(target.fields<string>(arg.prefix + arg.value));
                    else {
                        const auto &str_s = target.fields<vec<string>>(arg.value);

                        for (size_t k = 0; k < str_s.size(); ++k)
                            cmd.args.push_back(arg.prefix + str_s[k]);
                    }
                }
                else if (arg.value == NAME_FIELD_TARGET_NAME)
                    cmd.args.push_back(arg.prefix + target.name);
                else if (arg.value == NAME_FIELD_TARGET_TYPE)
                    cmd.args.push_back(arg.prefix + sc::target_type_str(target.type));
                else if (arg.value == NAME_FIELD_TARGET_CFG)
                    cmd.args.push_back(arg.prefix + sc::target_cfg_str(target.cfg));
                else if (arg.value == NAME_FIELD_TARGET_VER)
                    cmd.args.push_back(arg.prefix + target.ver.get_str_version());
                else
                    _log << (log_message(log_type::fatal) << "There is no such parameter: " << arg.value);
            }
            else if (arg.type == sc::template_command::arg::e_type::features && arg.value == FEATURE_ARG_IF) {
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
                                generator_tools::get_name_output_file(
                                    pattern_output_file, current_template.ifiles[count_use_ifiles], count_use_ifiles),
                                _context->dfiles[current_template.ifiles[count_use_ifiles]]) ||
                            current_template.returns_target) {
                            cmd.args.push_back(current_template.ifiles[count_use_ifiles]);
                            ++real_count_use_ifiles;
                        }
            }
            else if (arg.type == sc::template_command::arg::e_type::features && arg.value == FEATURE_ARG_OF) {
                if (current_template.returns_target) {
                    returnable_target[target.name].push_back(output_file);
                    cmd.args.push_back(output_file);
                }
                else {
                    if (current_template.single_generates) {
                        if (current_template.returnable.type == sc::template_command::return_value::e_type::object)
                            internal_args_stack_tmp[current_template.returnable.value].push_back(output_file);

                        cmd.args.push_back(output_file);
                    }
                    else
                        for (size_t k = 0; k < count_use_ifiles; ++k) {
                            output_file = generator_tools::get_name_output_file(pattern_output_file,
                                                                                current_template.ifiles[k], k);
                            if (current_template.returnable.type == sc::template_command::return_value::e_type::object)
                                internal_args_stack_tmp[current_template.returnable.value].push_back(output_file);

                            cmd.args.push_back(output_file);
                        }
                }
            }
            else if (arg.type == sc::template_command::arg::e_type::internal) {
                if (auto it = returnable_target.find(arg.value); it != returnable_target.end())
                    for (const auto &str : it->second)
                        cmd.args.push_back(arg.prefix + str);
                else if (auto it = internal_args_stack_tmp.find(arg.value); it != internal_args_stack_tmp.end())
                    for (const auto &str : it->second)
                        cmd.args.push_back(arg.prefix + str);

                add_depends_cmd(arg.value);
            }
        }

        if (!real_count_use_ifiles && !current_template.returns_target)
            _log << (log_message(log_type::msg) << "Skipped command generation for the file: " << output_file);
        else {
            add_depends_cmd(sc::profile::FIELD_SOURCE_FILES);

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
