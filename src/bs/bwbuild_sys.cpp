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

static logger _log{"", 1};

static constexpr auto JSON_CONFIG_FILE          = "bweas-config.json";
static constexpr auto DEFAULT_BWEAS_JSON_CONFIG = "{\n\t\"cache-gn\": \"fast_bwcache\",\n}";

static const string INFO_STR =
    "bweas version " + std::string(VERSION_FULL_STR) + "\nrep on github - https://github.com/svec1/bweas";

static constexpr auto HELP_STR =
    "bweas-call: \n   bweas <parameter>... path_depending\n   bweas path_bweas_config <parameter>..."
    "\nAcceptable parameters:"
    "\n   --build - builds the project (either by executing the configuration file or deserializing the cache file "
    "if it exists)"
    "\n   --cfg - executes the configuration file if it has been changed and creates a new cache file"
    "\n   --threads - sets the default number of threads for the build"
    "\n   --help - outputs the syntax of the bweas call as well as its possible functions"
    "\n   --version - outputs the version of bweas";

builder::builder(size_t argv, char **args) {
    vec<string> vec_args;
    for (size_t i = 0; i < argv; ++i)
        vec_args.push_back(args[i]);

    try {
        handle_args(vec_args);

        if (mode_bweas != mode_working::undef)
            init();
    }
    catch (const bweas::exception &excp) {
        _log.handle_exception(excp);
    }
}

void builder::handle_args(vec<string> &args) {
    _context.path_bweas_config   = fs::current_path();
    _context.path_bweas_to_build = fs::current_path();

    if (args.size() == 1)
        mode_bweas = mode_working::collect_cfg;

    bool expected_path_bweas_config = 1, expected_path_to_build = 0;
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
            else if (args[i] == "nolog")
                _log.set_global_log(false);
            else if (args[i] == "help")
                _log << (log_message(log_type::msg) << HELP_STR);
            else if (args[i] == "version")
                _log << (log_message(log_type::msg) << INFO_STR);
            else if (args[i] == "ry")
                _log.set_next_yes();
            else if (size_t it = args[i].find("="); it != args[i].npos) {
                if (args[i].find("threads") == 0 && it == 7) {
                    string value = args[i].substr(8);
                    if (value == "max")
                        count_threads = -1;
                    else
                        count_threads = std::atoll(value.c_str());

                    if (!_log.request_yes_no(count_threads > 10, "A large number of threads may cause the system to "
                                                                 "malfunction or crash, do you really need this?")) {

                        count_threads = THREADS_COUNT_DEFAULT;
                    }
                }
                else
                    goto unknown_arg;
            }
            else {
            unknown_arg:
                _log << (log_message(log_type::error) << "Unknown argument: " + args[i]);
                return;
            }
        }
        else {
            // --cfg <arg> or arg
            if (expected_path_bweas_config) {
                _context.path_bweas_config = fs::absolute(args[i]).lexically_normal();
                mode_bweas                 = mode_working::collect_cfg;

                expected_path_bweas_config = 0;
            }
            // --build <arg>
            else if (expected_path_to_build) {
                _context.path_bweas_to_build = fs::absolute(args[i]).lexically_normal();
                expected_path_to_build       = 0;
            }
            else {
                _log << (log_message(log_type::error) << "Unknown argument: " + args[i]);
                return;
            }
        }
    }
}

