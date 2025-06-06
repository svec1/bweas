//
// BWEAS is distributed under the GNU General Public License 2.0 (GPL-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef EXPRESSION__H
#define EXPRESSION__H

#include <functional>

#include <bwaliases.hpp>

namespace var {
class scope;
}

// list of parameters that the function can expect
enum class param_type {

    // this parameter tells semantic analysis that:
    //  1. the current function (most likely) is a declaring function
    //  2. it needs to check that the name that was passed as a parameter
    //     of this type does not exist in the symbol table of the global or
    //     external scope
    FUTURE_VAR_ID,

    // this parameter tells the semantic parser that it does not need to worry
    // about
    // the issue of declaring the identifier; with this parameter it will
    // not create a declaration of the symbol in the symbol table, and will not
    // check whether it exists
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
    param(param_type _type, string_v _default_val = "") : type(_type), default_val(_default_val) {
    }

    inline bool decl_default_val() const {
        return !default_val.empty();
    }

    param_type type;
    string default_val;
};

struct statement;
struct expression;

using expressions = vec<expression>;
using statements  = vec<statement>;

// the notion of a function, which contains a reference
// to the function itself, the parameters that it expects when called,
// and two fields that define the call to this function
struct decl_func {
    using func_t = std::function<void(const expressions &, var::scope &)>;

  public:
    decl_func() = default;
    explicit decl_func(string_v _name_func, func_t _func, vec<param> _expected_params)
        : name_func(_name_func), func(_func), expected_params(_expected_params) {
    }

  public:
    inline size_t count_default_param() const;

  public:
    string name_func;

    func_t func;
    vec<param> expected_params;
};

// structure is a representation of a single function call
struct statement {
  public:
    statement() = default;
    explicit statement(const decl_func *_expr_func, expressions _expr_s, size_t _line = 0, size_t _column = 0)
        : expr_func(_expr_func), expr_s(_expr_s), line(_line), column(_column) {
    }

  public:
    string get_location() const {
        return "[" + std::to_string(line) + ":" + std::to_string(column) + "]";
    }

  public:
    const decl_func *expr_func;
    expressions expr_s;

    size_t line, column;
};

// structure is a list of parameters passed
// to the function when it is called
struct expression {
  public:
    enum class expression_t {
        NUMBER = 0,
        STRING,
        ID,

        SIZE_ENUM_RET_TYPE_EXPR
    };

  public:
    expression() = default;
    explicit expression(string_v _value, expression_t _type, size_t line = 0, size_t column = 0)
        : value(_value), type(_type) {
    }

  public:
    expression_t type;
    string value;

    size_t line, column;
};

inline size_t decl_func::count_default_param() const {
    size_t count_dp = 0;
    for (const param &_param : expected_params)
        if (!_param.default_val.empty())
            ++count_dp;

    return count_dp;
}

static inline expression::expression_t conv_param_type_to_expr_type(param_type _param) {
    if (_param == param_type::FUTURE_VAR_ID || _param == param_type::NCHECK_VAR_ID || _param == param_type::VAR_ID ||
        _param == param_type::VAR_STRUCT_ID)
        return expression::expression_t::ID;
    else if (_param == param_type::LIT_NUM || _param == param_type::LNUM_OR_ID_VAR)
        return expression::expression_t::NUMBER;
    else if (_param == param_type::LIT_STR || _param == param_type::LSTR_OR_ID_VAR)
        return expression::expression_t::STRING;
    return expression::expression_t::SIZE_ENUM_RET_TYPE_EXPR;
}

static inline bool operator==(expression::expression_t e_type, param_type p_type) {
    if (conv_param_type_to_expr_type(p_type) == e_type)
        return true;
    return false;
}

static inline string_v get_string_expr_type(expression::expression_t type) {
    if (type == expression::expression_t::NUMBER)
        return "NUMBER";
    else if (type == expression::expression_t::STRING)
        return "STRING";
    else if (type == expression::expression_t::ID)
        return "ID";
    return "UNDEFINED";
}

static inline param_type get_string_param_type(string_v str) {
    if (str == "FUTURE_VAR_ID")
        return param_type::FUTURE_VAR_ID;
    else if (str == "VAR_ID")
        return param_type::VAR_ID;
    else if (str == "NCHECK_VAR")
        return param_type::NCHECK_VAR_ID;
    else if (str == "VAR_STRUCT_ID")
        return param_type::VAR_STRUCT_ID;
    else if (str == "ANY_VALUE_WITHOUT_FUTUREID_NEXT")
        return param_type::ANY_VALUE_WITHOUT_FUTUREID_NEXT;
    else if (str == "LIT_STR")
        return param_type::LIT_STR;
    else if (str == "LIT_NUM")
        return param_type::LIT_NUM;
    else if (str == "LSTR_OR_ID_VAR")
        return param_type::LSTR_OR_ID_VAR;
    else if (str == "LNUM_OR_ID_VAR")
        return param_type::LSTR_OR_ID_VAR;
    else if (str == "NEXT_TOO")
        return param_type::NEXT_TOO;
    else
        return param_type::SIZE_ENUM_PARAMS;
}
#endif
