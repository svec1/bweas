//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWPACKAGE_HPP
#define BWPACKAGE_HPP

#include "bw_defs.hpp"
#include "bwmodule.hpp"

namespace bweas {
class package;
}
// Class defining bweas packages
class bweas::package {
  public:
    package() = default;

  public:
    // Data for creating a bweas package
    struct data_bw_package {
        string json_config;
        string src_lua_cache;
        vec<string> src_lua_generators;
    };

    // All its configuration is stored here in the usual format.
    struct config {
        struct cache_lua {
            cache_lua() = default;
            cache_lua(string _name_cache, string src_lua_cache)
                : name_cache(_name_cache), src_lua_cache(src_lua_cache) {
            }
            string name_cache;
            string src_lua_cache;
        };
        struct generator_lua {
            generator_lua() = default;
            generator_lua(string _name_generator, vec<string> _features_generator, bool _use_custom_build_graph_depends,
                          string _src_lua_generator)
                : name_generator(_name_generator), features_generator(_features_generator),
                  src_lua_generator(_src_lua_generator),
                  use_custom_build_graph_depends(_use_custom_build_graph_depends) {
            }
            string name_generator;
            vec<string> features_generator;
            string src_lua_generator;

            // Use a custom file dependency(search) system
            bool use_custom_build_graph_depends{0};
        };

        cache_lua cache;
        vec<generator_lua> generators;
        module_manager::modules modules;
        map<string, string> custom_ext_fields_project;
    };

    // Returns a packet compressed by the lz4 algorithm, with the signature of a bweas packet
    static string create_data_package(data_bw_package _data);

    // Initializes the current package based on data
    string init(data_bw_package _data, bool is_create_pckg = 0);

    // Decompresses the read bweas packet, and also determines the correctness of its structure
    void load(string_v raw_data_package);

  public:
    // Returns 1 - if the package is initialized (*package name is not non-zero), otherwise 0
    bool is_init();

  public:
    string name_package;
    var::struct_sb::version bw_version;

    config cfg_package;
};

#endif
