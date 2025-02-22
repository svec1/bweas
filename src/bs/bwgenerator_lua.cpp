//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include "bwgenerator_api.hpp"
#include "bwgntools.hpp"
#include "bwluatools.hpp"

using namespace bweas;
using namespace generator_api;
using namespace bweas::bwexception;

lua_generator::lua_generator(std::string src_lua) {
    if (!init_glob_gnlua) {
        assist.add_err("BWS-GNRT000", "Unable to parse basic arguments");
        assist.add_err("BWS-GNRT001", "Lua script not loaded");
        assist.add_err("BWS-GNRT002", "Failed to load lua script");
        assist.add_err("BWS-GNRT003", "No entry function for generator");
        assist.add_err("BWS-GNRT004", "Not found function for get input files");
        assist.add_err("BWS-GNRT005", "Run-time error");

        init_glob_gnlua = 1;
    }

    try {
        lua.create(src_lua);
    }
    catch (std::exception &excp) {
        throw bwgenerator_excp(excp.what(), "002");
    }
}

void lua_generator::init() {
    if (!lua.is_created())
        throw bwgenerator_excp("", "001");
    else if (!lua.is_function(NAME_FUNCTION_GENERATE_COMMAND_LUA))
        throw bwgenerator_excp("", "003");
    else if (!lua.is_function(NAME_FUNCTION_GET_INPUT_FILE_LUA))
        throw bwgenerator_excp("", "004");

    lua["get_name_output_file_lua"] << bwluatools::get_name_output_file_lua;
    lua["file_slc_mask"] << bwluatools::file_slc_mask_lua;
}
void lua_generator::_delete() {
    delete this;
}

std::unordered_set<std::string> lua_generator::build_graph_depends_file(std::string_view language,
                                                                        std::string_view name_file,
                                                                        std::string_view work_directory,
                                                                        std::vector<std::string> include_path) {
    try {
        bwluatools::array<std::string> dependencies = lua.call_function<DEFINITION_FUNCTION_BUILD_DEPENDS_LUA>(
            NAME_FUNCTION_BUILD_DEPENDS_LUA, language, name_file, work_directory, include_path);
        return std::unordered_set<std::string>{dependencies.begin(), dependencies.end()};
    }
    catch (std::exception &what) {
        throw bwgenerator_excp(what.what(), "005");
    }
}

void lua_generator::get_input_files(data_transfer &data_t) {
    bwluatools::array<bwluatools::array<std::string>> ccmps;
    for (const auto &call_component : data_t.context->call_components)
        ccmps.emplace_back(bwluatools::array<std::string>{call_component.name, call_component.name_program,
                                                          call_component.pattern_ret_files});
    bwluatools::array<bwluatools::table<std::string, std::any>> tcmd_s_vec;
    for (const auto &_template : data_t.context->templates)
        tcmd_s_vec.emplace_back(bwluatools::conv_to_table(_template));

    try {
        lua["CCMPS"] = ccmps;
        data_t.ifiles = bwlua::lua::to_map(lua.call_function<DEFINITION_FUNCTION_GET_INPUT_FILE_LUA>(
            NAME_FUNCTION_GET_INPUT_FILE_LUA, bwluatools::conv_to_table(*data_t.context->current_target), tcmd_s_vec,
            bwluatools::conv_to_table(data_t.dfiles)));
    }
    catch (std::exception &what) {
        throw bwgenerator_excp(what.what(), "005");
    }
}

gen_command lua_generator::generate_command(data_transfer &data_t) {

    generator::tools::parse_basic_args(*data_t.context->current_target, data_t.context->templates,
                                       data_t.context->global_external_args);

    std::vector<bwluatools::table<std::string, std::any>> tcmd_s_vec;
    for (const auto &_template : data_t.context->templates)
        tcmd_s_vec.emplace_back(bwluatools::conv_to_table(_template));

    lua["CURRENT_TARGET"] = bwluatools::conv_to_table(*data_t.context->current_target);
    lua["CURRENT_QUEUE_TEMPLATES"] = tcmd_s_vec;
    lua["CURRENT_DIR"] = data_t.work_directory;

    try {
        return bwlua::lua::to_map(lua.call_function<DEFINITION_FUNCTION_GENERATE_COMMAND_LUA>(
            NAME_FUNCTION_GENERATE_COMMAND_LUA, bwlua::lua::to_table(data_t.ifiles)));
    }
    catch (std::exception &what) {
        throw bwgenerator_excp(what.what(), "004");
    }
}
