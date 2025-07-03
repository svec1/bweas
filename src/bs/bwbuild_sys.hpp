//
// BWEAS is distributed under the GNU General Public License 2.0 (GPL-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWBUILD_SYS_HPP
#define BWBUILD_SYS_HPP

#include "bw_defs.hpp"

#include "bwcache_api.hpp"
#include "bwgenerator_api.hpp"
#include "bwmodule.hpp"
#include "bwpackage.hpp"

/*
 * \brief Main namespace bweas
 */
namespace bweas {
class builder;
} // namespace bweas

/**
 *  \brief The main class of the target program, which describes all its logic.
 *
 *  The logic of the class depends on the current mode, which is determined based on the values passed to its
 * constructor.
 */
class bweas::builder final {
  public:
    /**
     * \brief Constructor.
     *
     * The constructor defines the mode of operation of the bweas assembly system, based on the passed parameters. After
     * that, the init() function is called.
     *
     * \param [in] argc The number of parameters in the argv array.
     * \param [in] argv Pointer to an array of parameters.
     */
    builder(size_t argc, char **argv);

    builder()                           = delete;
    builder(const builder &)            = delete;
    builder &operator=(const builder &) = delete;

    ~builder() = default;

  public:
    /// \brief All possible bweas operating modes
    enum mode_working {
        collect_cfg = 0,     ///< [--cfg]: Interpreting a configuration file and creating a cache file based on it.
        build,               ///< [--build]: Building targets based on a cache file.
        collect_cfg_w_build, /*!< [--cfg --build]: Collaborative mode, in which, after the interpretation process,
                              * context bweas is initialized, without reading the cache file.
                              */
        build_package,       ///< [--package]: Package creation mode based on the transmitted json file.
        undef
    };

    /** \brief Returns the current build system mode. */
    mode_working get_current_mode();

  public:
    /** \brief Starts the build system.
     *
     * Depending on the presence of a cache file and its parameters passed when the program is launched, it reproduces
     * processes inherent to bweas.
     *
     *
     *
     */
    void start();

  protected:
    /**
     * \brief Controls how bweas works, depending on the arguments provided.
     *
     * The following parameters can be expected:
     * --cfg - sets the collect_cfg mode.
     * --build - sets the build mode.
     * --package - sets the build_package mode.
     *
     * \param [in] args Vector of parameters.
     *
     * \warning If a vector with size 0 has been passed (i.e. without the parameters of the bweas call), bweas will
     * finish its work with the output of BWEAS_HELP.
     */
    void handle_args(vec<string> &args);

    /** \brief Creates a package based on the json package configuration file.
     *
     *  \param [in] path_json_config_package The path to the package configuration json file.
     *  \return The packet size in bytes.
     */
    size_t create_package(string path_json_config_package);

    /// \brief Loads the bweas json config.
    void init();

    /// \brief Running the interpreter with the configuration.
    void run_interpreter();

    /** \brief Generates a cache file of all targets that were created by the interpreter.
     *
     *  \return 0 - if the cache file is generated, 1 - otherwise.
     */
    size_t gen_cache_target();

    depends_files::depends_map load_depends_file(std::unique_ptr<depends_files> &_depends_files,
                                                 const sc::target_out target);

  private:
    /** \brief Collects projects(out_targets) by initializing the generator and calling(bwIGenerator::gen_commands)
     *
     */
    void build_targets();

  protected:
    /**
     * \brief The state of the entire build system.
     */
    bw_context context;

  private:
    std::unique_ptr<cache_api::base_cache> cache;
    map<string, std::shared_ptr<generator_api::base_generator>> generators;

  private:
    vec<package> loaded_packages;

    module_manager module_m;
    vec<decl_func> external_modules_funcs;

  private:
    sc::version version{VERSION_FULL_STR};
    mode_working mode_bweas{mode_working::undef};
};

#endif
