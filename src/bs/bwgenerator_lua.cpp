//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include <bwgenerator_api.hpp>
#include <bwgntools.hpp>
#include <bwluatools.hpp>

using namespace bweas;

static constexpr const char *NAME_FUNCTION_GENERATE       = "generate";
static constexpr const char *NAME_FUNCTION_BUILD_DEPENDS  = "build_graph_depends_file";
static constexpr const char *NAME_FUNCTION_GET_INPUT_FILE = "get_input_files";

static logger _log{"BWGENERATOR[LUA]"};

generator_api::lua_generator::lua_generator(string_v src_lua) {
    try {
        lua.create(src_lua.data());
    }
    catch (std::exception &excp) {
        (_log << bwtools::fatal) << (log_message(log_type::fatal) << "Couldn't load lua code");
    }
}

void generator_api::lua_generator::init() {
    if (!lua.is_created())
        (_log << bwtools::fatal) << (log_message(log_type::fatal) << "Lua script not loaded");
    else if (!lua.is_function(NAME_FUNCTION_GENERATE))
        (_log << bwtools::fatal) << (log_message(log_type::fatal) << "No entry function for generator");
    else if (!lua.is_function(NAME_FUNCTION_GET_INPUT_FILE))
        (_log << bwtools::fatal) << (log_message(log_type::fatal) << "No entry function for get input files");

    lua["get_name_output_file_lua"] << lua_tools::get_name_output_file_lua;
    lua["file_slc_mask"] << lua_tools::file_slc_mask_lua;
}

uset<string> generator_api::lua_generator::build_graph_depends_file(string_v language, string_v name_file,
                                                                    string_v work_directory, vec<string> include_path) {
    try {
        lua_tools::array<string> dependencies =
            lua.call_function<lua_tools::array<string>, string_v, string_v, string_v, vec<string>>(
                NAME_FUNCTION_BUILD_DEPENDS, language, name_file, work_directory, include_path);
        return uset<string>{dependencies.begin(), dependencies.end()};
    }
    catch (std::exception &what) {
        (_log << bwtools::fatal) << (log_message(log_type::fatal) << "Run-time error: " << what.what());
    }

    return {};
}

void generator_api::lua_generator::get_input_files(data_transfer &data_t) {
    lua_tools::array<lua_tools::table<string_v, string>> ccmps;
    for (const auto &call_component : data_t.context->call_components)
        ccmps.emplace_back(lua_tools::conv_to_table(call_component));
    lua_tools::array<lua_tools::table<string_v, any>> tcmd_s_vec;
    for (const auto &_template : data_t.context->templates)
        tcmd_s_vec.emplace_back(lua_tools::conv_to_table(_template));

    try {
        lua["CURRENT_TARGET"]          = lua_tools::conv_to_table(*data_t.context->current_target);
        lua["CURRENT_QUEUE_TEMPLATES"] = tcmd_s_vec;
        lua["CCMPS"]                   = ccmps;

        data_t.ifiles = bwlua::lua::to_map(lua.call_function<lua_tools::table<string, lua_tools::array<string>>,
                                                             lua_tools::table<string, lua_tools::array<string>>>(
            NAME_FUNCTION_GET_INPUT_FILE, lua_tools::conv_to_table(data_t.dfiles)));
    }
    catch (std::exception &excp) {
        _log << bwtools::fatal
             << (log_message(log_type::fatal) << "Couldn't get the input files for the current target's templates("
                                              << data_t.context->current_target->name << "):\n"
                                              << excp.what());
    }
}

generator_api::commands generator_api::lua_generator::generate_commands(data_transfer &data_t) {
    vec<lua_tools::table<string_v, any>> tcmd_s_vec;
    for (const auto &_template : data_t.context->templates)
        tcmd_s_vec.emplace_back(lua_tools::conv_to_table(_template));

    lua["CURRENT_TARGET"]          = lua_tools::conv_to_table(*data_t.context->current_target);
    lua["CURRENT_QUEUE_TEMPLATES"] = tcmd_s_vec;
    lua["CURRENT_DIR"]             = data_t.context->current_work_directory;

    try {
        generator_tools::parse_basic_args(*data_t.context->current_target, data_t.context->templates,
                                          data_t.context->global_external_args);
        map<string, pair<string, vec<string>>> commands_map = bwlua::lua::to_map(
            lua.call_function<lua_tools::table<string, lua_tools::key_value<string, lua_tools::array<string>>>,
                              lua_tools::table<string, lua_tools::array<string>>>(NAME_FUNCTION_GENERATE,
                                                                                  bwlua::lua::to_table(data_t.ifiles)));

        generator_api::commands cmd_s;
        for (const auto &[name_used_file, command] : commands_map)
            cmd_s.push_back(generator_api::command{name_used_file, command.first, command.second});

        return cmd_s;
    }
    catch (std::exception &excp) {
        _log << bwtools::fatal
             << (log_message(log_type::fatal) << "Failed to generate a template command for the current target("
                                              << data_t.context->current_target->name << "):\n"
                                              << excp.what());
    }

    return {};
}
