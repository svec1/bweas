#ifndef TOKEN__H
#define TOKEN__H

#include "../../kernel/low_level/type.h"
#include <string>

namespace token_expr {

// listing all possible token types
enum class token_type {
    KEYWORD = 0,

    // 1,2,3...(UINT_MAX32 / 2)
    LITERAL,

    // "text"
    LITERALS,

    ID,

    COMMA,
    OPEN_BR,
    CLOSE_BR,

    // > < = +
    // only the + operator is a string addition operator
    // (arithmetic operations are prohibited at the operator level)
    // -----------------------------------------------------------
    // Also, strings cannot be compared (for now)
    OPERATOR,
    SIZE_ENUM_TOKEN_TYPE
};

// struct of token
struct token {
    token() = default;
    token(token_type tk_t, std::string tk_val, u32t pos_def_line, u32t pos_beg_def_sym)
        : token_t(tk_t), token_val(tk_val), pos_defined_line(pos_def_line), pos_beg_defined_sym(pos_beg_def_sym) {
    }

    token_type token_t;
    std::string token_val;

    // token position in the text (line number)
    u32t pos_defined_line{0};

    // token position in the line (symbol number)
    u32t pos_beg_defined_sym{0};
};
} // namespace token_expr

#endif