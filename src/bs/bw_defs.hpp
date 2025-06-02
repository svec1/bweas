//
// BWEAS is distributed under the GNU General Public License 2.0 (GPL-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWDEFS__H
#define BWDEFS__H

// bweas main header file

#include <bwmacros_platform.h>
#include <bwtype.h>

#include <bwlogger.hpp>

#include <lang/interpreter.hpp>

#include <algorithm>
#include <memory>
#include <vector>

#define BWEAS_VERSION_STR "0.1.1"

#define BWEAS_VERSION_MAJOR "0"
#define BWEAS_VERSION_MINOR "1"
#define BWEAS_VERSION_PATCH "1"

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

#define JSON_CONFIG_FILE                                                                                               \
    "bweas-config.json"           // Bweas configuration file, defines all packages that should be loaded by the builder
#define MAIN_FILE "bweasconf.txt" // The current file of a project. It defines all target information
#define CACHE_FILE                                                                                                     \
    "bwcache" /* Cache file, all information about all targets is saved there for quick access, which makes it         \
                 possible not to reinterpret bweasconf.txt*/
#define BW_FORMAT_PACKAGE ".bweas-package" // Package expansion
#define DIRWORK_ENV ".bweas"               // The name of the directory where the build files will be created
#define DEPENDS_FILE_POSTFIX                                                                                           \
    ".d" // A postfix (extension) to the file name containing the names of the source file dependencies

namespace bweas {

using bwarg             = std::pair<std::string, std::string>;
using bwargs            = std::vector<bwarg>;
using bwarg_files       = std::pair<std::string, std::vector<std::string>>;
using bwargs_files      = std::vector<bwarg_files>;
using bwqueue_templates = std::vector<var::struct_sb::template_command>;

using command  = std::string;
using commands = std::vector<command>;

// The structure defining the main data for the build
struct bw_context {
    std::vector<var::struct_sb::target_out> out_targets;
    std::vector<var::struct_sb::template_command> templates;
    std::vector<var::struct_sb::call_component> call_components;
    std::vector<std::pair<std::string, std::string>> global_external_args;

    var::struct_sb::target_out *current_target;
};

} // namespace bweas

#endif
