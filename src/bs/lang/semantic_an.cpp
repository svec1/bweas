#include "../../mdef.hpp"

#include "semantic_an.hpp"

#include <algorithm>

using namespace parser;
using namespace aef_expr;
using namespace semantic_an;

semantic_analyzer::semantic_analyzer() {
    if (!init_glob) {
        init_glob = 1;
        assist.add_err("SMT000", "The function symbol was not found when creating the symbol table");
        assist.add_err("SMT001", "The function values passed do not correspond to the expected ones");
        assist.add_err("SMT002", "Incorrect definition of function parameter types was observed");
        assist.add_err("SMT003", "When going through the aef, at one of the stages of "
                                 "semantic analysis, an undefined error was noticed");
        assist.add_err("SMT004", "There is no variable with the expected type");
        assist.add_err("SMT005", "Arithmetic operations are prohibited at the external level");
        assist.add_err("SMT006", "It's not possible to compare non-numeric values");
        assist.add_err("SMT007", "It's not possible to use a structural variable in the set command");
        assist.add_err("SMT008", "It's impossible to determine the type of a non-existent variable");
        assist.add_err("SMT009", "Undefined error, it's impossible to determine the type of token");
        assist.add_err("SMT010", "A new symbol was expected");
        assist.add_err("SMT011", "Expected an existing symbol");
        assist.add_err("SMT012", "Another type of expression was expected");
        assist.add_err("SMT013", "The expected keyword was endif");
        assist.add_err("SMT014", "The endif keyword is only expected after the if or else keyword");
        assist.add_err("SMT015", "The expected keyword was if");
        assist.add_err("SMT016", "Keyword cannot be called");
        assist.add_err("SMT017", "The specified keyword operator is binary");
        assist.add_err("SMT018", "Undefined operands for the keyword operator");

        assist.add_err("SMT-RT001", "An existing character was expected when "
                                    "accessed during semantic analysis");
        assist.add_err("SMT-RT002", "The number of arguments has been exceeded");
        assist.add_err("SMT-RT003", "The type of the identifier does not match the types of parameters");
        assist.add_err("SMT-RT004", "Internal error of the declared function");
    }
}

void semantic_analyzer::add_func_flink(std::string name_token_func,
                                       void (*func_ref)(const std::vector<subexpressions> &, var::scope &curr_scope),
                                       std::vector<params> expected_args) {

    notion_func nfunc;
    nfunc.func_ref = func_ref;
    nfunc.expected_args = expected_args;

    defining_call_func(name_token_func, nfunc);

    std::pair<std::string, notion_func> n_link_func_spec = {name_token_func, nfunc};

    notion_all_func.insert(n_link_func_spec);
}

void semantic_analyzer::analysis(abstract_expr_func &expr_s, var::scope &global_scope) {
    smt_zero_pass(expr_s);
    smt_first_pass(expr_s);
    smt_second_pass(expr_s, global_scope);
}

void semantic_analyzer::load_external_func_table(const table_func &notion_external_func) {
    this->notion_external_func = notion_external_func;
}
void semantic_analyzer::append_external_name_func_w_smt(const std::vector<std::string> &list_name_func) {
    for (u32t i = 0; i < list_name_func.size(); ++i) {
        name_func_with_semantic_an.push_back(list_name_func[i]);
    }
}

