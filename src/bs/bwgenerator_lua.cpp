#include "bwgenerator.hpp"

using namespace bweas;

bwGeneratorLua::bwGeneratorLua(std::string src_lua) {
    if (!init_glob_gnlua) {
        assist.add_err("BWS-GNRT000", "Lua script not loaded");
        assist.add_err("BWS-GNRT001", "Failed to load lua script");
        assist.add_err("BWS-GNRT002", "No entry function for generator");
        assist.add_err("BWS-GNRT003", "Run-time error");

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

    bwlua::lua::fastTable<bwlua::lua::integer, std::vector<std::string>> table_ccmps;
    for (u32t i = 0; i < ccmp_p->size(); ++i)
        table_ccmps.emplace_back(std::pair<bwlua::lua::integer, std::vector<std::string>>(
            i, {(*ccmp_p)[i].name, (*ccmp_p)[i].name_program, (*ccmp_p)[i].pattern_ret_files}));

    // creation isolated env
    lua.create_global_table__nmutex();
    lua["CCMPS"] = table_ccmps;
    lua["EARGS"] = *external_args_p;
}
void bwGeneratorLua::deleteGenerator() {
    lua.delete_last_table__nmutex();
    delete this;
}

commands bwGeneratorLua::gen_commands(const var::struct_sb::target_out &trg, bwqueue_templates &tcmd_s) {
    std::vector<std::vector<std::pair<std::string, std::any>>> tcmd_s_vec;
    for (u32t i = 0; i < tcmd_s.size(); ++i)
        tcmd_s_vec.emplace_back(tcmd_s[i].to_vec_args());

    lua["CURRENT_TARGET"] = trg.to_vec_args();
    lua["CURRENT_QUEUE_TEMPLATES"] = tcmd_s_vec;

    lua["TEST"] = bwlua::lua::table<std::string, std::string>{std::pair<std::string, std::string>(std::string("Hello"), std::string("World!"))};
    assist << lua["TEST"].getval<bwlua::lua::table<std::string, std::string>>()["Hello"];
    assist << lua["CCMPS"].getval<bwlua::lua::fastTable<bwlua::lua::integer, std::vector<std::string>>>()[0].second[0];

    assist << std::to_string(std::any_cast<bwlua::lua::number>(
        std::any_cast<bwlua::lua::keyValue<std::any, std::any>>(
            std::any_cast<std::vector<std::any>>(
                lua["CURRENT_TARGET"].getval<bwlua::lua::fastTable<std::string, std::any>>()[3].second)[10])
            .second));

    try {
        return lua.call_function DEFINITION_FUNCTION_GENLUA(NAME_FUNCTION_GENLUA, bwlua::lua::nil{});
    }
    catch (std::exception &what) {
        throw exception::bwgenerator_excp(what.what(), "003");
    }
}