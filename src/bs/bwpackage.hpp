#ifndef BWPACKAGE__H
#define BWPACKAGE__H

#include "bw_defs.hpp"
#include "tools/bwlz4.hpp"

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

// Length in bytes of the ending json configuration of the bweas package
#define BW_PACKAGE_SEPARATE_JSON_BYTES_LENGHT 3

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
        std::string src_lua_generator;
    };

    // All its configuration is stored here in the usual format.
    struct config {
        std::string name_package;
        var::struct_sb::version bw_version;

        std::string name_generator;
        std::vector<std::pair<std::string, std::string>> features_generator;
    };

    // Package structure. All his current data is stored here
    struct package {
        config cfg_package;
        std::string src_lua_generator;
    };

    // Returns a packet compressed by the lz4 algorithm, with the signature of a bweas packet
    static std::string create_data_package(data_bw_package _data);

    // Initializes the current package based on data
    std::string init(data_bw_package _data);

    // Decompresses the read bweas packet, and also determines the correctness of its structure
    void load(std::string raw_data_package);

  public:
    // Returns 1 - if the package is initialized (*package name is not non-zero), otherwise 0
    bool is_init();

  public:
    package data;

  private:
    std::string path_to_package;

    static inline bool init_glob{0};
};
} // namespace bweas

#endif