void builder::init() {
    fs::current_path(_context.path_bweas_config);

    _context.path_bweas_config = _context.path_bweas_config / CONFIG_FILE;

    nlohmann::json config_json;
    auto file_json_config =
        file_utils::open_file((file_utils::get_path_program() / JSON_CONFIG_FILE).string(), mf::open::r);
    if (!file_json_config.is_open) {
        file_json_config.open(mf::open::w);
        file_utils::write_file(file_json_config, DEFAULT_BWEAS_JSON_CONFIG);

        config_json = nlohmann::json::parse(DEFAULT_BWEAS_JSON_CONFIG);
    }
    else {
        config_json = nlohmann::json::parse(file_utils::read_file(file_json_config, mf::input::read_default));
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

    string path_to_packages = (file_utils::get_path_program() / "packages").string();
    if (!fs::is_directory(path_to_packages))
        _log << (log_message(log_type::error) << "\"" << path_to_packages << "\" package directory is expected.");
    else {
        for (const auto &fs_object : fs::directory_iterator(path_to_packages)) {
            string name_file = fs_object.path().string();
            if (!fs::is_regular_file(fs_object) || name_file.find(".json") != name_file.size() - 5)
                continue;

            auto it_package = file_utils::open_file(name_file, mf::open::rb);
            if (!it_package.is_open) {
                _log << (log_message(log_type::warning) << "\"" << name_file << "\" bweas package not found");
                continue;
            }

            package current_pkg(file_utils::read_file(it_package, mf::input::read_binary));
            loaded_packages.push_back(current_pkg);

            if (cache == NULL && current_pkg.cfg.cache.name == config_json["cache-gn"].template get<std::string>())
                cache = std::unique_ptr<cache_api::base_cache>(
                    cache_api::base_cache::create_lua_cache(current_pkg.cfg.cache.src_lua));

            _log << (log_message(log_type::success) << "\'" << name_file << "\' loaded");
        }

        module_manager module_m;
        for (auto &package : loaded_packages) {
            auto package_modules = module_m.init_modules(package.cfg.modules);
            modules.insert(modules.begin(), package_modules.begin(), package_modules.end());
        }
    }

    if (cache == NULL)
        _log << (log_message(log_type::fatal) << "\"" << config_json["cache-gn"] << "\" - unknown cache generator.");
    cache->init(&_context);
}

builder::mode_working builder::get_current_mode() {
    return mode_bweas;
}

void builder::start() {
    try {
        if (mode_bweas == mode_working::build) {
            auto bweas_cache = file_utils::open_file(CACHE_FILE, mf::open::rb);
            string cache_str = file_utils::read_file(bweas_cache);

            _context.path_bweas_config = cache->get_path_config(cache_str);
            fs::path path_bweas_cache  = _context.path_bweas_to_build / CACHE_FILE;

            if (!fs::exists(_context.path_bweas_config))
                _log << (log_message(log_type::fatal) << "Bweas config not found");
            if (!fs::exists(path_bweas_cache))
                _log << (log_message(log_type::fatal) << "Bweas cache not found");

            fs::file_time_type config_ftime = fs::last_write_time(_context.path_bweas_config);
            fs::file_time_type cache_ftime  = fs::last_write_time(path_bweas_cache);
            if (config_ftime > cache_ftime) {
                _log << (log_message(log_type::msg) << "The configuration file has been modified");
                goto interpreter_start;
            }

            cache->extract_cache_data(cache_str);

            _log << (log_message(log_type::success)
                     << "Cache deserialization was successful(loaded " << _context.targets.size() << " targets)");
        }
        else if (mode_bweas == mode_working::collect_cfg || mode_bweas == mode_working::collect_cfg_w_build) {
            if (!fs::exists(_context.path_bweas_config))
                _log << (log_message(log_type::fatal)
                         << "Bweas config not found \'" << _context.path_bweas_config << "\'");

        interpreter_start:
            fs::current_path(fs::path(_context.path_bweas_config).parent_path());
            run_interpreter();

            if (logger::global_status == log_type::error)
                return;

            fs::current_path(_context.path_bweas_to_build);
            if (gen_cache_target())
                return;
        }

        if (mode_bweas == mode_working::build || mode_bweas == mode_working::collect_cfg_w_build) {
            _log << (log_message(log_type::msg) << "Build start time: " << file_utils::get_time());
            build_targets();
        }
    }
    catch (const bweas::exception &excp) {
        _log.handle_exception(excp);
    }
}

void builder::run_interpreter() {
    try {
        lang bwlang{&_context,
                    utils::file_utils::read_file(utils::file_utils::open_file(_context.path_bweas_config.string()))};

        bwlang.import_std_module();
        bwlang.import_bweas_build_module();
        bwlang.import_modules(modules);
        bwlang.execute();
        bwlang.init_context();
    }
    catch (std::runtime_error &excp) {
        _log << (log_message(log_type::fatal) << "Error executing the configuration file.\n" << excp.what());
    }

    _log << (log_message(log_type::msg) << "Successful configuration of targets.");
}

size_t builder::gen_cache_target() {
    if (!_context.targets.size()) {
        _log << (log_message(log_type::warning) << "Generating a file with a cache will not be performed.");
        return 1;
    }

    auto bweas_cache = file_utils::open_file(CACHE_FILE, mf::open::w);

    file_utils::write_file(bweas_cache, cache->create_cache(), mf::output::write_binary);

    _log << (log_message(log_type::success) << "Cache generation was successful!");

    return 0;
}

dependency_finder::dependency_map &builder::load_depends_file(dependency_finder &dfinder,
                                                              const vec<string> &include_paths,
                                                              const vec<string> &source_files) {
    log_console_unlock unlock_c;
    string depends_str;

    if (fs::exists(DEPENDS_FILE)) {
        depends_str = file_utils::read_file(file_utils::open_file(DEPENDS_FILE));

        size_t it;
        for (const auto &name_file : source_files) {
            if ((it = depends_str.find(name_file + ":")) != depends_str.npos) {
                string depends_file_str = depends_str.erase(0, it + name_file.size() + 2);

                if ((it = depends_file_str.find(":")) != depends_file_str.npos)
                    depends_file_str.erase(it, depends_file_str.size());
                dfinder.build_graph_depends_file_string(name_file, depends_file_str);
            }
            else
                goto find_depends_file;
        }
    }
    else {
    find_depends_file:
        for (const auto &name_file : source_files) {
            dfinder.build_graph(include_paths, name_file);
            depends_str += name_file + ":\n" + dfinder.get_string_depends_file(name_file);
        }

        file_utils::write_file(file_utils::open_file(DEPENDS_FILE, mf::open::w), depends_str);
    }

    return dfinder.get_graphs_depends_files();
}

void builder::build_targets() {
    fs::current_path(_context.path_bweas_to_build);

    std::sort(_context.targets.begin(), _context.targets.end(), [](const sc::target &t1, const sc::target &t2) {
        return std::find(t1.dependencies.begin(), t1.dependencies.end(), t2.name) == t1.dependencies.end() ? 1 : 0;
    });

    size_t current_count_threads;
    static auto check_target_field = [&](const sc::target &trg) {
        if (auto value = trg.ext.get_if<pdiff>("threads"); value)
            current_count_threads = *value;

        if (!trg.ext.contains(sc::profile::FIELD_SOURCE_FILES))
            _log << (log_message(log_type::fatal) << "Expected \'" << sc::profile::FIELD_SOURCE_FILES << "\' field.");
        else if (!trg.ext.contains(sc::profile::FIELD_INCLUDE_PATHS))
            _log << (log_message(log_type::fatal) << "Expected \'" << sc::profile::FIELD_INCLUDE_PATHS << "\' field.");
    };

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

        current_count_threads = count_threads;
        check_target_field(target);

        _log << (log_message(log_type::msg)
                 << "Build target("
                 << ((current_count_threads == (size_t)-1) ? string("max") : std::to_string(current_count_threads))
                 << " threads): " << target.name);

        size_t count_errors = 0;
        {
            log_console_lock lock_c;

            generator_command generator(&_context);

            target.queue_templates =
                sc::template_command::create_queue_target_templates(_context.templates, target.templates);
            _context.current_target         = &target;
            _context.current_work_directory = _context.path_bweas_to_build / target.name;

            if (!fs::is_directory(_context.current_work_directory))
                fs::create_directories(_context.current_work_directory);

            if (!target.ext.lang.dfinder_data.search_regex.empty()) {
                dependency_finder dfinder(target.ext.lang.dfinder_data.search_regex,
                                          target.ext.lang.dfinder_data.char_global_search);
                _context.dfiles =
                    load_depends_file(dfinder, target.fields<vec<string>>(sc::profile::FIELD_INCLUDE_PATHS),
                                      target.fields<vec<string>>(sc::profile::FIELD_SOURCE_FILES));
            }

            generator.get_input_files();
            commands cmd_s = generator.generate();

            double build_state = 0.f;

            processes_handler p_handler(cmd_s, current_count_threads);
            p_handler.start([&cmd_s, &build_state, &count_errors](string_v name_output_file, bool success) {
                log_console_unlock unlock_c;
                if (!success) {
                    _log << (log_message(log_type::error) << "failed: " << name_output_file);
                    ++count_errors;
                }
                else {
                    build_state += 1.f / (double)cmd_s.size() * 100;
                    _log << (log_message(log_type::success)
                             << "[" << std::right << std::setw(5)
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
