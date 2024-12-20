#include "bwmodule.hpp"

using namespace bweas;
using namespace bwexception;

module::module_mg::module_mg() {
    if (!init_glob) {
        assist.add_err("BWS-MDL000", "Failed to load module dependent dll file");
        assist.add_err("BWS-MDL001", "Failed to get the specified module function");
    }
}

semantic_an::table_func module::module_mg::init_tfunc(module &md) {
    semantic_an::table_func tfuncs;
    try {
#if defined(WIN)
        HMODULE dll = assist.load_dll(md.name_dll);
        if (DWORD it_err = assist.get_error_win32())
            throw bwmodule_excp("WinAPI error: " + std::to_string(it_err), "000");
#elif defined(UNIX)
        u32t dll = assist.load_dl(md.name_dll);
        if (!assist.get_error_dl().empty())
            throw bwmodule_excp(assist.get_error_dl(), "000");
#endif

        for (auto &func : md.funcs) {
            func.second.func_ref = (aef_expr::notion_func::func_t)assist.get_ptr_func(md.name_dll, func.first);
#if defined(WIN)
            if (DWORD it_err = assist.get_error_win32())
                throw bwmodule_excp("WinAPI error: " + std::to_string(it_err), "001");
#elif defined(UNIX)
            if (!assist.get_error_dl().empty())
                throw bwmodule_excp(assist.get_error_dl(), "001");
#endif
        }
    }
    catch (bwmodule_excp &_excp) {
        throw bwmodule_excp(md.name_dll + std::string(" - Module ") + _excp.what(),
                            std::string(_excp.get_assist_err()).erase(0, 7));
    }
    return md.funcs;
}

semantic_an::table_func module::module_mg::init_tsfunc(modules &mds) {
    semantic_an::table_func tfuncs;
    for (auto &md : mds) {
        semantic_an::table_func tfuncs_md = init_tfunc(md);
        for (const auto &func_md : tfuncs_md)
            tfuncs.insert(func_md);
    }
    return tfuncs;
}