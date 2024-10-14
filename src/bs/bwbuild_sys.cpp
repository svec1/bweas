#include "bwbuild_sys.hpp"
#include "lang/static_struct.hpp"

#include <unordered_set>

#include <filesystem>

bwbuilder::bwbuilder() {
    if (!init_glob) {
        assist << "Initializing system build - bweas " + bwbuilde_ver.get_str_version();
        assist.add_err("BWS000", "Unable to open configuration file \'bweasconf.txt\'");
        assist.add_err("BWS001", "Incorrect cache structure");
    }
}

void
bwbuilder::start_build() {
    HND handle_f = assist.open_file(MAIN_FILE, MODE_READ_FILE);
    if (!assist.exist_file(handle_f))
        assist.call_err("BWS000");
    assist.close_file(handle_f);

    std::filesystem::path cache_file = std::filesystem::current_path() / CACHE_FILE;
    std::ifstream{cache_file.c_str()};

    if (std::filesystem::exists(cache_file)) {
        std::filesystem::path main_file = std::filesystem::current_path() / MAIN_FILE;
        std::ifstream{main_file.c_str()};

        std::filesystem::file_time_type cftime = std::filesystem::last_write_time(cache_file);
        std::filesystem::file_time_type mftime = std::filesystem::last_write_time(main_file);
        if (mftime >= cftime)
            goto interpreter_start;

        assist.next_output_important();
        assist << " - [BWEAS]: The configuration file will not be interpreted";

        deserl_cache();
    }
    else {
    interpreter_start:
        run_interpreter();
        load_target();
        gen_cache_target();
    }

    assist.next_output_important();
    assist << "*Build start time: " + assist.get_time();

    build_targets();
}

void
bwbuilder::switch_log(u32t value) {
    log = value;
    assist.switch_log(log);
}

void
bwbuilder::switch_output_log(u32t value) {
    output_log = value;
    assist.switch_otp(output_log);
}

void
bwbuilder::run_interpreter() {
    interpreter::interpreter_exec::config new_config;

    new_config.debug_output = 0;
    new_config.import_module = 1;
    new_config.filename_interp = MAIN_FILE;
    new_config.file_import_file_f = IMPORT_FILE;
    new_config.transmit_smt_name_func_with_smt = 1;
    new_config.use_external_scope = 0;

    _interpreter.set_config(new_config);

    assist << " - [BWEAS]: Interpreting the configuration file...";

    _interpreter.build_aef();
    _interpreter.interpreter_run();

    assist << " - [BWEAS]: The interpretation was successful!";
}