void semantic_analyzer::smt_zero_pass(const abstract_expr_func &expr_s) {
    notion_func nfunc;
    for (u32t i = 0; i < expr_s.size(); ++i) {
        if (notion_all_func.find(expr_s[i].expr_func.func_t.token_val) != notion_all_func.end())
            continue;
        if (expr_s[i].expr_func.func_t.token_t == token_expr::token_type::KEYWORD) {
            if (expr_s[i].expr_func.func_t.token_val == STR_KEYWORD_IF)
                add_func_flink(expr_s[i].expr_func.func_t.token_val, NULL, {params::LNUM_OR_ID_VAR});
            else if (expr_s[i].expr_func.func_t.token_val == STR_KEYWORD_ELSE)
                add_func_flink(expr_s[i].expr_func.func_t.token_val, NULL, {});
            else if (expr_s[i].expr_func.func_t.token_val == STR_KEYWORD_ENDIF)
                add_func_flink(expr_s[i].expr_func.func_t.token_val, NULL, {});
        }
        else if (expr_s[i].expr_func.func_t.token_val == "set")
            add_func_flink(expr_s[i].expr_func.func_t.token_val, sl_func::set,
                           {params::NCHECK_VAR_ID, params::ANY_VALUE_WITHOUT_FUTUREID_NEXT, params::NEXT_TOO});
        else if (expr_s[i].expr_func.func_t.token_val == "project")
            add_func_flink(expr_s[i].expr_func.func_t.token_val, sl_func::project,
                           {params::FUTURE_VAR_ID, params::LNUM_OR_ID_VAR, params::LSTR_OR_ID_VAR, params::NEXT_TOO});
        else if (expr_s[i].expr_func.func_t.token_val == "executable")
            add_func_flink(expr_s[i].expr_func.func_t.token_val, sl_func::executable,
                           {params::FUTURE_VAR_ID, params::LNUM_OR_ID_VAR, params::VAR_STRUCT_ID});
        else if (expr_s[i].expr_func.func_t.token_val == "link_lib")
            add_func_flink(expr_s[i].expr_func.func_t.token_val, sl_func::link_lib,
                           {params::VAR_STRUCT_ID, params::LSTR_OR_ID_VAR, params::NEXT_TOO});
        else if (expr_s[i].expr_func.func_t.token_val == "exp_data")
            add_func_flink(expr_s[i].expr_func.func_t.token_val, sl_func::exp_data, {params::LSTR_OR_ID_VAR});
        else if (expr_s[i].expr_func.func_t.token_val == "cmd")
            add_func_flink(expr_s[i].expr_func.func_t.token_val, sl_func::cmd,
                           {params::LNUM_OR_ID_VAR, params::LSTR_OR_ID_VAR});
        else if (expr_s[i].expr_func.func_t.token_val == "debug")
            add_func_flink(expr_s[i].expr_func.func_t.token_val, sl_func::debug,
                           {params::LSTR_OR_ID_VAR, params::NEXT_TOO});
        else if (expr_s[i].expr_func.func_t.token_val == "debug_struct")
            add_func_flink(expr_s[i].expr_func.func_t.token_val, sl_func::debug_struct, {params::VAR_STRUCT_ID});
        else if (expr_s[i].expr_func.func_t.token_val == "flags_compiler")
            add_func_flink(expr_s[i].expr_func.func_t.token_val, sl_func::flags_compiler,
                           {params::VAR_STRUCT_ID, params::LNUM_OR_ID_VAR, params::LSTR_OR_ID_VAR, params::NEXT_TOO});
        else if (expr_s[i].expr_func.func_t.token_val == "flags_linker")
            add_func_flink(expr_s[i].expr_func.func_t.token_val, sl_func::flags_linker,
                           {params::VAR_STRUCT_ID, params::LNUM_OR_ID_VAR, params::LSTR_OR_ID_VAR, params::NEXT_TOO});
        else if (expr_s[i].expr_func.func_t.token_val == "path_compiler")
            add_func_flink(expr_s[i].expr_func.func_t.token_val, sl_func::path_compiler,
                           {params::VAR_STRUCT_ID, params::LSTR_OR_ID_VAR});
        else if (expr_s[i].expr_func.func_t.token_val == "path_linker")
            add_func_flink(expr_s[i].expr_func.func_t.token_val, sl_func::path_linker,
                           {params::VAR_STRUCT_ID, params::LSTR_OR_ID_VAR});
        else if (expr_s[i].expr_func.func_t.token_val == "standart_c")
            add_func_flink(expr_s[i].expr_func.func_t.token_val, sl_func::standart_c,
                           {params::VAR_STRUCT_ID, params::LNUM_OR_ID_VAR});
        else if (expr_s[i].expr_func.func_t.token_val == "standart_cpp")
            add_func_flink(expr_s[i].expr_func.func_t.token_val, sl_func::standart_cpp,
                           {params::VAR_STRUCT_ID, params::LNUM_OR_ID_VAR});
        else if (expr_s[i].expr_func.func_t.token_val == "lang")
            add_func_flink(expr_s[i].expr_func.func_t.token_val, sl_func::lang,
                           {params::VAR_STRUCT_ID, params::LNUM_OR_ID_VAR});
        else if (expr_s[i].expr_func.func_t.token_val == "add_param_template")
            add_func_flink(expr_s[i].expr_func.func_t.token_val, sl_func::add_param_template,
                           {params::FUTURE_VAR_ID, params::VAR_ID});
        else if (expr_s[i].expr_func.func_t.token_val == "create_templates")
            add_func_flink(expr_s[i].expr_func.func_t.token_val, sl_func::create_templates,
                           {params::FUTURE_VAR_ID, params::LSTR_OR_ID_VAR});
        else if (expr_s[i].expr_func.func_t.token_val == "create_call_component")
            add_func_flink(expr_s[i].expr_func.func_t.token_val, sl_func::create_call_component,
                           {params::FUTURE_VAR_ID, params::LSTR_OR_ID_VAR, params::LSTR_OR_ID_VAR});
        else if (expr_s[i].expr_func.func_t.token_val == "use_templates")
            add_func_flink(expr_s[i].expr_func.func_t.token_val, sl_func::use_templates,
                           {params::VAR_STRUCT_ID, params::LSTR_OR_ID_VAR, params::NEXT_TOO});
        else if (expr_s[i].expr_func.func_t.token_val == "use_it_template")
            add_func_flink(expr_s[i].expr_func.func_t.token_val, sl_func::use_it_template,
                           {params::VAR_STRUCT_ID, params::LNUM_OR_ID_VAR});
        else {
            if (notion_external_func.find(expr_s[i].expr_func.func_t.token_val) != notion_external_func.end()) {
                std::pair<std::string, notion_func> n_link_func_spec = {
                    expr_s[i].expr_func.func_t.token_val, notion_external_func[expr_s[i].expr_func.func_t.token_val]};
                defining_call_func(n_link_func_spec.first, n_link_func_spec.second);
                notion_all_func.insert(n_link_func_spec);
                continue;
            }
            throw semantic_excp(build_pos_tokenb_str(expr_s[i].expr_func.func_t), "000");
        }
    }
}

