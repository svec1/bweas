#include "bwgntools.hpp"

using namespace bweas;

std::string generator::tools::get_name_output_file(std::string pattern_file, u32t index, std::string dir_work_endv) {
    if (pattern_file.find(".") == pattern_file.npos)
        return dir_work_endv + "/" + pattern_file + std::to_string(index);
    std::string name_output_file_curr = pattern_file, extension_output_file_curr = pattern_file;
    name_output_file_curr.erase(name_output_file_curr.find("."), name_output_file_curr.size());
    extension_output_file_curr.erase(0, extension_output_file_curr.find("."));
    if (index != 0)
        return dir_work_endv + "/" + name_output_file_curr + std::to_string(index) + extension_output_file_curr;
    return dir_work_endv + "/" + name_output_file_curr + extension_output_file_curr;
}

void generator::tools::parse_basic_args(const var::struct_sb::target_out &target,
                                        bwqueue_templates &target_queue_templates, const bwargs &global_extern_args) {
    for (auto &trg_template : target_queue_templates) {
        for (u32t i = 0; i < trg_template.args.size(); ++i) {
            var::struct_sb::template_command::arg &current_arg = trg_template.args[i];
            if (current_arg.arg_t == var::struct_sb::template_command::arg::type::string ||
                current_arg.arg_t == var::struct_sb::template_command::arg::type::features ||
                current_arg.arg_t == var::struct_sb::template_command::arg::type::internal)
                continue;
            else if (current_arg.arg_t == var::struct_sb::template_command::arg::type::extglobal) {
                const auto &extern_arg =
                    std::find_if(global_extern_args.begin(), global_extern_args.end(),
                                 [current_arg](const std::pair<std::string, std::string> extern_arg_tmp) {
                                     return extern_arg_tmp.first == current_arg.str_arg;
                                 });
                if (extern_arg == global_extern_args.end())
                    throw bwexception::bwgenerator_excp("[" + trg_template.name +
                                                            "] The specified external parameter does not exist - " +
                                                            current_arg.str_arg,
                                                        "000");

                current_arg.str_arg = extern_arg->second;
            }
            else if (current_arg.arg_t == var::struct_sb::template_command::arg::type::trgfield) {
                if (current_arg.str_arg == NAME_FIELD_TARGET_NAME)
                    current_arg.str_arg = target.name_target;
                else if (current_arg.str_arg == NAME_FIELD_TARGET_LIBS) {
                    current_arg.str_arg = "";
                    for (u32t k = 0; k < target.target_vec_libs.size(); ++k) {
                        current_arg.str_arg += target.target_vec_libs[k];
                        if (k < target.target_vec_libs.size() - 1)
                            current_arg.str_arg += " ";
                    }
                }
                else if (current_arg.str_arg == NAME_FIELD_TARGET_TYPE)
                    current_arg.str_arg = var::struct_sb::target_t_str(target.target_t);
                else if (current_arg.str_arg == NAME_FIELD_TARGET_CFG)
                    current_arg.str_arg = var::struct_sb::cfg_str(target.target_cfg);
                else if (current_arg.str_arg == NAME_FIELD_TARGET_VER)
                    current_arg.str_arg = target.version_target.get_str_version();
                else if (current_arg.str_arg == NAME_FIELD_PROJECT_NAME)
                    current_arg.str_arg = target.prj.name_project;
                else if (current_arg.str_arg == NAME_FIELD_PROJECT_VER)
                    current_arg.str_arg = target.prj.version_project.get_str_version();
                else if (current_arg.str_arg == NAME_FIELD_PROJECT_LANG)
                    current_arg.str_arg = var::struct_sb::lang_str(target.prj.lang);
                else if (current_arg.str_arg == NAME_FIELD_PROJECT_PCOMPILER)
                    current_arg.str_arg = target.prj.path_compiler;
                else if (current_arg.str_arg == NAME_FIELD_PROJECT_PLINKER)
                    current_arg.str_arg = target.prj.path_linker;
                else if (current_arg.str_arg == NAME_FIELD_PROJECT_RFCOMPILER)
                    current_arg.str_arg = target.prj.rflags_compiler;
                else if (current_arg.str_arg == NAME_FIELD_PROJECT_RFLINKER)
                    current_arg.str_arg = target.prj.rflags_linker;
                else if (current_arg.str_arg == NAME_FIELD_PROJECT_DFCOMPILER)
                    current_arg.str_arg = target.prj.dflags_compiler;
                else if (current_arg.str_arg == NAME_FIELD_PROJECT_DFLINKER)
                    current_arg.str_arg = target.prj.dflags_linker;
                else if (current_arg.str_arg == NAME_FIELD_PROJECT_STD_C)
                    current_arg.str_arg = std::to_string(target.prj.standart_c);
                else if (current_arg.str_arg == NAME_FIELD_PROJECT_STD_CPP)
                    current_arg.str_arg = std::to_string(target.prj.standart_cpp);
                else if (current_arg.str_arg.find(NAME_FIELD_PROJECT_SRC_FILES) == 0)
                    continue;
                else
                    throw bwexception::bwgenerator_excp(
                        "[" + trg_template.name + "] There is no such parameter - " + current_arg.str_arg, "000");
            }

            current_arg.arg_t = var::struct_sb::template_command::arg::type::string;
        }
    }
}