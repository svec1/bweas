//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include "bwpackage.hpp"

#include "tools/bwlz4.hpp"
#include <nlohmann/json.hpp>

using namespace bweas;

// The first bytes in the bweas package file are the signature
static const string PACKAGE_PREFIX_BYTE = "sbw";
// Size in bytes of bweas packet signature
static constexpr size_t PACKAGE_PREFIX_BYTE_LENGHT = 3;

/* The bweas version, which is indicated after the signature, for subsequent verification of the package for version
 * compliance*/
static const string PACKAGE_VERSION = {VERSION_MAJOR_C, '.', VERSION_MINOR_C};
// Bweas version length for package
static constexpr size_t PACKAGE_VERSION_BWEAS_VERSION_LENGHT = 3;
/* Length in bytes of the entire signature (including version) of the bweas package */
static constexpr size_t PACKAGE_START_BYTES_LENGHT = PACKAGE_PREFIX_BYTE_LENGHT + PACKAGE_VERSION_BWEAS_VERSION_LENGHT;

// Bytes separating the json configuration data and the lua script (indicates the end of the json configuration of the
// bweas package)
static constexpr auto PACKAGE_SEPARATE_JSON_BYTES = "ejc";
// Source code separator bytes of cache
static constexpr auto PACKAGE_SEPARATE_LUA_CACHE = "elc";
// Source code separator bytes of generators
static constexpr auto PACKAGE_SEPARATE_LUA_GENERATE = "elg";
// Source code separator bytes of modules
static constexpr auto PACKAGE_SEPARATE_LUA_MODULE = "elm";

// Length of separating bytes json configuration of the bweas package
static constexpr size_t PACKAGE_SEPARATE_JSON_BYTES_LENGHT = 3;
// Length of separating bytes between cache source codes
static constexpr size_t PACKAGE_SEPARATE_LUA_CACHE_LENGHT = 3;
// Length of separating bytes between generator source codes
static constexpr size_t PACKAGE_SEPARATE_LUA_GENERATE_LENGHT = 3;
// Length of separating bytes between module source codes
static constexpr size_t PACKAGE_SEPARATE_LUA_MODULE_LENGHT = 3;

// Maximum bweas packet size(5 mb)
static constexpr size_t MAX_SIZE_PACKAGE = 5 MB;

static logger _log{"BWPACKAGE"};

string bwpackage::create_data_package(data_bw_package _data) {
    string data_str = PACKAGE_PREFIX_BYTE + PACKAGE_VERSION + _data.json_config + PACKAGE_SEPARATE_JSON_BYTES +
                      _data.src_lua_cache + PACKAGE_SEPARATE_LUA_CACHE;

    for (const auto &src_lua_generator : _data.src_lua_generators)
        data_str += src_lua_generator + PACKAGE_SEPARATE_LUA_GENERATE;

    return bwlz4::compress_data(data_str);
}

