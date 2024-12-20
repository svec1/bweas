#include "bwbuild_sys.hpp"
#include "bwgenerator_integral.hpp"
#include "lang/static_struct.hpp"

#include <algorithm>
#include <filesystem>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_set>

using namespace bweas;
using namespace bwexception;

using mf = assistant::file::mode_file;
using file_it = assistant::file_it;

bwbuilder::bwbuilder(int argv, char **args) {
    if (!init_glob) {
        assist.add_err("BWS000", "Unable to open configuration file \'bweasconf.txt\'");
        assist.add_err("BWS001", "Incorrect cache structure");
        assist.add_err("BWS002", "Unable to generate command for target");
        assist.add_err("BWS003", "Unable to generate cache");
        assist.add_err("BWS004", "Failed build, external error while calling command");
        assist.add_err("BWS005", "Incorrect structure of the bweas-json configuration file");
        assist.add_err("BWS006", "The bweas-json configuration for package package file was not found");
        assist.add_err("BWS007", "Bweas package not found");
        assist.add_err("BWS008", "Failed to generate commands");

        assist.add_err("BWS-HAX000", "Unknown parameter");
        assist.add_err("BWS-HAX001", "The package command is a separate subroutine");
        assist.add_err("BWS-HAX002", "The package routine takes two parameters");

        init_glob = 1;
    }

    std::vector<std::string> vec_args;
    for (u32t i = 0; i < argv; ++i)
        vec_args.push_back(args[i]);

    handle_args(vec_args);

    if (mode_bweas != mode_working::build_package) {
        assist << "Initializing system build - bweas " + bwbuilde_ver.get_str_version();
        init();
    }
}

bwbuilder::~bwbuilder() {
    for (const auto &generator : generators)
        generator.second->deleteGenerator();
}

void bwbuilder::handle_args(std::vector<std::string> args) {
    args[0].erase(0, args[0].find_last_of("/\\") + 1);
    name_bweas_prg = args[0];
    path_bweas_config = std::filesystem::current_path().string();

    if (args.size() == 1)
        mode_bweas = mode_working::collect_cfg;

    std::string path_json_cfg_package;

    bool expected_path_bweas_config = 1, expected_path_to_build = 0;
    bool expected_path_json_cfg_package = 0;
    for (u32t i = 1; i < args.size(); ++i) {
        if (args[i].find("--") == 0) {
            if (i == 1)
                expected_path_bweas_config = 0;
            args[i].erase(args[i].find("--"), 2);
            if (args[i] == "build") {
                if (!path_bweas_to_build.empty()) {
                    assist.next_output_important();
                    assist << " - [BWEAS]: The build folder has already been set";
                    continue;
                }
                else if (mode_bweas == mode_working::build_package)
                    throw bwbuilder_excp(args[i], "001", "-HAX");

                if (mode_bweas == mode_working::collect_cfg)
                    mode_bweas = mode_working::collect_cfg_w_build;
                else
                    mode_bweas = mode_working::build;
                expected_path_to_build = 1;
            }
            else if (args[i] == "cfg") {
                if (mode_bweas == mode_working::build)
                    mode_bweas = mode_working::collect_cfg_w_build;
                else
                    mode_bweas = mode_working::collect_cfg;
                expected_path_bweas_config = 1;
            }
            else if (args[i] == "package") {
                if ((mode_bweas == mode_working::build || mode_bweas == mode_working::collect_cfg) && i != 2)
                    throw bwbuilder_excp(path_json_cfg_package, "001", "-HAX");
                mode_bweas = mode_working::build_package;
                expected_path_json_cfg_package = 1;
            }
            else
                throw bwbuilder_excp("Parameter: " + args[i] + "\n" + BWEAS_HELP, "000", "-HAX");
        }
        else {
            // --cfg <arg> or arg
            if (expected_path_bweas_config) {
                path_bweas_config = std::filesystem::absolute(args[i]).string();
                expected_path_bweas_config = 0;
            }
            // --build <arg>
            else if (expected_path_to_build) {
                path_bweas_to_build = std::filesystem::absolute(args[i]).string();
                expected_path_to_build = 0;
            }
            // --package <arg>
            else if (expected_path_json_cfg_package) {
                path_json_cfg_package = args[i];
                expected_path_json_cfg_package = 0;

                u32t size_pckg = create_package(path_json_cfg_package);

                assist.next_output_important();
                assist << "[Bweas-Package] Created: " + std::to_string(size_pckg) + " bytes";
            }
            else
                throw bwbuilder_excp("Parameter: " + args[i] + "\n" + BWEAS_HELP, "000", "-HAX");
        }
    }

    if (expected_path_json_cfg_package)
        throw bwbuilder_excp(BWEAS_HELP, "002", "-HAX");
}

