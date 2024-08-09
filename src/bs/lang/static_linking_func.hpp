#ifndef _STATIC_LINKING_FUNC__H
#define _STATIC_LINKING_FUNC__H

#include "parser.hpp"

namespace sl_func{

    extern const std::vector<std::string> name_static_func_sm;

    extern void set(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope);

    extern void project(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope);
    extern void executable(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope);

    extern void link_lib(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope);
    extern void exp_data(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope);
    extern void cmd(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope);
    extern void debug(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope);
    
    extern void flags_compiler(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope);
    extern void flags_linker(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope);
    extern void path_compiler(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope);
    extern void path_linker(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope);
    extern void standart_c(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope);
    extern void standart_cpp(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope);
    extern void lang(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope);

    extern void add_param_template(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope);
    extern void use_templates(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope);
    extern void use_it_template(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope);

}

#endif