string bwpackage::init(data_bw_package _data, bool is_create_pckg) {
    nlohmann::json config_json = nlohmann::json::parse(_data.json_config);
    if (!config_json.contains("package-name") || ((name_package = config_json["package-name"]) == ""))
        (_log << bwtools::fatal) << (log_message(log_type::fatal) << "Bweas package name field is empty");
    else if (!config_json.contains("bweas-version") ||
             ((bw_version = var::struct_sb::version(config_json["bweas-version"])) == "0.0.0"))
        (_log << bwtools::fatal) << (log_message(log_type::fatal) << "Build system version field is empty");
    else if (config_json.contains("custom_fields_project") && config_json["custom_fields_project"].is_object())
        cfg_package.custom_ext_fields_project = config_json["custom_fields_project"];

    if (config_json.contains("cache-gn")) {
        nlohmann::json metainf_ch = config_json["cache-gn"];
        if (!metainf_ch.is_structured())
            (_log << bwtools::fatal) << (log_message(log_type::fatal) << "The Cache field must be a structure");

        else if (!metainf_ch.contains("name") || !metainf_ch["name"].is_string())
            (_log << bwtools::fatal) << (log_message(log_type::fatal) << "Cache metadata must include its name");
        else if (!metainf_ch.contains("src-luafile-cache") || !metainf_ch["src-luafile-cache"].is_string())
            (_log << bwtools::fatal) << (log_message(log_type::fatal)
                                         << "Cache metadata must include the path to the lua source file");

        cfg_package.cache.name_cache = metainf_ch["name"];

        if (is_create_pckg) {
            cfg_package.cache.src_lua_cache =
                bwtools::read_file(bwtools::get_ref_file(bwtools::open_file((string)metainf_ch["src-luafile-cache"])));
            _data.src_lua_cache = cfg_package.cache.src_lua_cache;
        }
        else
            cfg_package.cache.src_lua_cache = _data.src_lua_cache;
    }

    // reading lua script file
    if (config_json.contains("generators")) {
        if (!config_json["generators"].is_structured())
            (_log << bwtools::fatal) << (log_message(log_type::fatal)
                                         << "The \"generators\" field must be of type json structure");

        size_t i = 0;

        for (const auto &generator : config_json["generators"].items()) {
            nlohmann::json metainf_gn = generator.value();

            if (!metainf_gn.is_object())
                (_log << bwtools::fatal) << (log_message(log_type::fatal)
                                             << "The generator meta information unit must be a json object");
            else if (!metainf_gn.contains("src-luafile-gen") || !metainf_gn["src-luafile-gen"].is_string())
                (_log << bwtools::fatal)
                    << (log_message(log_type::fatal)
                        << "Generator metadata must include the path to the lua (generator) source code file");
            else if (!metainf_gn.contains("features-generator") || !metainf_gn["features-generator"].is_array())
                (_log << bwtools::fatal) << (log_message(log_type::fatal)
                                             << "Generator metadata should include a list of new generator features");
            else if (!metainf_gn.contains("use_custom_search_dependencies") ||
                     !metainf_gn["use_custom_search_dependencies"].is_boolean())
                (_log << bwtools::fatal) << (log_message(log_type::fatal)
                                             << "Field - uses custom dependency lookup, expected boolean type");
            auto it_gnlua_script = bwtools::open_file((string)metainf_gn["src-luafile-gen"]);
            if (is_create_pckg) {
                cfg_package.generators.emplace_back(generator.key(), metainf_gn["features-generator"],
                                                    metainf_gn["use_custom_search_dependencies"],
                                                    bwtools::read_file(bwtools::get_ref_file(it_gnlua_script)));
                _data.src_lua_generators.push_back(
                    cfg_package.generators[cfg_package.generators.size() - 1].src_lua_generator);
            }
            else {
                cfg_package.generators.emplace_back(generator.key(), metainf_gn["features-generator"],
                                                    metainf_gn["use_custom_search_dependencies"],
                                                    _data.src_lua_generators[i]);
                ++i;
            }
        }
    }
    if (config_json.contains("modules")) {
        if (!config_json["modules"].is_structured())
            (_log << bwtools::fatal) << (log_message(log_type::fatal)
                                         << "The \"generators\" field must be of type json structure");
        for (const auto &module : config_json["modules"].items()) {
            nlohmann::json metainf_md = module.value();
            vec<decl_func> funcs;
            if (!metainf_md.contains("src-luafile-md") || !metainf_md["src-luafile-md"].is_string())
                (_log << bwtools::fatal) << (log_message(log_type::fatal)
                                             << "Module metadata must include the name of the lua source file");
            else if (!metainf_md.contains("functions") || !metainf_md["functions"].is_object())
                (_log << bwtools::fatal) << (log_message(log_type::fatal)
                                             << "Module metadata must include the functions they provide for import");

            for (const auto &func : metainf_md["functions"].items()) {
                auto it_func = func.value();
                decl_func def_func_tmp;
                def_func_tmp.name_func = func.key();
                for (const auto &field : it_func.items()) {
                    if (field.key() == "accepted") {
                        if (!field.value().is_array())
                            (_log << bwtools::fatal)
                                << (log_message(log_type::fatal)
                                    << "The field for listing the types of function parameters must be an array");
                        for (size_t i = 0; i < field.value().size(); ++i) {
                            if (field.value()[i].is_string())
                                def_func_tmp.expected_params.push_back(
                                    param{get_string_param_type((string)field.value()[i])});
                            else
                                def_func_tmp.expected_params.push_back(param{field.value()[i]});
                        }
                    }
                }
                funcs.push_back(def_func_tmp);
            }
            cfg_package.modules.emplace_back(module.key(), metainf_md["src-luafile-md"], funcs);
        }
    }

    if (_log.error_status())
        return "";
    return create_data_package(_data);
}

void bwpackage::load(string_v raw_data_package) {
    string data_pckg = bwlz4::decompress_data(raw_data_package, MAX_SIZE_PACKAGE);
    if (data_pckg.size() == 0)
        (_log << bwtools::fatal) << (log_message(log_type::fatal) << "Unsuccessful decompression of package bweas");

    if (data_pckg.find(PACKAGE_PREFIX_BYTE) == data_pckg.npos &&
        data_pckg.find(PACKAGE_SEPARATE_JSON_BYTES) == data_pckg.npos)
        (_log << bwtools::fatal) << (log_message(log_type::fatal) << "Incorrect bweas package structure"
                                                                  << "[Package: " << data_pckg.size() << " bytes]");

    string prefix_package = data_pckg;
    prefix_package.erase(0, PACKAGE_PREFIX_BYTE_LENGHT);
    prefix_package.erase(PACKAGE_VERSION_BWEAS_VERSION_LENGHT);

    if (var::struct_sb::version(prefix_package) < var::struct_sb::version(PACKAGE_PREFIX_BYTE))
        (_log << bwtools::fatal) << (log_message(log_type::fatal)
                                     << "The package of this version is not supported by the build system"
                                     << "[Package: " << data_pckg.size() << " bytes] Ver pckg: " << prefix_package);
    data_pckg.erase(0, PACKAGE_START_BYTES_LENGHT);

    data_bw_package data_package;
    data_package.json_config = data_pckg;
    data_package.json_config.erase(data_package.json_config.find(PACKAGE_SEPARATE_JSON_BYTES));

    data_package.src_lua_cache = data_pckg;
    data_package.src_lua_cache.erase(0, data_package.json_config.size() + PACKAGE_SEPARATE_JSON_BYTES_LENGHT);
    data_package.src_lua_cache.erase(data_package.src_lua_cache.find(PACKAGE_SEPARATE_LUA_CACHE));

    string src_codes_luagn = data_pckg;
    string current_src_code;
    src_codes_luagn.erase(0, src_codes_luagn.find(PACKAGE_SEPARATE_LUA_CACHE) + PACKAGE_SEPARATE_LUA_CACHE_LENGHT);
    while (src_codes_luagn.size()) {
        current_src_code = src_codes_luagn;
        current_src_code.erase(current_src_code.find(PACKAGE_SEPARATE_LUA_GENERATE));
        data_package.src_lua_generators.push_back(current_src_code);

        src_codes_luagn.erase(0, current_src_code.size() + PACKAGE_SEPARATE_LUA_GENERATE_LENGHT);
    }

    init(data_package);
}

bool bwpackage::is_init() {
    return name_package.size();
}