u32t bwbuilder::create_package(std::string path_json_config_package) {
    bwpackage::data_bw_package data_package;
    const file_it &json_config_package = assist.open_file(path_json_config_package, mf::open::r);
    if (!assist.exist_file(json_config_package))
        throw bwbuilder_excp(path_json_config_package, "006");

    data_package.json_config = assist.read_file(assist.get_ref_file(json_config_package), mf::input::read_default);
    assist.close_file(json_config_package);

    bweas::bwpackage loaded_package;
    std::string pckg = loaded_package.init(data_package, 1);
    const file_it &package = assist.open_file(
        std::filesystem::current_path().string() + "/" + loaded_package.name_package + BW_FORMAT_PACKAGE, mf::open::wb);
    assist.write_file(assist.get_ref_file(package), pckg, mf::output::write_binary);
    assist.close_file(package);

    return pckg.size();
}

void bwbuilder::init() {
    std::string bweas_path = assist.get_path_program() + "/" + JSON_CONFIG_FILE;

    nlohmann::json config_json;
    file_it file_json_config = assist.open_file(bweas_path, mf::open::r);
    if (!assist.exist_file(file_json_config)) {
        assist.get_ref_file(file_json_config).open(mf::open::w);
        assist.write_file(assist.get_ref_file(file_json_config), DEFAULT_BWEAS_JSON_CONFIG);
        assist.close_file(file_json_config);

        config_json = nlohmann::json::parse(DEFAULT_BWEAS_JSON_CONFIG);
    }
    else {
        config_json =
            nlohmann::json::parse(assist.read_file(assist.get_ref_file(file_json_config), mf::input::read_default));
        assist.close_file(file_json_config);
    }

    if (config_json.contains("packages")) {
        if (!config_json["packages"].is_array())
            throw bwbuilder_excp("At least one package must be defined", "005");
        for (const auto &package : config_json["packages"].items()) {
            if (!package.value().is_string())
                throw bwbuilder_excp("Package names are expected", "005");
            bweas::bwpackage loaded_package;

            std::string path_to_package =
                assist.get_path_program() + "/packages/" + (std::string)package.value() + BW_FORMAT_PACKAGE;
            std::string raw_data_package;
            file_it package = assist.open_file(path_to_package, mf::open::rb);
            if (!assist.exist_file(package))
                throw bwbuilder_excp(path_to_package, "007");
            raw_data_package = assist.read_file(assist.get_ref_file(package), mf::input::read_binary);
            assist.close_file(package);

            loaded_package.load(raw_data_package);
            loaded_packages.push_back(loaded_package);
            assist.next_output_success();
            assist << " - [BWEAS]: The package was loaded successfully(" + std::to_string(raw_data_package.size()) +
                          " bytes)";
        }
    }

    generators.emplace("bwgenerator", generator_api::base_generator::createGeneratorInt(generator::bwgenerator,
                                                                                        generator::bwfile_inputs));
    for (const auto &package : loaded_packages)
        for (const auto &generator : package.cfg_package.generators)
            generators.emplace(generator.name_generator,
                               generator_api::base_generator::createGeneratorLua(generator.src_lua_generator));

    for (auto &package : loaded_packages) {
        semantic_an::table_func tfuncs = module_mg.init_tsfunc(package.cfg_package.mds);
        for (const auto &func : tfuncs)
            module_tfuncs.insert(func);
    }
}

bwbuilder::mode_working bwbuilder::get_current_mode() {
    return mode_bweas;
}

