#include <bwdepends_api.hpp>
#include <bwlogger.hpp>

#include <bwluatools.hpp>

using namespace bweas;

static constexpr auto NAME_FUNCTION_BUILD_GRAPH = "build_graph_depends";

static logger _log{"BWDEPENDS_FINDER[LUA]"};
static utils::lua lua;

depends_lua::depends_lua(const string_v src_lua) {
    lua.create(src_lua.data());
}

void depends_lua::build_graph_depends_file(string_v name_file, uset<string> &graph_depends_file) {
    try {
        vec<string> dependencies = lua.call_function<lua_tools::array<string>, string, lua_tools::array<string>>(
            NAME_FUNCTION_BUILD_GRAPH, name_file.data(), include_paths);

        graph_depends_file.insert(dependencies.begin(), dependencies.end());
    }
    catch (std::exception &what) {
        _log << (log_message(log_type::fatal) << what.what());
    }
}
