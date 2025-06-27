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

#include <bwbuild_sys.hpp>
#include <bwdepends_files.hpp>
#include <bwdepends_generator.hpp>
#include <bwdepends_integral.hpp>
#include <bwgenerator_integral.hpp>
#include <bwlang.hpp>
#include <bwprocesses_handler.hpp>
#include <bwstructs_context.hpp>

#include <nlohmann/json.hpp>

using namespace bweas;

static const string INFO_STR =
    "bweas version " + std::string(VERSION_FULL_STR) + "\nrep on github - https://github.com/svec1/bweas";

static constexpr auto HELP_STR =
    "bweas-call: \n   bweas <parameter>... path_depending\n   bweas path_bweas_config <parameter>..."
    "\nAcceptable parameters:"
    "\n   --build - builds the project (either by executing the configuration file or deserializing the cache file "
    "if it exists)"
    "\n   --cfg - executes the configuration file if it has been changed and creates a new cache file"
    "\n   --package - creates a bweas package based on the transferred json file(json config)"
    "\n   --help - outputs the syntax of the bweas call as well as its possible functions"
    "\n   --version - outputs the version of bweas";

static constexpr auto JSON_CONFIG_FILE = "bweas-config.json";
static constexpr auto DIRWORK_ENV      = ".bweas";

static logger _log{"BWEAS"};

builder::builder(size_t argv, char **args) : path_bweas_to_build(DIRWORK_ENV) {
    vec<string> vec_args;
    for (size_t i = 0; i < argv; ++i)
        vec_args.push_back(args[i]);

    handle_args(vec_args);

    if (mode_bweas != mode_working::build_package && mode_bweas != mode_working::undef) {
        (_log << bwtools::message) << (log_message(log_type::msg)
                                       << "Initializing system build - bweas " + version.get_str_version());
        init();
    }
}

void builder::handle_args(vec<string> &args) {
    path_bweas_config   = fs::current_path().string();
    path_bweas_to_build = fs::current_path().string();

    if (args.size() == 1)
        mode_bweas = mode_working::collect_cfg;

    string path_json_cfg_package;

    bool expected_path_bweas_config = 1, expected_path_to_build = 0;
    bool expected_path_json_cfg_package = 0;
    for (size_t i = 1; i < args.size(); ++i) {
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
                (_log << bwtools::message) << (log_message(log_type::msg) << HELP_STR);
            else if (args[i] == "version")
                (_log << bwtools::message) << (log_message(log_type::msg) << INFO_STR);
            else {
                (_log << bwtools::error) << (log_message(log_type::error) << "Unknown argument: " + args[i]);
                return;
            }
        }
        else {
            // --cfg <arg> or arg
            if (expected_path_bweas_config) {
                path_bweas_config = fs::absolute(args[i]).string();
                mode_bweas        = mode_working::collect_cfg;

                expected_path_bweas_config = 0;
            }
            // --build <arg>
            else if (expected_path_to_build) {
                path_bweas_to_build    = fs::absolute(args[i]).string();
                expected_path_to_build = 0;
            }
            // --package <arg>
            else if (expected_path_json_cfg_package) {
                path_json_cfg_package          = args[i];
                expected_path_json_cfg_package = 0;

                size_t size_pckg = create_package(path_json_cfg_package);

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
        (_log << bwtools::error) << (log_message(log_type::error) << "Invalid syntax.\n" << HELP_STR);
}

size_t builder::create_package(string path_json_config_package) {
    package::data_bw_package data_package;
    const file_it &json_config_package = bwtools::open_file(path_json_config_package, mf::open::r);

    if (!bwtools::exist_file(json_config_package)) {
        (_log << bwtools::error) << (log_message(log_type::error)
                                     << "The bweas-json configuration for package package file was not found");
        return 0;
    }
    data_package.json_config = bwtools::read_file(bwtools::get_ref_file(json_config_package), mf::input::read_default);
    bwtools::close_file(json_config_package);

    package loaded_package;
    string pckg = loaded_package.init(data_package, 1);
    if (_log.error_status()) {
        (_log << bwtools::error) << (log_message(log_type::error) << "Failed to create a bweas package");
        return 0;
    }
    const file_it &package = bwtools::open_file(
        fs::current_path().string() + "/" + loaded_package.name_package + FORMAT_PACKAGE, mf::open::wb);
    bwtools::write_file(bwtools::get_ref_file(package), pckg, mf::output::write_binary);
    bwtools::close_file(package);

    return pckg.size();
}

void builder::init() {
    fs::current_path(path_bweas_config);

    path_bweas_config = path_bweas_config + "/" + CONFIG_FILE;
    string bweas_path = bwtools::get_path_program() + "/" + JSON_CONFIG_FILE;

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
            cache = std::unique_ptr<cache_api::base_cache>(cache_api::base_cache::create_fast_cache(&context));
        else if (config_json["cache-gn"] == "json_bwcache")
            cache = std::unique_ptr<cache_api::base_cache>(cache_api::base_cache::create_json_cache(&context));
    }
    else
        cache = std::unique_ptr<cache_api::base_cache>(cache_api::base_cache::create_fast_cache(&context));

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

            bweas::package loaded_package;
            string raw_data_package,
                path_to_package{bwtools::get_path_program() + "/packages/" + (string)package.value() + FORMAT_PACKAGE};

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

            if (cache == NULL &&
                loaded_package.cfg_package.cache.name_cache == config_json["cache-gn"].template get<std::string>())
                cache = std::unique_ptr<cache_api::base_cache>(
                    cache_api::base_cache::create_lua_cache(&context, loaded_package.cfg_package.cache.src_lua_cache));
            _log << bwtools::success
                 << (log_message(log_type::msg)
                     << "\"" << path_to_package << "\" bweas package was loaded successfully("
                     << raw_data_package.size() << " bytes)");
        }
    }

    generators.emplace(
        "bwgenerator",
        std::shared_ptr<generator_api::base_generator>(generator_api::base_generator::create_generator_int(
            nullptr, integral_generator::get_input_files, integral_generator::generate)));

    for (const auto &package : loaded_packages)
        for (const auto &generator : package.cfg_package.generators) {
            generators.emplace(generator.name_generator,
                               std::shared_ptr<generator_api::base_generator>(
                                   generator_api::base_generator::create_generator_lua(generator.src_lua_generator)));
            if (generator.use_custom_build_graph_depends)
                generators[generator.name_generator]->set_use_build_graph_depends();
        }
    for (auto &package : loaded_packages) {
        auto module_funcs = module_m.init_mfuncs(package.cfg_package.modules);
        for (const auto &module_func : module_funcs)
            external_modules_funcs.push_back(module_func);
    }
}

