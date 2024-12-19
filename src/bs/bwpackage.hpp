#ifndef BWPACKAGE__H
#define BWPACKAGE__H

#include "bw_defs.hpp"
#include "lang\expression.hpp"

// Package expansion
#define BW_FORMAT_PACKAGE ".bweas-package"

// The first bytes in the bweas package file are the signature
#define BW_PACKAGE_PREFIX_BYTE "sbw"

// size in bytes of bweas packet signature
#define BW_PACKAGE_PREFIX_BYTE_LENGHT 3

// The bweas version, which is indicated after the signature, for subsequent verification of the package for version
// compliance
#define BW_PACKAGE_VERSION BWEAS_VERSION_MAJOR "." BWEAS_VERSION_MINOR

// Bweas version length for package
#define BW_PACKAGE_VERSION_BWEAS_VERSION_LENGHT 3

// length in bytes of the entire signature (including version) of the bweas package
#define BW_PACKAGE_START_BYTES_LENGHT BW_PACKAGE_PREFIX_BYTE_LENGHT + BW_PACKAGE_VERSION_BWEAS_VERSION_LENGHT

// Bytes separating the json configuration data and the lua script (indicates the end of the json configuration of the
// bweas package)
#define BW_PACKAGE_SEPARATE_JSON_BYTES "ejc"

// Source code separator bytes of generators
#define BW_PACKAGE_SEPARATE_LUA_GENERATE "elg"

// Length of separating bytes json configuration of the bweas package
#define BW_PACKAGE_SEPARATE_JSON_BYTES_LENGHT 3

// Length of separating bytes between generator source codes
#define BW_PACKAGE_SEPARATE_LUA_GENERATE_LENGHT 3

// Maximum bweas packet size(5 mb)
#define MAX_SIZE_BW_PACKAGE 5 MB

namespace bweas {

// Class defining bweas packages
class bwpackage {
  public:
    bwpackage();

  public:
    // Data for creating a bweas package
    struct data_bw_package {
        std::string json_config;
        std::vector<std::string> src_lua_generators;
    };

    // All its configuration is stored here in the usual format.
    struct config {
        struct generator_lua {
            generator_lua(std::string _name_generator, std::vector<std::string> _features_generator,
                          std::string _src_lua_generator)
                : name_generator(_name_generator), features_generator(_features_generator),
                  src_lua_generator(_src_lua_generator) {
            }
            std::string name_generator;
            std::vector<std::string> features_generator;
            std::string src_lua_generator;
        };
        struct module {
            struct def_func {
                std::vector<aef_expr::params> params;
                std::string returnes;

                // call in semantic analysis
                bool cst{0};
            };
            module(std::string _name_module, std::string _name_dll,
                   std::vector<def_func> _funcs) :name_module(_name_module),
                name_dll(_name_dll), funcs(_funcs) {
            }
            std::string name_module;
            std::string name_dll;
            std::vector<def_func> funcs;
        };

        std::vector<generator_lua> generators;
        std::vector<module> modules;
    };

    // Returns a packet compressed by the lz4 algorithm, with the signature of a bweas packet
    static std::string create_data_package(data_bw_package _data);

    // Initializes the current package based on data
    std::string init(data_bw_package _data, bool is_create_pckg = 0);

    // Decompresses the read bweas packet, and also determines the correctness of its structure
    void load(std::string raw_data_package);

  public:
    // Returns 1 - if the package is initialized (*package name is not non-zero), otherwise 0
    bool is_init();

  public:
    std::string name_package;
    var::struct_sb::version bw_version;

    config cfg_package;

  private:
    std::string path_to_package;

    static inline bool init_glob{0};
};
} // namespace bweas

#endif