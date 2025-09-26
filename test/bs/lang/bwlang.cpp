#ifndef LANG_TESTS__H_
#define LANG_TESTS__H_

#include <gtest/gtest.h>
#include <iostream>

#include <lang/parser.hpp>

using namespace bwlang;

static constexpr auto TEST_CASE =
    "t_object"
    ": ctemplate ={str = 'clang++:{}.o(NULL) -> objects: -c [source_files:*.cpp] -o OUTPUT_FILE -I[include_paths] "
    "[compile_flags]'}\n"
    "t_lib: ctemplate = {str = 'ar:bweas.a(objects) -> LIBRARY: rcs OUTPUT_FILE {objects}'}\n"
    "t_exe: ctemplate ={str = ' clang++:{}(bweas_slib) -> EXECUTABLE: INPUT_FILE {bweas_slib} -o OUTPUT_FILE "
    "-I[include_paths] -l[libs] [compile_flags]'}\n"
    "lib_names : string[] = [ 'lz4', 'lua', 'luajit-5.1' ]\n"
    "target_compile_flags: string[]             = [ '-std=gnu++23', '-Wall', '-fPIC' ]\n"
    "target_include_paths: string[]             = [ '/usr/include', '/usr/local/include', '/src/bs' ]\n"
    "bweas_slib : target = {type          = 1,"
    "templates     = [ 't_object', 't_lib' ],"
    "compile_flags = target_compile_flags,"
    "include_paths = target_include_paths,\n"
    "source_files  = ['src/bs/*.cpp']}\n"
    "bweas_exe : target = {type          = 0, templates = ['t_exe'], dependencies = "
    "['bweas_slib'], libs = lib_names, compile_flags = target_compile_flags,include_paths = target_include_paths, "
    "source_files = ['src/bs/bweas.cpp']}\n";

static constexpr auto TEST_CASE_EXPR_1 = "(2+2*(2+2*2)*100)/2";
static constexpr auto TEST_CASE_EXPR_2 = "'Hello, ' + 'World!'";

class parser_test : public parser {
  public:
    parser_test(string_v src) : parser(src) {
    }

    parser_utils::value _parse_statements() {
        return parse_statements();
    }
    std::unique_ptr<expression::ext::base> _parse_expression() {
        return parse_expression();
    }
};
static parser_utils::value parse_expression(string_v src) {
    parser_test p(src);
    return p._parse_expression()->get_value();
}

TEST(BWLANG, Lexer) {
    vec<bwlang::tokens::token> tokens;
    ASSERT_NO_THROW({
        const lexer l(TEST_CASE);
        tokens = l.get_tokens();
    });
    ASSERT_EQ(tokens.size(), 145);
    EXPECT_TRUE(std::holds_alternative<tokens::init_type>(tokens[1]));
    EXPECT_TRUE(std::holds_alternative<tokens::keyword<>>(tokens[2]));
    ASSERT_EQ(std::get<tokens::keyword<>>(tokens[2]).value, "ctemplate");
    EXPECT_TRUE(std::holds_alternative<tokens::literal_string>(tokens[7]));
}
TEST(BWLANG, Parser) {

    ASSERT_NO_THROW({
        parser p(TEST_CASE);
        p.parse();
    });
    ASSERT_NO_THROW({
        auto expr1 = parse_expression(TEST_CASE_EXPR_1);
        auto expr2 = parse_expression(TEST_CASE_EXPR_2);

        EXPECT_TRUE(std::holds_alternative<pdiff>(expr1));
        EXPECT_TRUE(std::holds_alternative<string>(expr2));

        ASSERT_EQ(std::get<pdiff>(expr1), 601);
        ASSERT_EQ(std::get<string>(expr2), "Hello, World!");
    });
}

#endif