void semantic_analyzer::smt_first_pass(abstract_expr_func &expr_s) {
    for (u32t i = 0; i < expr_s.size(); ++i) {
        expr_s[i].expr_func.func_n = notion_all_func[expr_s[i].expr_func.func_t.token_val];
        if (expr_s[i].expr_func.func_n.expected_args.size() != expr_s[i].sub_expr_s.size()) {
            if (expr_s[i].expr_func.func_n.expected_args.size() != 0 &&
                expr_s[i].expr_func.func_n.expected_args[expr_s[i].expr_func.func_n.expected_args.size() - 1] ==
                    params::NEXT_TOO &&
                (expr_s[i].expr_func.func_n.expected_args.size() - 1 == expr_s[i].sub_expr_s.size() ||
                 expr_s[i].expr_func.func_n.expected_args.size() < expr_s[i].sub_expr_s.size()))
                goto check_valid_exp_type;
            throw semantic_excp(build_pos_tokenb_str(expr_s[i].expr_func.func_t), "001");
        }
        else if (expr_s[i].sub_expr_s.size() == 0)
            continue;

    check_valid_exp_type:
        for (u32t j = 0; j < expr_s[i].sub_expr_s.size(); ++j) {
            if ((expr_s[i].expr_func.func_n.expected_args[j] == params::FUTURE_VAR_ID ||
                 expr_s[i].expr_func.func_n.expected_args[j] == params::VAR_ID ||
                 expr_s[i].expr_func.func_n.expected_args[j] == params::NCHECK_VAR_ID ||
                 expr_s[i].expr_func.func_n.expected_args[j] == params::VAR_STRUCT_ID) &&
                expr_s[i].sub_expr_s[j].subexpr_t != subexpressions::type_subexpr::ID)
                throw semantic_excp(build_pos_subexpr_str(expr_s[i].sub_expr_s[j]) + " Expected: [VAR ID]\n", "001");
            else if (expr_s[i].expr_func.func_n.expected_args[j] == params::NEXT_TOO) {
                if (j == 0 || j != expr_s[i].expr_func.func_n.expected_args.size() - 1)
                    throw semantic_excp(build_pos_tokenb_str(expr_s[i].expr_func.func_t), "002");
                for (u32t b = j; b < expr_s[i].sub_expr_s.size(); ++b) {
                    if ((expr_s[i].expr_func.func_n.expected_args[j - 1] == params::FUTURE_VAR_ID ||
                         expr_s[i].expr_func.func_n.expected_args[j - 1] == params::VAR_ID ||
                         expr_s[i].expr_func.func_n.expected_args[j - 1] == params::VAR_STRUCT_ID) &&
                        expr_s[i].sub_expr_s[b].subexpr_t != subexpressions::type_subexpr::ID)
                        throw semantic_excp(build_pos_subexpr_str(expr_s[i].sub_expr_s[b]) + " Expected: [VAR ID]\n",
                                            "001");
                    else if (expr_s[i].expr_func.func_n.expected_args[j - 1] ==
                             params::ANY_VALUE_WITHOUT_FUTUREID_NEXT) {
                        if (expr_s[i].sub_expr_s[j - 1].token_of_subexpr[0].token_t ==
                                token_expr::token_type::LITERAL &&
                            expr_s[i].sub_expr_s[b].returned_type_subexpr() != subexpressions::ret_type_subexpr::INT &&
                            expr_s[i].sub_expr_s[b].subexpr_t != subexpressions::type_subexpr::ID)
                            throw semantic_excp(build_pos_subexpr_str(expr_s[i].sub_expr_s[b]) +
                                                    " Expected: [NUMBER] or VAR ID->[NUMBER]\n",
                                                "001");
                        else if (expr_s[i].sub_expr_s[j - 1].token_of_subexpr[0].token_t ==
                                     token_expr::token_type::LITERALS &&
                                 expr_s[i].sub_expr_s[b].returned_type_subexpr() !=
                                     subexpressions::ret_type_subexpr::STRING &&
                                 expr_s[i].sub_expr_s[b].subexpr_t != subexpressions::type_subexpr::ID)
                            throw semantic_excp(build_pos_subexpr_str(expr_s[i].sub_expr_s[b]) +
                                                    " Expected: [STRING] or [VAR ID->[STRING]]\n",
                                                "001");
                    }
                    else if (expr_s[i].expr_func.func_n.expected_args[j - 1] == params::LIT_STR &&
                             expr_s[i].sub_expr_s[b].returned_type_subexpr() !=
                                 subexpressions::ret_type_subexpr::STRING)
                        throw semantic_excp(build_pos_subexpr_str(expr_s[i].sub_expr_s[b]) + " Expected: [STRING]\n",
                                            "001");
                    else if (expr_s[i].expr_func.func_n.expected_args[j - 1] == params::LIT_NUM &&
                             expr_s[i].sub_expr_s[b].returned_type_subexpr() != subexpressions::ret_type_subexpr::INT)
                        throw semantic_excp(build_pos_subexpr_str(expr_s[i].sub_expr_s[b]) + " Expected: [NUMBER]\n",
                                            "001");
                    else if (expr_s[i].expr_func.func_n.expected_args[j - 1] == params::LSTR_OR_ID_VAR &&
                             expr_s[i].sub_expr_s[b].returned_type_subexpr() !=
                                 subexpressions::ret_type_subexpr::STRING &&
                             expr_s[i].sub_expr_s[b].subexpr_t != subexpressions::type_subexpr::ID)
                        throw semantic_excp(build_pos_subexpr_str(expr_s[i].sub_expr_s[b]) +
                                                " Expected: [STRING] OR [VAR ID]\n",
                                            "001");
                    else if (expr_s[i].expr_func.func_n.expected_args[j - 1] == params::LNUM_OR_ID_VAR &&
                             expr_s[i].sub_expr_s[b].returned_type_subexpr() != subexpressions::ret_type_subexpr::INT &&
                             expr_s[i].sub_expr_s[b].subexpr_t != subexpressions::type_subexpr::ID)
                        throw semantic_excp(build_pos_subexpr_str(expr_s[i].sub_expr_s[b]) +
                                                " Expected: [NUMBER] OR [VAR ID]\n",
                                            "001");
                }
                break;
            }
            else if (expr_s[i].expr_func.func_n.expected_args[j] == params::ANY_VALUE_WITHOUT_FUTUREID_NEXT &&
                     expr_s[i].sub_expr_s[j].returned_type_subexpr() != subexpressions::ret_type_subexpr::STRING &&
                     expr_s[i].sub_expr_s[j].returned_type_subexpr() != subexpressions::ret_type_subexpr::INT &&
                     expr_s[i].sub_expr_s[j].subexpr_t != subexpressions::type_subexpr::ID)
                throw semantic_excp(build_pos_subexpr_str(expr_s[i].sub_expr_s[j]) +
                                        " Expected: [STRING] OR [NUMBER] OR [VAR ID]\n",
                                    "001");
            else if (expr_s[i].expr_func.func_n.expected_args[j] == params::LIT_STR &&
                     expr_s[i].sub_expr_s[j].returned_type_subexpr() != subexpressions::ret_type_subexpr::STRING)
                throw semantic_excp(build_pos_subexpr_str(expr_s[i].sub_expr_s[j]) + " Expected: [STRING]\n", "001");
            else if (expr_s[i].expr_func.func_n.expected_args[j] == params::LIT_NUM &&
                     expr_s[i].sub_expr_s[j].returned_type_subexpr() != subexpressions::ret_type_subexpr::INT)
                throw semantic_excp(build_pos_subexpr_str(expr_s[i].sub_expr_s[j]) + " Expected: [NUMBER]\n", "001");
            else if (expr_s[i].expr_func.func_n.expected_args[j] == params::LSTR_OR_ID_VAR &&
                     expr_s[i].sub_expr_s[j].returned_type_subexpr() != subexpressions::ret_type_subexpr::STRING &&
                     expr_s[i].sub_expr_s[j].subexpr_t != subexpressions::type_subexpr::ID)
                throw semantic_excp(
                    build_pos_subexpr_str(expr_s[i].sub_expr_s[j]) + " Expected: [STRING] OR [VAR ID]\n", "001");
            else if (expr_s[i].expr_func.func_n.expected_args[j] == params::LNUM_OR_ID_VAR &&
                     expr_s[i].sub_expr_s[j].returned_type_subexpr() != subexpressions::ret_type_subexpr::INT &&
                     expr_s[i].sub_expr_s[j].subexpr_t != subexpressions::type_subexpr::ID)
                throw semantic_excp(
                    build_pos_subexpr_str(expr_s[i].sub_expr_s[j]) + " Expected: [NUMBER] OR [VAR ID]\n", "001");
        }
    }
}
void semantic_analyzer::smt_second_pass(abstract_expr_func &expr_s, var::scope &curr_scope) {

    // <0 - if_skip, 1 - else_skip; 0 - current if, 1- current else>
    std::vector<std::pair<bool, bool>> branch_s;

    u32t nested_if = 0;

    bool skip = 0;

    for (u32t i = 0; i < expr_s.size(); ++i) {
        params before_nextt_param = params::SIZE_ENUM_PARAMS;
        u32t index_before_nextt_param = 0;
        for (u32t j = 0; j < expr_s[i].sub_expr_s.size(); ++j) {
            if (expr_s[i].sub_expr_s.size() == 0 || expr_s[i].sub_expr_s[0].token_of_subexpr.size() == 0)
                continue;
            else if (before_nextt_param == params::SIZE_ENUM_PARAMS &&
                     expr_s[i].expr_func.func_n.expected_args[j] == params::NEXT_TOO) {
                before_nextt_param = expr_s[i].expr_func.func_n.expected_args[j - 1];
                if (before_nextt_param == params::ANY_VALUE_WITHOUT_FUTUREID_NEXT)
                    index_before_nextt_param = j - 1;
            }
            for (u32t b = 0; b < expr_s[i].sub_expr_s[j].token_of_subexpr.size(); ++b) {
                if (before_nextt_param != params::SIZE_ENUM_PARAMS) {
                    if (before_nextt_param == params::FUTURE_VAR_ID) {
                        u32t index_type = curr_scope.what_type(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val);
                        if (index_type != 0)
                            throw semantic_excp(build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) +
                                                    " Expected: NEW [VAR ID]\n",
                                                "010");
                    }
                    else if (before_nextt_param == params::NCHECK_VAR_ID)
                        continue;
                    else if (before_nextt_param == params::VAR_STRUCT_ID) {
                        u32t index_type = curr_scope.what_type(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val);
                        if (index_type != 5 && index_type != 6)
                            throw semantic_excp(build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) +
                                                    " Expected: [VAR STRUCT ID]\n",
                                                "011");
                    }
                    else if (before_nextt_param == params::ANY_VALUE_WITHOUT_FUTUREID_NEXT) {
                        if (expr_s[i].sub_expr_s[index_before_nextt_param].returned_type_subexpr() ==
                                subexpressions::ret_type_subexpr::INT &&
                            expr_s[i].sub_expr_s[j].returned_type_subexpr() != subexpressions::ret_type_subexpr::INT) {
                            if (expr_s[i].sub_expr_s[j].returned_type_subexpr() ==
                                subexpressions::ret_type_subexpr::ID) {
                                u32t index_type =
                                    curr_scope.what_type(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val);
                                if (index_type == 1 || index_type == 3)
                                    continue;
                            }
                            throw semantic_excp(build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) +
                                                    " Expected: Expression with return type of INT\n",
                                                "012");
                        }
                        else if (expr_s[i].sub_expr_s[index_before_nextt_param].returned_type_subexpr() ==
                                     subexpressions::ret_type_subexpr::STRING &&
                                 expr_s[i].sub_expr_s[j].returned_type_subexpr() !=
                                     subexpressions::ret_type_subexpr::STRING) {
                            if (expr_s[i].sub_expr_s[j].returned_type_subexpr() ==
                                subexpressions::ret_type_subexpr::ID) {
                                u32t index_type =
                                    curr_scope.what_type(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val);
                                if (index_type == 2 || index_type == 4)
                                    continue;
                            }
                            throw semantic_excp(build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) +
                                                    " Expected: Expression with return type of STRING\n",
                                                "012");
                        }
                        else if (expr_s[i].sub_expr_s[index_before_nextt_param].returned_type_subexpr() ==
                                 subexpressions::ret_type_subexpr::ID) {
                            u32t index_type = curr_scope.what_type(
                                expr_s[i].sub_expr_s[index_before_nextt_param].token_of_subexpr[0].token_val);
                            if ((index_type == 1 || index_type == 3) &&
                                expr_s[i].sub_expr_s[j].returned_type_subexpr() !=
                                    subexpressions::ret_type_subexpr::INT) {
                                if (expr_s[i].sub_expr_s[j].returned_type_subexpr() ==
                                    subexpressions::ret_type_subexpr::ID) {
                                    u32t index_type =
                                        curr_scope.what_type(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val);
                                    if (index_type == 1 || index_type == 3)
                                        continue;
                                }
                                throw semantic_excp(build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) +
                                                        " Expected: Expression with return type of INT\n",
                                                    "012");
                            }
                            else if ((index_type == 2 || index_type == 4) &&
                                     expr_s[i].sub_expr_s[j].returned_type_subexpr() !=
                                         subexpressions::ret_type_subexpr::STRING) {
                                if (expr_s[i].sub_expr_s[j].returned_type_subexpr() ==
                                    subexpressions::ret_type_subexpr::ID) {
                                    u32t index_type =
                                        curr_scope.what_type(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val);
                                    if (index_type == 2 || index_type == 4)
                                        continue;
                                }
                                throw semantic_excp(build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) +
                                                        " Expected: Expression with return type of STRING\n",
                                                    "012");
                            }
                        }
                    }
                    else if (expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_t == token_expr::token_type::ID) {
                        u32t index_type = curr_scope.what_type(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val);
                        if (before_nextt_param == params::VAR_ID && index_type == 0)
                            throw semantic_excp(build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) +
                                                    " Expected: exist variable with any type\n",
                                                "008");
                        else if (index_type == 3 || index_type == 4)
                            continue;
                        else if (before_nextt_param == params::LNUM_OR_ID_VAR && index_type != 1 &&
                                 expr_s[i].sub_expr_s[j].returned_type_subexpr() !=
                                     subexpressions::ret_type_subexpr::INT)
                            throw semantic_excp(build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) +
                                                    " Expected: [VAR ID->[NUMBER]]\n",
                                                "004");
                        else if (before_nextt_param == params::LSTR_OR_ID_VAR && index_type != 2 &&
                                 expr_s[i].sub_expr_s[j].returned_type_subexpr() !=
                                     subexpressions::ret_type_subexpr::STRING)
                            throw semantic_excp(build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) +
                                                    " Expected: [VAR ID->[STRING]]\n",
                                                "004");
                    }
                }
                else if (expr_s[i].expr_func.func_n.expected_args[j] == params::FUTURE_VAR_ID) {
                    u32t index_type = curr_scope.what_type(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val);
                    if (index_type != 0)
                        throw semantic_excp(build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) +
                                                " Expected: NEW [VAR ID]\n",
                                            "010");
                }
                else if (expr_s[i].expr_func.func_n.expected_args[j] == params::NCHECK_VAR_ID) {
                    continue;
                }
                else if (expr_s[i].expr_func.func_n.expected_args[j] == params::VAR_STRUCT_ID) {
                    u32t index_type = curr_scope.what_type(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val);
                    if (index_type != 5 && index_type != 6)
                        throw semantic_excp(build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) +
                                                " Expected: [VAR STRUCT ID]\n",
                                            "011");
                }
                else if (expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_t == token_expr::token_type::ID) {
                    u32t index_type = curr_scope.what_type(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val);
                    if (index_type == 0)
                        throw semantic_excp(build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) +
                                                " Expected: exist variable with any type\n",
                                            "008");
                    else if ((index_type == 3 || index_type == 4) &&
                             j < expr_s[i].expr_func.func_n.expected_args.size() - 1) {
                        if (expr_s[i].expr_func.func_n.expected_args[j + 1] != params::NEXT_TOO)
                            if (expr_s[i].expr_func.func_n.expected_args[j] == params::LNUM_OR_ID_VAR)
                                throw semantic_excp(build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) +
                                                        " Expected: [VAR ID->[NUMBER]]\n",
                                                    "004");
                            else
                                throw semantic_excp(build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) +
                                                        " Expected: [VAR ID->[STRING]]\n",
                                                    "004");
                    }
                    else if (expr_s[i].expr_func.func_n.expected_args[j] == params::LNUM_OR_ID_VAR && index_type != 1 &&
                             expr_s[i].sub_expr_s[j].returned_type_subexpr() != subexpressions::ret_type_subexpr::INT)
                        throw semantic_excp(build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) +
                                                " Expected: [VAR ID->[NUMBER]]\n",
                                            "004");
                    else if (expr_s[i].expr_func.func_n.expected_args[j] == params::LSTR_OR_ID_VAR && index_type != 2 &&
                             expr_s[i].sub_expr_s[j].returned_type_subexpr() !=
                                 subexpressions::ret_type_subexpr::STRING)
                        throw semantic_excp(build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) +
                                                " Expected: [VAR ID->[STRING]]\n",
                                            "004");
                    else if (index_type == 5 || index_type == 6)
                        throw semantic_excp(build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) +
                                                " Expected: [VAR ID->([NUMBER] or [STRING])]\n",
                                            "004");
                }
            }
        }
        for (u32t j = 0, b = 0; j < expr_s[i].sub_expr_s.size(); ++j, ++b) {
            if (b >= expr_s[i].expr_func.func_n.expected_args.size() &&
                before_nextt_param != params::SIZE_ENUM_PARAMS) {
                parse_subexpr_param(
                    expr_s[i].sub_expr_s[j], expr_s[i].sub_expr_s, j, curr_scope,
                    expr_s[i].expr_func.func_n.expected_args[expr_s[i].expr_func.func_n.expected_args.size() - 1]);
                continue;
            }
            parse_subexpr_param(expr_s[i].sub_expr_s[j], expr_s[i].sub_expr_s, j, curr_scope,
                                expr_s[i].expr_func.func_n.expected_args[b]);
        }

        // skips branches
        if (skip) {
            if (expr_s[i].expr_func.func_t.token_val == STR_KEYWORD_IF)
                ++nested_if;

            else if (expr_s[i].expr_func.func_t.token_val == STR_KEYWORD_ELSE)
                ++nested_if;

            else if (expr_s[i].expr_func.func_t.token_val == STR_KEYWORD_ENDIF) {
                if (!nested_if)
                    goto endif_end;

                --nested_if;
            }
            expr_s.erase(expr_s.begin() + i);
            --i;
        }
        // keyword handler(if, else, endif)
        else if (expr_s[i].expr_func.func_t.token_t == token_expr::token_type::KEYWORD) {
            if (expr_s[i].expr_func.func_t.token_val == STR_KEYWORD_IF) {
                u32t val = std::stoi(expr_s[i].sub_expr_s[0].token_of_subexpr[0].token_val);
                if (!val) {
                    skip = 1;
                    branch_s.push_back(std::pair<bool, bool>(0, 0));
                }
                else
                    branch_s.push_back(std::pair<bool, bool>(1, 0));
            }
            else if (expr_s[i].expr_func.func_t.token_val == STR_KEYWORD_ELSE) {
                if (!branch_s.size() || branch_s[branch_s.size() - 1].second)
                    throw semantic_excp(build_pos_tokenb_str(expr_s[i].expr_func.func_t), "015");

                if (branch_s[branch_s.size() - 1].first)
                    skip = 1;
            }
            else if (expr_s[i].expr_func.func_t.token_val == STR_KEYWORD_ENDIF) {
            endif_end:
                if (!branch_s.size())
                    throw semantic_excp(build_pos_tokenb_str(expr_s[i].expr_func.func_t), "014");

                skip = 0;
                if (branch_s[branch_s.size() - 1].second ||
                    (i < expr_s.size() - 1 && expr_s[i + 1].expr_func.func_t.token_val != STR_KEYWORD_ELSE))
                    branch_s.pop_back();
            }
            else
                throw semantic_excp(build_pos_tokenb_str(expr_s[i].expr_func.func_t), "016");
            expr_s.erase(expr_s.begin() + i);
            --i;
        }
        else if (expr_s[i].execute_with_semantic_an()) {
            wrap_callf_declaration(expr_s[i].expr_func.func_n.func_ref(expr_s[i].sub_expr_s, curr_scope))
#if defined(WIN)
                assist.check_safe_call_dll_func();
#elif defined(UNIX)
                assist.check_safe_call_dl_func();
#endif
        }
    }

    if (branch_s.size())
        throw semantic_excp("End Of AEF", "013");
}

