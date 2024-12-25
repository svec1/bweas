#include "../../mdef.hpp"

#include "parser.hpp"

using namespace aef_expr;
using namespace token_expr;
using namespace parser::utility;

#define is_param(token)                                                                                                \
    (token.token_t == token_type::ID || token.token_t == token_type::LITERAL ||                                        \
     token.token_t == token_type::LITERALS || token.token_t == token_type::KW_OPERATOR)

using namespace parser;

pars_an::pars_an() {
    if (!init_glob) {
        init_glob = 1;
        assist.add_err("PARS000", "The beginning of the expression must be a token of type \"ID\" or \"KEYWORD\"");
        assist.add_err("PARS001", "After the start of the expression, a token is expected to begin enumerating "
                                  "arguments with type \"SYMBOL\" and value \"(\"");
        assist.add_err("PARS002", "Function arguments can only be tokens of type (\"ID\", \"LITERAL\" or \"LITERALS\" "
                                  "with the possibility of switching on \"OPERATOR\") <- subexpression");
        assist.add_err("PARS003", "A token of type \"OPERATOR\" is expected");
        assist.add_err("PARS004", "A token of type \"ID\", \"LITERAL\" or \"LITERALS\" is expected");
        assist.add_err("PARS005", "A token with the type \"ID\", \"LITERAL\" or \"LITERALS\" is expected after a token "
                                  "with the type \"OPERATOR\" or \"KW_OPERATOR\"");
        assist.add_err("PARS006", "It is impossible to add two different tokens by type");
        assist.add_err("PARS007", "It is impossible to compare two different tokens by type");
        assist.add_err("PARS008", "It is impossible to add strings when comparing");
        assist.add_err("PARS009", "Arithmetic operations are prohibited");
        assist.add_err("PARS010", "Impossible to compare strings");
        assist.add_err("PARS011", "You cannot use operators together with operator keywords");
        assist.add_err("PARS012", "Keyword operator is nonbinary");
    }
}

pars_an::pars_an(const std::vector<token> &tk_s) {
    pars_an();
    tokens = tk_s;
}

