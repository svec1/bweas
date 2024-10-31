#include "bwgenerator.hpp"

using namespace bweas;

bwGeneratorLua::bwGeneratorLua() {
    if (!init_glob) {
        assist.add_err("BWS-GNRT000", "Lua script not loaded");
        assist.add_err("BWS-GNRT001", "Failed to load lua script");

        init_glob = 1;
    }
}

void bwGeneratorLua::load_lua(std::string src_lua) {
    try {
        lua.create__nmutex(src_lua);
    }
    catch (std::exception &excp) {
        throw exception::bwgenerator_excp(excp.what(), "001");
    }
}
void bwGeneratorLua::init() {
    if (!lua.is_created())
        throw exception::bwgenerator_excp("", "000");
}

command bwGeneratorLua::gen_commands(const var::struct_sb::target_out &, bwqueue_templates &) {
    return "";
}