builder::mode_working builder::get_current_mode() {
    return mode_bweas;
}

void builder::start() {
    if (mode_bweas == mode_working::build_package)
        return;
    else if (!bwtools::exist_file(path_bweas_config))
        (_log << bwtools::fatal) << (log_message(log_type::fatal)
                                     << "Unable to open configuration file \'" << path_bweas_config << "\'");

    if (mode_bweas == mode_working::build) {
        string cache_file = path_bweas_to_build + "/" + CACHE_FILE;
        if (!bwtools::exist_file(cache_file))
            (_log << bwtools::fatal) << (log_message(log_type::fatal) << "Bweas cache not found");
        fs::path main_file = path_bweas_config;

        fs::file_time_type cftime = fs::last_write_time(cache_file);
        fs::file_time_type mftime = fs::last_write_time(main_file);
        if (mftime >= cftime) {
            (_log << bwtools::message) << (log_message(log_type::msg) << "The configuration file has been modified");
            goto interpreter_start;
        }

        deserl_cache();
    }
    else if (mode_bweas == mode_working::collect_cfg || mode_bweas == mode_working::collect_cfg_w_build) {
    interpreter_start:
        run_interpreter();

        fs::current_path(path_bweas_to_build);
        if (!gen_cache_target())
            return;
    }

    if (mode_bweas == mode_working::build || mode_bweas == mode_working::collect_cfg_w_build) {
        (_log << bwtools::message) << (log_message(log_type::msg) << "*Build start time: " << bwtools::get_time());
        build_targets();
    }
}

void builder::run_interpreter() {
    lang bwlang{path_bweas_config};
    bwlang.init_external_funcs(external_modules_funcs);
    for (const auto &loaded_package : loaded_packages)
        bwlang.set_custom_ext_fields_project(loaded_package.cfg_package.custom_ext_fields_project);

    (_log << bwtools::message) << (log_message(log_type::msg) << "Interpreting the configuration file...");
    bwlang.execute();
    (_log << bwtools::success) << (log_message(log_type::msg) << "The interpretation was successful!");

    context = bwlang.get_context();
}

