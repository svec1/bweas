//
// BWEAS is distributed under the GNU General Public License 2.0 (GPL-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWBUILD_SYS__H
#define BWBUILD_SYS__H

#include "../mdef.hpp"

#include <stack>

#include "bw_defs.hpp"
#include "bwcache_api.hpp"
#include "bwgenerator_api.hpp"
#include "bwmodule.hpp"
#include "bwpackage.hpp"

namespace bweas {

// Builder class. It is a holistic program. It has operating modes (mode_working), which itself determines by passing
// parameters to it when launching programs
class bwbuilder final {
  public:
    bwbuilder() = delete;
    bwbuilder(int argv, char **args);

    bwbuilder(bwbuilder &&) = delete;
    bwbuilder(const bwbuilder &) = delete;
    bwbuilder &operator=(bwbuilder &&) = delete;

    ~bwbuilder() = default;

  public:
    // all possible bweas operating modes
    enum mode_working {
        collect_cfg = 0,
        build,
        collect_cfg_w_build,
        build_package,
        undef
    };

    // returns the current operating mode of bweas
    mode_working get_current_mode();

    // Depending on the existence of the cache file and its parameters that were transferred when the program was
    // launched, the build begins.
    // ----
    // - If the option: --build was passed to the builder, the builder (if a cache file exists) will begin building the
    // targets by deselecting the bwcache file.
    // However, the parameter: --cfg will force the build system to interpret bweasconf.txt, creating a cache file
    // (bwcache) based on it and also building targets.
    // - If the parameter is specified: --package. The builder will generate a bweas package based on two files passed
    // to it (package_config.json - the name can be anything, lua_generator.lua - the name can be anything). Details:
    // after specifying --package you must list two files: package configuration and lua generator, respectively
    void start();

  protected:
    // Controls how bweas works, depending on the arguments provided.
    // -----
    // ### Struct of call bweas:
    // #### <arg1>(mode), <arg2>(path_depending)...
    // #### <arg1>(path_to_config)
    // --------------------------------------------------------------
    // arg1 (starts with --):
    //  --cfg - executes the configuration file if it has been changed and creates a new cache file
    //
    //  --build - builds the project (either by executing the configuration file or deserializing the cache file if
    //            it exists)
    //  --package - creates a bweas package based on the transferred files (json config, lua - generator script)
    //
    void handle_args(std::vector<std::string> &args);
    // Creates a bweas package based on the provided package configuration json file
    u32t create_package(std::string path_json_config_package);
    // loads the bweas json config
    void init();

    // running the interpreter with the configuration
    void run_interpreter();

    // generates a cache file of all targets that were created by the interpreter
    u32t gen_cache_target();

  public:
    // Sets the logging mode
    void set_logging();

    // Sets output to the console, 1 - yes, output all information, 0 - no
    void switch_output_log(u32t value);

  private:
    // Collects projects(out_targets) by initializing the generator and calling(bwIGenerator::gen_commands)
    void build_targets();

    // Deserializes the bweas cache file
    void deserl_cache();

  private:
    // Creates a stack of templates for the correct sequential generation of commands(for every targets)
    bwqueue_templates create_queue_target_templates(const var::struct_sb::target_out &target);

    // Recursive function, for create_stack_target_templates
    void recovery_queue_target_templates(std::vector<var::struct_sb::template_command> &vec_templates,
                                         const std::string &name_internal_param,
                                         bwqueue_templates &queue_target_templates);

  private:
    std::unique_ptr<cache_api::base_bwcache> cache;
    std::map<std::string, std::shared_ptr<generator_api::base_generator>> generators;

    std::vector<bwpackage> loaded_packages;

    module::module_mg module_manager;
    semantic_an::table_func module_tfuncs;

    bw_context context;

    std::string name_bweas_prg;
    std::string path_bweas_config, path_bweas_to_build{DIRWORK_ENV};

    mode_working mode_bweas{mode_working::undef};

    var::struct_sb::version bwbuilde_ver{BWEAS_VERSION_STR};
    bool log{1}, output_log{1};

    static inline bool init_glob{0};
};
} // namespace bweas

#endif
