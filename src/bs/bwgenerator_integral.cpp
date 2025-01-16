#include "bwgenerator_integral.hpp"
#include "bwgenerator_api.hpp"
#include "bwgntools.hpp"
#include "tools/bwfile.hpp"

using namespace bweas;
using namespace generator_api;
using namespace generator;
using namespace bweas::bwexception;

integral_generator::integral_generator(func_generator _generator_p, func_get_files_input _files_input_p) {
    if (!init_glob_gnint) {
        assist.add_err("BWS-GNRT001", "Internal global argument not found");
        assist.add_err("BWS-GNRT002", "This generator does not have the specified features");
        init_glob_gnint = 1;
    }
    generator_p = _generator_p;
    files_input_p = _files_input_p;
}

void integral_generator::init() {
}
void integral_generator::deleteGenerator() {
    delete this;
}
std::map<std::string, std::vector<std::string>> integral_generator::input_files(const var::struct_sb::target_out &trg,
                                                                                const bwqueue_templates &templates,
                                                                                std::string dir_work_endv) {
    return files_input_p(trg, templates, *ccmp_p, dir_work_endv);
}

std::map<std::string, std::string> integral_generator::gen_commands(
    const var::struct_sb::target_out &trg, bwqueue_templates &templates, std::string dir_work_endv,
    std::map<std::string, std::vector<std::string>> files_input) {
    return generator_p(trg, templates, *ccmp_p, files_input, dir_work_endv);
}

std::map<std::string, std::vector<std::string>> bweas::generator::bwfile_inputs(
    const var::struct_sb::target_out &target, const bwqueue_templates &target_queue_templates,
    const std::vector<var::struct_sb::call_component> &ccmp_p, std::string dir_work_endv) {
    std::map<std::string, std::vector<std::string>> input_files;
    u32t count_uses_input_files = 0;

    for (const auto &template_tmp : target_queue_templates) {
        const auto &call_component_tmp =
            std::find_if(ccmp_p.begin(), ccmp_p.end(), [template_tmp](const var::struct_sb::call_component &ccmp_tmp) {
                return ccmp_tmp.name == template_tmp.name_call_component;
            });

        for (const auto &arg : template_tmp.args) {
            var::struct_sb::template_command::arg current_arg = arg;
            if (current_arg.arg_t == var::struct_sb::template_command::arg::type::trgfield &&
                current_arg.str_arg.find(NAME_FIELD_PROJECT_SRC_FILES) == 0) {
                std::string mask;
                u32t it_str = current_arg.str_arg.find(":");
                if (it_str != current_arg.str_arg.npos) {
                    mask = current_arg.str_arg;
                    mask.erase(0, it_str + 1);
                }

                if (std::atoll(mask.c_str()) != 0) {
                    if (std::atoll(mask.c_str()) == 1) {
                        while (count_uses_input_files < target.prj.src_files.size()) {
                            if (std::filesystem::last_write_time(target.prj.src_files[count_uses_input_files]) >
                                std::filesystem::last_write_time(tools::get_name_output_file(
                                    call_component_tmp->pattern_ret_files, count_uses_input_files, dir_work_endv)))
                                input_files[template_tmp.name].push_back(target.prj.src_files[count_uses_input_files]);
                            ++count_uses_input_files;
                        }
                    }
                    else {
                        for (u32t k = 0; k < target.prj.src_files.size() && k < std::atoll(mask.c_str());
                             ++k, ++count_uses_input_files) {
                            if (std::filesystem::last_write_time(target.prj.src_files[k]) >
                                std::filesystem::last_write_time(tools::get_name_output_file(
                                    call_component_tmp->pattern_ret_files, k, dir_work_endv)))
                                input_files[template_tmp.name].push_back(target.prj.src_files[k]);
                        }
                    }
                }
                current_arg.str_arg = "";

                std::vector<std::string> slc_files = bwfile::file_slc_mask(mask, target.prj.src_files);
                for (u32t i = 0; i < slc_files.size(); ++i) {
                    if (std::filesystem::last_write_time(slc_files[i]) >
                        std::filesystem::last_write_time(
                            tools::get_name_output_file(call_component_tmp->pattern_ret_files, i, dir_work_endv)))
                        input_files[template_tmp.name].push_back(slc_files[i]);
                    ++count_uses_input_files;
                }
            }
            else if (arg.arg_t == var::struct_sb::template_command::arg::type::features &&
                     arg.str_arg.find(FEATURE_FIELD_BS_CURRENT_IF) == 0) {
                while (count_uses_input_files < target.prj.src_files.size()) {
                    if (!std::filesystem::exists(tools::get_name_output_file(call_component_tmp->pattern_ret_files,
                                                                             count_uses_input_files, dir_work_endv)) ||
                        std::filesystem::last_write_time(target.prj.src_files[count_uses_input_files]) >
                            std::filesystem::last_write_time(tools::get_name_output_file(
                                call_component_tmp->pattern_ret_files, count_uses_input_files, dir_work_endv)))
                        input_files[template_tmp.name].push_back(target.prj.src_files[count_uses_input_files]);
                    ++count_uses_input_files;
                }
            }
        }
    }

    return input_files;
}

