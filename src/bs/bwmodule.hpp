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

/** \brief Modules manager. */
class bweas::module_manager {
  public:
    /** \brief Constructor. */
    module_manager() = default;

  public:
    /** \brief The structure is the defining module configuration. */
    struct module_cfg {
        module_cfg(string _name, string _name_src_file) : name(_name), name_src_file(_name_src_file) {
        }
        string name;          ///< The name of the module.
        string name_src_file; ///< The path to the module file.
    };

    /** \brief The structure is the defining module. */
    struct _module {
        _module(string_v _name, bwlang::parser_utils::context &&_ctx = {}) : name(_name), ctx(_ctx) {
        }

        string name;                       ///< The name of the module.
        bwlang::parser_utils::context ctx; ///< The context that defines the state of the module.
    };

  public:
    /** \brief Initializes modules based on its configuration.
     * \param [in] modules_cfg An array of module configurations.
     * \return vec<_module>
     */
    vec<_module> init_modules(vec<module_cfg> &modules_cfg);
};

#endif
