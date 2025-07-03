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
    struct module {
        module(string _name_module, string _name_lua_source_file, vec<decl_func> _funcs) :name_module(_name_module),
            name_lua_source_file(_name_lua_source_file), funcs(_funcs) {
        }
        string name_module;
        string name_lua_source_file;
        vec<decl_func> funcs;
    };

    using modules = vec<module>;

  public:
    // Initializes lua modules functions for subsequent calls
    vec<decl_func> init_mfuncs(modules &mds);
};

#endif
