//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWMODULE_HPP
#define BWMODULE_HPP

#include <bw_defs.hpp>
#include <lang/parser_utils.hpp>

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
        module_cfg(string _name, string _name_src_file, const umap<string, bweas::sc::profile> &_profiles)
            : name(_name), name_src_file(_name_src_file), profiles(_profiles) {
        }
        string name;
        string name_src_file;
        umap<string, bweas::sc::profile> profiles;
    };

    struct _module {
        _module(string_v _name, bwlang::parser_utils::context &&_ctx = {}) : name(_name), ctx(std::move(_ctx)) {
        }

        string name;
        bwlang::parser_utils::context ctx;
    };

  public:
    // Initializes lua modules functions for subsequent calls
    vec<_module> init_modules(vec<module_cfg> &modules_cfg);
};

#endif
