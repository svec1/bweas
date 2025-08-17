//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses />
// ------------------------------------------
//

#ifndef _SEMANTIC_AN__H
#define _SEMANTIC_AN__H

#include <lang/scope.hpp>

class semantic_analyzer {
  public:
    semantic_analyzer(bweas::logger &_log);

    semantic_analyzer(semantic_analyzer &&)            = delete;
    semantic_analyzer(const semantic_analyzer &)       = delete;
    semantic_analyzer &operator=(semantic_analyzer &&) = delete;

    ~semantic_analyzer() = default;

  public:
    // Semantic analysis:
    // ------------------
    // - linking all representations of functions
    //   in expressions with functions that describe their behavior.
    //   Checks the correspondence of the transmitted types of indicators
    //   with the expected types of the function.
    //
    //   ^^^
    //   first pass
    //
    // - creates a *table* of symbols (creates, in the scope
    //   of the current file that is being processed, all
    //   variables that were declared independently, through the set command)
    //   and checks the correspondence between indicators.
    //
    //   ^^^
    //   second pass
    //
    void analysis(statements &st_s, scope &current_scope);

  private:
    void smt_first_pass(statements &st_s, scope &current_scope);

    // The set command (initialization or assignment) is called here
    void smt_second_pass(statements &st_s, scope &current_scope);

  private:
    // Parses a subexpression if it has not token the type
    // INT, STRING, or VAR_STRUCT_ID after parsing at the AEF construction
    void parse_expr_param(expression &expr, expressions &expr_s, size_t &pos_expr_in_vec, scope &current_scope);

  private:
    bweas::logger &_log;
};

#endif
