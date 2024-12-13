#include "bwpackage.hpp"

#include "tools/bwlz4.hpp"
#include <nlohmann/json.hpp>

using namespace bweas;
using namespace bwexception;

bwpackage::bwpackage() {
    if (!init_glob) {
        assist.add_err("BWS-PCKG000", "Incorrect bweas package structure");
        assist.add_err("BWS-PCKG001", "The package of this version is not supported by the build system");
        assist.add_err("BWS-PCKG002", "Incorrect json file structure");
        assist.add_err("BWS-PCKG003", "The bweas-lua generator for package file was not found");

        init_glob = 1;
    }
}

std::string bwpackage::create_data_package(data_bw_package _data) {
    std::string data_str = BW_PACKAGE_PREFIX_BYTE BW_PACKAGE_VERSION + _data.json_config +
                           BW_PACKAGE_SEPARATE_JSON_BYTES + _data.src_lua_generator;

    // if the files were created on windows
    data_str.erase(std::remove(data_str.begin(), data_str.end(), '\r'), data_str.end());
    return bwlz4::compress_data(data_str);
}

std::string bwpackage::init(data_bw_package _data, bool is_create_pckg) {
    nlohmann::json config_json = nlohmann::json::parse(_data.json_config);
    if (config_json.find("features generator") != config_json.end() && config_json["features generator"].is_array())
        for (u32t i = 0; i < config_json["features generator"].size(); ++i)
            data.cfg_package.features_generator.push_back(config_json["features generator"][i]);
    if (config_json.find("name") == config_json.end() || ((data.cfg_package.name_package = config_json["name"]) == ""))
        throw bwpackage_excp("Bweas package name field is empty", "002");
    else if (config_json.find("bweas version") == config_json.end() ||
             ((data.cfg_package.bw_version = var::struct_sb::version(config_json["bweas version"])) == "0.0.0"))
        throw bwpackage_excp("Build system version field is empty", "002");
    else if (config_json.find("name generator") == config_json.end() ||
             ((data.cfg_package.name_generator = config_json["name generator"]) == ""))
        throw bwpackage_excp("Name generator field is empty", "002");

    if (is_create_pckg) {
        // reading lua script file
        std::string tmp_name_flua_scr;
        if (config_json.find("file lscript") == config_json.end() ||
            ((tmp_name_flua_scr = config_json["file lscript"]) == ""))
            throw bwpackage_excp("Name file with lua script field is empty", "002");
        const assistant::file_it &lua_src_generator_package =
            assist.open_file(tmp_name_flua_scr, assistant::file::mode_file::open::r);
        if (!assist.exist_file(lua_src_generator_package))
            throw bwpackage_excp(tmp_name_flua_scr, "003");
        _data.src_lua_generator = assist.read_file(assist.get_ref_file(lua_src_generator_package),
                                                   assistant::file::mode_file::input::read_default);
        assist.close_file(lua_src_generator_package);
    }
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
    init(data_package);
}

bool bwpackage::is_init() {
    return data.cfg_package.name_package.size();
}