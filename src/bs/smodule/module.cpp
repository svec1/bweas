#include "module.hpp"

using namespace module;

void mdl_manager::import_module_decl(const char *import_file) {
    HND handle_f = assist.open_file(import_file, MODE_READ_FILE);
    if (!assist.exist_file(handle_f))
        assist.call_err("RTT004", "File: " + std::string(import_file));
    std::string buf = assist.read_file(handle_f);
    assist.close_file(handle_f);

    std::pair<std::vector<srl::token>, std::vector<std::string>> data_after_parse = srl::parser(buf);

    mdls = srl::build_modules(data_after_parse.first);
    func_with_smt = data_after_parse.second;
}
const std::vector<std::string> &mdl_manager::get_func_with_smt() {
    return func_with_smt;
}
semantic_an::table_func mdl_manager::load_module(std::string name) {
    semantic_an::table_func external_func_table;
    for (u32t i = 0; i < mdls.size(); ++i) {
        const auto &it = mdls[i].find(name);
        if (it != mdls[i].end()) {
#if defined(WIN)
            if (!assist.load_dll(it->first))
                assist.call_err("RTT001",
                                "Error code(+" + std::to_string(assist.get_error_win32()) + "); Library: " + it->first);
#elif defined(UNIX)
            if (assist.load_dl(it->first))
                assist.call_err("RTT001", "Error(+" + std::string(assist.get_error_dl()) + "); Library: " + it->first);
#endif
            for (const auto &it_j : it->second) {
                aef_expr::notion_func nfunc;
                for (const auto &it_b : it_j) {
                    nfunc.func_ref = (void (*)(const std::vector<aef_expr::subexpressions> &,
                                               var::scope &))assist.get_ptr_func(it->first, it_b.first);
                    if (nfunc.func_ref == nullptr)
#if defined(WIN)
                        assist.call_err("RTT002", "Error code(+" + std::to_string(assist.get_error_win32()) +
                                                      "); Improper handling: " + it->first + "->" + it_b.first);
#elif defined(UNIX)
                        assist.call_err("RTT002", "Error(+" + std::string(assist.get_error_dl()) +
                                                      "); Improper handling: " + it->first + "->" + it_b.first);
#endif
                    nfunc.expected_args = it_b.second;
                    external_func_table.emplace(it_b.first, nfunc);
                }
            }
            return external_func_table;
        }
    }

    return external_func_table;
}
semantic_an::table_func mdl_manager::load_modules(const std::vector<std::string> &load_module_name) {
    semantic_an::table_func external_func_table;
    for (u32t i = 0; i < mdls.size(); ++i) {
        for (const auto &it : mdls[i]) {
            for (u32t j = 0; j < load_module_name.size(); ++j) {
                if (it.first == load_module_name[j])
                    goto load_module;
            }
            continue;

        load_module:
#if defined(WIN)
            if (!assist.load_dll(it.first))
                assist.call_err("RTT001",
                                "Error code(+" + std::to_string(assist.get_error_win32()) + "); Library: " + it.first);
#elif defined(UNIX)
            if (assist.load_dl(it.first))
                assist.call_err("RTT001", "Error(+" + std::string(assist.get_error_dl()) + "); Library: " + it.first);
#endif
            for (const auto &it_j : it.second) {
                aef_expr::notion_func nfunc;
                for (const auto &it_b : it_j) {
                    nfunc.func_ref = (void (*)(const std::vector<aef_expr::subexpressions> &,
                                               var::scope &))assist.get_ptr_func(it.first, it_b.first);
                    if (nfunc.func_ref == nullptr)
#if defined(WIN)
                        assist.call_err("RTT002", "Error code(+" + std::to_string(assist.get_error_win32()) +
                                                      "); Improper handling: " + it.first + "->" + it_b.first);
#elif defined(UNIX)
                        assist.call_err("RTT002", "Error(+" + std::string(assist.get_error_dl()) +
                                                      "); Improper handling: " + it.first + "->" + it_b.first);
#endif
                    nfunc.expected_args = it_b.second;
                    external_func_table.emplace(it_b.first, nfunc);
                }
            }
        }
    }

    return external_func_table;
}

semantic_an::table_func mdl_manager::load_modules_all() {
    semantic_an::table_func external_func_table;
    for (u32t i = 0; i < mdls.size(); ++i) {
        for (const auto &it : mdls[i]) {
#if defined(WIN)
            if (!assist.load_dll(it.first))
                assist.call_err("RTT001",
                                "Error code(+" + std::to_string(assist.get_error_win32()) + "); Library: " + it.first);
#elif defined(UNIX)
            if (assist.load_dl(it.first))
                assist.call_err("RTT001", "Error(+" + std::string(assist.get_error_dl()) + "); Library: " + it.first);
#endif
            for (const auto &it_j : it.second) {
                aef_expr::notion_func nfunc;
                for (const auto &it_b : it_j) {
                    nfunc.func_ref = (void (*)(const std::vector<aef_expr::subexpressions> &,
                                               var::scope &))assist.get_ptr_func(it.first, it_b.first);
                    if (nfunc.func_ref == nullptr)
#if defined(WIN)
                        assist.call_err("RTT002", "Error code(+" + std::to_string(assist.get_error_win32()) +
                                                      "); Improper handling: " + it.first + "->" + it_b.first);
#elif defined(UNIX)
                        assist.call_err("RTT002", "Error(+" + std::string(assist.get_error_dl()) +
                                                      "); Improper handling: " + it.first + "->" + it_b.first);
#endif
                    nfunc.expected_args = it_b.second;
                    external_func_table.emplace(it_b.first, nfunc);
                }
            }
        }
    }

    return external_func_table;
}