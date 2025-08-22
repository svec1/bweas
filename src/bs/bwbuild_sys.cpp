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
#include <bwgenerator_command.hpp>
#include <bwlang.hpp>
#include <bwprocesses_handler.hpp>
#include <bwstructs_context.hpp>

#include <nlohmann/json.hpp>

using namespace bweas;
using namespace bweas::utils;

static logger _log{"BWEAS"};

static constexpr auto JSON_CONFIG_FILE          = "bweas-config.json";
static constexpr auto DEFAULT_BWEAS_JSON_CONFIG = "{\n\t\"cache-gn\": \"fast_bwcache\",\n\t\"packages\": []\n}";

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

builder::builder(size_t argv, char **args) {
    vec<string> vec_args;
    for (size_t i = 0; i < argv; ++i)
        vec_args.push_back(args[i]);

    handle_args(vec_args);

    if (mode_bweas != mode_working::build_package && mode_bweas != mode_working::undef) {
        _log << (log_message(log_type::msg) << "Initializing system build - bweas " + version.get_str_version());
        init();
    }
}

void builder::handle_args(vec<string> &args) {
    _context.path_bweas_config   = fs::current_path().string() + "/";
    _context.path_bweas_to_build = fs::current_path().string() + "/";

    if (args.size() == 1)
        mode_bweas = mode_working::collect_cfg;

    bool expected_path_bweas_config = 1, expected_path_to_build = 0;
    bool expected_path_json_cfg_package = 0;
    for (size_t i = 1; i < args.size(); ++i) {
        if (args[i].find("--") == 0) {
            args[i].erase(args[i].find("--"), 2);

            if (mode_bweas != undef) {
                _log << (log_message(log_type::error)
                         << "Unknown argument: " + args[i] + ". The bweas mode has already been defined");
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
                _log << (log_message(log_type::msg) << HELP_STR);
            else if (args[i] == "version")
                _log << (log_message(log_type::msg) << INFO_STR);
            else {
                _log << (log_message(log_type::error) << "Unknown argument: " + args[i]);
                return;
            }
        }
        else {
            // --cfg <arg> or arg
            if (expected_path_bweas_config) {
                _context.path_bweas_config = fs::absolute(args[i]).lexically_normal().string();
                mode_bweas                 = mode_working::collect_cfg;

                expected_path_bweas_config = 0;
            }
            // --build <arg>
            else if (expected_path_to_build) {
                _context.path_bweas_to_build = fs::absolute(args[i]).lexically_normal().string();
                expected_path_to_build       = 0;
            }
            // --package <arg>
            else if (expected_path_json_cfg_package) {
                expected_path_json_cfg_package = 0;

                create_package(args[i]);
            }
            else {
                _log << (log_message(log_type::error) << "Unknown argument: " + args[i]);
                return;
            }
        }
    }

    if (expected_path_json_cfg_package)
        _log << (log_message(log_type::error) << "Invalid syntax.\n" << HELP_STR);
}

void builder::create_package(string path_json_config_package) {
    package::data_bw_package data_package;

    file_it json_config_package = file_utils::open_file(path_json_config_package, mf::open::r);
    if (!file_utils::exist_file(json_config_package)) {
        _log << (log_message(log_type::error) << "The bweas-json configuration for package package file was not found");
        return;
    }

    data_package.json_config =
        file_utils::read_file(file_utils::get_ref_file(json_config_package), mf::input::read_default);
    file_utils::close_file(json_config_package);

    package loaded_package;
    string pckg = loaded_package.init(data_package, 1);
    if (_log.error_status()) {
        _log << (log_message(log_type::error) << "Failed to create a bweas package");
        return;
    }
    file_it package =
        file_utils::open_file(fs::current_path().string() + "/" + loaded_package.name + FORMAT_PACKAGE, mf::open::wb);
    file_utils::write_file(file_utils::get_ref_file(package), pckg, mf::output::write_binary);
    file_utils::close_file(package);
}

void builder::init() {
    fs::current_path(_context.path_bweas_config);

    _context.path_bweas_config    = _context.path_bweas_config + CONFIG_FILE;
    string bweas_json_config_path = file_utils::get_path_program() + JSON_CONFIG_FILE;

    nlohmann::json config_json;
    file_it file_json_config = file_utils::open_file(bweas_json_config_path, mf::open::r);
    if (file_utils::exist_file(file_json_config) && !file_utils::get_ref_file(file_json_config).file_opened) {
        file_utils::get_ref_file(file_json_config).open(mf::open::w);
        file_utils::write_file(file_utils::get_ref_file(file_json_config), DEFAULT_BWEAS_JSON_CONFIG);
        file_utils::close_file(file_json_config);

        config_json = nlohmann::json::parse(DEFAULT_BWEAS_JSON_CONFIG);
    }
    else {
        config_json = nlohmann::json::parse(
            file_utils::read_file(file_utils::get_ref_file(file_json_config), mf::input::read_default));
        file_utils::close_file(file_json_config);
    }

    if (config_json.contains("cache-gn")) {
        if (!config_json["cache-gn"].is_string()) {
            _log << (log_message(log_type::error) << "Cache generator name expected");
            return;
        }
        if (config_json["cache-gn"] == "fast_bwcache")
            cache = std::unique_ptr<cache_api::base_cache>(cache_api::base_cache::create_fast_cache());
        else if (config_json["cache-gn"] == "json_bwcache")
            cache = std::unique_ptr<cache_api::base_cache>(cache_api::base_cache::create_json_cache());
    }
    else
        cache = std::unique_ptr<cache_api::base_cache>(cache_api::base_cache::create_fast_cache());

    vec<package> loaded_packages;
    package loaded_package;

    string path_to_packages{file_utils::get_path_program() + "packages/"};
    for (const auto &fs_object : fs::directory_iterator(path_to_packages)) {
        if (!fs::is_regular_file(fs_object))
            continue;

        file_it it_package = file_utils::open_file(fs_object.path().string(), mf::open::rb);
        if (!file_utils::exist_file(it_package)) {
            _log << (log_message(log_type::warning)
                     << "\"" << fs_object.path().string() << "\" bweas package not found");
            continue;
        }
        string raw_data_package = file_utils::read_file(file_utils::get_ref_file(it_package), mf::input::read_binary);

        loaded_package.load(raw_data_package);
        loaded_packages.push_back(loaded_package);

        if (cache == NULL && loaded_package.cfg.cache.name == config_json["cache-gn"].template get<std::string>())
            cache = std::unique_ptr<cache_api::base_cache>(
                cache_api::base_cache::create_lua_cache(loaded_package.cfg.cache.src_lua));

        file_utils::close_file(it_package);

        _log << (log_message(log_type::success)
                 << "\"" << fs_object.path().string() << "\" bweas package was loaded successfully("
                 << raw_data_package.size() << " bytes)");
    }

    cache->init(&_context);

    dependency_finders.emplace("CXX", std::shared_ptr<depends_files>(depends_files::create_depends_integral_cxx()));

    for (auto &package : loaded_packages) {
        for (const auto &finder : package.cfg.finders) {
            dependency_finders.emplace(
                finder.language, std::shared_ptr<depends_files>(depends_files::create_depends_lua(finder.src_lua)));
        }

        auto package_modules = module_m.init_modules(package.cfg.modules);
        modules.merge(std::move(package_modules));
    }
}

builder::mode_working builder::get_current_mode() {
    return mode_bweas;
}

void builder::start() {
    if (mode_bweas == mode_working::build_package)
        return;
    else if (mode_bweas == mode_working::build) {
        file_it bweas_cache = file_utils::open_file(CACHE_FILE, mf::open::rb);
        string cache_str    = file_utils::read_file(file_utils::get_ref_file(bweas_cache));

        _context.path_bweas_config = cache->get_path_config(cache_str);
        string path_bweas_cache    = _context.path_bweas_to_build + CACHE_FILE;

        if (!file_utils::exist_file(_context.path_bweas_config))
            _log << (log_message(log_type::fatal) << "Bweas config not found");
        if (!file_utils::exist_file(path_bweas_cache))
            _log << (log_message(log_type::fatal) << "Bweas cache not found");

        fs::file_time_type config_ftime = fs::last_write_time(_context.path_bweas_config);
        fs::file_time_type cache_ftime  = fs::last_write_time(path_bweas_cache);
        if (config_ftime > cache_ftime) {
            _log << (log_message(log_type::msg) << "The configuration file has been modified");
            goto interpreter_start;
        }

        cache->extract_cache_data(cache_str);

        _log << (log_message(log_type::success)
                 << "Cache deserialization was successful!\n"
                 << "Was loaded " << _context.templates.size() << " template of command!");
    }
    else if (mode_bweas == mode_working::collect_cfg || mode_bweas == mode_working::collect_cfg_w_build) {
        if (!file_utils::exist_file(_context.path_bweas_config))
            _log << (log_message(log_type::fatal) << "Bweas config not found \'" << _context.path_bweas_config << "\'");

    interpreter_start:
        fs::current_path(fs::path(_context.path_bweas_config).parent_path());
        run_interpreter();

        fs::current_path(_context.path_bweas_to_build);
        if (gen_cache_target())
            return;
    }

    if (mode_bweas == mode_working::build || mode_bweas == mode_working::collect_cfg_w_build) {
        _log << (log_message(log_type::msg) << "*Build start time: " << file_utils::get_time());
        build_targets();
    }
}

void builder::run_interpreter() {
    _log << (log_message(log_type::msg) << "Interpreting the configuration file...");

    lang bwlang{&_context};

    bwlang.get_container_vars<scope::module_data>() = std::move(modules);

    bwlang.execute();

    bwlang.init_context();
}

size_t builder::gen_cache_target() {
    if (!_context.targets.size()) {
        _log << (log_message(log_type::warning) << "Generating a file with a cache will not be performed.");
        return 1;
    }

    file_it bweas_cache = file_utils::open_file(CACHE_FILE, mf::open::w);

    file_utils::write_file(file_utils::get_ref_file(bweas_cache), cache->create_cache(), mf::output::write_binary);
    file_utils::close_file(bweas_cache);

    _log << (log_message(log_type::success) << "Cache generation was successful!");

    return 0;
}

depends_files::depends_map &builder::load_depends_file(std::shared_ptr<depends_files> &dfinder,
                                                       const vec<string> &include_paths,
                                                       const vec<string> &source_files) {
    dfinder->set_include_paths(include_paths);
    string depends_str;

    if (file_utils::exist_file(DEPENDS_FILE)) {
        depends_str = file_utils::read_file(file_utils::get_ref_file(file_utils::open_file(DEPENDS_FILE)));

        size_t it;
        for (const auto &name_file : source_files) {
            if ((it = depends_str.find(name_file + ":")) != depends_str.npos) {
                string depends_file_str = depends_str.erase(0, it + name_file.size() + 2);

                if ((it = depends_file_str.find(":")) != depends_file_str.npos)
                    depends_file_str.erase(it, depends_file_str.size());
                dfinder->build_graph_depends_file_string(name_file, depends_file_str);
            }
            else
                goto find_depends_file;
        }
    }
    else {
    find_depends_file:
        for (const auto &name_file : source_files) {
            dfinder->build_graphs_depends_file_v(name_file);
            depends_str += name_file + ":\n" + dfinder->get_string_depends_file(name_file);
        }
        file_utils::write_file(file_utils::get_ref_file(file_utils::open_file(DEPENDS_FILE, mf::open::w)), depends_str);
    }

    return dfinder->get_graphs_depends_files();
}

void builder::build_targets() {
    fs::current_path(_context.path_bweas_to_build);

    std::sort(_context.targets.begin(), _context.targets.end(), [](const sc::target &t1, const sc::target &t2) {
        return std::find(t1.dependencies.begin(), t1.dependencies.end(), t2.name) == t1.dependencies.end() ? 1 : 0;
    });

    for (auto &target : _context.targets) {
        for (const auto &dependence : target.dependencies)
            if (auto it = std::find_if(_context.targets.begin(), _context.targets.end(),
                                       [&dependence](const sc::target &target) { return target.name == dependence; });
                it != _context.targets.end() && !it->built_success)
                _log << (log_message(log_type::fatal) << target.name << " target expects a dependency: " << dependence);

        if (target.templates.size() == 0) {
            _log << (log_message(log_type::error) << "There are no templates for the target - " << target.name);
            return;
        }
        _log << (log_message(log_type::msg) << "Build target: " << target.name);

        size_t count_errors = 0;
        {
            log_console_lock lock_c;

            generator_command generator(&_context);

            target.queue_templates =
                sc::template_command::create_queue_target_templates(_context.templates, target.templates, target.type);
            _context.current_target         = &target;
            _context.current_work_directory = _context.path_bweas_to_build + target.name;

            if (!fs::is_directory(_context.current_work_directory))
                fs::create_directories(_context.current_work_directory);

            if (dependency_finders.contains(target.fields<string>("language")))
                _context.dfiles = load_depends_file(dependency_finders[target.fields<string>("language")],
                                                    target.fields<vec<string>>("inlclude_paths"),
                                                    target.fields<vec<string>>("source_files"));

            generator.get_input_files();
            commands cmd_s = generator.generate();

            double build_state = 0.f;

            processes_handler p_handler(cmd_s, 4);
            p_handler.start([&cmd_s, &build_state, &count_errors](string_v name_output_file, bool success) {
                log_console_unlock unlock_c;
                if (!success) {
                    _log << (log_message(log_type::error) << "failed: " << name_output_file);
                    ++count_errors;
                }
                else {
                    build_state += 1.f / (double)cmd_s.size() * 100;
                    _log << (log_message(log_type::success)
                             << "["
                             << std::to_string(build_state).erase(std::to_string((size_t)build_state).size() + 2, 5)
                             << "%] " << name_output_file);
                }
            });
        }

        if (count_errors)
            _log << (log_message(log_type::error)
                     << target.name << " target was not built: " << count_errors << " errors");
        else
            target.built_success = 1;
    }
}

