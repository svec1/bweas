#include "bw_defs.hpp"
#include "bwgenerator.hpp"
#include "lang/static_struct.hpp"
#include "tools/bwfile.hpp"

using namespace bweas;
using namespace exception;

std::string tools_generator::get_file_w_index(std::string pattern_file, u32t index) {
    std::string name_output_file_curr = pattern_file, extension_output_file_curr = pattern_file;
    name_output_file_curr.erase(name_output_file_curr.find("."), name_output_file_curr.size());
    extension_output_file_curr.erase(0, extension_output_file_curr.find("."));
    if (index != 0)
        return name_output_file_curr + std::to_string(index) + extension_output_file_curr;
    return name_output_file_curr + extension_output_file_curr;
}

bwGeneratorIntegral::bwGeneratorIntegral(
    commands (*generator)(const var::struct_sb::target_out &, bwqueue_templates &,
                          const std::vector<var::struct_sb::call_component> &ccmp_p, const bwargs &)) {
    if (!init_glob_gnint) {

        init_glob_gnint = 1;
    }
    func_p = generator;
}

void bwGeneratorIntegral::init() {
}
void bwGeneratorIntegral::deleteGenerator() {
    delete this;
}
commands bwGeneratorIntegral::gen_commands(const var::struct_sb::target_out &trg, bwqueue_templates &templates) {
    return func_p(trg, templates, *ccmp_p, *external_args_p);
}

