//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include <bwmodule.hpp>

#include <bwluatools.hpp>

using namespace bweas;
using namespace bweas::utils;

static logger _log{"BWMODULE"};

umap<string, scope::module_data> module_manager::init_modules(vec<module_cfg> &modules_cfg) {
    umap<string, scope::module_data> modules;
    for (auto &md : modules_cfg) {
        scope::module_data module_data_tmp;
        module_data_tmp.profiles = std::move(md.profiles);

        umap<string, decl_func> &funcs = module_data_tmp.funcs;
        for (auto &[name, _decl_func] : md.funcs) {
            _decl_func.func = [&md, &_decl_func](const expressions &expr_s, scope &curr_scope) {
                static umap<string, lua> lua_stream_s;
                if (!lua_stream_s[md.name].is_created())
                    lua_stream_s.emplace(md.name, file_utils::read_file(file_utils::get_ref_file(
                                                      file_utils::open_file(md.name_lua_source_file))));

                lua_stream_s[md.name].call_function<string_v, lua_tools::integer, lua_tools::integer>(
                    _decl_func.name, *((lua_tools::integer *)&expr_s), *((lua_tools::integer *)&curr_scope));
            };
        }

        modules[md.name] = module_data_tmp;
    }

    return modules;
}
