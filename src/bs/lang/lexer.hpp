#ifndef _LEXER__H
#define _LEXER__H

#include "token.hpp"
#include "../tools/exception.hpp"

#include <string>
#include <vector>

namespace lexer {

#define STR_KEYWORD_IF "if"
#define STR_KEYWORD_ELSE "else"
#define STR_KEYWORD_ENDIF "endif"

class lexer_excp : public bw_excp::bweas_exception {
  public:
    lexer_excp(std::string _what_hp, std::string number_err)
        : what_hp(_what_hp), bweas_exception("LEX" + number_err) {
    }
    ~lexer_excp() noexcept override final = default;

  public:
    const char *what() const noexcept override final {
        return what_hp.c_str();
    }

  private:
    std::string what_hp;
};

class lex_an {
  public:
    lex_an();
    lex_an(const std::string &_symbols);

    lex_an(lex_an &&) = delete;
    lex_an(const lex_an &) = delete;
    lex_an &operator=(lex_an &&) = delete;

    ~lex_an() = default;

  public:
    void set_symbols(const std::string &_symbols);
    std::vector<token_expr::token> get_tokens();
    void clear_tokens();

    // lexical analysis
    // ----------------
    // Creates tokens with type "token_type".
    // Prohibits:
    //  1. use of symbols: * / ! - ' \.
    //  2. form mixed lexemes: "32ggtn5var".
    //  3. form a literal greater than UINT_MAX32/2.
    // ---------------------------------------------
    // When the @ symbol is encountered, it begins to form a lexeme until the end of the line
    // (until the sequence of characters \r\n or \n is found), this lexeme will be a special word for the lexer,
    // which, depending on its value, will perform one or another action.
    // Available special words:
    //  1. lexer_stop
    std::vector<token_expr::token> analysis();

  private:
    char get();

    // Prohibits characters
    bool check_sym_valid_grammar(char ch);

  private:
    static inline bool init_glob{0};

    std::vector<token_expr::token> tokens;
    std::string symbols;

    u32t pos{0};
};

} // namespace lexer

#endif