commands bweas::bwign0_1v(const var::struct_sb::target_out &trg, bwqueue_templates &templates,
                          const std::vector<var::struct_sb::call_component> &ccmp_p, const bwargs &global_extern_args) {
    commands cmd_s;

    std::string command;
    bwargs_files global_internal_args;

    bool template_for_ev_files = 0;
    u32t left_src_files = trg.prj.src_files.size();
    u32t count_uses_input_files = 0, count_uses_output_pattern_files = 0;
    std::string input_file_curr, output_files_curr;

    bwarg_files return_internal_arg;

    // generating command on current a template
    while (templates.size() || template_for_ev_files) {
        if (template_for_ev_files && left_src_files == 0) {
            template_for_ev_files = 0;
            count_uses_output_pattern_files = 0;

            global_internal_args.push_back(return_internal_arg);
            return_internal_arg.second.clear();

            templates.erase(templates.begin());
            if (!ccmp_p.size())
                break;
        }

        const auto &template_tmp = templates[0];
        const auto &call_component_tmp =
            std::find_if(ccmp_p.begin(), ccmp_p.end(), [template_tmp](const var::struct_sb::call_component &ccmp_tmp) {
                return ccmp_tmp.name == template_tmp->name_call_component;
            });

        if (!template_for_ev_files)
            return_internal_arg.first = template_tmp->returnable;

        std::string current_arg;

        command += call_component_tmp->name_program + " ";
        if (left_src_files != 0)
            input_file_curr = trg.prj.src_files[trg.prj.src_files.size() - left_src_files];

        if (call_component_tmp->pattern_ret_files.find(".") == call_component_tmp->pattern_ret_files.npos)
            output_files_curr =
                call_component_tmp->pattern_ret_files + std::to_string(trg.prj.src_files.size() - left_src_files);
        else
            output_files_curr = tools_generator::get_file_w_index(call_component_tmp->pattern_ret_files,
                                                                  trg.prj.src_files.size() - left_src_files);

        for (u32t j = 0; j < template_tmp->name_args.size(); ++j) {
            current_arg = template_tmp->name_args[j];
            if (current_arg.find("{") == current_arg.npos) {
                const auto &extern_arg =
                    std::find_if(global_extern_args.begin(), global_extern_args.end(),
                                 [current_arg](const std::pair<std::string, std::string> extern_arg_tmp) {
                                     return extern_arg_tmp.first == current_arg;
                                 });
                if (extern_arg == global_extern_args.end())
                    throw bwbuilder_excp("[" + template_tmp->name +
                                             "] The specified external parameter does not exist - " + current_arg,
                                         "002");

                command += extern_arg->second + " ";
                continue;
            }
            else {
                if (current_arg.find("}") == current_arg.npos)
                    throw bwbuilder_excp(
                        "[" + template_tmp->name + "] Incorrect use of arguments in template - " + current_arg, "002");
                if (current_arg.find("STR{") == 0) {
                    current_arg.erase(0, 4);
                    current_arg.erase(current_arg.find("}"));
                }
                else if (current_arg.find("ACP{") == 0) {
                    current_arg.erase(0, 4);
                    current_arg.erase(current_arg.find("}"));

                    const auto &it =
                        find_if(global_internal_args.begin(), global_internal_args.end(),
                                [current_arg](const std::pair<std::string, std::vector<std::string>> internal_arg) {
                                    return current_arg == internal_arg.first;
                                });
                    if (it == global_internal_args.end())
                        throw bwbuilder_excp("[" + template_tmp->name +
                                                 "] The specified internal parameter does not exist - " + current_arg,
                                             "002");
                    const auto &it_str = it->first.find(":");
                    if (it_str == it->first.npos) {
                        current_arg = "";
                        for (u32t k = 0; k < it->second.size(); ++k) {
                            current_arg += it->second[k];
                            if (k < it->second.size() - 1)
                                current_arg += " ";
                        }

                        count_uses_input_files = it->second.size();
                    }
                    else {
                        current_arg.erase(0, it_str + 1);
                        std::string mask = current_arg;

                        if (std::atoll(mask.c_str()) != 0) {
                            if (std::atoll(mask.c_str()) == 1) {
                                current_arg = input_file_curr;
                                template_for_ev_files = 1;
                            }
                            else {
                                for (u32t k = 0; k < it->second.size() && k < std::atoll(mask.c_str());
                                     ++k, ++count_uses_input_files) {
                                    current_arg += it->second[k];
                                    if (k < it->second.size() - 1)
                                        current_arg += " ";
                                }
                            }
                            goto add_current_args_to_cmd;
                        }
                        current_arg = "";
                        std::vector<std::string> slc_files = bwfile::file_slc_mask(mask, it->second);
                        for (u32t k = 0; k < slc_files.size(); ++k) {
                            current_arg += slc_files[k];
                            if (k < slc_files.size() - 1)
                                current_arg += " ";
                        }

                        count_uses_input_files += slc_files.size();
                    }
                }
                else if (current_arg.find("FTRS{") == 0) {
                    current_arg.erase(0, 5);
                    current_arg.erase(current_arg.find("}"));

                    std::string attribute;
                    size_t it_str = current_arg.find(":");
                    if (it_str != current_arg.npos) {
                        attribute = current_arg;
                        attribute.erase(0, it_str + 1);
                        current_arg.erase(it_str, attribute.size() + 1);
                    }

                    if (current_arg == FEATURE_FIELD_BS_CURRENT_IF && left_src_files != 0)
                        current_arg = input_file_curr;
                    else if (current_arg == FEATURE_FIELD_BS_CURRENT_OF) {
                        if (attribute == "all" || attribute.empty()) {
                            if (left_src_files < trg.prj.src_files.size() && left_src_files > 0) {
                                current_arg = output_files_curr;
                                return_internal_arg.second.push_back(output_files_curr);
                            }
                            else {
                                std::string tmp_output_file_index;

                                current_arg = "";
                                for (; count_uses_output_pattern_files < count_uses_input_files;
                                     ++count_uses_output_pattern_files) {
                                    tmp_output_file_index = tools_generator::get_file_w_index(
                                        call_component_tmp->pattern_ret_files, count_uses_output_pattern_files);

                                    current_arg += tmp_output_file_index;
                                    return_internal_arg.second.push_back(tmp_output_file_index);

                                    if (count_uses_output_pattern_files < count_uses_input_files - 1)
                                        current_arg += " ";
                                }
                            }
                        }
                        else if (attribute == "1") {
                            if (left_src_files != 0)
                                current_arg = output_files_curr;
                            else
                                current_arg = tools_generator::get_file_w_index(call_component_tmp->pattern_ret_files,
                                                                                count_uses_output_pattern_files);
                            return_internal_arg.second.push_back(output_files_curr);
                        }
                        else
                            throw bwbuilder_excp("[" + template_tmp->name +
                                                     "] The specified attribute of feature (field) does not "
                                                     "match the possible ones - " +
                                                     current_arg + ": attr[" + attribute + "]",
                                                 "002");

                        goto add_current_args_to_cmd;
                    }
                    else
                        throw bwbuilder_excp("[" + template_tmp->name +
                                                 "] The specified feature (field) does not exist - " + current_arg,
                                             "002");
                    template_for_ev_files = 1;
                }
                else if (current_arg.find("TRG{") == 0) {
                    current_arg.erase(0, 4);
                    current_arg.erase(current_arg.find("}"));
                    if (current_arg == NAME_FIELD_TARGET_NAME)
                        current_arg = trg.name_target;
                    else if (current_arg == NAME_FIELD_TARGET_LIBS) {
                        current_arg = "";
                        for (u32t k = 0; k < trg.target_vec_libs.size(); ++k) {
                            current_arg += trg.target_vec_libs[k];
                            if (k < trg.target_vec_libs.size() - 1)
                                current_arg += " ";
                        }
                    }
                    else if (current_arg == NAME_FIELD_TARGET_TYPE)
                        current_arg = var::struct_sb::target_t_str(trg.target_t);
                    else if (current_arg == NAME_FIELD_TARGET_CFG)
                        current_arg = var::struct_sb::cfg_str(trg.target_cfg);
                    else if (current_arg == NAME_FIELD_TARGET_VER)
                        current_arg = trg.version_target.get_str_version();
                    else if (current_arg == NAME_FIELD_PROJECT_NAME)
                        current_arg += trg.prj.name_project;
                    else if (current_arg == NAME_FIELD_PROJECT_VER)
                        current_arg += trg.prj.version_project.get_str_version();
                    else if (current_arg == NAME_FIELD_PROJECT_LANG)
                        current_arg += var::struct_sb::lang_str(trg.prj.lang);
                    else if (current_arg == NAME_FIELD_PROJECT_PCOMPILER)
                        current_arg += trg.prj.path_compiler;
                    else if (current_arg == NAME_FIELD_PROJECT_PLINKER)
                        current_arg += trg.prj.path_linker;
                    else if (current_arg == NAME_FIELD_PROJECT_RFCOMPILER)
                        current_arg += trg.prj.rflags_compiler;
                    else if (current_arg == NAME_FIELD_PROJECT_RFLINKER)
                        current_arg += trg.prj.rflags_linker;
                    else if (current_arg == NAME_FIELD_PROJECT_DFCOMPILER)
                        current_arg += trg.prj.dflags_compiler;
                    else if (current_arg == NAME_FIELD_PROJECT_DFLINKER)
                        current_arg += trg.prj.dflags_linker;
                    else if (current_arg == NAME_FIELD_PROJECT_STD_C)
                        current_arg += std::to_string(trg.prj.standart_c);
                    else if (current_arg == NAME_FIELD_PROJECT_STD_CPP)
                        current_arg += std::to_string(trg.prj.standart_cpp);
                    else if (current_arg.find(NAME_FIELD_PROJECT_SRC_FILES) == 0) {
                        const auto &it_str = current_arg.find(":");
                        if (it_str != current_arg.npos) {
                            current_arg.erase(0, it_str + 1);
                            std::string mask = current_arg;

                            if (std::atoll(mask.c_str()) != 0) {
                                if (std::atoll(mask.c_str()) == 1) {
                                    current_arg = input_file_curr;
                                    template_for_ev_files = 1;
                                    ++count_uses_input_files;
                                }
                                else {
                                    for (u32t k = 0; k < trg.prj.src_files.size() && k < std::atoll(mask.c_str());
                                         ++k, ++count_uses_input_files) {
                                        current_arg += trg.prj.src_files[k];
                                        if (k < trg.prj.src_files.size() - 1)
                                            current_arg += " ";
                                    }
                                }
                                goto add_current_args_to_cmd;
                            }
                            current_arg = "";

                            std::vector<std::string> slc_files = bwfile::file_slc_mask(mask, trg.prj.src_files);
                            for (u32t k = 0; k < slc_files.size(); ++k) {
                                current_arg += slc_files[k];
                                if (k < slc_files.size() - 1)
                                    current_arg += " ";
                            }

                            count_uses_input_files += slc_files.size();
                        }
                        else {
                            current_arg = "";
                            for (u32t k = 0; k < trg.prj.src_files.size(); ++k) {
                                current_arg += trg.prj.src_files[k];
                                if (k < trg.prj.src_files.size() - 1)
                                    current_arg += " ";
                            }

                            count_uses_input_files = trg.prj.src_files.size();
                        }
                    }
                    else
                        throw bwbuilder_excp("[" + template_tmp->name + "] There is no such parameter - " + current_arg,
                                             "002");
                }
            add_current_args_to_cmd:
                command += current_arg + " ";
            }
        }

        assist << "WAS GENERATED COMMAND: " + command;

        if (template_for_ev_files)
            --left_src_files;
        else {
            if (templates.size())
                templates.erase(templates.begin());
            global_internal_args.push_back(return_internal_arg);
            return_internal_arg.second.clear();
            count_uses_output_pattern_files = 0;
        }

        cmd_s.push_back(command);
        command.clear();
    }

    return cmd_s;
}