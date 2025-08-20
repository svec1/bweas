//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWMODULE_HPP
#define BWMODULE_HPP

#include <bw_defs.hpp>

namespace bweas {
class module_manager;
}

// Class defining modules
class bweas::module_manager {
  public:
    module_manager()  = default;
    ~module_manager() = default;

  public:
    struct module_cfg {
        module_cfg(string _name, umap<string, sc::profile> _profiles, string _name_lua_source_file,
                   umap<string, decl_func> _funcs)
            : name(_name), profiles(_profiles), name_lua_source_file(_name_lua_source_file), funcs(_funcs) {
        }
        string name;
        string name_lua_source_file;

        umap<string, decl_func> funcs;
        umap<string, sc::profile> profiles;
    };

  public:
    // Initializes lua modules functions for subsequent calls
    umap<string, scope::module_data> init_modules(vec<module_cfg> &modules_cfg);
};

#endif
