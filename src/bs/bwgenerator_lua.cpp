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

static logger log{"BWGENERATOR[LUA]"};

lua_generator::lua_generator(std::string src_lua) {
    try {
        lua.create(src_lua);
    }
    catch (std::exception &excp) {
        (log << bwtools::fatal) << (log_message(log_type::fatal) << "Couldn't load lua code");
    }
}

void lua_generator::init() {
    if (!lua.is_created())
        (log << bwtools::fatal) << (log_message(log_type::fatal) << "Lua script not loaded");
    else if (!lua.is_function(NAME_FUNCTION_GENERATE_COMMAND_LUA))
        (log << bwtools::fatal) << (log_message(log_type::fatal) << "No entry function for generator");
    else if (!lua.is_function(NAME_FUNCTION_GET_INPUT_FILE_LUA))
        (log << bwtools::fatal) << (log_message(log_type::fatal) << "No entry function for get input files");

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
        (log << bwtools::fatal) << (log_message(log_type::fatal) << "Run-time error: " << what.what());
    }
}

void lua_generator::get_input_files(data_transfer &data_t) {
    bwluatools::array<bwluatools::table<std::string, std::string>> ccmps;
    for (const auto &call_component : data_t.context->call_components)
        ccmps.emplace_back(bwluatools::conv_to_table(call_component));
    bwluatools::array<bwluatools::table<std::string, std::any>> tcmd_s_vec;
    for (const auto &_template : data_t.context->templates)
        tcmd_s_vec.emplace_back(bwluatools::conv_to_table(_template));

    try {
        lua["CCMPS"]  = ccmps;
        data_t.ifiles = bwlua::lua::to_map(lua.call_function<DEFINITION_FUNCTION_GET_INPUT_FILE_LUA>(
            NAME_FUNCTION_GET_INPUT_FILE_LUA, bwluatools::conv_to_table(*data_t.context->current_target), tcmd_s_vec,
            bwluatools::conv_to_table(data_t.dfiles)));
    }
    catch (std::exception &what) {
        (log << bwtools::fatal) << (log_message(log_type::fatal) << "Run-time error: " << what.what());
    }
}

gen_command lua_generator::generate_command(data_transfer &data_t) {
    generator_tools::parse_basic_args(*data_t.context->current_target, data_t.context->templates,
                                      data_t.context->global_external_args);

    std::vector<bwluatools::table<std::string, std::any>> tcmd_s_vec;
    for (const auto &_template : data_t.context->templates)
        tcmd_s_vec.emplace_back(bwluatools::conv_to_table(_template));

    lua["CURRENT_TARGET"]          = bwluatools::conv_to_table(*data_t.context->current_target);
    lua["CURRENT_QUEUE_TEMPLATES"] = tcmd_s_vec;
    lua["CURRENT_DIR"]             = data_t.work_directory;

    try {
        return bwlua::lua::to_map(lua.call_function<DEFINITION_FUNCTION_GENERATE_COMMAND_LUA>(
            NAME_FUNCTION_GENERATE_COMMAND_LUA, bwlua::lua::to_table(data_t.ifiles)));
    }
    catch (std::exception &what) {
        (log << bwtools::fatal) << (log_message(log_type::fatal) << "Run-time error: " << what.what());
    }
}
