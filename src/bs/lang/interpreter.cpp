#include "interpreter.hpp"
#include "static_linking_func.hpp"

using namespace interpreter;

interpreter_exec::interpreter_exec(){
    if(!init_glob){
        srl::init_err();
        assist.add_err("RTT000", "Undefined behavior");
        assist.add_err("RTT001", "The DLL cannot be loaded");
        assist.add_err("RTT002", "Function declarations cannot be found");
        assist.add_err("RTT003", "It is impossible to find the structure");
    }
}
interpreter_exec::interpreter_exec(config conf){
    interpreter_exec();
    interp_conf = conf;
}

void interpreter_exec::set_config(config conf){
    interp_conf = conf;
}

void interpreter_exec::build_aef(){
    HND handle_f = assist.open_file(interp_conf.filename_interp, MODE_READ_FILE);
    lexer.set_symbols(assist.read_file(handle_f));
    assist.close_file(handle_f);
    
    build_external_func_table();
    
    global_scope.clear();

    clock_t beg;

    if(interp_conf.debug_output){
        debug_mark_time_func(lexer.analysis(), "lexer")
        parser.set_tokens(lexer.get_tokens());
        debug_mark_time_func(parser.analysis(), "parser")
        aef = parser.get_exprs();

        smt_analyzer.load_external_func_table(external_func_table);
        debug_mark_time_func(smt_analyzer.analysis(aef, global_scope), "semantic analyzer")
        //assist << tree_build_visually_str(aef);
    }
    else{
        parser.set_tokens(lexer.analysis());
        smt_analyzer.load_external_func_table(external_func_table);
        smt_analyzer.analysis(parser.analysis(), global_scope);
    }
}

void interpreter_exec::interpreter_run(){
    curr_expr = 0;
    assist.safe_call_dll_func_begin();
    for(; curr_expr < aef.size(); ++curr_expr){
        if(aef[curr_expr].expr_func.func_n.func_ref == sl_func::set)
            continue;
        wrap_callf_interpreter(
            aef[curr_expr].expr_func.func_n.func_ref(aef[curr_expr].sub_expr_s, global_scope);
        ,)
        assist.check_safe_call_dll_func();
    }
    assist.safe_call_dll_func_end();
}
void interpreter_exec::interpreter_next_expr(){
    if(curr_expr < aef.size()){
        wrap_callf_interpreter(
            aef[curr_expr].expr_func.func_n.func_ref(aef[curr_expr].sub_expr_s, global_scope);
        ,)
        ++curr_expr;
        return;
    }
}

void interpreter_exec::build_external_func_table(){
    import_module_decl(interp_conf.file_import_file_f);
    external_func_table = load_modules_all();
}