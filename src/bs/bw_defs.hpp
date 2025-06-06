//
// BWEAS is distributed under the GNU General Public License 2.0 (GPL-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWDEFS_HPP
#define BWDEFS_HPP

// bweas main header file

#include <algorithm>
#include <memory>

#include <bwaliases.hpp>
#include <bwmacros_platform.h>

#include <bwlogger.hpp>
#include <lang/interpreter.hpp>

namespace bweas {

static constexpr char VERSION_MAJOR_C = '0';
static constexpr char VERSION_MINOR_C = '1';
static constexpr char VERSION_PATCH_C = '1';

static constexpr char VERSION_FULL_STR[5] = {VERSION_MAJOR_C, '.', VERSION_MINOR_C, '.', VERSION_PATCH_C};

static const string INFO_STR =
    "bweas version " + std::string(VERSION_FULL_STR) + "\nrep on github - https://github.com/svec1/bweas";

static constexpr string_v HELP_STR =
    "bweas-call: \n   bweas <parameter>... path_depending\n   bweas path_bweas_config <parameter>..."
    "\nAcceptable parameters:"
    "\n   --build - builds the project (either by executing the configuration file or deserializing the cache file "
    "if it exists)"
    "\n   --cfg - executes the configuration file if it has been changed and creates a new cache file"
    "\n   --package - creates a bweas package based on the transferred json file(json config)"
    "\n   --help - outputs the syntax of the bweas call as well as its possible functions"
    "\n   --version - outputs the version of bweas";

static constexpr auto JSON_CONFIG_FILE     = "bweas-config.json";
static constexpr auto CONFIG_FILE          = "bweasconf.txt";
static constexpr auto CACHE_FILE           = "bwcache";
static constexpr auto FORMAT_PACKAGE       = ".bweas-package";
static constexpr auto DIRWORK_ENV          = ".bweas";
static constexpr auto DEPENDS_FILE_POSTFIX = ".d";

// The structure defining the main data for the build
struct bw_context {
    vec<var::struct_sb::target_out> out_targets;
    vec<var::struct_sb::template_command> templates;
    vec<var::struct_sb::call_component> call_components;
    vec<pair<string, string>> global_external_args;

    var::struct_sb::target_out *current_target;
};

} // namespace bweas

#endif
