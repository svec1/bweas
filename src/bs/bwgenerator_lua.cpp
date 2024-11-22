#include "bwgenerator.hpp"

using namespace bweas;

bwGeneratorLua::bwGeneratorLua(std::string src_lua) {
    if (!init_glob_gnlua) {
        assist.add_err("BWS-GNRT000", "Lua script not loaded");
        assist.add_err("BWS-GNRT001", "Failed to load lua script");
        assist.add_err("BWS-GNRT002", "No entry function for generator");

        init_glob_gnlua = 1;
    }

    try {
        lua.create__nmutex(src_lua);
    }
    catch (std::exception &excp) {
        throw exception::bwgenerator_excp(excp.what(), "001");
    }
}

void bwGeneratorLua::init() {
    if (!lua.is_created__nmutex())
        throw exception::bwgenerator_excp("", "000");
    else if (!lua.is_function__nmutex(NAME_FUNCTION_GENLUA))
        throw exception::bwgenerator_excp("", "002");

    std::vector<std::pair<int, std::vector<std::any>>> table_ccmps;
    for (u32t i = 0; i < ccmp_p->size(); ++i)
        table_ccmps.emplace_back(std::pair<int, std::vector<std::any>>(
            i, {(*ccmp_p)[i].name, (*ccmp_p)[i].name_program, (*ccmp_p)[i].pattern_ret_files}));

    // creation isolated env
    lua.create_global_table__nmutex();
    lua.create_var__nmutex("CCMPS", std::move(table_ccmps));
    lua.create_var__nmutex("EARGS", std::move(*external_args_p));
}
void bwGeneratorLua::deleteGenerator() {
    lua.delete_last_table__nmutex();
    delete this;
}

commands bwGeneratorLua::gen_commands(const var::struct_sb::target_out &trg, bwqueue_templates &tcmd_s) {
    std::vector<std::vector<std::pair<std::string, std::any>>> tcmd_s_vec;
    for (u32t i = 0; i < tcmd_s.size(); ++i)
        tcmd_s_vec.emplace_back(tcmd_s[i].to_vec_args());

    lua.create_var__nmutex("CURRENT_TARGET", std::move(trg.to_vec_args()));
    lua.create_var__nmutex("CURRENT_QUEUE_TEMPLATES", std::move(tcmd_s_vec));
    return lua.call_function DEFINITION_FUNCTION_GENLUA(NAME_FUNCTION_GENLUA, bwlua::lua::nil{});
}