//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses />
// ------------------------------------------
//

#include "interpreter.hpp"

static logger _log{"INTERPRETER"};

extern FILE *yyin;
extern int yyparse(void);

extern bweas::logger *log_bison;

extern scope *current_scope;
extern statements stm_s;

interpreter::interpreter(string_v name_file) : global_scope{_log}, smt_analyzer{_log} {
    yyin          = fopen(name_file.data(), "r");
    log_bison     = &_log;
    current_scope = &global_scope;

    global_scope.create_var<decl_func>(STR_KEYWORD_IF, decl_func(STR_KEYWORD_IF, NULL, {param_type::LIT_NUM}));
    global_scope.create_var<decl_func>(STR_KEYWORD_ELSE, decl_func(STR_KEYWORD_ELSE, NULL, {}));
    global_scope.create_var<decl_func>(STR_KEYWORD_ENDIF, decl_func(STR_KEYWORD_ENDIF, NULL, {}));

    if (yyin == NULL)
        (_log << bwtools::fatal) << (log_message(log_type::fatal) << "Not found file: " << name_file);
}

void interpreter::interpret() {
    yyparse();
    smt_analyzer.analysis(stm_s, *current_scope);
}

void interpreter::set_scope(scope *external_scope) {
    current_scope = &(*external_scope);
}

scope &interpreter::get_scope() {
    return *current_scope;
}

const scope &interpreter::get_scope() const {
    return *current_scope;
}

void interpreter::create_function(const decl_func &func) {
    current_scope->create_var<decl_func>(func.name_func, func);
}
void interpreter::create_function(string_v name_func, decl_func::func_t func, vec<param> expected_params) {
    current_scope->create_var<decl_func>(name_func.data(), decl_func{name_func, func, expected_params});
}
