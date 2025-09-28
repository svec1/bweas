//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWPACKAGE_HPP
#define BWPACKAGE_HPP

#include <bw_defs.hpp>
#include <bwmodule.hpp>

namespace bweas {
class package;
}

// Class defining bweas packages
class bweas::package {
  public:
    package(string_v json_config);

  public:
    // All its configuration is stored here in the usual format.
    struct config {
        struct cache_lua {
            cache_lua() = default;
            cache_lua(string _name, string src_lua) : name(_name), src_lua(src_lua) {
            }
            string name;
            string src_lua;
        };

        cache_lua cache;
        vec<module_manager::module_cfg> modules;
    };

  public:
    // Returns 1 - if the package is initialized (*package name is not non-zero), otherwise 0
    bool is_init();

  public:
    string name;
    config cfg;
};

#endif