void semantic_analyzer::convert_id_to_literal(aef_expr::subexpressions &sub_expr, var::scope &curr_scope,
                                              aef_expr::params expected_param) {
    for (u32t i = 0; i < sub_expr.token_of_subexpr.size(); ++i) {
        if (sub_expr.token_of_subexpr[i].token_t == token_expr::token_type::ID &&
            (expected_param != params::VAR_ID && expected_param != params::FUTURE_VAR_ID &&
             expected_param != params::NCHECK_VAR_ID)) {
            u32t index_var = curr_scope.what_type(sub_expr.token_of_subexpr[i].token_val);
            if (index_var == 1) {
                sub_expr.token_of_subexpr[i] = token_expr::token(
                    token_expr::token_type::LITERAL,
                    std::to_string(curr_scope.get_var_value<int>(sub_expr.token_of_subexpr[i].token_val.c_str())),
                    sub_expr.token_of_subexpr[i].pos_defined_line, sub_expr.token_of_subexpr[i].pos_beg_defined_sym);
            }
            else if (index_var == 2) {
                sub_expr.token_of_subexpr[i] = token_expr::token(
                    token_expr::token_type::LITERALS,
                    curr_scope.get_var_value<std::string>(sub_expr.token_of_subexpr[i].token_val),
                    sub_expr.token_of_subexpr[i].pos_defined_line, sub_expr.token_of_subexpr[i].pos_beg_defined_sym);
            }
        }
        else if (sub_expr.token_of_subexpr[i].token_t == token_expr::token_type::KW_OPERATOR &&
                 IS_CONSTANT_KW_OP(sub_expr.token_of_subexpr[i].token_val))
            parse_keywords_op_param(sub_expr, i, curr_scope, expected_param, 1);
    }
}

