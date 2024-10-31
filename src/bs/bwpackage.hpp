#ifndef BWPACKAGE__H
#define BWPACKAGE__H

#include "bw_defs.hpp"
#include "tools/bwlz4.hpp"

#define BW_FORMAT_PACKAGE ".bweas-package"
#define BW_PACKAGE_PREFIX_BYTE "sbw"
#define BW_PACKAGE_PREFIX_BYTE_LENGHT 3
#define BW_PACKAGE_VERSION BWEAS_VERSION_MAJOR "." BWEAS_VERSION_MINOR
#define BW_PACKAGE_VERSION_BWEAS_VERSION_LENGHT 3
#define BW_PACKAGE_START_BYTES_LENGHT BW_PACKAGE_PREFIX_BYTE_LENGHT + BW_PACKAGE_VERSION_BWEAS_VERSION_LENGHT
#define BW_PACKAGE_SEPARATE_JSON_BYTES "ejc"
#define BW_PACKAGE_SEPARATE_JSON_BYTES_LENGHT 3
#define MAX_SIZE_BW_PACKAGE 5 MB

namespace bweas {
class bwpackage {
  public:
    bwpackage();

  public:
    struct data_bw_package {
        std::string json_config;
        std::string src_lua_generator;
    };
    struct config {
        std::string name_package;
        var::struct_sb::version bw_version;

        std::string name_generator;
        std::vector<std::pair<std::string, std::string>> features_generator;
    };
    struct package {
        config cfg_package;
        std::string src_lua_generator;
    };

    static std::string create_data_package(data_bw_package _data);
    std::string init(data_bw_package _data);
    void load(std::string raw_data_package);

  public:
    package data;

  private:
    std::string path_to_package;

    static inline bool init_glob{0};
};
} // namespace bweas

#endif