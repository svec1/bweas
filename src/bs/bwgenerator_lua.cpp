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
    else if(!lua.is_function__nmutex(NAME_FUNCTION_GENLUA))
        throw exception::bwgenerator_excp("", "002");
    
    // creation isolated env
    lua.create_global_table__nmutex();
    lua.execute_str__nmutex("");


}
void bwGeneratorLua::deleteGenerator() {
    lua.delete_last_table__nmutex();
    delete this;
}

commands bwGeneratorLua::gen_commands(const var::struct_sb::target_out &, bwqueue_templates &) {
    return lua.call_function DEFINITION_FUNCTION_GENLUA(NAME_FUNCTION_GENLUA, bwlua::lua::nil{});
}