#include "bwpackage.hpp"

#include "tools/bwlz4.hpp"
#include <nlohmann/json.hpp>

using namespace bweas;
using namespace bweas::bwexception;

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
                           BW_PACKAGE_SEPARATE_JSON_BYTES + _data.src_lua_cache + BW_PACKAGE_SEPARATE_LUA_CACHE;

    for (const auto &src_lua_generator : _data.src_lua_generators)
        data_str += src_lua_generator + BW_PACKAGE_SEPARATE_LUA_GENERATE;

    return bwlz4::compress_data(data_str);
}

std::string bwpackage::init(data_bw_package _data, bool is_create_pckg) {
    nlohmann::json config_json = nlohmann::json::parse(_data.json_config);
    if (!config_json.contains("package-name") || ((name_package = config_json["package-name"]) == ""))
        throw bwpackage_excp("Bweas package name field is empty", "002");
    else if (!config_json.contains("bweas-version") ||
             ((bw_version = var::struct_sb::version(config_json["bweas-version"])) == "0.0.0"))
        throw bwpackage_excp("Build system version field is empty", "002");

    if (config_json.contains("cache-gn")) {
        nlohmann::json metainf_ch = config_json["cache-gn"];
        if (!metainf_ch.is_structured())
            throw bwpackage_excp("The Cache field must be a structure", "002");
        else if (!metainf_ch.contains("name") || !metainf_ch["name"].is_string())
            throw bwpackage_excp("Cache metadata must include its name", "002");
        else if (!metainf_ch.contains("src-luafile-cache") || !metainf_ch["src-luafile-cache"].is_string())
            throw bwpackage_excp("Cache metadata must include the path to the lua source file", "002");

        cfg_package.cache.name_cache = metainf_ch["name"];

        if (is_create_pckg) {
            cfg_package.cache.src_lua_cache =
                assist.read_file(assist.get_ref_file(assist.open_file(metainf_ch["src-luafile-cache"])));
            _data.src_lua_cache = cfg_package.cache.src_lua_cache;
        }
        else
            cfg_package.cache.src_lua_cache = _data.src_lua_cache;
    }
    // reading lua script file
    if (config_json.contains("generators")) {
        if (!config_json["generators"].is_structured())
            throw bwpackage_excp("The \"generators\" field must be of type json structure", "002");

        u32t i = 0;

        for (const auto &generator : config_json["generators"].items()) {
            nlohmann::json metainf_gn = generator.value();

            if (!metainf_gn.is_object())
                throw bwpackage_excp("The generator meta information unit must be a json object", "002");
            else if (!metainf_gn.contains("src-luafile-gen") || !metainf_gn["src-luafile-gen"].is_string())
                throw bwpackage_excp("Generator metadata must include the path to the lua (generator) source code file",
                                     "002");
            else if (!metainf_gn.contains("features-generator") || !metainf_gn["features-generator"].is_array())
                throw bwpackage_excp("Generator metadata should include a list of new generator features", "002");

            auto it_gnlua_script = assist.open_file(metainf_gn["src-luafile-gen"]);
            if (is_create_pckg) {
                cfg_package.generators.emplace_back(generator.key(), metainf_gn["features-generator"],
                                                    assist.read_file(assist.get_ref_file(it_gnlua_script)));
                _data.src_lua_generators.push_back(
                    cfg_package.generators[cfg_package.generators.size() - 1].src_lua_generator);
            }
            else {
                cfg_package.generators.emplace_back(generator.key(), metainf_gn["features-generator"],
                                                    _data.src_lua_generators[i]);
                ++i;
            }
        }
    }
    if (config_json.contains("modules")) {
        if (!config_json["modules"].is_structured())
            throw bwpackage_excp("The \"generators\" field must be of type json structure", "002");
        for (const auto &module : config_json["modules"].items()) {
            nlohmann::json metainf_md = module.value();
            semantic_an::table_func funcs_md;
            if (!metainf_md.contains("dll") || !metainf_md["dll"].is_string())
                throw bwpackage_excp("Module metadata must include the name of the dll file", "002");
            else if (!metainf_md.contains("functions") || !metainf_md["functions"].is_object())
                throw bwpackage_excp("Module metadata must include the functions they provide for import", "002");

            for (const auto &func : metainf_md["functions"].items()) {
                auto it_func = func.value();
                aef_expr::notion_func def_func_tmp;
                for (const auto &field : it_func.items()) {
                    if (field.key() == "accepted") {
                        if (!field.value().is_array())
                            throw bwpackage_excp(
                                "The field for listing the types of function parameters must be an array", "002");
                        for (u32t i = 0; i < field.value().size(); ++i) {
                            if (field.value()[i].is_string())
                                def_func_tmp.expected_params.push_back(
                                    aef_expr::param{parser::utility::type_param_in_str(field.value()[i])});
                            else
                                def_func_tmp.expected_params.push_back(aef_expr::param{field.value()[i]});
                        }
                    }
                    else if (field.key() == "csn") {
                        if (!field.value().is_boolean())
                            throw bwpackage_excp(
                                "Field for selecting the location of the function call, must be an boolean", "002");
                        def_func_tmp.only_with_semantic = field.value();
                    }
                }
                funcs_md[func.key()] = def_func_tmp;
            }
            cfg_package.mds.emplace_back(module.key(), metainf_md["dll"], funcs_md);
        }
    }
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
    data_package.json_config.erase(data_package.json_config.find(BW_PACKAGE_SEPARATE_JSON_BYTES));

    data_package.src_lua_cache = data_pckg;
    data_package.src_lua_cache.erase(0, data_package.json_config.size() + BW_PACKAGE_SEPARATE_JSON_BYTES_LENGHT);
    data_package.src_lua_cache.erase(data_package.src_lua_cache.find(BW_PACKAGE_SEPARATE_LUA_CACHE));

    std::string src_codes_luagn = data_pckg;
    std::string current_src_code;
    src_codes_luagn.erase(0,
                          src_codes_luagn.find(BW_PACKAGE_SEPARATE_LUA_CACHE) + BW_PACKAGE_SEPARATE_LUA_CACHE_LENGHT);
    while (src_codes_luagn.size()) {
        current_src_code = src_codes_luagn;
        current_src_code.erase(current_src_code.find(BW_PACKAGE_SEPARATE_LUA_GENERATE));
        data_package.src_lua_generators.push_back(current_src_code);

        src_codes_luagn.erase(0, current_src_code.size() + BW_PACKAGE_SEPARATE_LUA_GENERATE_LENGHT);
    }

    init(data_package);
}

bool bwpackage::is_init() {
    return name_package.size();
}