void bwbuilder::start() {
    if (mode_bweas == mode_working::build_package)
        return;
    file_it bweas_config = assist.open_file(path_bweas_config + "/" MAIN_FILE, mf::open::r);
    if (!assist.exist_file(bweas_config))
        assist.call_err("BWS000");
    assist.close_file(bweas_config);

    if (mode_bweas == mode_working::build) {
        std::filesystem::path cache_file = std::filesystem::current_path() / CACHE_FILE;
        if (!std::filesystem::exists(cache_file)) {
            assist.next_output_unsuccess();
            assist << " - [BWEAS]: Bweas cache not found";
            return;
        }
        std::filesystem::path main_file = std::filesystem::current_path() / MAIN_FILE;

        std::filesystem::file_time_type cftime = std::filesystem::last_write_time(cache_file);
        std::filesystem::file_time_type mftime = std::filesystem::last_write_time(main_file);
        if (mftime >= cftime) {
            assist.next_output_important();
            assist << " - [BWEAS]: The configuration file has been modified";
            goto interpreter_start;
        }

        deserl_cache();
    }
    else if (mode_bweas == mode_working::collect_cfg || mode_bweas == mode_working::collect_cfg_w_build) {
    interpreter_start:
        run_interpreter();
        load_target();
        gen_cache_target();
        imp_data_interpreter_for_bs();
    }

    if (mode_bweas == mode_working::build || mode_bweas == mode_working::collect_cfg_w_build) {
        assist.next_output_important();
        assist << "*Build start time: " + assist.get_time();
        build_targets();
    }
}

void bwbuilder::switch_log(u32t value) {
    log = value;
    assist.switch_log(log);
}

void bwbuilder::switch_output_log(u32t value) {
    output_log = value;
    assist.switch_otp(output_log);
}

void bwbuilder::run_interpreter() {
    interpreter::interpreter_exec::config new_config;

    new_config.debug_output = 0;
    new_config.filename_interp = MAIN_FILE;
    new_config.file_import_file_f = IMPORT_FILE;
    new_config.transmit_smt_name_func_with_smt = 1;
    new_config.use_external_scope = 0;

    interpreter.set_config(new_config);
    interpreter.load_external_func(std::move(module_tfuncs));

    assist << " - [BWEAS]: Interpreting the configuration file...";

    interpreter.build_aef();
    interpreter.interpret();

    assist << " - [BWEAS]: The interpretation was successful!";
}

