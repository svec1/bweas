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

/** \brief Bweas package. */
class bweas::package {
  public:
    /** Constructor.
     * \param [in] json_config Data about the bweas package, which is presented in json format.
     */
    package(string_v json_config);

  public:
    /** \brief The structure defining the configuration of the bweas package. */
    struct config {
        /** \brief The structure defining the configuration of the lua cache generator. */
        struct cache_lua {
            /** \brief Constructor.*/
            cache_lua() = default;
            /** \brief Constructor.
             * \param [in] _name
             * \param [in] _src_lua
             */
            cache_lua(string _name, string _src_lua) : name(_name), src_lua(_src_lua) {
            }

          public:
            string name;    ///< The name of the generator.
            string src_lua; ///< The source code of the lua generator cache.
        };

      public:
        cache_lua cache;                         ///< Configuration of the lua cache generator.
        vec<module_manager::module_cfg> modules; ///< An array of module configurations.
    };

  public:
    string name; ///< The name of the package.
    config cfg;  ///< Package configuration.
};

#endif
