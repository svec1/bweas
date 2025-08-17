#ifndef LANG_TESTS__H_
#define LANG_TESTS__H_

#include <lang/scope.hpp>

static bweas::logger _log{"BWTEST_LANG"};
static scope global_scope{_log};

extern void init_buffer(string_v src);
extern void delete_buffer();
extern int yyparse(void);

extern bweas::logger *log_bison;
extern scope *current_scope;
extern statements stm_s;

statements parse(string_v src) {
    if (!log_bison)
        log_bison = &_log;

    init_buffer(src);
    yyparse();
    delete_buffer();

    statements tmp_stm_s = stm_s;
    stm_s.clear();

    return tmp_stm_s;
}

TEST(BWLANG, CorrectBuiltStatement) {
    statements tmp_stm_s;

    ASSERT_DEATH(tmp_stm_s = parse("fn1(0,1,2,\"3\",4+4*4)"), "Parsing error");
    ASSERT_EQ(tmp_stm_s[0].expr_func->name, "fn1");
    ASSERT_EQ(tmp_stm_s[0].expr_s.size(), 5);
    ASSERT_EQ(std::stoi(tmp_stm_s[0].expr_s[4].value), 20);
}

#endif
