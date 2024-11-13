#include <gtest/gtest.h>
#include <bs/lang/lexer.hpp>

TEST(BW_LEXER, CorrectOutputTokensClassBwLexer){
    ASSERT_NO_THROW({
        lexer::lex_an _lexer("set(1, \"0\", text)");
        _lexer.analysis();
        ASSERT_EQ(_lexer.get_tokens()[0], token_expr::token(token_expr::token_type::ID, "set", 1, 1));
        ASSERT_EQ(_lexer.get_tokens()[2], token_expr::token(token_expr::token_type::LITERAL, "1", 1, 5));
        ASSERT_EQ(_lexer.get_tokens()[6], token_expr::token(token_expr::token_type::ID, "text", 1, 11));
    });

    // Expected - KEYWORD 'if'
    ASSERT_NO_THROW({
        lexer::lex_an _lexer("if(true)");
        _lexer.analysis();
        ASSERT_EQ(_lexer.get_tokens()[0], token_expr::token(token_expr::token_type::KEYWORD, "if", 1, 1));
    });


    // Expected - OPEN_BR '(' and 12 tokens
    ASSERT_NO_THROW({
        lexer::lex_an _lexer("if(0, 0, 0, 0, 0)");
        _lexer.analysis();
        ASSERT_EQ(_lexer.get_tokens().size(), 12);
        ASSERT_EQ(_lexer.get_tokens()[1], token_expr::token(token_expr::token_type::OPEN_BR, "", 1, 1));
    });
}

TEST(BW_LEXER, CorrectInvalidSyntaxTokensClassBwLexer){
    ASSERT_THROW({
        lexer::lex_an _lexer("set(1var)");
        _lexer.analysis();
    }, lexer::lexer_excp);

    ASSERT_THROW({
        lexer::lex_an _lexer("set(text\")");
        _lexer.analysis();
    }, lexer::lexer_excp);
    
    ASSERT_THROW({
        lexer::lex_an _lexer("set(var, 2147483648)");
        _lexer.analysis();
    }, lexer::lexer_excp);
}