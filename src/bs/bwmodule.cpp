//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include <bwmodule.hpp>

#include <bwluatools.hpp>

using namespace bweas;

static logger _log{"BWMODULE"};

vec<decl_func> module_manager::init_mfuncs(modules &mds) {
    vec<decl_func> funcs;
    for (auto &md : mds) {
        for (auto &_decl_func : md.funcs) {
            _decl_func.func = [&md, &_decl_func](const expressions &expr_s, scope &curr_scope) {
                static umap<string, bwlua::lua> lua_stream_s;
                if (!lua_stream_s[md.name_module].is_created())
                    lua_stream_s.emplace(md.name_module, bwtools::read_file(bwtools::get_ref_file(
                                                             bwtools::open_file(md.name_lua_source_file))));

                lua_stream_s[md.name_module].call_function<string_v, lua_tools::integer, lua_tools::integer>(
                    _decl_func.name, *((lua_tools::integer *)&expr_s), *((lua_tools::integer *)&curr_scope));
            };
            funcs.push_back(_decl_func);
        }
    }
    return funcs;
}
