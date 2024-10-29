#include "bwpackage.hpp"
#include <nlohmann/json.hpp>

using namespace bweas;
using namespace bweas::exception;

bwpackage::bwpackage() {
    if (!init_glob) {
        assist.add_err("BWS-PCKG000", "Incorrect bweas package structure");
        assist.add_err("BWS-PCKG001", "The package of this version is not supported by the build system");
        assist.add_err("BWS-PCKG002", "Incorrect json file structure");

        init_glob = 1;
    }
}

std::string bwpackage::create_data_package(data_bw_package _data) {
    return bwlz4::compress_data(BW_PACKAGE_PREFIX_BYTE BW_PACKAGE_VERSION + _data.json_config +
                                BW_PACKAGE_SEPARATE_JSON_BYTES + _data.src_lua_generator);
}

std::string bwpackage::create(data_bw_package _data) {
    nlohmann::json config_json = nlohmann::json::parse(_data.json_config);
    data.cfg_package.name_package = config_json["name"];
    data.cfg_package.bw_version = var::struct_sb::version(config_json["bweas version"]);
    data.cfg_package.name_generator = config_json["name generator"];
    if (!config_json["features generator"].is_null() && config_json["features generator"] != "")
        for (u32t i = 0; i < config_json["features generator"].size(); ++i)
            data.cfg_package.features_generator.push_back(config_json["features generator"][i]);
    if (data.cfg_package.name_package == "")
        throw bwpackage_excp("Build system name field not found", "002");
    else if (data.cfg_package.bw_version == "0.0.0")
        throw bwpackage_excp("Build system version field not found", "002");
    else if (data.cfg_package.name_generator == "")
        throw bwpackage_excp("Name generator field not found", "002");
    data.src_lua_generator = _data.src_lua_generator;
    return create_data_package(_data);
}

void bwpackage::load(std::string raw_data_package) {
    std::string data_pckg = bwlz4::decompress_data(raw_data_package, MAX_SIZE_BW_PACKAGE);
    if (data_pckg.size() == 0)
        throw bwpackage_excp("Unsuccessful decompression of package bweas", "000");

    if (data_pckg.find(BW_PACKAGE_PREFIX_BYTE) == data_pckg.npos &&
        data_pckg.find(BW_PACKAGE_SEPARATE_JSON_BYTES) == data_pckg.npos)
        throw bwpackage_excp("[Package: " + path_to_package + "]", "000");

    std::string prefix_package = data_pckg;
    prefix_package.erase(0, BW_PACKAGE_PREFIX_BYTE_LENGHT);
    prefix_package.erase(BW_PACKAGE_VERSION_BWEAS_VERSION_LENGHT);

    if (var::struct_sb::version(prefix_package) < var::struct_sb::version(BW_PACKAGE_PREFIX_BYTE))
        throw bwpackage_excp("[Package: " + path_to_package + "] Ver pckg: " + prefix_package, "001");
    data_pckg.erase(0, BW_PACKAGE_START_BYTES_LENGHT);

    data_bw_package data_package;
    data_package.json_config = data_pckg;
    data_package.src_lua_generator = data_pckg;
    data_package.json_config.erase(data_package.json_config.find(BW_PACKAGE_SEPARATE_JSON_BYTES));
    data_package.src_lua_generator.erase(0, data_package.src_lua_generator.find(BW_PACKAGE_SEPARATE_JSON_BYTES) +
                                                BW_PACKAGE_SEPARATE_JSON_BYTES_LENGHT);
    create(data_package);
}