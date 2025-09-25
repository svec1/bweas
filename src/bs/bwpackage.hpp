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
    package() = default;

  public:
    // Data for creating a bweas package
    struct data_bw_package {
        string json_config;
        string src_lua_cache;
        vec<string> src_finders;
    };

    // All its configuration is stored here in the usual format.
    struct config {
        struct cache_lua {
            cache_lua() = default;
            cache_lua(string _name, string src_lua) : name(_name), src_lua(src_lua) {
            }
            string name;
            string src_lua;
        };
        struct dependency_finder {
            dependency_finder() = default;
            dependency_finder(string _language, string _src) : language(_language), src(_src) {
            }

            string language;
            string src;
        };

        cache_lua cache;
        vec<dependency_finder> finders;
        vec<module_manager::module_cfg> modules;
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
    string name;
    sc::version bw_version;

    config cfg;
};

#endif
