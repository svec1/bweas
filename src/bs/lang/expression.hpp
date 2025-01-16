#ifndef EXPRESSION__H
#define EXPRESSION__H

#include "scope.hpp"
#include "token.hpp"

#include <vector>

namespace aef_expr {

// list of parameters that the function can expect
enum class param_type {

    // this parameter tells semantic analysis that:
    //  1. the current function (most likely) is a declaring function
    //  2. it needs to check that the name that was passed as a parameter
    //     of this type does not exist in the symbol table of the global or external scope
    FUTURE_VAR_ID,

    // this parameter tells the semantic parser that it does not need to worry about
    // the issue of declaring the identifier; with this parameter it will
    // not create a declaration of the symbol in the symbol table, and will not check whether it exists
    NCHECK_VAR_ID,

    VAR_ID,

    // this parameter means that the passed identifier is a link
    // to either a project or target variable
    VAR_STRUCT_ID,

    // a parameter that indicates that the function
    // accepts any value at a given parameter index
    ANY_VALUE_WITHOUT_FUTUREID_NEXT,

    LIT_STR,
    LIT_NUM,
    LSTR_OR_ID_VAR,
    LNUM_OR_ID_VAR,

    // this type of parameter means that the current parameter at index,
    // and the next ones, will be of the same type as the parameter before it
    NEXT_TOO,

    // end enum
    SIZE_ENUM_PARAMS
};

struct param {
    param(param_type _param_t, std::string _default_val = "") : param_t(_param_t), default_val(_default_val) {
    }

    inline bool decl_default_val() {
        return !default_val.empty();
    }

    param_type param_t;
    std::string default_val;
};

struct subexpressions;

// the notion of a function, which contains a reference
// to the function itself, the parameters that it expects when called,
// and two fields that define the call to this function
struct notion_func {
    notion_func() = default;
    explicit notion_func(void (*func)(const std::vector<subexpressions> &, var::scope &), std::vector<param> params)
        : func_ref(func), expected_params(params) {
    }

    using func_t = void (*)(const std::vector<subexpressions> &, var::scope &);

    inline u32t count_default_param();

    func_t func_ref;
    std::vector<param> expected_params;

    // does the function declare anyone
    // (if so, then the function will be called during semantic analysis)
    bool is_declaration_var{0};

    // whether to call a function in semantic parsing
    // (provided so that certain functions that cannot declare
    // anything can be called in semantic analysis)
    bool only_with_semantic{0};
};

// a structure that is a representation
// (notion) of a function
struct notion_expr_func {

    // function call token
    token_expr::token func_t;
    notion_func func_n;
};

// structure is a representation of a single function call
struct expression {
    inline bool execute_with_semantic_an();

    notion_expr_func expr_func;
    std::vector<subexpressions> sub_expr_s;
};

// structure is a list of parameters passed
// to the function when it is called
struct subexpressions {
    // declaration of all possible parameter types
    enum class type_subexpr {
        INT_COMPARE,
        STRING_ADD,
        INT,
        STRING,
        ID,

        KEYWORD_OP,

        SIZE_ENUM_TYPE_SUBEXPR
    };
    enum class ret_type_subexpr {
        INT,
        STRING,
        ID,

        SIZE_ENUM_RET_TYPE_SUBEXPR
    };
    subexpressions() = default;
    explicit subexpressions(std::vector<token_expr::token> _token_of_subexpr, type_subexpr _subexpr_t)
        : token_of_subexpr(_token_of_subexpr), subexpr_t(_subexpr_t) {
    }

    inline ret_type_subexpr returned_type_subexpr() const;
    inline bool param_match_ret_type_subexpr(param_type) const;