void pars_an::set_tokens(const std::vector<token> &tk_s) {
    tokens = tk_s;
    expr_s.clear();
}
abstract_expr_func pars_an::get_exprs() {
    return expr_s;
}
void pars_an::clear_aef() {
    expr_s.clear();
}
void pars_an::check_valid_subexpr_first_pass(const subexpressions &sub_expr) {
    if (sub_expr.token_of_subexpr.empty())
        return;

    bool expected_op = 0, expr_with_op = 0;
    bool expected_param_kw_op = 0;

    for (u32t i = 0; i < sub_expr.token_of_subexpr.size(); ++i) {
        if (sub_expr.token_of_subexpr[i].token_t == token_type::ID ||
            sub_expr.token_of_subexpr[i].token_t == token_type::LITERAL ||
            sub_expr.token_of_subexpr[i].token_t == token_type::LITERALS) {
            if (expected_op)
                throw parser_excp(build_pos_tokenb_str(sub_expr.token_of_subexpr[i]), "003");
            else if (expected_param_kw_op)
                expected_param_kw_op = 0;
            expected_op = 1;
        }
        else if (sub_expr.token_of_subexpr[i].token_t == token_type::KW_OPERATOR) {
            if (expr_with_op)
                throw parser_excp(build_pos_tokenb_str(sub_expr.token_of_subexpr[i]), "011");
            else if (expected_op) {
                if (!IS_BIBARY_KW_OP(sub_expr.token_of_subexpr[i].token_val))
                    throw parser_excp(build_pos_tokenb_str(sub_expr.token_of_subexpr[i]), "012");
                expected_op = 0;
            }
            expected_param_kw_op = 1;
        }
        else if (sub_expr.token_of_subexpr[i].token_t == token_type::OPERATOR) {
            if (!expected_op || expected_param_kw_op)
                throw parser_excp(build_pos_tokenb_str(sub_expr.token_of_subexpr[i]), "004");
            expected_op = 0;
            expr_with_op = 1;
            continue;
        }
    }
    if ((!expected_op &&
         sub_expr.token_of_subexpr[sub_expr.token_of_subexpr.size() - 1].token_t == token_type::OPERATOR) ||
        (expected_param_kw_op &&
         !IS_CONSTANT_KW_OP(sub_expr.token_of_subexpr[sub_expr.token_of_subexpr.size() - 1].token_val)))
        throw parser_excp(build_pos_tokenb_str(sub_expr.token_of_subexpr[sub_expr.token_of_subexpr.size() - 1]), "005");
}
void pars_an::check_valid_subexpr_second_pass(subexpressions &sub_expr) {
    if (sub_expr.token_of_subexpr.empty())
        return;

    bool str_type_expr = 0, num_type_expr = 0, id_type_expr = 0;
    bool operator_plus = 0, operator_compare = 0, keyword_op = 0;

    for (u32t i = 0; i < sub_expr.token_of_subexpr.size(); ++i) {
        if (sub_expr.token_of_subexpr[i].token_t == token_type::LITERALS) {
            if (num_type_expr) {
                if (operator_plus)
                    throw parser_excp(build_pos_tokenb_str(sub_expr.token_of_subexpr[i]), "006");
                else if (operator_compare)
                    throw parser_excp(build_pos_tokenb_str(sub_expr.token_of_subexpr[i]), "007");
            }
            str_type_expr = 1;
        }
        else if (sub_expr.token_of_subexpr[i].token_t == token_type::LITERAL) {
            if (str_type_expr) {
                if (operator_plus)
                    throw parser_excp(build_pos_tokenb_str(sub_expr.token_of_subexpr[i]), "006");
                else if (operator_compare)
                    throw parser_excp(build_pos_tokenb_str(sub_expr.token_of_subexpr[i]), "007");
            }
            num_type_expr = 1;
        }
        else if (sub_expr.token_of_subexpr[i].token_t == token_type::ID)
            id_type_expr = 1;
        else if (sub_expr.token_of_subexpr[i].token_t == token_type::OPERATOR &&
                 sub_expr.token_of_subexpr[i].token_val == "+") {
            if (operator_compare)
                if (str_type_expr)
                    throw parser_excp(build_pos_tokenb_str(sub_expr.token_of_subexpr[i]), "008");
                else if (num_type_expr)
                    throw parser_excp(build_pos_tokenb_str(sub_expr.token_of_subexpr[i]), "009");
            operator_plus = 1;
        }
        else if (sub_expr.token_of_subexpr[i].token_t == token_type::OPERATOR &&
                 (sub_expr.token_of_subexpr[i].token_val == ">" || sub_expr.token_of_subexpr[i].token_val == "<" ||
                  sub_expr.token_of_subexpr[i].token_val == "=")) {
            if (operator_plus)
                if (str_type_expr)
                    throw parser_excp(build_pos_tokenb_str(sub_expr.token_of_subexpr[i]), "010");
                else if (num_type_expr)
                    throw parser_excp(build_pos_tokenb_str(sub_expr.token_of_subexpr[i]), "009");
            operator_compare = 1;
        }
        else if (sub_expr.token_of_subexpr[i].token_t == token_type::KW_OPERATOR)
            keyword_op = 1;
    }
    if (operator_compare) {
        sub_expr.subexpr_t = subexpressions::type_subexpr::INT_COMPARE;
        try_parse_subexpr(sub_expr);
    }
    else if (operator_plus) {
        sub_expr.subexpr_t = subexpressions::type_subexpr::STRING_ADD;
        try_parse_subexpr(sub_expr);
    }
    else if (keyword_op)
        sub_expr.subexpr_t = subexpressions::type_subexpr::KEYWORD_OP;
    else if (num_type_expr)
        sub_expr.subexpr_t = subexpressions::type_subexpr::INT;
    else if (str_type_expr)
        sub_expr.subexpr_t = subexpressions::type_subexpr::STRING;
    else if (id_type_expr)
        sub_expr.subexpr_t = subexpressions::type_subexpr::ID;
}