std::map<std::string, std::string> bweas::generator::bwgenerator(
    const var::struct_sb::target_out &trg, bwqueue_templates &templates,
    const std::vector<var::struct_sb::call_component> &ccmp_p,
    std::map<std::string, std::vector<std::string>> input_files, std::string dir_work_endv) {
    std::map<std::string, std::string> cmd_s;
    command current_cmd;

    std::map<std::string, std::vector<std::string>> global_internal_args;

    std::string input_file, output_file;

    u32t used_input_files = 0;
    u32t used_input_files_in_curr_template = 0;
    u32t used_output_files = 0;

    bool generate_single_input = 0;
    bool is_ind_template = 0;

    while (templates.size() || generate_single_input) {
        auto &current_template = templates[0];
        const auto &current_ccmp = std::find_if(ccmp_p.begin(), ccmp_p.end(),
                                                [current_template](const var::struct_sb::call_component &ccmp_tmp) {
                                                    return ccmp_tmp.name == current_template.name_call_component;
                                                });
    generate:
        if (used_input_files < input_files[current_template.name].size()) {
            u32t ind_file = 0;
            for (; ind_file < trg.prj.src_files.size(); ++ind_file)
                if (input_files[current_template.name][used_input_files] == trg.prj.src_files[ind_file])
                    break;
            input_file = input_files[current_template.name][used_input_files];
            output_file = tools::get_name_output_file(current_ccmp->pattern_ret_files, ind_file, dir_work_endv);
            ++used_input_files;
            is_ind_template = 1;
        }
        else if (current_template.returnable == var::struct_sb::target_t_str(trg.target_t)) {
            output_file = tools::get_name_output_file(current_ccmp->pattern_ret_files,
                                                      used_input_files_in_curr_template++, dir_work_endv);
            if (!std::filesystem::exists(output_file) || used_input_files)
                is_ind_template = 1;
        }
        else {
            for (; used_output_files < trg.prj.src_files.size(); ++used_output_files) {
                if (input_files[current_template.name].size() == 0)
                    goto init_ofile;
                for (u32t i = 0; i < input_files[current_template.name].size(); ++i)
                    if (input_files[current_template.name][i] != trg.prj.src_files[used_output_files])
                        goto init_ofile;
            }
        init_ofile:
            output_file =
                tools::get_name_output_file(current_ccmp->pattern_ret_files, used_output_files, dir_work_endv);
            ++used_output_files;
        }
        current_cmd = current_ccmp->name_program + " ";

        for (auto arg : current_template.args) {
            if (arg.arg_t == var::struct_sb::template_command::arg::type::internal) {
                const auto &current_internal_args = global_internal_args.find(arg.str_arg);
                if (current_internal_args == global_internal_args.end())
                    throw bwgenerator_excp("Parameter in the template(" + current_template.name + ") - " + arg.str_arg,
                                           "001");
                arg.str_arg = "";
                for (const std::string &file : current_internal_args->second)
                    arg.str_arg += file + " ";
            }
            else if (arg.arg_t != var::struct_sb::template_command::arg::type::string) {
                if (arg.str_arg == FEATURE_FIELD_BS_CURRENT_IF) {
                    arg.str_arg = input_file;
                    ++used_input_files_in_curr_template;
                    generate_single_input = 1;
                }
                else if (arg.str_arg.find(FEATURE_FIELD_BS_CURRENT_OF) == 0) {
                    size_t attr_it = arg.str_arg.find(":");
                    if (attr_it == arg.str_arg.npos) {
                        arg.str_arg = "";
                        for (u32t i = 0; i < used_input_files_in_curr_template; ++i) {
                            std::string name_file =
                                tools::get_name_output_file(current_ccmp->pattern_ret_files, i, dir_work_endv);
                            arg.str_arg += name_file + " ";
                            global_internal_args[current_template.returnable].push_back(name_file);
                        }
                    }
                    else {
                        std::string attribute = arg.str_arg;
                        attribute.erase(0, attr_it + 1);
                        size_t val_attr = std::atoll(attribute.c_str());
                        if (val_attr != 0) {
                            if (val_attr == 1) {
                                arg.str_arg = output_file;
                                global_internal_args[current_template.returnable].push_back(arg.str_arg);
                            }
                            else {
                                arg.str_arg = "";
                                for (u32t i = 0; i < val_attr && i < input_files.size(); ++i) {
                                    std::string name_file =
                                        tools::get_name_output_file(current_ccmp->pattern_ret_files, i, dir_work_endv);
                                    arg.str_arg += name_file + " ";
                                    global_internal_args[current_template.returnable].push_back(name_file);
                                }
                            }
                        }
                        else if (attribute == "0")
                            continue;
                        else {
                            std::vector<std::string> files =
                                bwfile::file_slc_mask(attribute, input_files[current_template.name]);
                            arg.str_arg = "";
                            for (const auto &file : files) {
                                arg.str_arg += file + " ";
                                global_internal_args[current_template.returnable].push_back(file);
                            }
                        }
                    }
                }
                else if (arg.str_arg.find(NAME_FIELD_PROJECT_SRC_FILES) == 0) {
                    size_t attr_it = arg.str_arg.find(":");
                    if (attr_it == arg.str_arg.npos) {
                        arg.str_arg = "";
                        for (u32t i = 0; i < trg.prj.src_files.size(); ++i) {
                            arg.str_arg += trg.prj.src_files[i] + " ";
                            global_internal_args[current_template.returnable].push_back(trg.prj.src_files[i]);
                        }
                    }
                    else {
                        std::string attribute = arg.str_arg;
                        attribute.erase(0, attr_it + 1);
                        size_t val_attr = std::atoll(attribute.c_str());
                        if (val_attr != 0) {
                            if (val_attr == 1) {
                                arg.str_arg = trg.prj.src_files[used_input_files];
                                global_internal_args[current_template.returnable].push_back(arg.str_arg);
                            }
                            else {
                                arg.str_arg = "";
                                for (u32t i = 0; i < val_attr && i < trg.prj.src_files.size(); ++i) {
                                    arg.str_arg += trg.prj.src_files[i] + " ";
                                    global_internal_args[current_template.returnable].push_back(trg.prj.src_files[i]);
                                }
                            }
                        }
                        else if (attribute == "0")
                            continue;
                        else {
                            std::vector<std::string> files = bwfile::file_slc_mask(attribute, trg.prj.src_files);
                            arg.str_arg = "";
                            for (const auto &file : files) {
                                arg.str_arg += file + " ";
                                global_internal_args[current_template.returnable].push_back(file);
                            }
                        }
                    }
                }
                else
                    throw bwgenerator_excp("Parameter in the template(" + current_template.name + ") - " + arg.str_arg,
                                           "002");
            }
            current_cmd += arg.str_arg + " ";
        }
        assist.next_output_important();
        assist << "GENERATED COMMAND: " + current_cmd;

        if (is_ind_template)
            cmd_s.emplace(output_file, current_cmd);
        current_cmd.clear();

        if (!generate_single_input || used_input_files_in_curr_template == trg.prj.src_files.size()) {
            generate_single_input = 0;
            used_input_files_in_curr_template = 0;
            templates.erase(templates.begin());
        }
        else {
            input_file = "";
            output_file = "";
            is_ind_template = 0;
            goto generate;
        }
    }

    return cmd_s;
}