void semantic_analyzer::convert_kwop_uc_to_literal(aef_expr::subexpressions &sub_expr, var::scope &curr_scope,
                                                   aef_expr::params expected_param) {
    for (u32t i = 0; i < sub_expr.token_of_subexpr.size(); ++i) {
        if (sub_expr.token_of_subexpr[i].token_t == token_expr::token_type::KW_OPERATOR &&
            IS_UNARY_KW_OP(sub_expr.token_of_subexpr[i].token_val))
            parse_keywords_op_param(sub_expr, i, curr_scope, expected_param, 1);
    }
}

void semantic_analyzer::parse_subexpr_param(subexpressions &sub_expr, std::vector<subexpressions> &sub_exprs,
                                            u32t &pos_sub_expr_in_vec, var::scope &curr_scope, params expected_param) {
    subexpressions parse_subexpr, tmp_parse_subexpr;
    if (sub_expr.subexpr_t == subexpressions::type_subexpr::INT_COMPARE) {
        for (u32t i = 0; i < sub_expr.token_of_subexpr.size(); ++i) {
            if (sub_expr.token_of_subexpr[i].token_t == token_expr::token_type::ID) {
                parse_subexpr.token_of_subexpr.push_back(token_expr::token(
                    token_expr::token_type::LITERAL,
                    std::to_string(curr_scope.get_var_value<int>(sub_expr.token_of_subexpr[i].token_val.c_str())),
                    sub_expr.token_of_subexpr[i].pos_defined_line, sub_expr.token_of_subexpr[i].pos_beg_defined_sym));
                if (parse_subexpr.token_of_subexpr.size() == 3)
                    goto parse_integer;
                continue;
            }
            parse_subexpr.token_of_subexpr.push_back(sub_expr.token_of_subexpr[i]);
            if (parse_subexpr.token_of_subexpr.size() == 3) {
            parse_integer:
                bool condition = 0;
                switch (parse_subexpr.token_of_subexpr[1].token_val[0]) {
                case '>':
                    if (std::stoi(parse_subexpr.token_of_subexpr[0].token_val.c_str()) >
                        std::stoi(parse_subexpr.token_of_subexpr[2].token_val.c_str()))
                        condition = 1;
                    break;
                case '<':
                    if (std::stoi(parse_subexpr.token_of_subexpr[0].token_val.c_str()) <
                        std::stoi(parse_subexpr.token_of_subexpr[2].token_val.c_str()))
                        condition = 1;
                    break;
                case '=':
                    if (std::stoi(parse_subexpr.token_of_subexpr[0].token_val.c_str()) ==
                        std::stoi(parse_subexpr.token_of_subexpr[2].token_val.c_str()))
                        condition = 1;
                    break;
                }
                tmp_parse_subexpr = parse_subexpr;
                parse_subexpr.token_of_subexpr.clear();
                if (condition)
                    parse_subexpr.token_of_subexpr.push_back(token_expr::token(
                        token_expr::token_type::LITERAL, "1", tmp_parse_subexpr.token_of_subexpr[0].pos_defined_line,
                        tmp_parse_subexpr.token_of_subexpr[0].pos_beg_defined_sym));
                else
                    parse_subexpr.token_of_subexpr.push_back(token_expr::token(
                        token_expr::token_type::LITERAL, "0", tmp_parse_subexpr.token_of_subexpr[0].pos_defined_line,
                        tmp_parse_subexpr.token_of_subexpr[0].pos_beg_defined_sym));
            }
        }
    }
    else if (sub_expr.subexpr_t == subexpressions::type_subexpr::STRING_ADD) {
        for (u32t i = 0; i < sub_expr.token_of_subexpr.size(); ++i) {
            if (sub_expr.token_of_subexpr[i].token_t == token_expr::token_type::ID) {
                parse_subexpr.token_of_subexpr.push_back(token_expr::token(
                    token_expr::token_type::LITERALS,
                    curr_scope.get_var_value<std::string>(sub_expr.token_of_subexpr[i].token_val),
                    sub_expr.token_of_subexpr[i].pos_defined_line, sub_expr.token_of_subexpr[i].pos_beg_defined_sym));
                if (parse_subexpr.token_of_subexpr.size() == 3)
                    goto parse_string;
                continue;
            }
            parse_subexpr.token_of_subexpr.push_back(sub_expr.token_of_subexpr[i]);
            if (parse_subexpr.token_of_subexpr.size() == 3) {
            parse_string:
                tmp_parse_subexpr = parse_subexpr;
                parse_subexpr.token_of_subexpr.clear();
                parse_subexpr.token_of_subexpr.push_back(token_expr::token(
                    token_expr::token_type::LITERALS,
                    tmp_parse_subexpr.token_of_subexpr[0].token_val + tmp_parse_subexpr.token_of_subexpr[2].token_val,
                    tmp_parse_subexpr.token_of_subexpr[0].pos_defined_line,
                    tmp_parse_subexpr.token_of_subexpr[0].pos_beg_defined_sym));
            }
        }
    }
    else if (sub_expr.subexpr_t == subexpressions::type_subexpr::ID &&
             (expected_param != params::VAR_ID && expected_param != params::FUTURE_VAR_ID &&
              expected_param != params::NCHECK_VAR_ID)) {
        u32t index_var = curr_scope.what_type(sub_expr.token_of_subexpr[0].token_val);
        if (index_var == 1) {
            parse_subexpr.subexpr_t = subexpressions::type_subexpr::INT;
            parse_subexpr.token_of_subexpr.push_back(token_expr::token(
                token_expr::token_type::LITERAL,
                std::to_string(curr_scope.get_var_value<int>(sub_expr.token_of_subexpr[0].token_val.c_str())),
                sub_expr.token_of_subexpr[0].pos_defined_line, sub_expr.token_of_subexpr[0].pos_beg_defined_sym));
        }
        else if (index_var == 2) {
            parse_subexpr.subexpr_t = subexpressions::type_subexpr::STRING;
            parse_subexpr.token_of_subexpr.push_back(token_expr::token(
                token_expr::token_type::LITERALS,
                curr_scope.get_var_value<std::string>(sub_expr.token_of_subexpr[0].token_val),
                sub_expr.token_of_subexpr[0].pos_defined_line, sub_expr.token_of_subexpr[0].pos_beg_defined_sym));
        }
        else if (index_var == 3) {
            std::vector<subexpressions> new_sub_exprs;
            subexpressions tmp_sub_expr;
            tmp_sub_expr.subexpr_t = subexpressions::type_subexpr::INT;
            const std::vector<int> vec_int_id =
                curr_scope.get_var_value<std::vector<int>>(sub_expr.token_of_subexpr[0].token_val);
            for (u32t i = 0; i < pos_sub_expr_in_vec; ++i)
                new_sub_exprs.push_back(sub_exprs[i]);
            for (u32t i = 0; i < vec_int_id.size(); ++i) {
                tmp_sub_expr.token_of_subexpr.push_back(token_expr::token(
                    token_expr::token_type::LITERAL, std::to_string(vec_int_id[i]),
                    sub_expr.token_of_subexpr[0].pos_defined_line, sub_expr.token_of_subexpr[0].pos_beg_defined_sym));
                new_sub_exprs.push_back(tmp_sub_expr);
                tmp_sub_expr.token_of_subexpr.clear();
            }
            for (u32t i = pos_sub_expr_in_vec + 1; i < sub_exprs.size(); ++i)
                new_sub_exprs.push_back(sub_exprs[i]);
            pos_sub_expr_in_vec += vec_int_id.size() - 1;
            sub_exprs = new_sub_exprs;
        }
        else if (index_var == 4) {
            std::vector<subexpressions> new_sub_exprs;
            subexpressions tmp_sub_expr;
            tmp_sub_expr.subexpr_t = subexpressions::type_subexpr::STRING;
            const std::vector<std::string> vec_str_id =
                curr_scope.get_var_value<std::vector<std::string>>(sub_expr.token_of_subexpr[0].token_val);
            for (u32t i = 0; i < pos_sub_expr_in_vec; ++i)
                new_sub_exprs.push_back(sub_exprs[i]);
            for (u32t i = 0; i < vec_str_id.size(); ++i) {
                tmp_sub_expr.token_of_subexpr.push_back(token_expr::token(
                    token_expr::token_type::LITERALS, vec_str_id[i], sub_expr.token_of_subexpr[0].pos_defined_line,
                    sub_expr.token_of_subexpr[0].pos_beg_defined_sym));
                new_sub_exprs.push_back(tmp_sub_expr);
                tmp_sub_expr.token_of_subexpr.clear();
            }
            for (u32t i = pos_sub_expr_in_vec + 1; i < sub_exprs.size(); ++i)
                new_sub_exprs.push_back(sub_exprs[i]);
            pos_sub_expr_in_vec += vec_str_id.size() - 1;
            sub_exprs = new_sub_exprs;
        }
        if (parse_subexpr.token_of_subexpr.size() > 0)
            sub_expr = parse_subexpr;
        return;
    }
    else if (sub_expr.subexpr_t == subexpressions::type_subexpr::KEYWORD_OP) {
        convert_id_to_literal(sub_expr, curr_scope, expected_param);
        convert_kwop_uc_to_literal(sub_expr, curr_scope, expected_param);
        parse_keywords_op_param(sub_expr, UINT32_MAX, curr_scope, expected_param);
        if (sub_expr.token_of_subexpr[0].token_t == token_expr::token_type::LITERAL)
            sub_expr.subexpr_t = subexpressions::type_subexpr::INT;
        else if (sub_expr.token_of_subexpr[0].token_t == token_expr::token_type::LITERALS)
            sub_expr.subexpr_t = subexpressions::type_subexpr::STRING;
        return;
    }
    else
        return;

    if (parse_subexpr.token_of_subexpr[0].token_t == token_expr::token_type::LITERAL)
        parse_subexpr.subexpr_t = subexpressions::type_subexpr::INT;
    else
        parse_subexpr.subexpr_t = subexpressions::type_subexpr::STRING;
    sub_expr = parse_subexpr;
}
void semantic_analyzer::parse_keywords_op_param(aef_expr::subexpressions &sub_expr, u32t pos_token_kw_in_subexpr,
                                                var::scope &curr_scope, aef_expr::params expected_param,
                                                u32t parse_okeyword) {
    if (pos_token_kw_in_subexpr == UINT32_MAX) {
        for (u32t i = 0; i < sub_expr.token_of_subexpr.size(); ++i)
            if (sub_expr.token_of_subexpr[i].token_t == token_expr::token_type::KW_OPERATOR) {
                pos_token_kw_in_subexpr = i;
                break;
            }
        if (pos_token_kw_in_subexpr == UINT32_MAX)
            return;
    }

    if (sub_expr.token_of_subexpr[pos_token_kw_in_subexpr].token_val == STR_KEYWORD_OP_EQUAL) {
        if (expected_param != params::LNUM_OR_ID_VAR && expected_param != params::ANY_VALUE_WITHOUT_FUTUREID_NEXT ||
            pos_token_kw_in_subexpr == 0 || pos_token_kw_in_subexpr == sub_expr.token_of_subexpr.size() - 1)
            throw semantic_excp(build_pos_tokenb_str(sub_expr.token_of_subexpr[pos_token_kw_in_subexpr]) +
                                    " Two operands are expected",
                                "017");

        if (sub_expr.token_of_subexpr[pos_token_kw_in_subexpr - 1].token_t == token_expr::token_type::LITERAL) {
            if (sub_expr.token_of_subexpr[pos_token_kw_in_subexpr + 1].token_t != token_expr::token_type::LITERAL)
                throw semantic_excp(
                    build_pos_tokenb_str(sub_expr.token_of_subexpr[pos_token_kw_in_subexpr]) +
                        " Expected literal(int): " + sub_expr.token_of_subexpr[pos_token_kw_in_subexpr + 1].token_val,
                    "018");
        }
        else if (sub_expr.token_of_subexpr[pos_token_kw_in_subexpr - 1].token_t == token_expr::token_type::LITERALS) {
            if (sub_expr.token_of_subexpr[pos_token_kw_in_subexpr + 1].token_t != token_expr::token_type::LITERALS)
                throw semantic_excp(build_pos_tokenb_str(sub_expr.token_of_subexpr[pos_token_kw_in_subexpr]) +
                                        " Expected literal(string): " +
                                        sub_expr.token_of_subexpr[pos_token_kw_in_subexpr - 1].token_val,
                                    "018");
        }
        else
            throw semantic_excp(build_pos_tokenb_str(sub_expr.token_of_subexpr[pos_token_kw_in_subexpr]) +
                                    " Inappropriate parameter for keyword operator: " +
                                    sub_expr.token_of_subexpr[pos_token_kw_in_subexpr + 1].token_val,
                                "018");

        if (sub_expr.token_of_subexpr[pos_token_kw_in_subexpr - 1].token_val ==
            sub_expr.token_of_subexpr[pos_token_kw_in_subexpr + 1].token_val) {
            sub_expr.token_of_subexpr[pos_token_kw_in_subexpr - 1].token_t = token_expr::token_type::LITERAL;
            sub_expr.token_of_subexpr[pos_token_kw_in_subexpr - 1].token_val = "1";
        }
        else {
            sub_expr.token_of_subexpr[pos_token_kw_in_subexpr - 1].token_t = token_expr::token_type::LITERAL;
            sub_expr.token_of_subexpr[pos_token_kw_in_subexpr - 1].token_val = "0";
        }
        sub_expr.token_of_subexpr.erase(sub_expr.token_of_subexpr.begin() + pos_token_kw_in_subexpr + 1);
        sub_expr.token_of_subexpr.erase(sub_expr.token_of_subexpr.begin() + pos_token_kw_in_subexpr);
    }
    else if (sub_expr.token_of_subexpr[pos_token_kw_in_subexpr].token_val == STR_KEYWORD_OP_TSTR) {
        if (pos_token_kw_in_subexpr == sub_expr.token_of_subexpr.size() - 1)
            throw semantic_excp(build_pos_tokenb_str(sub_expr.token_of_subexpr[pos_token_kw_in_subexpr]) +
                                    " One operands are expected",
                                "017");

        if (sub_expr.token_of_subexpr[pos_token_kw_in_subexpr + 1].token_t == token_expr::token_type::LITERAL) {
            sub_expr.token_of_subexpr[pos_token_kw_in_subexpr].token_t = token_expr::token_type::LITERALS;
            sub_expr.token_of_subexpr[pos_token_kw_in_subexpr].token_val =
                sub_expr.token_of_subexpr[pos_token_kw_in_subexpr + 1].token_val;
        }
        else
            throw semantic_excp(build_pos_tokenb_str(sub_expr.token_of_subexpr[pos_token_kw_in_subexpr]) +
                                    " Inappropriate parameter for keyword operator: " +
                                    sub_expr.token_of_subexpr[pos_token_kw_in_subexpr + 1].token_val,
                                "018");
        sub_expr.token_of_subexpr.erase(sub_expr.token_of_subexpr.begin() + pos_token_kw_in_subexpr + 1);
    }
    else if (IS_CONSTANT_KW_OP(sub_expr.token_of_subexpr[pos_token_kw_in_subexpr].token_val)) {
        if (sub_expr.token_of_subexpr[pos_token_kw_in_subexpr].token_val == STR_KEYWORD_OP_CONST_TRUE ||
            sub_expr.token_of_subexpr[pos_token_kw_in_subexpr].token_val == STR_KEYWORD_OP_CONST_RELEASE)
            sub_expr.token_of_subexpr[pos_token_kw_in_subexpr].token_val = "1";
        else if (sub_expr.token_of_subexpr[pos_token_kw_in_subexpr].token_val == STR_KEYWORD_OP_CONST_FALSE ||
                 sub_expr.token_of_subexpr[pos_token_kw_in_subexpr].token_val == STR_KEYWORD_OP_CONST_DEBUG)
            sub_expr.token_of_subexpr[pos_token_kw_in_subexpr].token_val = "0";
        sub_expr.token_of_subexpr[pos_token_kw_in_subexpr].token_t = token_expr::token_type::LITERAL;
    }

    if (sub_expr.token_of_subexpr.size() != 1 && !parse_okeyword)
        parse_keywords_op_param(sub_expr, UINT32_MAX, curr_scope, expected_param);
}

void semantic_analyzer::defining_call_func(const std::string &name, notion_func &nfunc) {
    if (std::find(nfunc.expected_args.begin(), nfunc.expected_args.end(), params::FUTURE_VAR_ID) !=
            nfunc.expected_args.end() ||
        std::find(nfunc.expected_args.begin(), nfunc.expected_args.end(), params::NCHECK_VAR_ID) !=
            nfunc.expected_args.end())
        nfunc.is_declaration_var = 1;
    else if (std::find(name_func_with_semantic_an.begin(), name_func_with_semantic_an.end(), name) !=
             name_func_with_semantic_an.end())
        nfunc.only_with_semantic = 1;
}