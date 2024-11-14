#include "../../mdef.hpp"

#include "parser.hpp"

using namespace aef_expr;
using namespace token_expr;

#define is_param(token)                                                                                                \
    (token.token_t == token_type::ID || token.token_t == token_type::LITERAL || token.token_t == token_type::LITERALS)

std::string parser::type_token_str(token_type token_t) {
    switch (token_t) {
    case token_type::KEYWORD:
        return "KEYWORD";
    case token_type::ID:
        return "ID";
    case token_type::LITERAL:
        return "LITERAL";
    case token_type::LITERALS:
        return "LITERALS";
    case token_type::OPERATOR:
        return "OPERATOR";
    case token_type::OPEN_BR:
        return "OPEN_BR";
    case token_type::CLOSE_BR:
        return "CLOSE_BR";
    case token_type::COMMA:
        return "COMMA";
    default:
        return "UNDEF";
    }
}
std::string parser::build_pos_tokenb_str(const token &tk) {
    if (!tk.token_val.empty())
        return "\n(Line: " + std::to_string(tk.pos_defined_line) +
               "; Symbols start pos: " + std::to_string(tk.pos_beg_defined_sym) + "): Type token - " +
               type_token_str(tk.token_t) + "; Token value - \"" + tk.token_val + "\"\n";
    return "\n(Line: " + std::to_string(tk.pos_defined_line) +
           "; Symbols start pos: " + std::to_string(tk.pos_beg_defined_sym) + "): Type token - " +
           type_token_str(tk.token_t) + "\n";
}
std::string parser::build_pos_subexpr_str(const subexpressions &sub_expr) {
    std::string str;
    if (sub_expr.subexpr_t == subexpressions::type_subexpr::INT_COMPARE ||
        sub_expr.subexpr_t == subexpressions::type_subexpr::STRING_ADD) {
        str = "\n(Line: " + std::to_string(sub_expr.token_of_subexpr[0].pos_defined_line) +
              "; Symbols start pos: " + std::to_string(sub_expr.token_of_subexpr[0].pos_beg_defined_sym) + "): ";
        for (u32t i = 0; i < sub_expr.token_of_subexpr.size(); ++i) {
            str += sub_expr.token_of_subexpr[i].token_val + " ";
        }
        str += " \n";
    }
    else
        str = build_pos_tokenb_str(sub_expr.token_of_subexpr[0]);
    return str;
}
std::string parser::type_sybexpr_str(subexpressions::type_subexpr sub_expr_t) {
    switch (sub_expr_t) {
    case subexpressions::type_subexpr::ID:
        return "<ID>";
    case subexpressions::type_subexpr::INT:
        return "<INT>";
    case subexpressions::type_subexpr::STRING:
        return "<STRING>";
    case subexpressions::type_subexpr::INT_COMPARE:
        return "<INT_COMPARE>";
    case subexpressions::type_subexpr::STRING_ADD:
        return "<STRING_ADD>";
    default:
        return "<UNDEF>";
    }
}
std::string parser::tree_build_visually_str(const abstract_expr_func &aef) {
    std::stringstream tree_visible;
    for (u32t i = 0; i < aef.size(); ++i) {
        tree_visible << "EXPRESSION:\n";
        tree_visible << " |TOKEN FUNC: " << "TYPE TOKEN: " << parser::type_token_str(aef[i].expr_func.func_t.token_t)
                     << "; LEXEM: " << aef[i].expr_func.func_t.token_val << '\n';
        for (u32t j = 0; j < aef[i].sub_expr_s.size(); ++j) {
            tree_visible << " |  | " << j << " SUBEXPRESSION " << type_sybexpr_str(aef[i].sub_expr_s[j].subexpr_t)
                         << ": \n";
            std::string value_expr;
            for (u32t k = 0; k < aef[i].sub_expr_s[j].token_of_subexpr.size(); ++k) {
                tree_visible << " |  |  | -> "
                             << parser::type_token_str(aef[i].sub_expr_s[j].token_of_subexpr[k].token_t) << "("
                             << aef[i].sub_expr_s[j].token_of_subexpr[k].token_val << ")\n";
                value_expr += aef[i].sub_expr_s[j].token_of_subexpr[k].token_val;
            }
            tree_visible << " |  | SOURCE: " << value_expr << '\n';
            tree_visible << " |  ------------------" << '\n';
            value_expr.clear();
        }
    }
    return tree_visible.str();
}
params parser::type_param_in_str(std::string str) {
    if (str == "FUTURE_VAR_ID")
        return params::FUTURE_VAR_ID;
    else if (str == "VAR_ID")
        return params::VAR_ID;
    else if (str == "VAR_STRUCT_ID")
        return params::VAR_STRUCT_ID;
    else if (str == "ANY_VALUE_WITHOUT_FUTUREID_NEXT")
        return params::ANY_VALUE_WITHOUT_FUTUREID_NEXT;
    else if (str == "LIT_STR")
        return params::LIT_STR;
    else if (str == "LIT_NUM")
        return params::LIT_NUM;
    else if (str == "LSTR_OR_ID_VAR")
        return params::LSTR_OR_ID_VAR;
    else if (str == "LNUM_OR_ID_VAR")
        return params::LSTR_OR_ID_VAR;
    else if (str == "NEXT_TOO")
        return params::NEXT_TOO;
    else
        return params::SIZE_ENUM_PARAMS;
}

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
                                  "with the type \"OPERATOR\"");
        assist.add_err("PARS006", "It is impossible to add two different tokens by type");
        assist.add_err("PARS007", "It is impossible to compare two different tokens by type");
        assist.add_err("PARS008", "It is impossible to add strings when comparing");
        assist.add_err("PARS009", "Arithmetic operations are prohibited");
        assist.add_err("PARS010", "Impossible to compare strings");
        assist.add_err("PARS011", "You cannot use operators together with operator keywords");
    }
}

pars_an::pars_an(const std::vector<token> &tk_s) {
    pars_an();
    tokens = tk_s;
}

void pars_an::set_tokens(const std::vector<token> &tk_s) {
    tokens = tk_s;
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
            if(expr_with_op)
                throw parser_excp(build_pos_tokenb_str(sub_expr.token_of_subexpr[i]), "011");
            if (expected_op)
                expected_op = 0;
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
    if (!expected_op)
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
        if (!keyword_op)
            try_parse_subexpr(sub_expr);
    }
    else if (operator_plus) {
        sub_expr.subexpr_t = subexpressions::type_subexpr::STRING_ADD;
        if (!keyword_op)
            try_parse_subexpr(sub_expr);
    }
    else if (keyword_op){
        sub_expr.subexpr_t = subexpressions::type_subexpr::KEYWORD_OP;
    }
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