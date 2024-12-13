#include "parser.hpp"

using namespace aef_expr;
using namespace token_expr;

std::string parser::utility::type_token_str(token_type token_t) {
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
    case token_type::KW_OPERATOR:
        return "KW_OPERATOR";
    default:
        return "UNDEF";
    }
}

std::string parser::utility::type_ret_subexpr(aef_expr::subexpressions::ret_type_subexpr subexpr_ret_t) {
    switch (subexpr_ret_t) {
    case subexpressions::ret_type_subexpr::INT:
        return "NUMBER";
    case subexpressions::ret_type_subexpr::STRING:
        return "STRING";
    case subexpressions::ret_type_subexpr::ID:
        return "VAR ID";
        break;
    default:
        break;
    }
}

std::string parser::utility::build_pos_tokenb_str(const token &tk) {
    if (!tk.token_val.empty())
        return "\n(Line: " + std::to_string(tk.pos_defined_line) +
               "; Symbols start pos: " + std::to_string(tk.pos_beg_defined_sym) + "): Type token - " +
               type_token_str(tk.token_t) + "; Token value - \"" + tk.token_val + "\"\n";
    return "\n(Line: " + std::to_string(tk.pos_defined_line) +
           "; Symbols start pos: " + std::to_string(tk.pos_beg_defined_sym) + "): Type token - " +
           type_token_str(tk.token_t) + "\n";
}
std::string parser::utility::build_pos_subexpr_str(const subexpressions &sub_expr) {
    std::string str;
    str = "\n(Line: " + std::to_string(sub_expr.token_of_subexpr[0].pos_defined_line) +
          "; Symbols start pos: " + std::to_string(sub_expr.token_of_subexpr[0].pos_beg_defined_sym) +
          "): Expression(" + type_ret_subexpr(sub_expr.returned_type_subexpr()) + ") <";
    for (u32t i = 0; i < sub_expr.token_of_subexpr.size(); ++i) {
        str += sub_expr.token_of_subexpr[i].token_val;
        if (i < sub_expr.token_of_subexpr.size() - 1)
            str += " ";
    }
    str += ">\n";
    return str;
}
std::string parser::utility::type_sybexpr_str(subexpressions::type_subexpr sub_expr_t) {
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
std::string parser::utility::tree_build_visually_str(const abstract_expr_func &aef) {
    std::stringstream tree_visible;
    for (u32t i = 0; i < aef.size(); ++i) {
        tree_visible << "EXPRESSION:\n";
        tree_visible << " |TOKEN FUNC: " << "TYPE TOKEN: " << type_token_str(aef[i].expr_func.func_t.token_t)
                     << "; LEXEM: " << aef[i].expr_func.func_t.token_val << '\n';
        for (u32t j = 0; j < aef[i].sub_expr_s.size(); ++j) {
            tree_visible << " |  | " << j << " SUBEXPRESSION " << type_sybexpr_str(aef[i].sub_expr_s[j].subexpr_t)
                         << ": \n";
            std::string value_expr;
            for (u32t k = 0; k < aef[i].sub_expr_s[j].token_of_subexpr.size(); ++k) {
                tree_visible << " |  |  | -> " << type_token_str(aef[i].sub_expr_s[j].token_of_subexpr[k].token_t)
                             << "(" << aef[i].sub_expr_s[j].token_of_subexpr[k].token_val << ")\n";
                value_expr += aef[i].sub_expr_s[j].token_of_subexpr[k].token_val;
            }
            tree_visible << " |  | SOURCE: " << value_expr << '\n';
            tree_visible << " |  ------------------" << '\n';
            value_expr.clear();
        }
    }
    return tree_visible.str();
}
params parser::utility::type_param_in_str(std::string str) {
    if (str == "FUTURE_VAR_ID")
        return params::FUTURE_VAR_ID;
    else if (str == "VAR_ID")
        return params::VAR_ID;
    else if (str == "NCHECK_VAR")
        return params::NCHECK_VAR_ID;
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