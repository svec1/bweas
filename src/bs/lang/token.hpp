#ifndef TOKEN__H
#define TOKEN__H

#include "../../kernel/high_level/bwtype.h"
#include <string>

#define STR_KEYWORD_IF "if"
#define STR_KEYWORD_ELSE "else"
#define STR_KEYWORD_ENDIF "endif"

#define STR_KEYWORD_OP_EQUAL "EQUAL"
#define STR_KEYWORD_OP_AND "AND"
#define STR_KEYWORD_OP_OR "OR"
#define STR_KEYWORD_OP_NOT "NOT"
#define STR_KEYWORD_OP_TSTR "TO_STR"

#define STR_KEYWORD_OP_CONST_TRUE "TRUE"
#define STR_KEYWORD_OP_CONST_FALSE "FALSE"
#define STR_KEYWORD_OP_CONST_RELEASE "RELEASE"
#define STR_KEYWORD_OP_CONST_DEBUG "DEBUG"

#define IS_BIBARY_KW_OP(STR) (STR == STR_KEYWORD_OP_EQUAL || STR == STR_KEYWORD_OP_AND || STR == STR_KEYWORD_OP_OR)
#define IS_UNARY_KW_OP(STR) (STR == STR_KEYWORD_OP_TSTR || STR == STR_KEYWORD_OP_NOT)
#define IS_CONSTANT_KW_OP(STR)                                                                                         \
    (STR == STR_KEYWORD_OP_CONST_RELEASE || STR == STR_KEYWORD_OP_CONST_DEBUG || STR == STR_KEYWORD_OP_CONST_TRUE ||   \
     STR == STR_KEYWORD_OP_CONST_FALSE)

#define IS_CONSTANT_RET_INT(STR)                                                                                       \
    (STR == STR_KEYWORD_OP_CONST_RELEASE || STR == STR_KEYWORD_OP_CONST_DEBUG || STR == STR_KEYWORD_OP_CONST_TRUE ||   \
     STR == STR_KEYWORD_OP_CONST_FALSE)

#define RET_INT_KW_OP(STR)                                                                                             \
    (STR == STR_KEYWORD_OP_EQUAL || STR == STR_KEYWORD_OP_AND || STR == STR_KEYWORD_OP_NOT ||                          \
     STR == STR_KEYWORD_OP_OR || IS_CONSTANT_RET_INT(STR))
#define RET_STR_KW_OP(STR) (STR == STR_KEYWORD_OP_TSTR)

namespace token_expr {

extern const std::array<std::string, 9> keywords_op;

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

    // Special operators as keywords
    KW_OPERATOR,
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
static inline bool operator==(const token &tk1, const token &tk2) {
    if (tk1.token_t == tk2.token_t && tk1.token_val == tk2.token_val && tk1.pos_defined_line == tk2.pos_defined_line &&
        tk1.pos_beg_defined_sym == tk2.pos_beg_defined_sym)
        return 1;
    return 0;
}
} // namespace token_expr

#endif