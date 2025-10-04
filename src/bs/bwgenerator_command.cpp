//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include <bwgenerator_command.hpp>
#include <utils/file_utils.hpp>

using namespace bweas;
using namespace bweas::utils;

static logger _log{"BWGENERATOR_COMMAND"};

void generator_command::get_input_files() {
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
                        vec<string> slc_files = file_utils::file_slc_mask(mask, source_files);
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

                arg.value = sc::template_command::FEATURE_INPUT_FILE;
                arg.type  = sc::template_command::arg::e_type::string;
            }
            else if (arg.type == sc::template_command::arg::e_type::string &&
                     arg.value == sc::template_command::FEATURE_INPUT_FILE) {
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
            current_template.returnable.value == sc::template_command::return_value::RETURN_VALUE_TARGET)
            current_template.returns_target = 1;
        else if (current_template.returnable.type == sc::template_command::return_value::e_type::extension_field) {
            vec<string> vec_tmp;
            for (size_t i = 0; i < current_template.ifiles.size(); ++i)
                vec_tmp.push_back(
                    get_name_output_file(_context->current_work_directory + "/" + call_component->pattern_ret_files,
                                         current_template.ifiles[i]));
            if (auto it = target.ext.get_if<vec<string>>(current_template.returnable.value); it)
                it->insert(it->end(), vec_tmp.begin(), vec_tmp.end());
            else
                target.ext.get_fields()[current_template.returnable.value] = vec_tmp;
        }
    }
}
commands generator_command::generate() {
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

        string output_file = get_name_output_file(
            pattern_output_file, target.name,
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
            if (arg.type == sc::template_command::arg::e_type::trgfield) {
                if (auto value = target.ext.get_if<pdiff>(arg.value); value)
                    cmd.args.push_back(std::to_string(*value));
                else if (auto value = target.ext.get_if<string>(arg.value); value)
                    cmd.args.push_back(*value);
                else if (auto value = target.ext.get_if<vec<string>>(arg.value); value) {
                    const auto &str_s = *value;

                    for (size_t k = 0; k < str_s.size(); ++k)
                        cmd.args.push_back(arg.prefix + str_s[k]);
                }
                else
                    _log << (log_message(log_type::fatal) << "There is no such parameter: " << arg.value);
            }

            else if (arg.type == sc::template_command::arg::e_type::internal) {
                if (auto it = returnable_target.find(arg.value); it != returnable_target.end())
                    for (const auto &str : it->second)
                        cmd.args.push_back(arg.prefix + str);
                else if (auto it = internal_args_stack_tmp.find(arg.value); it != internal_args_stack_tmp.end())
                    for (const auto &str : it->second)
                        cmd.args.push_back(arg.prefix + str);
                else
                    _log << (log_message(log_type::fatal) << "The parameter was not passed \'" << arg.value << "\'.");

                add_depends_cmd(arg.value);
            }
            else if (arg.type == sc::template_command::arg::e_type::string) {
                if (arg.value == sc::template_command::FEATURE_INPUT_FILE) {
                    if (!current_template.ifiles.size())
                        continue;
                    if (current_template.single_generates) {
                        if (should_uses_src_file(current_template.ifiles[count_use_ifiles], output_file,
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
                            if (should_uses_src_file(current_template.ifiles[count_use_ifiles],
                                                     get_name_output_file(pattern_output_file, target.name,
                                                                          current_template.ifiles[count_use_ifiles],
                                                                          count_use_ifiles),
                                                     _context->dfiles[current_template.ifiles[count_use_ifiles]]) ||
                                current_template.returns_target) {
                                cmd.args.push_back(current_template.ifiles[count_use_ifiles]);
                                ++real_count_use_ifiles;
                            }
                }
                else if (arg.value == sc::template_command::FEATURE_OUTPUT_FILE) {
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
                                output_file = get_name_output_file(pattern_output_file, target.name,
                                                                   current_template.ifiles[k], k);
                                if (current_template.returnable.type ==
                                    sc::template_command::return_value::e_type::object)
                                    internal_args_stack_tmp[current_template.returnable.value].push_back(output_file);

                                cmd.args.push_back(output_file);
                            }
                    }
                }
                else if (arg.value == sc::template_command::FEATURE_DEPENDENCIES) {
                    for (const auto &dependency : target.dependencies) {
                        if (auto it = returnable_target.find(dependency); it != returnable_target.end())
                            for (const auto &str : it->second)
                                cmd.args.push_back(arg.prefix + str);
                        else
                            _log << (log_message(log_type::fatal)
                                     << "The '" << arg.value << "' dependency was not found.\'");
                    }
                }
                else
                    cmd.args.push_back(arg.value);
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

            _log << (log_message(log_type::success) << "Generated: " << cmd.build_string_command());
        }

        if (current_template.single_generates && count_use_ifiles < current_template.ifiles.size())
            continue;

        count_use_ifiles = 0;

        ++j;
    }
    return cmd_s;
}
string generator_command::get_name_output_file(string pattern_file, string_v name_target, string_v name_file,
                                               size_t index) {
    size_t it;
    if (it = pattern_file.find("{}"); it != pattern_file.npos) {
        string extension = pattern_file.substr(it + 2);
        pattern_file.erase(it);
        pattern_file += fs::path(name_file).filename().string() + extension;
    }
    else if (it = pattern_file.find("{target_name}"); it != pattern_file.npos) {
        pattern_file.erase(it, it + 8);
        pattern_file += name_target;
    }
    else if (it = pattern_file.find("{index_file}"); it != pattern_file.npos) {
        pattern_file.erase(it, it + 7);
        pattern_file += std::to_string(index);
    }
    else
        pattern_file += index ? std::to_string(index) : "";

    return pattern_file;
}

bool generator_command::should_uses_src_file(string_v src_file, string_v output_file, const uset<string> &dfiles) {
    if (fs::is_regular_file(output_file) && fs::last_write_time(CACHE_FILE) > fs::last_write_time(output_file))
        return 1;
    else if (!fs::is_regular_file(output_file) || fs::last_write_time(output_file) < fs::last_write_time(src_file))
        return 1;

    for (const auto &dfile : dfiles)
        if (fs::last_write_time(output_file) < fs::last_write_time(dfile))
            return 1;

    return 0;
}

string command::build_string_command() {
    string cmd_str = name_program + " ";
    for (const auto &arg : args)
        cmd_str += arg + " ";

    return cmd_str;
}
