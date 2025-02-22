//
// BWEAS is distributed under the GNU General Public License 2.0 (GPL-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWDEFS__H
#define BWDEFS__H

// bweas main header file

#include "lang/interpreter.hpp"
#include "tools/bwexception.hpp"

#include <memory>
#include <vector>

#define BWEAS_VERSION_STR "0.1.0"

#define BWEAS_VERSION_MAJOR "0"
#define BWEAS_VERSION_MINOR "1"
#define BWEAS_VERSION_PATCH "0"

#define BWEAS_INFO "bweas version " BWEAS_VERSION_STR "\nrep on github - https://github.com/svec1/bweas"

#define BWEAS_HELP                                                                                                     \
    "bweas-call: \n   bweas <parameter>... path_depending\n   bweas path_bweas_config <parameter>..."                  \
    "\nAcceptable parameters:"                                                                                         \
    "\n   --build - builds the project (either by executing the configuration file or deserializing the cache file "   \
    "if it exists)"                                                                                                    \
    "\n   --cfg - executes the configuration file if it has been changed and creates a new cache file"                 \
    "\n   --package - creates a bweas package based on the transferred json file(json config)"                         \
    "\n   --help - outputs the syntax of the bweas call as well as its possible functions"                             \
    "\n   --version - outputs the version of bweas"

// bweas configuration file, defines all packages that should be loaded by the builder
#define JSON_CONFIG_FILE "bweas-config.json"

// The current file of a project. It defines all target information
#define MAIN_FILE "bweasconf.txt"

// Cache file, all information about all targets is saved there for quick access, which makes it possible not to
// reinterpret bweasconf.txt
#define CACHE_FILE "bwcache"

// The name of the directory where the build files will be created
#define DIRWORK_ENV ".bweas"

// The file in which all actions of the build system will be logged
#define LOG_FILE "bweas-last.log"

// A postfix (extension) to the file name containing the names of the source file dependencies
#define DEPENDS_FILE_POSTFIX ".d"

namespace bweas {

using bwarg = std::pair<std::string, std::string>;
using bwargs = std::vector<bwarg>;
using bwarg_files = std::pair<std::string, std::vector<std::string>>;
using bwargs_files = std::vector<bwarg_files>;
using bwqueue_templates = std::vector<var::struct_sb::template_command>;

using command = std::string;
using commands = std::vector<command>;

// The structure defining the main data for the build
struct bw_context {
    std::vector<var::struct_sb::target_out> out_targets;
    std::vector<var::struct_sb::template_command> templates;
    std::vector<var::struct_sb::call_component> call_components;
    std::vector<std::pair<std::string, std::string>> global_external_args;

    var::struct_sb::target_out *current_target;
};

namespace bwexception {

// Exception class for builder only.
class bwbuilder_excp : public ::bwexception::bweas_exception {
  public:
    bwbuilder_excp(std::string _what_hp, std::string number_err, std::string prefix_err = "")
        : what_hp(_what_hp), bweas_exception("BWS" + prefix_err + number_err) {
    }
    ~bwbuilder_excp() noexcept override = default;

  public:
    const char *what() const noexcept override final {
        return what_hp.c_str();
    }

  protected:
    std::string what_hp;
};

// Exception class for bweas-module only.
class bwmodule_excp : public bwbuilder_excp {
  public:
    bwmodule_excp(std::string _what_hp, std::string number_err) : bwbuilder_excp(_what_hp, number_err, "-MDL") {
    }
    ~bwmodule_excp() noexcept override final = default;
};

// Exception class for bweas-package only.
class bwpackage_excp : public bwbuilder_excp {
  public:
    bwpackage_excp(std::string _what_hp, std::string number_err) : bwbuilder_excp(_what_hp, number_err, "-PCKG") {
    }
    ~bwpackage_excp() noexcept override final = default;
};

// Exception class for bweas-generator only.
class bwcache_excp : public bwbuilder_excp {
  public:
    bwcache_excp(std::string _what_hp, std::string number_err) : bwbuilder_excp(_what_hp, number_err, "-CACHE") {
    }
    ~bwcache_excp() noexcept override final = default;
};

// Exception class for bweas-generator only.
class bwgenerator_excp : public bwbuilder_excp {
  public:
    bwgenerator_excp(std::string _what_hp, std::string number_err) : bwbuilder_excp(_what_hp, number_err, "-GNRT") {
    }
    ~bwgenerator_excp() noexcept override final = default;
};
} // namespace bwexception

} // namespace bweas

#endif
