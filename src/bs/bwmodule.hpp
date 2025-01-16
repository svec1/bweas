#ifndef BWMODULE__H
#define BWMODULE__H

#include "bw_defs.hpp"

namespace bweas {
namespace module {

// Class defining modules
class module_mg {
  public:
    module_mg();
    ~module_mg() = default;

  public:
    struct module {
        module(std::string _name_module, std::string _name_dll,
               semantic_an::table_func _funcs) :name_module(_name_module),
            name_dll(_name_dll), funcs(_funcs) {
        }
        std::string name_module;
        std::string name_dll;
        semantic_an::table_func funcs;
    };

    using modules = std::vector<module>;

  public:
    // Load the dynamic library corresponding to the passed module,
    // and initialize (receive) function pointers
    semantic_an::table_func init_tfunc(module &md);

    // Initializes multiple modules, returning a common function table
    semantic_an::table_func init_tsfunc(modules &mds);

  private:
    static inline bool init_glob{0};
};
} // namespace module
} // namespace bweas

#endif