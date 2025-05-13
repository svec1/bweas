//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWPACKAGE__H
#define BWPACKAGE__H

#include "bw_defs.hpp"
#include "bwmodule.hpp"

#define BW_PACKAGE_PREFIX_BYTE "sbw"    // The first bytes in the bweas package file are the signature
#define BW_PACKAGE_PREFIX_BYTE_LENGHT 3 // Size in bytes of bweas packet signature
#define BW_PACKAGE_VERSION                                                                                             \
    BWEAS_VERSION_MAJOR "." BWEAS_VERSION_MINOR   /* The bweas version, which is indicated after the signature, for    \
                                                   subsequent verification of the package for version compliance*/
#define BW_PACKAGE_VERSION_BWEAS_VERSION_LENGHT 3 // Bweas version length for package
#define BW_PACKAGE_START_BYTES_LENGHT                                                                                  \
    BW_PACKAGE_PREFIX_BYTE_LENGHT +                                                                                    \
        BW_PACKAGE_VERSION_BWEAS_VERSION_LENGHT // Length in bytes of the entire signature (including version) of the
                                                // bweas package

// Bytes separating the json configuration data and the lua script (indicates the end of the json configuration of the
// bweas package)
#define BW_PACKAGE_SEPARATE_JSON_BYTES "ejc"
#define BW_PACKAGE_SEPARATE_LUA_CACHE "elc"    // Source code separator bytes of cache
#define BW_PACKAGE_SEPARATE_LUA_GENERATE "elg" // Source code separator bytes of generators
#define BW_PACKAGE_SEPARATE_LUA_MODULE "elm"   // Source code separator bytes of modules

#define BW_PACKAGE_SEPARATE_JSON_BYTES_LENGHT 3   // Length of separating bytes json configuration of the bweas package
#define BW_PACKAGE_SEPARATE_LUA_CACHE_LENGHT 3    // Length of separating bytes between cache source codes
#define BW_PACKAGE_SEPARATE_LUA_GENERATE_LENGHT 3 // Length of separating bytes between generator source codes
#define BW_PACKAGE_SEPARATE_LUA_MODULE_LENGHT 3   // Length of separating bytes between module source codes

#define MAX_SIZE_BW_PACKAGE 5 MB // Maximum bweas packet size(5 mb)

namespace bweas {

// Class defining bweas packages
class bwpackage {
  public:
    bwpackage() = default;

  public:
    // Data for creating a bweas package
    struct data_bw_package {
        std::string json_config;
        std::string src_lua_cache;
        std::vector<std::string> src_lua_generators;
    };

    // All its configuration is stored here in the usual format.
    struct config {
        struct cache_lua {
            cache_lua() = default;
            cache_lua(std::string _name_cache, std::string src_lua_cache)
                : name_cache(_name_cache), src_lua_cache(src_lua_cache) {
            }
            std::string name_cache;
            std::string src_lua_cache;
        };
        struct generator_lua {
            generator_lua() = default;
            generator_lua(std::string _name_generator, std::vector<std::string> _features_generator,
                          bool _use_custom_build_graph_depends, std::string _src_lua_generator)
                : name_generator(_name_generator), features_generator(_features_generator),
                  src_lua_generator(_src_lua_generator),
                  use_custom_build_graph_depends(_use_custom_build_graph_depends) {
            }
            std::string name_generator;
            std::vector<std::string> features_generator;
            std::string src_lua_generator;

            // Use a custom file dependency(search) system
            bool use_custom_build_graph_depends{0};
        };

        cache_lua cache;
        std::vector<generator_lua> generators;
        bwmodule_mg::modules modules;
        std::map<std::string, std::string> custom_ext_fields_project;
    };

    // Returns a packet compressed by the lz4 algorithm, with the signature of a bweas packet
    static std::string create_data_package(data_bw_package _data);

    // Initializes the current package based on data
    std::string init(data_bw_package _data, bool is_create_pckg = 0);

    // Decompresses the read bweas packet, and also determines the correctness of its structure
    void load(std::string_view raw_data_package);

  public:
    // Returns 1 - if the package is initialized (*package name is not non-zero), otherwise 0
    bool is_init();

  public:
    std::string name_package;
    var::struct_sb::version bw_version;

    config cfg_package;
};
} // namespace bweas

#endif
