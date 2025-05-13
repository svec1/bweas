//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWMODULE__H
#define BWMODULE__H

#include <bw_defs.hpp>
#include <bwluatools.hpp>

namespace bweas {

// Class defining modules
class bwmodule_mg {
  public:
    bwmodule_mg()  = default;
    ~bwmodule_mg() = default;

  public:
    struct module {
        module(std::string _name_module, std::string _name_lua_source_file,
               std::vector<decl_func> _funcs) :name_module(_name_module),
            name_lua_source_file(_name_lua_source_file), funcs(_funcs) {
        }
        std::string name_module;
        std::string name_lua_source_file;
        std::vector<decl_func> funcs;
    };

    using modules = std::vector<module>;

  public:
    // Initializes lua modules functions for subsequent calls
    std::vector<decl_func> init_mfuncs(modules &mds);

  private:
    static std::unordered_map<std::string, bwlua::lua> lua_stream_s;
};
} // namespace bweas

#endif
