//
// BWEAS is distributed under the GNU General Public License 2.0 (GPL-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_set>

#include "bwbuild_sys.hpp"
#include "bwdepends_files.hpp"
#include "bwdepends_generator.hpp"
#include "bwdepends_integral.hpp"
#include "bwgenerator_integral.hpp"
#include "bwlang.hpp"
#include "lang/static_struct.hpp"

#include <nlohmann/json.hpp>

using namespace bweas;

using mf      = bwtools::file::mode_file;
using file_it = bwtools::file_it;

static logger _log{"BWEAS"};

bwbuilder::bwbuilder(int argv, char **args) {
    std::vector<std::string> vec_args;
    for (u32t i = 0; i < argv; ++i)
        vec_args.push_back(args[i]);

    handle_args(vec_args);

    if (mode_bweas != mode_working::build_package && mode_bweas != mode_working::undef) {
        (_log << bwtools::message) << (log_message(log_type::msg)
                                       << "Initializing system build - bweas " + bwbuilde_ver.get_str_version());
        init();
    }
}

void bwbuilder::handle_args(std::vector<std::string> &args) {
    args[0].erase(0, args[0].find_last_of("/\\") + 1);
    name_bweas_prg    = args[0];
    path_bweas_config = std::filesystem::current_path().string();

    if (args.size() == 1)
        mode_bweas = mode_working::collect_cfg;

    std::string path_json_cfg_package;

    bool expected_path_bweas_config = 1, expected_path_to_build = 0;
    bool expected_path_json_cfg_package = 0;
    for (u32t i = 1; i < args.size(); ++i) {
        if (args[i].find("--") == 0) {
            args[i].erase(args[i].find("--"), 2);

            if (mode_bweas != undef) {
                (_log << bwtools::error) << (log_message(log_type::error)
                                             << "Unknown argument: " + args[i] +
                                                    ". The bweas mode has already been defined");
                return;
            }
            if (i == 1)
                expected_path_bweas_config = 0;

            if (args[i] == "build") {
                mode_bweas             = mode_working::build;
                expected_path_to_build = 1;
            }
            else if (args[i] == "cfg") {
                mode_bweas                 = mode_working::collect_cfg;
                expected_path_bweas_config = 1;
            }
            else if (args[i] == "package") {
                mode_bweas                     = mode_working::build_package;
                expected_path_json_cfg_package = 1;
            }
            else if (args[i] == "help")
                (_log << bwtools::message) << (log_message(log_type::msg) << BWEAS_HELP);
            else if (args[i] == "version")
                (_log << bwtools::message) << (log_message(log_type::msg) << BWEAS_INFO);
            else {
                (_log << bwtools::error) << (log_message(log_type::error) << "Unknown argument: " + args[i]);
                return;
            }
        }
        else {
            // --cfg <arg> or arg
            if (expected_path_bweas_config) {
                path_bweas_config = std::filesystem::absolute(args[i]).string();
                mode_bweas        = mode_working::collect_cfg;

                expected_path_bweas_config = 0;
            }
            // --build <arg>
            else if (expected_path_to_build) {
                path_bweas_to_build    = std::filesystem::absolute(args[i]).string();
                expected_path_to_build = 0;
            }
            // --package <arg>
            else if (expected_path_json_cfg_package) {
                path_json_cfg_package          = args[i];
                expected_path_json_cfg_package = 0;

                u32t size_pckg = create_package(path_json_cfg_package);

                (_log << bwtools::message)
                    << (log_message(log_type::msg) << "Created: " + std::to_string(size_pckg) + " bytes");
            }
            else {
                (_log << bwtools::error) << (log_message(log_type::error) << "Unknown argument: " + args[i]);
                return;
            }
        }
    }

    if (expected_path_json_cfg_package)
        (_log << bwtools::error) << (log_message(log_type::error) << "Invalid syntax.\n" << BWEAS_HELP);
}

