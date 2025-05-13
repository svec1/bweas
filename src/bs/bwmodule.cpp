//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include <bwmodule.hpp>

using namespace bweas;

static logger _log{"BWMODULE"};

std::unordered_map<std::string, bwlua::lua> bwmodule_mg::lua_stream_s;

std::vector<decl_func> bwmodule_mg::init_mfuncs(modules &mds) {
    std::vector<decl_func> funcs;
    for (auto &md : mds) {
        lua_stream_s.emplace(md.name_module,
                             bwtools::read_file(bwtools::get_ref_file(bwtools::open_file(md.name_lua_source_file))));
        for (auto &_decl_func : md.funcs) {
            _decl_func.func = [md, _decl_func, lua_stream_s](const expressions &expr_s, var::scope &curr_scope) {
                lua_stream_s[md.name_module].call_function<std::string_view, bwluatools::integer, bwluatools::integer>(
                    _decl_func.name_func, *((bwluatools::integer *)&expr_s), *((bwluatools::integer *)&curr_scope));
            };
            funcs.push_back(_decl_func);
        }
    }
    return funcs;
}
