#ifndef _LEXER__H
#define _LEXER__H

#include "../../kernel/low_level/type.h"

#include <vector>
#include <string>

namespace lexer{

    enum class token_type{
        KEYWORD = 0,
        
        LITERAL,
        LITERALS,

        ID,

        COMMA,
        OPEN_BR,
        CLOSE_BR,
        OPERATOR
    };

    struct token{
        token() = default;
        token(token_type tk_t, std::string tk_val, u32t pos_def_line, u32t pos_beg_def_sym)
          : token_t(tk_t),
            token_val(tk_val),
            pos_defined_line(pos_def_line),
            pos_beg_defined_sym(pos_beg_def_sym) {}

        token_type token_t;
        std::string token_val;

        u32t pos_defined_line{0};
        u32t pos_beg_defined_sym{0};
    };

    class lex_an{
        public:
            lex_an();
            lex_an(const std::string& _symbols);
            
            lex_an(lex_an&&) = delete;
            lex_an(const lex_an&) = delete;
            lex_an& operator=(lex_an&&) = delete;

            ~lex_an() = default;

        public:
            void set_symbols(const std::string& _symbols);
            std::vector<token> get_tokens();
            void clear_tokens();

            std::vector<token> analysis();

        private:
            char get();
            bool check_sym_valid_grammar(char ch);

        private:
            static inline bool init_glob{0};

            std::vector<token> tokens;
            std::string symbols;
            
            u32t pos{0};
    };

}

#endif