void pars_an::try_parse_subexpr(subexpressions &sub_expr) {
    subexpressions parse_subexpr;
    subexpressions const_parse_subexpr, tmp_parse_subexpr;
    if (sub_expr.subexpr_t == subexpressions::type_subexpr::INT_COMPARE) {
        for (u32t i = 0; i < sub_expr.token_of_subexpr.size(); ++i) {
            if (sub_expr.token_of_subexpr[i].token_t == token_type::ID) {
                for (u32t j = 0; j < const_parse_subexpr.token_of_subexpr.size(); ++j) {
                    parse_subexpr.token_of_subexpr.push_back(const_parse_subexpr.token_of_subexpr[j]);
                }
                parse_subexpr.token_of_subexpr.push_back(sub_expr.token_of_subexpr[i]);
                const_parse_subexpr.token_of_subexpr.clear();
                continue;
            }
            else if (sub_expr.token_of_subexpr[i].token_t == token_type::LITERALS)
                throw parser_excp(build_pos_tokenb_str(sub_expr.token_of_subexpr[i]), "010");
            if (const_parse_subexpr.token_of_subexpr.size() == 0 &&
                (sub_expr.token_of_subexpr[i].token_t == token_type::OPERATOR ||
                 (i == sub_expr.token_of_subexpr.size() - 1)))
                parse_subexpr.token_of_subexpr.push_back(sub_expr.token_of_subexpr[i]);
            else
                const_parse_subexpr.token_of_subexpr.push_back(sub_expr.token_of_subexpr[i]);
            if (const_parse_subexpr.token_of_subexpr.size() == 3) {
                bool condition = 0;
                switch (const_parse_subexpr.token_of_subexpr[1].token_val[0]) {
                case '>':
                    if (std::stoi(const_parse_subexpr.token_of_subexpr[0].token_val.c_str()) >
                        std::stoi(const_parse_subexpr.token_of_subexpr[2].token_val.c_str()))
                        condition = 1;
                    break;
                case '<':
                    if (std::stoi(const_parse_subexpr.token_of_subexpr[0].token_val.c_str()) <
                        std::stoi(const_parse_subexpr.token_of_subexpr[2].token_val.c_str()))
                        condition = 1;
                    break;
                case '=':
                    if (std::stoi(const_parse_subexpr.token_of_subexpr[0].token_val.c_str()) ==
                        std::stoi(const_parse_subexpr.token_of_subexpr[2].token_val.c_str()))
                        condition = 1;
                    break;
                }
                tmp_parse_subexpr = const_parse_subexpr;
                const_parse_subexpr.token_of_subexpr.clear();
                if (condition)
                    const_parse_subexpr.token_of_subexpr.push_back(
                        token(token_type::LITERAL, "1", tmp_parse_subexpr.token_of_subexpr[0].pos_defined_line,
                              tmp_parse_subexpr.token_of_subexpr[0].pos_beg_defined_sym));
                else
                    const_parse_subexpr.token_of_subexpr.push_back(
                        token(token_type::LITERAL, "0", tmp_parse_subexpr.token_of_subexpr[0].pos_defined_line,
                              tmp_parse_subexpr.token_of_subexpr[0].pos_beg_defined_sym));
            }
        }
    }
    else if (sub_expr.subexpr_t == subexpressions::type_subexpr::STRING_ADD) {
        for (u32t i = 0; i < sub_expr.token_of_subexpr.size(); ++i) {
            if (sub_expr.token_of_subexpr[i].token_t == token_type::ID) {
                for (u32t j = 0; j < const_parse_subexpr.token_of_subexpr.size(); ++j) {
                    parse_subexpr.token_of_subexpr.push_back(const_parse_subexpr.token_of_subexpr[j]);
                }
                parse_subexpr.token_of_subexpr.push_back(sub_expr.token_of_subexpr[i]);
                const_parse_subexpr.token_of_subexpr.clear();
                continue;
            }
            else if (sub_expr.token_of_subexpr[i].token_t == token_type::LITERAL)
                throw parser_excp(build_pos_tokenb_str(sub_expr.token_of_subexpr[i]), "009");
            if (const_parse_subexpr.token_of_subexpr.size() == 0 &&
                (sub_expr.token_of_subexpr[i].token_t == token_type::OPERATOR ||
                 (i == sub_expr.token_of_subexpr.size() - 1)))
                parse_subexpr.token_of_subexpr.push_back(sub_expr.token_of_subexpr[i]);
            else
                const_parse_subexpr.token_of_subexpr.push_back(sub_expr.token_of_subexpr[i]);
            if (const_parse_subexpr.token_of_subexpr.size() == 3) {
                tmp_parse_subexpr = const_parse_subexpr;
                const_parse_subexpr.token_of_subexpr.clear();
                const_parse_subexpr.token_of_subexpr.push_back(token(
                    token_type::LITERALS,
                    tmp_parse_subexpr.token_of_subexpr[0].token_val + tmp_parse_subexpr.token_of_subexpr[2].token_val,
                    tmp_parse_subexpr.token_of_subexpr[0].pos_defined_line,
                    tmp_parse_subexpr.token_of_subexpr[0].pos_beg_defined_sym));
            }
        }
    }
    else
        return;

    for (u32t j = 0; j < const_parse_subexpr.token_of_subexpr.size(); ++j) {
        parse_subexpr.token_of_subexpr.push_back(const_parse_subexpr.token_of_subexpr[j]);
    }
    if (parse_subexpr.token_of_subexpr.size() == 1) {
        if (parse_subexpr.token_of_subexpr[0].token_t == token_type::LITERAL)
            parse_subexpr.subexpr_t = subexpressions::type_subexpr::INT;
        else
            parse_subexpr.subexpr_t = subexpressions::type_subexpr::STRING;
    }
    else
        parse_subexpr.subexpr_t = sub_expr.subexpr_t;
    sub_expr = parse_subexpr;
}