u32t bwbuilder::create_package(std::string path_json_config_package) {
    bwpackage::data_bw_package data_package;
    const file_it &json_config_package = bwtools::open_file(path_json_config_package, mf::open::r);
    if (!bwtools::exist_file(json_config_package)) {
        (_log << bwtools::error) << (log_message(log_type::error)
                                     << "The bweas-json configuration for package package file was not found");
        return 0;
    }
    data_package.json_config = bwtools::read_file(bwtools::get_ref_file(json_config_package), mf::input::read_default);
    bwtools::close_file(json_config_package);

    bweas::bwpackage loaded_package;
    std::string pckg = loaded_package.init(data_package, 1);
    if (_log.error_status()) {
        (_log << bwtools::error) << (log_message(log_type::error) << "Failed to create a bweas package");
        return 0;
    }
    const file_it &package = bwtools::open_file(
        std::filesystem::current_path().string() + "/" + loaded_package.name_package + BW_FORMAT_PACKAGE, mf::open::wb);
    bwtools::write_file(bwtools::get_ref_file(package), pckg, mf::output::write_binary);
    bwtools::close_file(package);

    return pckg.size();
}

void bwbuilder::init() {
    std::string bweas_path = bwtools::get_path_program() + "/" + JSON_CONFIG_FILE;

    nlohmann::json config_json;
    file_it file_json_config = bwtools::open_file(bweas_path, mf::open::r);
    if (!bwtools::exist_file(file_json_config)) {
        bwtools::get_ref_file(file_json_config).open(mf::open::w);
        bwtools::write_file(bwtools::get_ref_file(file_json_config), DEFAULT_BWEAS_JSON_CONFIG);
        bwtools::close_file(file_json_config);

        config_json = nlohmann::json::parse(DEFAULT_BWEAS_JSON_CONFIG);
    }
    else {
        config_json =
            nlohmann::json::parse(bwtools::read_file(bwtools::get_ref_file(file_json_config), mf::input::read_default));
        bwtools::close_file(file_json_config);
    }

    if (config_json.contains("cache-gn")) {
        if (!config_json["cache-gn"].is_string()) {
            (_log << bwtools::error) << (log_message(log_type::error) << "Cache generator name expected");
            return;
        }
        if (config_json["cache-gn"] == "fast_bwcache")
            cache = std::unique_ptr<cache_api::base_bwcache>(cache_api::base_bwcache::create_fast_bwcache(&context));
        else if (config_json["cache-gn"] == "json_bwcache")
            cache = std::unique_ptr<cache_api::base_bwcache>(cache_api::base_bwcache::create_json_bwcache(&context));
    }
    else
        cache = std::unique_ptr<cache_api::base_bwcache>(cache_api::base_bwcache::create_fast_bwcache(&context));

    if (config_json.contains("packages")) {
        if (!config_json["packages"].is_array()) {
            (_log << bwtools::error) << (log_message(log_type::error)
                                         << "Invalid json file structure. At least one package must be defined");
            return;
        }
        for (const auto &package : config_json["packages"].items()) {
            if (!package.value().is_string())
                (_log << bwtools::error) << (log_message(log_type::error)
                                             << "Invalid json file structure. Package names are expected");
            bweas::bwpackage loaded_package;
            std::string raw_data_package, path_to_package{bwtools::get_path_program() + "/packages/" +
                                                          (std::string)package.value() + BW_FORMAT_PACKAGE};

            file_it it_package = bwtools::open_file(path_to_package, mf::open::rb);
            if (!bwtools::exist_file(it_package)) {
                (_log << bwtools::warning)
                    << (log_message(log_type::warning) << "\"" << path_to_package << "\" bweas package not found");
                continue;
            }

            raw_data_package = bwtools::read_file(bwtools::get_ref_file(it_package), mf::input::read_binary);
            bwtools::close_file(it_package);

            loaded_package.load(raw_data_package);
            loaded_packages.push_back(loaded_package);

            if (cache == NULL && loaded_package.cfg_package.cache.name_cache == config_json["cache-gn"])
                cache = std::unique_ptr<cache_api::base_bwcache>(cache_api::base_bwcache::create_lua_bwcache(
                    &context, loaded_package.cfg_package.cache.src_lua_cache));
            _log << bwtools::success
                 << (log_message(log_type::msg)
                     << "\"" << path_to_package << "\" bweas package was loaded successfully("
                     << raw_data_package.size() << " bytes)");
        }
    }

    generators.emplace("bwgenerator", std::shared_ptr<generator_api::base_generator>(
                                          generator_api::base_generator::create_generator_int(
                                              generator::bwgenerator, generator::bwbuild_graph_depends_file,
                                              generator::bwget_input_files),
                                          [](generator_api::base_generator *ptr) { ptr->_delete(); }));

    for (const auto &package : loaded_packages)
        for (const auto &generator : package.cfg_package.generators) {
            generators.emplace(generator.name_generator,
                               std::shared_ptr<generator_api::base_generator>(
                                   generator_api::base_generator::create_generator_lua(generator.src_lua_generator),
                                   [](generator_api::base_generator *ptr) { ptr->_delete(); }));
            if (generator.use_custom_build_graph_depends)
                generators[generator.name_generator]->set_use_build_graph_depends();
        }
    for (auto &package : loaded_packages) {
        auto module_funcs = module_manager.init_mfuncs(package.cfg_package.modules);
        for (const auto &module_func : module_funcs)
            external_modules_funcs.push_back(module_func);
    }
}