u32t bwbuilder::gen_cache_target() {
    if (!out_targets.size()) {
        assist << "Generating a file with a cache will not be performed.";
        return 1;
    }
    assist << " - [BWEAS]: Generating a cache file...";

    file_it bweas_cache = assist.open_file(CACHE_FILE, mf::open::w);

    std::string serel_target_tmp;

    std::unordered_set<std::string> used_templates;
    std::unordered_set<std::string> all_used_globally_args;
    std::unordered_set<std::string> all_used_call_component;

    const auto &vec_global_template =
        interpreter.get_current_scope().get_vector_variables_t<var::struct_sb::template_command>();
    const auto &vec_c_components =
        interpreter.get_current_scope().get_vector_variables_t<var::struct_sb::call_component>();

    for (i32t i = 0; i < out_targets.size(); ++i) {
        char *prj_v = (char *)&out_targets[i];
        serel_target_tmp += std::to_string(out_targets[i].prj.src_files.size()) + " " +
                            std::to_string(out_targets[i].prj.vec_templates.size()) + " " +
                            std::to_string(out_targets[i].target_vec_libs.size()) + " ";
        for (u32t j = 0; j < sizeof(var::struct_sb::project); j += sizeof(std::string)) {

            // version project
            if (j == sizeof(std::string)) {
                serel_target_tmp += (*(var::struct_sb::version *)(prj_v + j)).get_str_version() + " " +
                                    std::to_string(*(unsigned long *)(prj_v + j + sizeof(var::struct_sb::version))) +
                                    " ";
                j += sizeof(var::struct_sb::version) + sizeof(size_t);
            }

            // standart c
            else if (j == sizeof(std::string) * 7 + sizeof(var::struct_sb::version) + sizeof(size_t)) {
                serel_target_tmp += std::to_string(*(i32t *)(prj_v + j)) + " " +
                                    std::to_string(*(i32t *)(prj_v + j + sizeof(i32t))) + " " +
                                    std::to_string(*(bool *)(prj_v + j + sizeof(i32t) * 2)) + " ";
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
                            " " + out_targets[i].name_generator + " " +
                            out_targets[i].version_target.get_str_version() + " ";
        for (u32t j = 0; j < out_targets[i].target_vec_libs.size(); ++j)
            serel_target_tmp += out_targets[i].target_vec_libs[j] + " ";
    }

    serel_target_tmp += "EOET" + std::to_string(vec_global_template.size()) + " "; // end of enum targets

    for (u32t i = 0; i < vec_global_template.size(); ++i) {
        serel_target_tmp +=
            std::to_string(vec_global_template[i].second.name_accept_params.size()) + " " +
            std::to_string(vec_global_template[i].second.args.size()) + " " + vec_global_template[i].second.name + " " +
            vec_global_template[i].second.name_call_component + " " + vec_global_template[i].second.returnable + " ";

        all_used_call_component.emplace(vec_global_template[i].second.name_call_component);

        for (u32t j = 0; j < vec_global_template[i].second.name_accept_params.size(); ++j)
            serel_target_tmp += vec_global_template[i].second.name_accept_params[j] + " ";
        for (u32t j = 0; j < vec_global_template[i].second.args.size(); ++j) {
            if (vec_global_template[i].second.args[j].arg_t == var::struct_sb::template_command::arg::type::extglobal)
                all_used_globally_args.emplace(vec_global_template[i].second.args[j].str_arg);
            serel_target_tmp += vec_global_template[i].second.args[j].str_arg + " " +
                                std::to_string((i32t)vec_global_template[i].second.args[j].arg_t) + " ";
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

    for (const auto &g_arg : all_used_globally_args) {
        if (interpreter.get_current_scope().what_type(
                interpreter.get_current_scope().get_var_value<std::string>(g_arg)) != 2)
            throw bwbuilder_excp("External global parameter for template is not defined correctly", "003");
        serel_target_tmp += g_arg + "-\"" +
                            interpreter.get_current_scope().get_var_value<std::string>(
                                interpreter.get_current_scope().get_var_value<std::string>(g_arg)) +
                            "\" ";
    }
    assist.write_file(assist.get_ref_file(bweas_cache), serel_target_tmp, mf::output::write_binary);
    assist.close_file(bweas_cache);

    assist.next_output_success();
    assist << " - [BWEAS]: Cache generation was successful!";

    return 0;
}

u32t bwbuilder::deserl_cache() {
    file_it bweas_cache = assist.open_file(CACHE_FILE, mf::open::rb);
    std::string serel_str = assist.read_file(assist.get_ref_file(bweas_cache));

    assist.close_file(bweas_cache);

    assist << " - [BWEAS]: Cache deserialization...";

    var::struct_sb::target_out trg_tmp;
    var::struct_sb::template_command tcmd_tmp;
    var::struct_sb::template_command::arg arg_tmp;
    var::struct_sb::call_component ccmp_tmp;

    std::string str_tmp;

    i32t count_word = 0, offset_byte_prj = 0, offset_byte_ccmp = 0;
    i32t size_src_files = 0, size_vec_templates = 0, size_vec_libs = 0;
    i32t size_templates = 0;
    i32t size_internal_args = 0, size_external_args = 0, size_call_components = 0, size_global_extern_args = 0;

    char *tproj_p = (char *)&trg_tmp.prj;
    char *ccmp_p = (char *)&ccmp_tmp;

    bool open_sk = 0;
    bool enum_templates = 0;
    bool enum_call_component = 0;
    bool enum_global_extern_args = 0;

    bool expected_arg_param_str = 1;

    try {
        for (i32t i = 0; i < serel_str.size(); ++i) {
            if (serel_str[i] == '\"' && !enum_templates) {
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
                              count_word < 6 + size_internal_args + (size_external_args * 2)) ||
                             count_word == 6 + size_internal_args) {
                        if (expected_arg_param_str) {
                            arg_tmp.str_arg = str_tmp;
                            expected_arg_param_str = 0;
                        }
                        else {
                            arg_tmp.arg_t = (var::struct_sb::template_command::arg::type)std::stoi(str_tmp);
                            tcmd_tmp.args.push_back(arg_tmp);
                            expected_arg_param_str = 1;
                        }
                    }
                    else if (count_word == 6 + size_internal_args + (size_external_args * 2)) {
                        templates.push_back(tcmd_tmp);
                        --size_templates;

                        tcmd_tmp.name_accept_params = {};
                        tcmd_tmp.args = {};

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
                            *(i32t *)(tproj_p + offset_byte_prj) = std::atoll(str_tmp.c_str());
                            offset_byte_prj += sizeof(i32t);
                        }
                        else if (count_word == 5) {
                            trg_tmp.prj.version_project = str_tmp;
                            offset_byte_prj += sizeof(var::struct_sb::version);
                        }
                        else {

                            if (count_word >= 16 && count_word < 16 + size_src_files)
                                trg_tmp.prj.src_files.push_back(str_tmp);
                            else if (count_word >= 16 + size_src_files &&
                                     count_word < 16 + size_src_files + size_vec_templates) {
                                if (str_tmp != "null")
                                    trg_tmp.prj.vec_templates.push_back(str_tmp);
                            }
                            else if (count_word == 16 + size_src_files + size_vec_templates)
                                trg_tmp.target_t = var::struct_sb::to_type_target(str_tmp);
                            else if (count_word == 16 + size_src_files + size_vec_templates + 1)
                                trg_tmp.target_cfg = var::struct_sb::to_cfg(str_tmp);
                            else if (count_word == 16 + size_src_files + size_vec_templates + 2)
                                trg_tmp.name_target = str_tmp;
                            else if (count_word == 16 + size_src_files + size_vec_templates + 3)
                                trg_tmp.name_generator = str_tmp;
                            else if (count_word == 16 + size_src_files + size_vec_templates + 4)
                                trg_tmp.version_target = str_tmp;
                            else if (count_word >= 16 + size_src_files + size_vec_templates + 5 &&
                                     count_word < 16 + size_src_files + size_vec_templates + 5 + size_vec_libs)
                                trg_tmp.target_vec_libs.push_back(str_tmp);
                            else if (count_word == 16 + size_src_files + size_vec_templates + 5 + size_vec_libs) {
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
        throw bwbuilder_excp("", "001");
    }

    assist << " - [BWEAS]: Cache deserialization was successful!";

    assist.next_output_success();
    assist << " - [BWEAS]: Was loaded " + std::to_string(templates.size()) + " template of command!";

    return 0;
}

void bwbuilder::build_targets() {
    assist << " - [BWEAS]: Building targets...";

    global_extern_args.push_back(std::pair<std::string, std::string>("", ""));

    try {

        for (u32t i = 0; i < out_targets.size(); ++i) {
            if (generators.find(out_targets[i].name_generator) == generators.end())
                throw bwbuilder_excp("The provided generator as the primary for the current target was not found - " +
                                         out_targets[i].name_target + ": " + out_targets[i].name_generator,
                                     "002");
            auto &current_generator = generators[out_targets[i].name_generator];
            current_generator->set_ccomponents(call_components);
            current_generator->init();
            if (out_targets[i].prj.vec_templates.size() == 0)
                throw bwbuilder_excp("There are no templates for the target - " + out_targets[i].name_target, "002");
            std::string dir_target = std::string(DIRWORK_ENV) + "/" + out_targets[i].name_target;
            if (!std::filesystem::is_directory(dir_target))
                std::filesystem::create_directories(dir_target);

            assist << "BUILDING A TARGET -" + out_targets[i].name_target;

            bwqueue_templates bw_tcmd;
            set_queue_templates(create_stack_target_templates(out_targets[i]), bw_tcmd);
            parse_basic_args(out_targets[i], bw_tcmd);
            commands cmd_s =
                current_generator->gen_commands(out_targets[i], bw_tcmd, dir_target,
                                                current_generator->input_files(out_targets[i], bw_tcmd, dir_target));

            for (const command &cmd : cmd_s) {
#if defined(WIN)
                if (system(cmd.c_str()))
#elif defined(UNIX)
                if (((int (*)(const char *))assist.get_realsystem_func())(cmd.c_str()))
#endif
                    throw bwbuilder_excp("Failed build. Command execution error - " + cmd, "004");
            }
        }
    }
    catch (bwgenerator_excp &_excp) {
        throw bweas::bwexception::bwbuilder_excp(assist.get_desc_err(_excp.get_assist_err()), "008");
    }
}

void bwbuilder::load_target() {
    std::vector<var::struct_sb::target> targets = interpreter.export_targets();

    var::struct_sb::target_out target_tmp;

    for (u32t i = 0; i < targets.size(); ++i) {
        target_tmp.name_target = targets[i].name_target;
        target_tmp.target_t = targets[i].target_t;
        target_tmp.target_cfg = targets[i].target_cfg;
        target_tmp.version_target = targets[i].version_target;
        target_tmp.name_generator = targets[i].name_generator;
        target_tmp.target_vec_libs = targets[i].target_vec_libs;
        target_tmp.prj = *targets[i].prj;

        out_targets.push_back(target_tmp);
    }
}

std::stack<std::string> bwbuilder::create_stack_target_templates(const var::struct_sb::target_out &target) {
    std::vector<var::struct_sb::template_command> vec_templates_tmp;
    std::stack<std::string> stack_templates;

    for (u32t i = 0; i < target.prj.vec_templates.size(); ++i) {
        for (u32t j = 0; j < templates.size(); ++j)
            if (target.prj.vec_templates[i] == templates[j].name)
                vec_templates_tmp.push_back(templates[j]);
    }

    const auto &it_template = find_if(vec_templates_tmp.begin(), vec_templates_tmp.end(),
                                      [target](const var::struct_sb::template_command &_template) {
                                          return _template.returnable == var::struct_sb::target_t_str(target.target_t);
                                      });

    if (it_template == vec_templates_tmp.end())
        throw bwbuilder_excp("There is no template that returns a target with the given type - " +
                                 var::struct_sb::target_t_str(target.target_t),
                             "002");

    stack_templates.push(it_template->name);
    for (u32t i = 0; i < it_template->name_accept_params.size(); ++i)
        recovery_stack_templates(vec_templates_tmp, it_template->name_accept_params[i], stack_templates);

    return stack_templates;
}

u32t bwbuilder::recovery_stack_templates(std::vector<var::struct_sb::template_command> &vec_templates,
                                         const std::string &name_internal_param,
                                         std::stack<std::string> &stack_templates) {
    const auto &it = find_if(vec_templates.begin(), vec_templates.end(),
                             [name_internal_param](const var::struct_sb::template_command &_template) {
                                 return _template.returnable == name_internal_param;
                             });
    if (it == vec_templates.end())
        return 1;

    stack_templates.push(it->name);
    for (u32t i = 0; i < it->name_accept_params.size(); ++i)
        recovery_stack_templates(vec_templates, it->name_accept_params[i], stack_templates);

    return 0;
}
void bwbuilder::set_queue_templates(std::stack<std::string> &&stack_target_templates,
                                    bwqueue_templates &target_queue_templates) {
    while (stack_target_templates.size()) {
        const auto &it_templates = find_if(templates.begin(), templates.end(),
                                           [stack_target_templates](const var::struct_sb::template_command &tcmd) {
                                               return tcmd.name == stack_target_templates.top();
                                           });
        target_queue_templates.push_back(*it_templates);
        stack_target_templates.pop();
    }
}

void bwbuilder::parse_basic_args(const var::struct_sb::target_out &target, bwqueue_templates &target_queue_templates) {
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
                    throw bwbuilder_excp("[" + trg_template.name +
                                             "] The specified external parameter does not exist - " +
                                             current_arg.str_arg,
                                         "002");

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
                    throw bwbuilder_excp(
                        "[" + trg_template.name + "] There is no such parameter - " + current_arg.str_arg, "002");
            }

            current_arg.arg_t = var::struct_sb::template_command::arg::type::string;
        }
    }
}

void bwbuilder::imp_data_interpreter_for_bs() {
    const auto &global_templates =
        interpreter.get_current_scope().get_vector_variables_t<var::struct_sb::template_command>();
    std::unordered_set<std::string> name_global_external_args;

    for (const auto &global_template_it : global_templates) {
        for (const auto &arg : global_template_it.second.args)
            if (arg.arg_t == var::struct_sb::template_command::arg::type::extglobal)
                name_global_external_args.emplace(arg.str_arg);
        templates.push_back(global_template_it.second);
    }

    const auto &global_call_components =
        interpreter.get_current_scope().get_vector_variables_t<var::struct_sb::call_component>();
    for (const auto &global_call_component : global_call_components)
        call_components.push_back(global_call_component.second);

    const auto &string_variables = interpreter.get_current_scope().get_vector_variables_t<std::string>();
    for (const auto &name_global_external_arg : name_global_external_args) {
        const auto &ref_extern_arg =
            find_if(string_variables.begin(), string_variables.end(),
                    [name_global_external_arg](const std::pair<std::string, std::string> &str_var) {
                        return str_var.first == name_global_external_arg;
                    });
        if (interpreter.get_current_scope().what_type(ref_extern_arg->second) != 2)
            throw bwbuilder_excp(
                "The parameter reference(name) points to a non-existent variable - " + ref_extern_arg->second, "002");
        const auto &extern_arg = interpreter.get_current_scope().get_var_value<std::string>(ref_extern_arg->second);
        global_extern_args.push_back(std::pair<std::string, std::string>(ref_extern_arg->first, extern_arg));
    }
}
