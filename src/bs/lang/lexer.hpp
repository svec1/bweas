#ifndef LEXER_HPP
#define LEXER_HPP

#include <lang/tokens.hpp>

namespace bwlang {

class lexer {
  public:
    lexer(string_v src);

  public:
    tokens::token peek();
    tokens::token consume();

    tokens::token consume_if();

  protected:
    vec<tokens::token> &get_tokens();

    string get_string_last_line() {
        if (!std::holds_alternative<tokens::end_line>(peek())) {
            for (const auto &token : tokens) {
                if (std::holds_alternative<tokens::end_line>(token))
                    break;
                string_last_line += tokens::get_string(token);
            }
        }
        return string_last_line;
    }
    string get_string_previous_line() {
        return previous_line;
    }

    pdiff get_number_last_line() const {
        return number_last_line;
    }

  private:
    tokens::token get_token(string_v value, bool in_quote = 0);

    vec<tokens::token> tokens;
    string string_last_line, previous_line;
    pdiff number_last_line = 1;
};
} // namespace bwlang

#endif
