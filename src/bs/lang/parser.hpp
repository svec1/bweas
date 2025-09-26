#ifndef PARSER_HPP
#define PARSER_HPP

#include <bwmodule.hpp>
#include <lang/expression.hpp>
#include <lang/lexer.hpp>

namespace bwlang {

class parser : private lexer {
  public:
    parser(string_v src);

  public:
    void parse();

    parser_utils::context &get_context() & {
        return *g_ctx;
    }
    void import_modules(vec<bweas::module_manager::_module> &&_modules) {
        modules = _modules;
    }

  protected:
    parser_utils::value parse_statements(bool skip = 0, bool is_branche = 0, bool is_func = 0);
    std::unique_ptr<expression::ext::base> parse_expression(pdiff lbinding_power = 0);
    parser_utils::value parse_if_else_branche(bool in_skip_branche = 0, bool is_func = 0);
    void parse_import();
    void parse_function();

  private:
    template <typename... Tokens> tokens::token expect_tokens() {
        tokens::token tk;
        string expected;
        bool is_not_token = true;

        auto _ = [&](auto &&type) -> std::void_t<decltype(tokens::token{std::decay_t<decltype(type)>{}})> {
            using Token = std::decay_t<decltype(type)>;
            try {
                if (is_not_token) {
                    if (!std::holds_alternative<Token>(peek()))
                        throw "";

                    tk           = consume();
                    is_not_token = false;
                }
                else
                    return;
            }
            catch (...) {
                expected += "\'" + tokens::get_string(Token{}) + "\' ";
                return;
            }
        };

        skip_token_end_line();
        (_(Tokens{}), ...);

        if (is_not_token)
            throw parser_utils::parser_error("Expected token " + expected + ".", consume_if());
        return tk;
    }
    template <typename Token> tokens::token expect_token() {
        return expect_tokens<Token>();
    }

    template <typename Keyword> void expect_keyword() {
        skip_token_end_line();
        if (!tokens::is_keyword<Keyword>(peek()))
            throw parser_utils::parser_error("Expected keyword \'" + string(Keyword::s_value) + "\'. ", consume_if());
        consume();
    }
    tokens::identifier expect_identifier() {
        skip_token_end_line();
        if (!std::holds_alternative<tokens::identifier>(peek()))
            throw parser_utils::parser_error("Expected identifier.", consume());
        return std::get<tokens::identifier>(consume_if());
    }

    void skip_token_end_line() {
        while (std::holds_alternative<tokens::end_line>(peek()))
            consume();
    }

  private:
    vec<bweas::module_manager::_module> modules;
    parser_utils::context *g_ctx;
};

} // namespace bwlang
#endif