u32t
bwbuilder::gen_cache_target() {
    if (!out_targets.size()) {
        assist << "Generating a file with a cache will not be performed.";
        return 1;
    }
    assist << " - [BWEAS]: Generating a cache file...";

    HND handle_f = assist.open_file(CACHE_FILE, MODE_WRITE_TO_FILE);

    std::string serel_target_tmp;

    std::unordered_set<std::string> used_templates;
    std::unordered_set<std::string> all_used_globally_args;
    std::unordered_set<std::string> all_used_call_component;

    const auto &vec_global_template =
        _interpreter.get_current_scope().get_vector_variables_t<var::struct_sb::template_command>();
    const auto &vec_c_components =
        _interpreter.get_current_scope().get_vector_variables_t<var::struct_sb::call_component>();

    for (u32t i = 0; i < out_targets.size(); ++i) {
        char *prj_v = (char *)&out_targets[i];
        serel_target_tmp += std::to_string(out_targets[i].prj.src_files.size()) + " " +
                            std::to_string(out_targets[i].prj.vec_templates.size()) + " " +
                            std::to_string(out_targets[i].target_vec_libs.size()) + " ";
        for (u32t j = 0; j < sizeof(var::struct_sb::project); j += sizeof(std::string)) {

            // version project
            if (j == sizeof(std::string)) {
                serel_target_tmp += (*(var::struct_sb::version *)(prj_v + j)).get_str_version() + " " +
                                    std::to_string(*(int *)(prj_v + j + 12)) + " ";
                j += 16;
            }

            // standart c
            else if (j == sizeof(std::string) * 7 + 16) {
                serel_target_tmp += std::to_string(*(int *)(prj_v + j)) + " " +
                                    std::to_string(*(int *)(prj_v + j + 4)) + " " +
                                    std::to_string(*(int *)(prj_v + j + 5)) + " ";
                break;
            }
            if ((*(std::string *)(prj_v + j)).find(" ") != serel_target_tmp.npos)
                serel_target_tmp += "\"" + *(std::string *)(prj_v + j) + "\" ";
            else
                serel_target_tmp += *(std::string *)(prj_v + j) + " ";
        }
        for (u32t j = 0; j < out_targets[i].prj.src_files.size(); ++j)
            serel_target_tmp += out_targets[i].prj.src_files[j] + " ";
        for (u32t j = 0; j < out_targets[i].prj.vec_templates.size(); ++j) {
            used_templates.emplace(out_targets[i].prj.vec_templates[j]);
            serel_target_tmp += out_targets[i].prj.vec_templates[j] + " ";
        }

        serel_target_tmp += var::struct_sb::target_t_str(out_targets[i].target_t) + " " +
                            var::struct_sb::cfg_str(out_targets[i].target_cfg) + " " + out_targets[i].name_target +
                            " " + out_targets[i].version_target.get_str_version() + " ";
        for (u32t j = 0; j < out_targets[i].target_vec_libs.size(); ++j)
            serel_target_tmp += out_targets[i].target_vec_libs[j] + " ";
    }

    serel_target_tmp += "EOET" + std::to_string(vec_global_template.size()) + " "; // end of enum targets

    for (u32t i = 0; i < vec_global_template.size(); ++i) {
        serel_target_tmp += std::to_string(vec_global_template[i].second.name_accept_params.size()) + " " +
                            std::to_string(vec_global_template[i].second.name_args.size()) + " " +
                            vec_global_template[i].second.name + " " +
                            vec_global_template[i].second.name_call_component + " " +
                            vec_global_template[i].second.returnable + " ";

        all_used_call_component.emplace(vec_global_template[i].second.name_call_component);

        for (u32t j = 0; j < vec_global_template[i].second.name_accept_params.size(); ++j)
            serel_target_tmp += vec_global_template[i].second.name_accept_params[j] + " ";
        for (u32t j = 0; j < vec_global_template[i].second.name_args.size(); ++j) {
            if (vec_global_template[i].second.name_args[j].find('{') == vec_global_template[i].second.name_args[j].npos)
                all_used_globally_args.emplace(vec_global_template[i].second.name_args[j]);
            serel_target_tmp += vec_global_template[i].second.name_args[j] + " ";
        }
    }

    serel_target_tmp += std::to_string(all_used_call_component.size()) + " ";

    for (const auto &c_component : all_used_call_component) {
        const auto &ref_c_component =
            find_if(vec_c_components.begin(), vec_c_components.end(),
                    [c_component](const std::pair<std::string, var::struct_sb::call_component> &ccmp) {
                        return ccmp.first == c_component;
                    })
                ->second;
        serel_target_tmp +=
            ref_c_component.name + " " + ref_c_component.name_program + " " + ref_c_component.pattern_ret_files + " ";
    }

    serel_target_tmp += std::to_string(all_used_globally_args.size()) + " ";

    for (const auto &g_arg : all_used_globally_args)
        serel_target_tmp += g_arg + "-\"" + _interpreter.get_current_scope().get_var_value<std::string>(g_arg) + "\" ";

    assist.write_file(handle_f, serel_target_tmp);
    assist.close_file(handle_f);

    assist << " - [BWEAS]: Cache generation was successful!";

    return 0;
}

