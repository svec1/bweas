//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include <bwgntools.hpp>

using namespace bweas;

string generator_tools::get_name_output_file(string pattern_file, size_t index, string dir_work_endv) {
    if (pattern_file.find(".") == pattern_file.npos)
        return dir_work_endv + "/" + pattern_file + std::to_string(index);

    string name_output_file_curr = pattern_file, extension_output_file_curr = pattern_file;

    name_output_file_curr.erase(name_output_file_curr.find("."), name_output_file_curr.size());
    extension_output_file_curr.erase(0, extension_output_file_curr.find("."));

    if (index != 0)
        return dir_work_endv + "/" + name_output_file_curr + std::to_string(index) + extension_output_file_curr;
    return dir_work_endv + "/" + name_output_file_curr + extension_output_file_curr;
}

bool generator_tools::should_uses_src_file(string_v src_file, string_v output_file, const uset<string> &dfiles) {
    if (fs::is_regular_file(output_file) && fs::last_write_time(CACHE_FILE) > fs::last_write_time(output_file))
        return 1;
    else if (!fs::is_regular_file(output_file) || fs::last_write_time(output_file) < fs::last_write_time(src_file))
        return 1;

    for (const auto &dfile : dfiles)
        if (fs::last_write_time(output_file) < fs::last_write_time(dfile))
            return 1;

    return 0;
}

void generator_tools::parse_basic_args(const sc::target_out &target, vec<sc::template_command> &target_queue_templates,
                                       const vec<pair<string, string>> &global_extern_args) {
    for (auto &trg_template : target_queue_templates) {
        for (size_t i = 0; i < trg_template.args.size(); ++i) {
            sc::template_command::arg current_arg = trg_template.args[i];
            if (current_arg.arg_t == sc::template_command::arg::type::string ||
                current_arg.arg_t == sc::template_command::arg::type::features ||
                current_arg.arg_t == sc::template_command::arg::type::internal)
                continue;
            else if (current_arg.arg_t == sc::template_command::arg::type::extglobal) {
                const auto &extern_arg = std::find_if(global_extern_args.begin(), global_extern_args.end(),
                                                      [current_arg](const std::pair<string, string> extern_arg_tmp) {
                                                          return extern_arg_tmp.first == current_arg.str_arg;
                                                      });
                if (extern_arg == global_extern_args.end())
                    throw std::runtime_error("The specified external parameter does not exist: " + current_arg.str_arg);

                current_arg.str_arg = extern_arg->second;
            }
            else if (current_arg.arg_t == sc::template_command::arg::type::trgfield) {
                if (current_arg.str_arg == NAME_FIELD_TARGET_NAME)
                    current_arg.str_arg = target.name;
                else if (current_arg.str_arg == NAME_FIELD_TARGET_LIBS) {
                    trg_template.args.erase(trg_template.args.begin() + i);
                    for (size_t k = 0; k < target.target_vec_libs.size(); ++k)
                        trg_template.args.emplace(trg_template.args.begin() + i, "-l" + target.target_vec_libs[k],
                                                  sc::template_command::arg::type::string);
                    continue;
                }
                else if (current_arg.str_arg == NAME_FIELD_PROJECT_INCLUDE_PATHS) {
                    trg_template.args.erase(trg_template.args.begin() + i);
                    for (size_t k = 0; k < target.prj.include_paths.size(); ++k)
                        trg_template.args.emplace(trg_template.args.begin() + i, "-I" + target.prj.include_paths[k],
                                                  sc::template_command::arg::type::string);
                    continue;
                }
                else if (current_arg.str_arg == NAME_FIELD_TARGET_TYPE)
                    current_arg.str_arg = sc::target_type_str(target.type);
                else if (current_arg.str_arg == NAME_FIELD_TARGET_CFG)
                    current_arg.str_arg = sc::target_cfg_str(target.cfg);
                else if (current_arg.str_arg == NAME_FIELD_TARGET_VER)
                    current_arg.str_arg = target.ver.get_str_version();
                else if (current_arg.str_arg == NAME_FIELD_PROJECT_NAME)
                    current_arg.str_arg = target.prj.name;
                else if (current_arg.str_arg == NAME_FIELD_PROJECT_VER)
                    current_arg.str_arg = target.prj.ver.get_str_version();
                else if (current_arg.str_arg == NAME_FIELD_PROJECT_LANG)
                    current_arg.str_arg = target.prj.language;
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
                else if (current_arg.str_arg.find(NAME_FIELD_PROJECT_SRC_FILES) != current_arg.str_arg.npos)
                    continue;
                else
                    throw std::runtime_error("There is no such parameter: " + current_arg.str_arg);
            }

            current_arg.arg_t = sc::template_command::arg::type::string;
        }
    }
}

string generator_tools::build_string_command(const generator_api::command &cmd) {
    string cmd_str = cmd.name_program + " ";
    for (const auto &arg : cmd.args)
        cmd_str += arg + " ";

    return cmd_str;
}