bwbuilder::mode_working bwbuilder::get_current_mode() {
    return mode_bweas;
}

void bwbuilder::start() {
    if (mode_bweas == mode_working::build_package)
        return;
    else if (!bwtools::exist_file(path_bweas_config + "/" MAIN_FILE))
        (_log << bwtools::fatal) << (log_message(log_type::fatal)
                                     << "Unable to open configuration file \'bweasconf.txt\'");

    if (mode_bweas == mode_working::build) {
        std::string cache_file = bwtools::get_current_path() + "/" + CACHE_FILE;
        if (!bwtools::exist_file(cache_file))
            (_log << bwtools::fatal) << (log_message(log_type::fatal) << "Bweas cache not found");
        std::filesystem::path main_file = std::filesystem::current_path() / MAIN_FILE;

        std::filesystem::file_time_type cftime = std::filesystem::last_write_time(cache_file);
        std::filesystem::file_time_type mftime = std::filesystem::last_write_time(main_file);
        if (mftime >= cftime) {
            (_log << bwtools::message) << (log_message(log_type::msg) << "The configuration file has been modified");
            goto interpreter_start;
        }

        deserl_cache();
    }
    else if (mode_bweas == mode_working::collect_cfg || mode_bweas == mode_working::collect_cfg_w_build) {
    interpreter_start:
        run_interpreter();
        gen_cache_target();
    }

    if (mode_bweas == mode_working::build || mode_bweas == mode_working::collect_cfg_w_build) {
        (_log << bwtools::message) << (log_message(log_type::msg) << "*Build start time: " << bwtools::get_time());
        build_targets();
    }
}

void bwbuilder::run_interpreter() {
    bwlang _bwlang{MAIN_FILE};
    _bwlang.init_external_funcs(external_modules_funcs);
    for (const auto &loaded_package : loaded_packages)
        _bwlang.set_custom_ext_fields_project(loaded_package.cfg_package.custom_ext_fields_project);

    (_log << bwtools::message) << (log_message(log_type::msg) << "Interpreting the configuration file...");
    _bwlang.execute();
    (_log << bwtools::success) << (log_message(log_type::msg) << "The interpretation was successful!");

    context = _bwlang.get_context();
}

u32t bwbuilder::gen_cache_target() {
    if (!context.out_targets.size()) {
        (_log << bwtools::warning) << (log_message(log_type::warning)
                                       << "Generating a file with a cache will not be performed.");
        return 0;
    }

    (_log << bwtools::message) << (log_message(log_type::msg) << "Generating a cache file...");

    file_it bweas_cache = bwtools::open_file(CACHE_FILE, mf::open::w);

    bwtools::write_file(bwtools::get_ref_file(bweas_cache), cache->create_cache(), mf::output::write_binary);
    bwtools::close_file(bweas_cache);

    (_log << bwtools::success) << (log_message(log_type::msg) << "Cache generation was successful!");

    return 1;
}

void bwbuilder::deserl_cache() {
    file_it bweas_cache = bwtools::open_file(CACHE_FILE, mf::open::rb);

    (_log << bwtools::message) << (log_message(log_type::msg) << "Cache deserialization...");

    cache->extract_cache_data(bwtools::read_file(bwtools::get_ref_file(bweas_cache)));

    (_log << bwtools::success) << (log_message(log_type::msg)
                                   << "Cache deserialization was successful!\n"
                                   << "Was loaded " << context.templates.size() << " template of command!");
}

