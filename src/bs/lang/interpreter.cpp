#include "interpreter.hpp"
#include "static_linking_func.hpp"

using namespace interpreter;

using mf = assistant::file::mode_file;
using file_it = assistant::file_it;

interpreter_exec::interpreter_exec() {
    if (!init_glob) {
        assist.add_err("RTT000", "Undefined behavior");
        assist.add_err("RTT001", "The DLL cannot be loaded");
        assist.add_err("RTT002", "Function declarations cannot be found");
        assist.add_err("RTT003", "It is impossible to find the structure");
        assist.add_err("RTT004", "Failed to open file");
        assist.add_err("RTT005", "Internal function error");
    }
}
interpreter_exec::interpreter_exec(config conf) {
    interpreter_exec();
    interp_conf = conf;
}

void interpreter_exec::set_config(config conf) {
    interp_conf = conf;
}
void interpreter_exec::set_external_scope(var::scope *_external_scope) {
    external_scope = _external_scope;
}

void interpreter_exec::build_aef() {
    file_it bweas_config = assist.open_file(interp_conf.filename_interp, mf::open::rb);

    lexer.set_symbols(assist.read_file(assist.get_ref_file(bweas_config), mf::input::read_binary));
    assist.close_file(bweas_config);

    global_scope.clear();

    clock_t beg;

    smt_analyzer.load_external_func_table(external_func_table);
#if defined(WIN)
    assist.safe_call_dll_func_begin();
#endif
    if (interp_conf.debug_output) {
        debug_mark_time_func(lexer.analysis(), "lexer")
        parser.set_tokens(lexer.get_tokens());
        debug_mark_time_func(parser.analysis(), "parser")
        aef = parser.get_exprs();
        if (interp_conf.use_external_scope) {
            debug_mark_time_func(smt_analyzer.analysis(aef, *external_scope), "semantic analyzer")
        }
        else {
            debug_mark_time_func(smt_analyzer.analysis(aef, global_scope), "semantic analyzer")
        }
    }
    else {
        parser.set_tokens(lexer.analysis());
        aef = parser.analysis();

        if (interp_conf.use_external_scope)
            smt_analyzer.analysis(aef, *external_scope);
        else
            smt_analyzer.analysis(aef, global_scope);
    }
#ifdef _WIN32
    assist.safe_call_dll_func_end();
#endif
}

void interpreter_exec::interpret() {
#if defined(WIN)
    curr_expr = 0;
    assist.safe_call_dll_func_begin();
    for (; curr_expr < aef.size(); ++curr_expr) {
        if (aef[curr_expr].execute_with_semantic_an())
            continue;
        aef[curr_expr].expr_func.func_n.func_ref(aef[curr_expr].sub_expr_s, global_scope);
        assist.check_safe_call_dll_func();
    }
    assist.safe_call_dll_func_end();
#elif defined(UNIX)
    curr_expr = 0;
    for (; curr_expr < aef.size(); ++curr_expr) {
        if (aef[curr_expr].execute_with_semantic_an())
            continue;
        aef[curr_expr].expr_func.func_n.func_ref(aef[curr_expr].sub_expr_s, global_scope);
        assist.check_safe_call_dl_func();
    }
#endif
}

std::vector<var::struct_sb::target> interpreter_exec::export_targets() {
    const std::vector<std::pair<std::string, var::struct_sb::target>> &vec_targets_ref =
        global_scope.get_vector_variables_t<var::struct_sb::target>();

    std::vector<var::struct_sb::target> targets;
    for (u32t i = 0; i < vec_targets_ref.size(); ++i) {
        targets.push_back(vec_targets_ref[i].second);
    }

    return targets;
}

var::scope &interpreter_exec::get_current_scope() {
    if (interp_conf.use_external_scope)
        return *external_scope;
    return global_scope;
}

void interpreter_exec::load_external_func(semantic_an::table_func &&tfuncs) {
    std::vector<std::string> tmp_w_smt_funcs;
    for (auto &func : tfuncs) {
        if (interp_conf.transmit_smt_name_func_with_smt &&
            (func.second.is_declaration_var || func.second.only_with_semantic))
            tmp_w_smt_funcs.push_back(func.first);
        external_func_table[func.first] = func.second;
    }

    if (interp_conf.transmit_smt_name_func_with_smt)
        smt_analyzer.append_external_name_func_w_smt(tmp_w_smt_funcs);
}