    // tokens of single parameter
    std::vector<token_expr::token> token_of_subexpr;
    type_subexpr subexpr_t;
};

// Determines the need to call this function in semantic analysis
inline bool expression::execute_with_semantic_an() {
    return expr_func.func_n.is_declaration_var || expr_func.func_n.only_with_semantic;
}

// Defines the return type (expected after semantic analysis)
inline subexpressions::ret_type_subexpr subexpressions::returned_type_subexpr() const {
    if (subexpr_t == type_subexpr::INT || subexpr_t == type_subexpr::INT_COMPARE)
        return ret_type_subexpr::INT;
    else if (subexpr_t == type_subexpr::STRING || subexpr_t == type_subexpr::STRING_ADD)
        return ret_type_subexpr::STRING;
    else if (subexpr_t == type_subexpr::ID)
        return ret_type_subexpr::ID;
    else if (subexpr_t == type_subexpr::KEYWORD_OP) {
        const auto &it =
            std::find_if(this->token_of_subexpr.begin(), this->token_of_subexpr.end(), [](const token_expr::token &tk) {
                return tk.token_t == token_expr::token_type::KW_OPERATOR && IS_BIBARY_KW_OP(tk.token_val);
            });
        if (it == this->token_of_subexpr.end()) {
            if (RET_INT_KW_OP(this->token_of_subexpr[0].token_val))
                return ret_type_subexpr::INT;
            else if (RET_STR_KW_OP(this->token_of_subexpr[0].token_val))
                return ret_type_subexpr::STRING;
        }
        else if (RET_INT_KW_OP(it->token_val))
            return ret_type_subexpr::INT;
        else if (RET_STR_KW_OP(it->token_val))
            return ret_type_subexpr::STRING;
    }
    return ret_type_subexpr::SIZE_ENUM_RET_TYPE_SUBEXPR;
}

inline bool subexpressions::param_match_ret_type_subexpr(param_type _param) const {
    ret_type_subexpr subexpr_rt = returned_type_subexpr();
    if ((subexpr_rt == ret_type_subexpr::ID &&
         (_param == param_type::FUTURE_VAR_ID || _param == param_type::NCHECK_VAR_ID || _param == param_type::VAR_ID ||
          _param == param_type::VAR_STRUCT_ID)) ||
        (subexpr_rt == ret_type_subexpr::INT &&
         (_param == param_type::LIT_NUM || _param == param_type::LNUM_OR_ID_VAR)) ||
        (subexpr_rt == ret_type_subexpr::STRING &&
         (_param == param_type::LIT_STR || _param == param_type::LSTR_OR_ID_VAR)))
        return 1;
    return 0;
}

inline u32t notion_func::count_default_param() {
    u32t count_dp = 0;
    for (const param &_param : expected_params)
        if (!_param.default_val.empty())
            ++count_dp;

    return count_dp;
}

static inline subexpressions::type_subexpr conv_param_type_to_subexpr_type(param_type _param) {
    if (_param == param_type::FUTURE_VAR_ID || _param == param_type::NCHECK_VAR_ID || _param == param_type::VAR_ID ||
        _param == param_type::VAR_STRUCT_ID)
        return subexpressions::type_subexpr::ID;
    else if (_param == param_type::LIT_NUM || _param == param_type::LNUM_OR_ID_VAR)
        return subexpressions::type_subexpr::INT;
    else if (_param == param_type::LIT_STR || _param == param_type::LSTR_OR_ID_VAR)
        return subexpressions::type_subexpr::STRING;
    return subexpressions::type_subexpr::SIZE_ENUM_TYPE_SUBEXPR;
}
static inline token_expr::token_type conv_param_type_to_token_type(param_type _param) {
    if (_param == param_type::FUTURE_VAR_ID || _param == param_type::NCHECK_VAR_ID || _param == param_type::VAR_ID ||
        _param == param_type::VAR_STRUCT_ID)
        return token_expr::token_type::ID;
    else if (_param == param_type::LIT_NUM || _param == param_type::LNUM_OR_ID_VAR)
        return token_expr::token_type::LITERAL;
    else if (_param == param_type::LIT_STR || _param == param_type::LSTR_OR_ID_VAR)
        return token_expr::token_type::LITERALS;
    return token_expr::token_type::SIZE_ENUM_TOKEN_TYPE;
}

} // namespace aef_expr
#endif