size_t builder::gen_cache_target() {
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

void builder::deserl_cache() {
    file_it bweas_cache = bwtools::open_file(CACHE_FILE, mf::open::rb);

    (_log << bwtools::message) << (log_message(log_type::msg) << "Cache deserialization...");

    cache->extract_cache_data(bwtools::read_file(bwtools::get_ref_file(bweas_cache)));

    (_log << bwtools::success) << (log_message(log_type::msg)
                                   << "Cache deserialization was successful!\n"
                                   << "Was loaded " << context.templates.size() << " template of command!");
}

depends_files::depends_map builder::load_depends_file(std::unique_ptr<depends_files> &_depends_files,
                                                      const sc::target_out target) {
    _depends_files->set_include_paths(target.prj.include_paths);
    string depends_str;

    if (bwtools::exist_file(DEPENDS_FILE)) {
        depends_str = bwtools::read_file(bwtools::get_ref_file(bwtools::open_file(DEPENDS_FILE)));

        size_t it;
        for (const auto &name_file : target.prj.src_files) {
            if ((it = depends_str.find(name_file + ":")) != depends_str.npos) {
                string depends_file_str = depends_str.erase(0, it + name_file.size() + 2);

                if ((it = depends_file_str.find(":")) != depends_file_str.npos)
                    depends_file_str.erase(it, depends_file_str.size());
                _depends_files->build_graph_depends_file_string(name_file, depends_file_str);
            }
            else
                goto find_depends_file;
        }
    }
    else {
    find_depends_file:
        for (const auto &name_file : target.prj.src_files) {
            _depends_files->build_graphs_depends_file_v(name_file);
            depends_str += name_file + ":\n" + _depends_files->get_string_depends_file(name_file);
        }
        bwtools::write_file(bwtools::get_ref_file(bwtools::open_file(DEPENDS_FILE, mf::open::w)), depends_str);
    }

    return _depends_files->get_graphs_depends_files();
}

void builder::build_targets() {
    (_log << bwtools::message) << (log_message(log_type::msg) << "Building targets...");

    fs::current_path(path_bweas_to_build);

    context.global_external_args.push_back(pair<string, string>("", ""));
    generator_api::data_transfer data_t{&context};

    for (auto &target : context.out_targets) {
        if (generators.find(target.name_generator) == generators.end()) {
            (_log << bwtools::error)
                << (log_message(log_type::error)
                    << "The provided generator as the primary for the current target was not found - " << target.name
                    << ": " << target.name_generator);
            return;
        }
        else if (target.prj.vec_templates.size() == 0) {
            (_log << bwtools::error) << (log_message(log_type::error)
                                         << "There are no templates for the target - " << target.name);
            return;
        }
        (_log << bwtools::message) << (log_message(log_type::msg) << "Build target: " << target.name);

        target.queue_templates = sc::template_command::create_queue_target_templates(
            context.templates, target.prj.vec_templates, target.type);
        context.current_target         = &target;
        context.current_work_directory = path_bweas_to_build + "/" + target.name;

        if (!fs::is_directory(context.current_work_directory))
            fs::create_directories(context.current_work_directory);

        auto &current_generator = generators[target.name_generator];
        current_generator->init();

        std::unique_ptr<depends_files> depends_files;
        if (current_generator->has_build_graph_depends())
            depends_files = std::make_unique<depends_generator>(current_generator, target.prj.language,
                                                                context.current_work_directory);
        else
            depends_files = std::make_unique<depends_integral>(target.prj.language, context.current_work_directory);

        data_t.dfiles = load_depends_file(depends_files, target);

        current_generator->get_input_files(data_t);
        generator_api::commands cmd_s = current_generator->generate_commands(data_t);

        (_log << bwtools::success) << (log_message(log_type::msg) << cmd_s.size() << " commands generated");

        processes_handler p_handler(cmd_s, 4);
        p_handler.start([&cmd_s](const generator_api::command &cmd) {
            static double build_state = 0.f;

            if (!cmd.success)
                (_log << bwtools::fatal) << (log_message(log_type::fatal)
                                             << "Command execution failed: compile " << cmd.name_used_file);
            else {
                build_state += 1.f / (double)cmd_s.size() * 100;
                (_log << bwtools::success)
                    << (log_message(log_type::msg)
                        << "[" << std::to_string(build_state).erase(std::to_string((size_t)build_state).size() + 2, 5)
                        << "%] " << cmd.name_used_file);
            }
        });

        (_log << bwtools::success) << (log_message(log_type::msg) << "Successfully built target");
    }
}

