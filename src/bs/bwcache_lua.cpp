//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include <bwcache_api.hpp>
#include <bwluatools.hpp>

#include <tools/bwlua.hpp>

using namespace bweas;
using namespace cache_api;

static constexpr auto NAME_FUNCTION_CREATE          = "create_cache";
static constexpr auto NAME_FUNCTION_GET_PATH_CONFIG = "get_path_config";
static constexpr auto NAME_FUNCTION_GET_DATA        = "get_cache_data";

static constexpr auto NAME_VARIABLE_TARGETS     = "targets";
static constexpr auto NAME_VARIABLE_TEMPLATES   = "templates";
static constexpr auto NAME_VARIABLE_CCOMPONENTS = "call_components";
static constexpr auto NAME_VARIABLE_GEARGS      = "global_external_args";

static logger _log{"BWCACHE[LUA]"};

lua_cache::lua_cache(string_v src_lua) {
    lua.create(src_lua.data());
}

void lua_cache::init(context *const __context) {
    base_cache::init(__context);

    if (!lua.is_function(NAME_FUNCTION_CREATE))
        (_log << bwtools::fatal) << (log_message(log_type::fatal) << "No entry function for creates cache");
    else if (!lua.is_function(NAME_FUNCTION_GET_PATH_CONFIG))
        (_log << bwtools::fatal) << (log_message(log_type::fatal) << "No entry function for get path config file");
    else if (!lua.is_function(NAME_FUNCTION_GET_DATA))
        (_log << bwtools::fatal) << (log_message(log_type::fatal) << "No entry function for get data of cache");
}

string lua_cache::create_cache() {
    lua_tools::param_targets ltargets_o;
    lua_tools::param_templates ltcmd_s;
    lua_tools::param_ccomponents lccmp_s;

    for (const auto &ltarget_o : _context->out_targets)
        ltargets_o.push_back(lua_tools::conv_to_table(ltarget_o));
    for (const auto &ltcmd : _context->templates)
        ltcmd_s.push_back(lua_tools::conv_to_table(ltcmd));
    for (const auto &lccmp : _context->call_components)
        lccmp_s.push_back(lua_tools::conv_to_table(lccmp));

    try {
        return lua.call_function<string, lua_tools::param_targets, lua_tools::param_templates,
                                 lua_tools::param_ccomponents, lua_tools::param_geargs>(
            NAME_FUNCTION_CREATE, ltargets_o, ltcmd_s, lccmp_s, _context->global_external_args);
    }
    catch (std::exception &what) {
        (_log << bwtools::fatal) << (log_message(log_type::fatal) << what.what());
    }
}
string lua_cache::get_path_config(const string &cache_str) {
    try {
        return lua.call_function<string, string>(NAME_FUNCTION_GET_PATH_CONFIG, cache_str);
    }
    catch (std::exception &what) {
        (_log << bwtools::fatal) << (log_message(log_type::fatal) << what.what());
    }
}

void lua_cache::extract_cache_data(const string &cache_str) {
    vec<lua_tools::table<string, any>> ltargets_o;
    vec<lua_tools::table<string, any>> ltcmd_s;
    vec<lua_tools::table<string, any>> lccmp_s;
    vec<pair<string, string>> lglobal_external_args;

    try {
        lua.call_function<void, string>(NAME_FUNCTION_GET_DATA, cache_str);

        ltargets_o            = lua[NAME_VARIABLE_TARGETS].getval<vec<lua_tools::table<string, any>>>();
        ltcmd_s               = lua[NAME_VARIABLE_TEMPLATES].getval<vec<lua_tools::table<string, any>>>();
        lccmp_s               = lua[NAME_VARIABLE_CCOMPONENTS].getval<vec<lua_tools::table<string, any>>>();
        lglobal_external_args = lua[NAME_VARIABLE_GEARGS].getval<vec<pair<string, string>>>();
    }
    catch (std::exception &what) {
        (_log << bwtools::fatal) << (log_message(log_type::fatal) << what.what());
    }

    for (auto &ltarget_o : ltargets_o)
        _context->out_targets.push_back(lua_tools::conv_to_target(ltarget_o));
    for (auto &ltcmd : ltcmd_s)
        _context->templates.push_back(lua_tools::conv_to_template(ltcmd));
    for (auto &lccmp : lccmp_s)
        _context->call_components.push_back(lua_tools::conv_to_call_components(lccmp));

    _context->global_external_args = lglobal_external_args;
}
