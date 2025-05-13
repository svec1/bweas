//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include "bwcache_api.hpp"
#include "bwluatools.hpp"

using namespace bweas;
using namespace cache_api;

static logger _log{"BWCACHE[LUA]"};

lua_bwcache::lua_bwcache(bw_context *const _context, std::string src_lua) : base_bwcache(_context) {
    if (!context)
        (_log << bwtools::fatal) << (log_message(log_type::fatal) << "Bweas the context is not defined");
    lua.create(src_lua);

    if (!lua.is_function(NAME_FUNCTION_GENERATE_CACHE_LUA))
        (_log << bwtools::fatal) << (log_message(log_type::fatal) << "No entry function for creates cache");
    else if (!lua.is_function(NAME_FUNCTION_GET_DATA_CACHE_LUA))
        (_log << bwtools::fatal) << (log_message(log_type::fatal) << "No entry function for get data of cache");
}

void lua_bwcache::delete_cache() {
    delete this;
}

std::string lua_bwcache::create_cache() {
    std::vector<bwluatools::table<std::string, std::any>> ltargets_o;
    std::vector<bwluatools::table<std::string, std::any>> ltcmd_s;
    std::vector<bwluatools::table<std::string, std::any>> lccmp_s;

    for (const auto &ltarget_o : context->out_targets)
        ltargets_o.push_back(bwluatools::conv_to_table(ltarget_o));
    for (const auto &ltcmd : context->templates)
        ltcmd_s.push_back(bwluatools::conv_to_table(ltcmd));
    for (const auto &lccmp : context->call_components)
        lccmp_s.push_back(bwluatools::conv_to_table(lccmp));

    try {
        return lua.call_function<DEFINITION_FUNCTION_GENERATE_CACHE_LUA>(
            NAME_FUNCTION_GENERATE_CACHE_LUA, ltargets_o, ltcmd_s, lccmp_s, context->global_external_args);
    }
    catch (std::exception &what) {
        (_log << bwtools::fatal) << (log_message(log_type::fatal) << what.what());
    }
}

void lua_bwcache::extract_cache_data(std::string &&cache_str) {

    std::vector<bwluatools::table<std::string, std::any>> ltargets_o;
    std::vector<bwluatools::table<std::string, std::any>> ltcmd_s;
    std::vector<bwluatools::table<std::string, std::any>> lccmp_s;
    std::vector<std::pair<std::string, std::string>> lglobal_external_args;

    try {
        lua.call_function<DEFINITION_FUNCTION_GET_DATA_CACHE_LUA>(NAME_FUNCTION_GET_DATA_CACHE_LUA, {});

        ltargets_o =
            lua[NAME_VARIABLE_TARGETS_F_EXTERN_LUA].getval<std::vector<bwluatools::table<std::string, std::any>>>();
        ltcmd_s =
            lua[NAME_VARIABLE_TEMPLATES_F_EXTERN_LUA].getval<std::vector<bwluatools::table<std::string, std::any>>>();
        lccmp_s =
            lua[NAME_VARIABLE_CCOMPONENTS_F_EXTERN_LUA].getval<std::vector<bwluatools::table<std::string, std::any>>>();
        lglobal_external_args =
            lua[NAME_VARIABLE_GEARGS_F_EXTERN_LUA].getval<std::vector<std::pair<std::string, std::string>>>();
    }
    catch (std::exception &what) {
        (_log << bwtools::fatal) << (log_message(log_type::fatal) << what.what());
    }
    for (auto &ltarget_o : ltargets_o)
        context->out_targets.push_back(bwluatools::conv_to_target(ltarget_o));
    for (auto &ltcmd : ltcmd_s)
        context->templates.push_back(bwluatools::conv_to_template(ltcmd));
    for (auto &lccmp : lccmp_s)
        context->call_components.push_back(bwluatools::conv_to_call_components(lccmp));

    context->global_external_args = lglobal_external_args;
}
