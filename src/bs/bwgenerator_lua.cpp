#include "bwgenerator_api.hpp"
#include "bwluatools.hpp"

using namespace bweas;
using namespace bwexception;

bwGeneratorLua::bwGeneratorLua(std::string src_lua) {
    if (!init_glob_gnlua) {
        assist.add_err("BWS-GNRT000", "Lua script not loaded");
        assist.add_err("BWS-GNRT001", "Failed to load lua script");
        assist.add_err("BWS-GNRT002", "No entry function for generator");
        assist.add_err("BWS-GNRT003", "Not found function for get input files");
        assist.add_err("BWS-GNRT004", "Run-time error");

        init_glob_gnlua = 1;
    }

    try {
        lua.create__nmutex(src_lua);
    }
    catch (std::exception &excp) {
        throw bwgenerator_excp(excp.what(), "001");
    }
}

void bwGeneratorLua::init() {
    if (!lua.is_created__nmutex())
        throw bwgenerator_excp("", "000");
    else if (!lua.is_function__nmutex(NAME_FUNCTION_GENLUA))
        throw bwgenerator_excp("", "002");
    else if (!lua.is_function__nmutex(NAME_FUNCTION_FILECLUA))
        throw bwgenerator_excp("", "003");

    bwlua::lua::array<bwlua::lua::array<std::string>> ccmps;
    for (u32t i = 0; i < ccmp_p->size(); ++i)
        ccmps.emplace_back(bwlua::lua::array<std::string>{(*ccmp_p)[i].name, (*ccmp_p)[i].name_program,
                                                          (*ccmp_p)[i].pattern_ret_files});

    lua["CCMPS"] = ccmps;
}
void bwGeneratorLua::deleteGenerator() {
    delete this;
}

std::map<std::string, std::vector<std::string>> bwGeneratorLua::input_files(
    const var::struct_sb::target_out &target, const bwqueue_templates &target_queue_templates,
    std::string dir_work_endv) {
    bwlua::lua::array<bwlua::lua::table<std::string, std::any>> tcmd_s_vec;
    for (u32t i = 0; i < target_queue_templates.size(); ++i)
        tcmd_s_vec.emplace_back(luatools_bwstruct::conv_to_table(target_queue_templates[i]));

    try {
        return bwlua::lua::to_map(lua.call_function DEFINITION_FUNCTION_FILECLUA(
            NAME_FUNCTION_FILECLUA, luatools_bwstruct::conv_to_table(target), tcmd_s_vec));
    }
    catch (std::exception &what) {
        throw bwgenerator_excp(what.what(), "004");
    }
}

commands bwGeneratorLua::gen_commands(const var::struct_sb::target_out &trg, bwqueue_templates &tcmd_s,
                                      std::string dir_work_endv,
                                      std::map<std::string, std::vector<std::string>> files_input) {
    std::vector<bwlua::lua::table<std::string, std::any>> tcmd_s_vec;
    for (u32t i = 0; i < tcmd_s.size(); ++i)
        tcmd_s_vec.emplace_back(luatools_bwstruct::conv_to_table(tcmd_s[i]));

    lua["CURRENT_TARGET"] = luatools_bwstruct::conv_to_table(trg);
    lua["CURRENT_QUEUE_TEMPLATES"] = tcmd_s_vec;
    lua["CURRENT_DIR"] = dir_work_endv;

    try {
        return lua.call_function DEFINITION_FUNCTION_GENLUA(NAME_FUNCTION_GENLUA, bwlua::lua::to_table(files_input));
    }
    catch (std::exception &what) {
        throw bwgenerator_excp(what.what(), "004");
    }
}