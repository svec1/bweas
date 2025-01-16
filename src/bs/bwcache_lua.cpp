#include "bwcache_api.hpp"
#include "bwluatoolslang.hpp"

using namespace bweas;
using namespace cache_api;
using namespace bweas::bwexception;

lua_bwcache::lua_bwcache(std::string src_lua) {
    if (!init_glob_chlua) {
        assist.add_err("BWS-CACHE000", "Failed to load lua script");
        assist.add_err("BWS-CACHE001", "No entry function for creates cache");
        assist.add_err("BWS-CACHE002", "No entry function for get data of cache");

        init_glob_chlua = 1;
    }
    lua.create(src_lua);

    if (!lua.is_function(NAME_FUNCTION_GENCACHE))
        throw bwcache_excp("", "001");
    else if (!lua.is_function(NAME_FUNCTION_GETDATA_CACHE))
        throw bwcache_excp("", "002");
}

void lua_bwcache::delete_cache() {
    delete this;
}

std::string lua_bwcache::create_cache() {
    std::vector<bwlua::lua::table<std::string, std::any>> ltargets_o;
    std::vector<bwlua::lua::table<std::string, std::any>> ltcmd_s;
    std::vector<bwlua::lua::table<std::string, std::any>> lccmp_s;

    for (const auto &ltarget_o : _cache_data.targets_o_p != nullptr ? *_cache_data.targets_o_p : _cache_data.targets_o)
        ltargets_o.push_back(luatools_lang::conv_to_table(ltarget_o));
    for (const auto &ltcmd : _cache_data.templates)
        ltcmd_s.push_back(luatools_lang::conv_to_table(ltcmd));
    for (const auto &lccmp : _cache_data.call_components)
        lccmp_s.push_back(luatools_lang::conv_to_table(lccmp));

    try {
        return lua.call_function DEFINITION_FUNCTION_GENCACHE(NAME_FUNCTION_GENCACHE, ltargets_o, ltcmd_s, lccmp_s,
                                                              _cache_data.global_external_args);
    }
    catch (std::exception &what) {
        throw bwcache_excp(what.what(), "000");
    }
}

const base_bwcache::cache_data &lua_bwcache::get_cache_data(std::string cache_str) {

    std::vector<bwlua::lua::table<std::string, std::any>> ltargets_o;
    std::vector<bwlua::lua::table<std::string, std::any>> ltcmd_s;
    std::vector<bwlua::lua::table<std::string, std::any>> lccmp_s;
    std::vector<std::pair<std::string, std::string>> lglobal_external_args;

    try {
        lua.call_function DEFINITION_FUNCTION_GETDATA_CACHE(NAME_FUNCTION_GETDATA_CACHE, {});

        ltargets_o =
            lua[NAME_VARIABLE_TARGETS_F_EXTERN].getval<std::vector<bwlua::lua::table<std::string, std::any>>>();
        ltcmd_s = lua[NAME_VARIABLE_TEMPLATES_F_EXTERN].getval<std::vector<bwlua::lua::table<std::string, std::any>>>();
        lccmp_s =
            lua[NAME_VARIABLE_CCOMPONENTS_F_EXTERN].getval<std::vector<bwlua::lua::table<std::string, std::any>>>();
        lglobal_external_args =
            lua[NAME_VARIABLE_GEARGS_F_EXTERN].getval<std::vector<std::pair<std::string, std::string>>>();
    }
    catch (std::exception &what) {
        throw bwcache_excp(what.what(), "000");
    }
    for (auto &ltarget_o : ltargets_o)
        _cache_data.targets_o.push_back(luatools_lang::conv_to_target(ltarget_o));
    for (auto &ltcmd : ltcmd_s)
        _cache_data.templates.push_back(luatools_lang::conv_to_template(ltcmd));
    for (auto &lccmp : lccmp_s)
        _cache_data.call_components.push_back(luatools_lang::conv_to_call_components(lccmp));

    _cache_data.global_external_args = lglobal_external_args;

    return _cache_data;
}