void bwbuilder::build_targets() {
    (_log << bwtools::message) << (log_message(log_type::msg) << "Building targets...");

    context.global_external_args.push_back(std::pair<std::string, std::string>("", ""));

    for (auto &target : context.out_targets) {
        if (generators.find(target.name_generator) == generators.end()) {
            (_log << bwtools::error)
                << (log_message(log_type::error)
                    << "The provided generator as the primary for the current target was not found - "
                    << target.name_target << ": " << target.name_generator);
            return;
        }
        else if (target.prj.vec_templates.size() == 0) {
            (_log << bwtools::error) << (log_message(log_type::error)
                                         << "There are no templates for the target - " << target.name_target);
            return;
        }
        std::string dir_target = path_bweas_to_build + "/" + target.name_target;
        double build_state     = 0.f;

        bwqueue_templates bw_tcmd = create_queue_target_templates(target);

        auto &current_generator = generators[target.name_generator];
        std::unique_ptr<bwdepends_files> depends_files;

        context.current_target = &target;
        bweas::generator_api::data_transfer data_t{&context, dir_target};

        if (!std::filesystem::is_directory(dir_target))
            std::filesystem::create_directories(dir_target);

        current_generator->init();

        if (current_generator->has_build_graph_depends())
            depends_files = std::make_unique<bwdepends_generator>(current_generator, target.prj.language, dir_target);
        else
            depends_files = std::make_unique<bwdepends_integral>(target.prj.language, dir_target);

        for (const auto &name_file : target.prj.src_files) {
            std::string name_depends_file = name_file + DEPENDS_FILE_POSTFIX;
            if (bwtools::exist_file(name_depends_file))
                depends_files->build_graph_depends_file_string(
                    name_file, bwtools::read_file(bwtools::get_ref_file(bwtools::open_file(name_depends_file))));
            else
                bwtools::write_file(bwtools::get_ref_file(bwtools::open_file(name_depends_file, mf::open::w)),
                                    depends_files->get_string_depends_file(name_file));
        }

        depends_files->build_graphs_depends_files(target.prj.src_files, target.prj.include_paths);
        data_t.dfiles = depends_files->get_graphs_depends_files();

        (_log << bwtools::message) << (log_message(log_type::msg) << "Build {" << target.name_target << "}");

        current_generator->get_input_files(data_t);
        auto cmd_s = current_generator->generate_command(data_t);

        for (const auto &cmd : cmd_s) {

            (_log << bwtools::message)
                << (log_message(log_type::msg)
                    << "[" << std::to_string(build_state).erase(std::to_string((u32t)build_state).size() + 3, 4)
                    << "%]Compile - " << cmd.first);

            if (system(cmd.second.c_str()))
                (_log << bwtools::warning)
                    << (log_message(log_type::warning) << "Failed build. Command execution error: \n"
                                                       << cmd.second);

            build_state += 1.f / (double)cmd_s.size() * 100;
        }
    }
}

bwqueue_templates bwbuilder::create_queue_target_templates(const var::struct_sb::target_out &target) {
    std::vector<var::struct_sb::template_command> vec_templates_tmp;
    bwqueue_templates target_queue_templates;

    for (u32t i = 0; i < target.prj.vec_templates.size(); ++i) {
        for (const auto &_template : context.templates)
            if (target.prj.vec_templates[i] == _template.name)
                vec_templates_tmp.push_back(_template);
    }

    const auto &it_template = find_if(vec_templates_tmp.begin(), vec_templates_tmp.end(),
                                      [target](const var::struct_sb::template_command &_template) {
                                          return _template.returnable == var::struct_sb::target_t_str(target.target_t);
                                      });

    if (it_template == vec_templates_tmp.end())
        (_log << bwtools::fatal) << (log_message(log_type::fatal)
                                     << "There is no template that returns a target with the given type - "
                                     << var::struct_sb::target_t_str(target.target_t));

    target_queue_templates.push_back(*it_template);
    for (u32t i = 0; i < it_template->name_accept_params.size(); ++i)
        recovery_queue_target_templates(vec_templates_tmp, it_template->name_accept_params[i], target_queue_templates);

    std::reverse(target_queue_templates.begin(), target_queue_templates.end());

    return target_queue_templates;
}

void bwbuilder::recovery_queue_target_templates(std::vector<var::struct_sb::template_command> &vec_templates,
                                                const std::string &name_internal_param,
                                                bwqueue_templates &target_queue_templates) {
    const auto &it = find_if(vec_templates.begin(), vec_templates.end(),
                             [name_internal_param](const var::struct_sb::template_command &_template) {
                                 return _template.returnable == name_internal_param;
                             });
    if (it == vec_templates.end())
        return;

    target_queue_templates.push_back(*it);
    for (u32t i = 0; i < it->name_accept_params.size(); ++i)
        recovery_queue_target_templates(vec_templates, it->name_accept_params[i], target_queue_templates);
}
