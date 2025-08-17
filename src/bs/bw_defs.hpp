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
#include <cstring>
#include <memory>

#include <bwaliases.hpp>
#include <bwmacros_platform.h>

#include <bwlogger.hpp>
#include <lang/interpreter.hpp>

#include <bwdepends_files.hpp>

namespace bweas {

using mf      = bwtools::file::mode_file;
using file_it = bwtools::file_it;

inline constexpr char VERSION_MAJOR_C = '0';
inline constexpr char VERSION_MINOR_C = '1';
inline constexpr char VERSION_PATCH_C = '1';

inline constexpr string VERSION_FULL_STR = {VERSION_MAJOR_C, '.', VERSION_MINOR_C, '.', VERSION_PATCH_C};

inline constexpr auto CONFIG_FILE    = "bweasconf.txt";
inline constexpr auto CACHE_FILE     = "bwcache";
inline constexpr auto DEPENDS_FILE   = "bwdependencies";
inline constexpr auto FORMAT_PACKAGE = ".bweas-package";

// The structure defining the main data for the build
struct context {
    vec<sc::profile> profiles;

    vec<sc::target> targets;
    vec<sc::template_command> templates;
    vec<sc::call_component> call_components;
    vec<pair<string, string>> global_external_args;

  public:
    depends_files::depends_map dfiles;

  public:
    sc::target *current_target;
    string current_work_directory;

    string path_bweas_config, path_bweas_to_build;
};

} // namespace bweas

#endif