abstract_expr_func &pars_an::analysis() {
    bool start_expr = 1, func_param_curr = 0;

    expression curr_expr;
    subexpressions curr_sub_expr;

    for (u32t i = 0; i < tokens.size(); ++i) {
        if (start_expr) {
            if (tokens[i].token_t != token_type::ID && tokens[i].token_t != token_type::KEYWORD)
                throw parser_excp(build_pos_tokenb_str(tokens[i]), "000");
            curr_expr.expr_func.func_t = tokens[i];
            start_expr = 0;
        }
        else {
            if (!func_param_curr) {
                if (tokens[i].token_t == token_type::OPEN_BR) {
                    func_param_curr = 1;
                    continue;
                }
                throw parser_excp(build_pos_tokenb_str(tokens[i]), "001");
            }
            else if (func_param_curr && tokens[i].token_t == token_type::COMMA) {
                if (!is_param(tokens[i - 1]))
                    throw parser_excp(build_pos_tokenb_str(tokens[i]), "004");
                check_valid_subexpr_first_pass(curr_sub_expr);
                check_valid_subexpr_second_pass(curr_sub_expr);
                curr_expr.sub_expr_s.push_back(curr_sub_expr);
                curr_sub_expr.token_of_subexpr.clear();
                continue;
            }
            else if (func_param_curr && tokens[i].token_t == token_type::CLOSE_BR) {
                if (!is_param(tokens[i - 1])) {
                    if (tokens[i - 1].token_t != token_type::OPEN_BR)
                        throw parser_excp(build_pos_tokenb_str(tokens[i]), "004");
                    goto start_expr_func;
                }
                check_valid_subexpr_first_pass(curr_sub_expr);
                check_valid_subexpr_second_pass(curr_sub_expr);
                curr_expr.sub_expr_s.push_back(curr_sub_expr);
                curr_sub_expr.token_of_subexpr.clear();

            start_expr_func:
                func_param_curr = 0;
                start_expr = 1;

                expr_s.push_back(curr_expr);
                curr_expr.sub_expr_s.clear();

                continue;
            }
            else if (func_param_curr && tokens[i].token_t != token_type::ID &&
                     tokens[i].token_t != token_type::LITERAL && tokens[i].token_t != token_type::LITERALS &&
                     tokens[i].token_t != token_type::OPERATOR && tokens[i].token_t != token_type::KW_OPERATOR)
                throw parser_excp(build_pos_tokenb_str(tokens[i]), "002");
            curr_sub_expr.token_of_subexpr.push_back(tokens[i]);
        }
    }

    return expr_s;
}