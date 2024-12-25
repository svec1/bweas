#include <bs/lang/parser.hpp>
#include <gtest/gtest.h>

TEST(BW_PARSER, CorrectOutputAEFClassBwParser) {
    parser::pars_an parser;
    std::vector<token_expr::token> tk_s;

    tk_s = {token_expr::token{token_expr::token_type::ID, "test_func"},
            token_expr::token{token_expr::token_type::OPEN_BR, ""},
            token_expr::token{token_expr::token_type::LITERAL, "0"},
            token_expr::token{token_expr::token_type::CLOSE_BR, ""}};
    parser.set_tokens(tk_s);

    ASSERT_NO_THROW({
        parser.analysis();
        ASSERT_EQ(parser.get_exprs()[0].expr_func.func_t.token_t, token_expr::token_type::ID);
        ASSERT_EQ(parser.get_exprs()[0].sub_expr_s[0].token_of_subexpr[0].token_t, token_expr::token_type::LITERAL);
    });

    tk_s = {token_expr::token{token_expr::token_type::ID, "test_func1"},
            token_expr::token{token_expr::token_type::OPEN_BR, ""},
            token_expr::token{token_expr::token_type::LITERALS, "test string"},
            token_expr::token{token_expr::token_type::COMMA, ""},
            token_expr::token{token_expr::token_type::LITERAL, "0"},
            token_expr::token{token_expr::token_type::COMMA, ""},
            token_expr::token{token_expr::token_type::LITERALS, "string1"},
            token_expr::token{token_expr::token_type::OPERATOR, "+"},
            token_expr::token{token_expr::token_type::LITERALS, "string2"},
            token_expr::token{token_expr::token_type::COMMA, ""},
            token_expr::token{token_expr::token_type::LITERAL, "1"},
            token_expr::token{token_expr::token_type::OPERATOR, ">"},
            token_expr::token{token_expr::token_type::LITERAL, "0"},
            token_expr::token{token_expr::token_type::CLOSE_BR, ""}};
    parser.set_tokens(tk_s);

    ASSERT_NO_THROW({
        parser.analysis();
        ASSERT_EQ(parser.get_exprs()[0].expr_func.func_t.token_t, token_expr::token_type::ID);
        ASSERT_EQ(parser.get_exprs()[0].sub_expr_s[0].token_of_subexpr[0].token_t, token_expr::token_type::LITERALS);
        ASSERT_EQ(parser.get_exprs()[0].sub_expr_s[1].token_of_subexpr[0].token_t, token_expr::token_type::LITERAL);
        ASSERT_EQ(parser.get_exprs()[0].sub_expr_s[2].token_of_subexpr[0].token_val, "string1string2");
        ASSERT_EQ(parser.get_exprs()[0].sub_expr_s[3].token_of_subexpr[0].token_val, "1");
    });
}
