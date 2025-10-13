//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses />
// ------------------------------------------
//

#ifndef LEXER_HPP
#define LEXER_HPP

#include <lang/tokens.hpp>

namespace bwlang {
class lexer;
} // namespace bwlang

/** \brief A class describing lexical analysis. */
class bwlang::lexer {
  public:
    lexer(string_v src);

  public:
    const vec<tokens::token> &get_tokens() const;

  protected:
    tokens::token peek();
    tokens::token consume();

    tokens::token consume_if();

  protected:
    vec<tokens::token> &get_tokens();

    string get_string_line(const tokens::token &tk) const {
        if (tk.line_index - 1 < lines.size())
            return lines[tk.line_index - 1];
        return "?";
    }
    string get_string_previous_line(const tokens::token &tk) const {
        if (tk.line_index - 1 < lines.size() && tk.line_index)
            return lines[tk.line_index - 2];
        return "";
    }

  private:
    tokens::token get_token(string_v value, bool in_quote = 0);

    vec<tokens::token> tokens;
    vec<string> lines;
};

#endif
