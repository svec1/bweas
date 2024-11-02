#ifndef _BUILD_SYS__H
#define _BUILD_SYS__H

#include "../mdef.hpp"

#include "bw_defs.hpp"
#include "bwgenerator.hpp"
#include "bwpackage.hpp"
#include "lang/interpreter.hpp"
#include "tools/bwfile.hpp"
#include "tools/call_cmd.hpp"

#include <stack>

// bweas configuration file, defines all packages that should be loaded by the builder
#define JSON_CONFIG_FILE "bweas-config.json"

// The current file of a project. It defines all target information
#define MAIN_FILE "bweasconf.txt"

// Markup file and definitions of modules that will be loaded
#define IMPORT_FILE "import-modules.imp"

// Cache file, all information about all targets is saved there for quick access, which makes it possible not to
// reinterpret bweasconf.txt
#define CACHE_FILE "bwcache"

namespace bweas {

// Builder class. It is a holistic program. It has operating modes (mode_working), which itself determines by passing
// parameters to it when launching programs
class bwbuilder {
  public:
    bwbuilder() = delete;
    bwbuilder(int argv, char **args);

    bwbuilder(bwbuilder &&) = delete;
    bwbuilder(const bwbuilder &) = delete;
    bwbuilder &operator=(bwbuilder &&) = delete;

    ~bwbuilder();

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
    void handle_args(std::vector<std::string> args);
    // Creates a bweas package based on the provided package configuration json file
    u32t create_package(std::string path_json_config_package, std::string path_lua_source_generator);
    // loads the bweas json config
    void init(std::string current_name_bweas_prg);
    // running the interpreter with the configuration
    void run_interpreter();
    // generates a cache file of all targets that were created by the interpreter
    u32t gen_cache_target();

    // Generates a dll that will call, based on the current bweasconf.txt, functions for configuration and creation of
    // targets. The build will be much faster if the project is large. (No implementation yet)
    void gen_DPCM();

  public:
    // Sets logging to a file, 1 - yes, log, 0 - no
    void switch_log(u32t value);

    // Sets output to the console, 1 - yes, output all information, 0 - no
    void switch_output_log(u32t value);

  private:
    // Collects projects(out_targets) by initializing the generator and calling(bwIGenerator::gen_commands)
    void build_targets();

    // Deserializes the bweas cache file
    u32t deserl_cache();

    // Loads all targets from the interpreter's global scope, converting them to target_out
    void load_target();

    // Imports all call templates and components declared in bweasconf.txt, which the interpreter also created
    void imp_data_interpreter_for_bs();

  private:
    // Creates a stack of templates for the correct sequential generation of commands(for every targets)
    std::stack<std::string> create_stack_target_templates(const var::struct_sb::target_out &target);

    // recursive function, for create_stack_target_templates
    u32t recovery_stack_templates(std::vector<var::struct_sb::template_command> &vec_templates,
                                  const std::string &name_internal_param, std::stack<std::string> &stack_templates);

    // converts the stack into an ordered pattern vector
    void set_queue_templates(std::stack<std::string> &&stack_target_templates,
                             bwqueue_templates &target_queue_templates);

  private:
    interpreter::interpreter_exec _interpreter;
    bwpackage loaded_package;

    bwGenerator *generator{nullptr};

    std::vector<var::struct_sb::target_out> out_targets;
    std::vector<var::struct_sb::template_command> templates;
    std::vector<var::struct_sb::call_component> call_components;

    bwargs global_extern_args;

    static inline bool init_glob{0};

    std::string name_bweas_prg;
    std::string path_bweas_config, path_bweas_to_build;
    mode_working mode_bweas{mode_working::undef};

    var::struct_sb::version bwbuilde_ver{BWEAS_VERSION_STR};
    bool log{1}, output_log{1};
};
} // namespace bweas

#endif