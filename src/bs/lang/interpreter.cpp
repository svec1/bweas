//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses />
// ------------------------------------------
//

#include "interpreter.hpp"

static logger _log{"INTERPRETER[BWLANG]"};

extern FILE *yyin;
extern int yyparse(void);

extern var::scope *current_scope;
extern bweas::logger *log_bison;
extern statements stm_s;

interpreter::interpreter(string_v name_file) : global_scope{_log}, smt_analyzer{_log} {
    yyin          = fopen(name_file.data(), "r");
    log_bison     = &_log;
    current_scope = &global_scope;

    if (yyin == NULL)
        (_log << bwtools::fatal) << (log_message(log_type::fatal) << "Not found file: " << name_file);
}

void interpreter::set_external_scope(var::scope *_external_scope) {
    external_scope = _external_scope;
    current_scope  = &(*external_scope);
}

void interpreter::interpret() {
    yyparse();
    smt_analyzer.analysis(stm_s, *current_scope);
}

vec<var::struct_sb::target> interpreter::export_targets() {
    const vec<std::pair<string, var::struct_sb::target>> &vec_targets_ref =
        global_scope.get_vector_variables_t<var::struct_sb::target>();

    vec<var::struct_sb::target> targets;
    for (size_t i = 0; i < vec_targets_ref.size(); ++i)
        targets.push_back(vec_targets_ref[i].second);

    return targets;
}

var::scope &interpreter::get_current_scope() {
    if (external_scope != &global_scope)
        return *external_scope;
    return *current_scope;
}

void interpreter::create_function(const decl_func &func) {
    current_scope->create_var<decl_func>(func.name_func, func);
}
void interpreter::create_function(string_v name_func, decl_func::func_t func, vec<param> expected_params) {
    current_scope->create_var<decl_func>(name_func.data(), decl_func{name_func, func, expected_params});
}