u32t
bwbuilder::deserl_cache() {
    HND handle_f = assist.open_file(CACHE_FILE, MODE_READ_FILE);
    std::string serel_str = assist.read_file(handle_f);

    assist.close_file(handle_f);

    assist << " - [BWEAS]: Cache deserialization...";

    var::struct_sb::target_out trg_tmp;
    var::struct_sb::template_command tcmd_tmp;
    var::struct_sb::call_component ccmp_tmp;

    std::string str_tmp;

    u32t count_word = 0, offset_byte_prj = 0, offset_byte_ccmp = 0;
    u32t size_src_files = 0, size_vec_templates = 0, size_vec_libs = 0;
    u32t size_templates = 0;
    u32t size_internal_args = 0, size_external_args = 0, size_call_components = 0, size_global_extern_args = 0;

    char *tproj_p = (char *)&trg_tmp.prj;
    char *ccmp_p = (char *)&ccmp_tmp;

    bool open_sk = 0;
    bool enum_templates = 0;
    bool enum_call_component = 0;
    bool enum_global_extern_args = 0;

    try {
        for (u32t i = 0; i < serel_str.size(); ++i) {
            if (serel_str[i] == '\"') {
                if (!open_sk)
                    open_sk = 1;
                else
                    open_sk = 0;
                continue;
            }

            if (serel_str[i] == ' ' && !open_sk) {
            next_word:
                ++count_word;
                if (enum_global_extern_args) {
                    if (!size_global_extern_args)
                        break;
                    std::string name_arg = str_tmp;
                    name_arg.erase(name_arg.find("-"));
                    str_tmp.erase(0, str_tmp.find("-") + 1);
                    global_extern_args.push_back(std::pair<std::string, std::string>(name_arg, str_tmp));
                }
                else if (enum_call_component) {
                    if (offset_byte_ccmp / sizeof(std::string) == 3) {
                        call_components.push_back(ccmp_tmp);
                        offset_byte_ccmp = 0;

                        --size_call_components;

                        if (!size_call_components) {
                            enum_global_extern_args = 1;
                            enum_call_component = 0;

                            size_global_extern_args = std::stoi(str_tmp);
                            goto next;
                        }
                    }
                    *(std::string *)(ccmp_p + offset_byte_ccmp) = str_tmp;
                    offset_byte_ccmp += sizeof(std::string);
                }
                else if (enum_templates) {
                    if (count_word == 1)
                        size_internal_args = std::stoi(str_tmp);
                    else if (count_word == 2)
                        size_external_args = std::stoi(str_tmp);
                    else if (count_word == 3)
                        tcmd_tmp.name = str_tmp;
                    else if (count_word == 4)
                        tcmd_tmp.name_call_component = str_tmp;
                    else if (count_word == 5)
                        tcmd_tmp.returnable = str_tmp;
                    else if (count_word >= 6 && count_word < 6 + size_internal_args)
                        tcmd_tmp.name_accept_params.push_back(str_tmp);
                    else if ((count_word >= 6 + size_internal_args &&
                              count_word < 6 + size_internal_args + size_external_args) ||
                             count_word == 6 + size_internal_args)
                        tcmd_tmp.name_args.push_back(str_tmp);
                    else if (count_word == 6 + size_internal_args + size_external_args) {
                        templates.push_back(tcmd_tmp);
                        --size_templates;

                        tcmd_tmp.name_accept_params = {};
                        tcmd_tmp.name_args = {};

                        if (!size_templates) {
                            enum_call_component = 1;
                            enum_templates = 0;

                            size_call_components = std::stoi(str_tmp);
                        }
                        else {
                            count_word = 0;
                            goto next_word;
                        }
                    }
                }
                else {
                    if (count_word == 1)
                        size_src_files = std::stoi(str_tmp);
                    else if (count_word == 2)
                        size_vec_templates = std::stoi(str_tmp);
                    else if (count_word == 3)
                        size_vec_libs = std::stoi(str_tmp);
                    else if (count_word > 3) {
                        if (count_word == 6 || (count_word >= 13 && count_word <= 15)) {
                            *(int *)(tproj_p + offset_byte_prj) = std::atoll(str_tmp.c_str());
                            offset_byte_prj += sizeof(int);
                        }
                        else if (count_word == 5) {
                            trg_tmp.prj.version_project = str_tmp;
                            offset_byte_prj += sizeof(var::struct_sb::version);
                        }
                        else {

                            if (count_word >= 16 && count_word < 16 + size_src_files)
                                trg_tmp.prj.src_files.push_back(str_tmp);
                            else if (count_word >= 16 + size_src_files &&
                                     count_word < 16 + size_src_files + size_vec_templates)
                                trg_tmp.prj.vec_templates.push_back(str_tmp);
                            else if (count_word == 16 + size_src_files + size_vec_templates)
                                trg_tmp.target_t = var::struct_sb::to_type_target(str_tmp);
                            else if (count_word == 16 + size_src_files + size_vec_templates + 1)
                                trg_tmp.target_cfg = var::struct_sb::to_cfg(str_tmp);
                            else if (count_word == 16 + size_src_files + size_vec_templates + 2)
                                trg_tmp.name_target = str_tmp;
                            else if (count_word == 16 + size_src_files + size_vec_templates + 3)
                                trg_tmp.version_target = str_tmp;
                            else if (count_word >= 16 + size_src_files + size_vec_templates + 4 &&
                                     count_word < 16 + size_src_files + size_vec_templates + 4 + size_vec_libs)
                                trg_tmp.target_vec_libs.push_back(str_tmp);
                            else if (count_word == 16 + size_src_files + size_vec_templates + 4 + size_vec_libs) {
                                out_targets.push_back(trg_tmp);

                                trg_tmp.target_vec_libs = {};
                                trg_tmp.prj.src_files = {};
                                trg_tmp.prj.vec_templates = {};

                                offset_byte_prj = 0;
                                count_word = 0;

                                if (str_tmp.find("EOET") != str_tmp.npos) {
                                    str_tmp.erase(0, str_tmp.find("EOET") + 4);

                                    size_templates = std::stoi(str_tmp);

                                    enum_templates = 1;
                                }
                                else
                                    goto next_word;
                            }
                            else {
                                *(std::string *)(tproj_p + offset_byte_prj) = str_tmp;
                                offset_byte_prj += sizeof(std::string);
                            }
                        }
                    }
                }

            next:
                str_tmp.clear();
                continue;
            }
            str_tmp += serel_str[i];
        }
    }
    catch (const std::logic_error &_excp) {
        assist.next_output_unsuccess();
        assist.call_err("BWS001");
    }

    assist << " - [BWEAS]: Cache deserialization was successful!";

    assist.next_output_success();
    assist << " - [BWEAS]: Was loaded " + std::to_string(templates.size()) + " template of command!";

    /*
    for (u32t i = 0; i < templates.size(); ++i) {
        assist << " --- " + std::to_string(i) + ". " + templates[i].name;
        assist << " --- Name of call component: " + templates[i].name_call_component;
        assist << " --- Return value: " + templates[i].returnable;
        assist << " --- Parameters: ";
        for (u32t j = 0; j < templates[i].name_accept_params.size(); ++j)
            assist << " ---  - " + templates[i].name_accept_params[j];

        assist << " --- Use externally parameters: ";
        for (u32t j = 0; j < templates[i].name_args.size(); ++j)
            assist << " ---  - " + templates[i].name_args[j];
    }

    assist << " - The following call component templates were loaded:";
    for (u32t i = 0; i < extern_args.size(); ++i) {
        assist << extern_args[i].first + " " + extern_args[i].second;
    }
    */

    return 0;
}

void
bwbuilder::build_targets() {
    assist << " - [BWEAS]: Building targets...";

    std::string command_compiler, comman_linker;

    for (u32t i = 0; i < out_targets.size(); ++i) {
    }
}

void
bwbuilder::load_target() {
    std::vector<var::struct_sb::target> targets = _interpreter.export_targets();

    var::struct_sb::target_out tmp_target;

    for (u32t i = 0; i < targets.size(); ++i) {
        tmp_target.name_target = targets[i].name_target;
        tmp_target.target_t = targets[i].target_t;
        tmp_target.target_cfg = targets[i].target_cfg;
        tmp_target.version_target = targets[i].version_target;
        tmp_target.target_vec_libs = targets[i].target_vec_libs;
        tmp_target.prj = *targets[i].prj;

        out_targets.push_back(tmp_target);
    }
}
