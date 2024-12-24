#ifndef _SEMANTIC_AN__H
#define _SEMANTIC_AN__H

#include "parser.hpp"
#include "scope.hpp"
#include "static_linking_func.hpp"

#include <unordered_map>

namespace semantic_an {

using table_func = std::unordered_map<std::string, aef_expr::notion_func>;

extern var::scope global_scope;

class semantic_excp : public bw_excp::bweas_exception {
  public:
    semantic_excp(std::string _what_hp, std::string number_err)
        : what_hp(_what_hp), bweas_exception("SMT" + number_err) {
    }
    ~semantic_excp() noexcept override final = default;

  public:
    const char *what() const noexcept override final {
        return what_hp.c_str();
    }

  private:
    std::string what_hp;
};

class rt_semantic_excp : public bw_excp::bweas_exception {
  public:
    rt_semantic_excp(std::string _what_hp, std::string number_err)
        : what_hp(_what_hp), bweas_exception("SMT-RT" + number_err) {
    }
    ~rt_semantic_excp() noexcept override final = default;

  public:
    const char *what() const noexcept override final {
        return what_hp.c_str();
    }

  private:
    std::string what_hp;
};

class semantic_analyzer {
  public:
    semantic_analyzer();
    semantic_analyzer(semantic_analyzer &&) = delete;
    semantic_analyzer(const semantic_analyzer &) = delete;
    semantic_analyzer &operator=(semantic_analyzer &&) = delete;

    ~semantic_analyzer() = default;

  public:
    void add_standart_function();

  public:
    // Semantic analysis:
    // ------------------
    //  - It goes through the vector of expressions
    //    and determines the use of external functions;
    //    if an external function was used,
    //    it will be added to the list of functions that can be called
    //
    //    ^^^
    //    zero pass
    //
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
    void analysis(parser::abstract_expr_func &expr_s, var::scope &global_scope);

    void load_external_func_table(const table_func &notion_external_func);
    void append_external_name_func_w_smt(const std::vector<std::string> &list_name_func);

  private:
    // Adding a function definition to the functions table
    void add_func_flink(std::string name_token_func, aef_expr::notion_func::func_t func_ref,
                        std::vector<aef_expr::param> expected_param);

    void smt_zero_pass(const parser::abstract_expr_func &expr_s);
    void smt_first_pass(parser::abstract_expr_func &expr_s);

    // The set command (initialization or assignment) is called here
    void smt_second_pass(parser::abstract_expr_func &expr_s, var::scope &curr_scope);

  private:
    // Converts all variable identifiers to their values and also keyword operators(constant) to literal ​​(for
    // subsequent processing of keyword operators)
    void convert_id_to_literal(aef_expr::subexpressions &sub_expr, var::scope &curr_scope,
                               aef_expr::param_type expected_param);

    // Converts all keyword operators that are unary
    void convert_kwop_u_to_literal(aef_expr::subexpressions &sub_expr, var::scope &curr_scope,
                                   aef_expr::param_type expected_param);
    // Parses a subexpression if it has not token the type
    // INT, STRING, or VAR_STRUCT_ID after parsing at the AEF construction
    void parse_subexpr_param(aef_expr::subexpressions &sub_expr, std::vector<aef_expr::subexpressions> &sub_exprs,
                             u32t &pos_sub_expr_in_vec, var::scope &curr_scope, aef_expr::param_type expected_param);

    // Auxiliary function for parse_subexpr_param. Processes all keyword operators
    void parse_keywords_op_param(aef_expr::subexpressions &sub_expr, u32t pos_token_kw_in_subexpr,
                                 var::scope &curr_scope, aef_expr::param_type expected_param, u32t parse_okeyword = 0);
    void defining_call_func(const std::string &name, aef_expr::notion_func &nfunc);

  private:
    static inline bool init_glob{0};

    std::vector<std::string> name_func_with_semantic_an{sl_func::name_static_func_sm};

    // functions tables
    table_func notion_all_func;
    table_func notion_external_func;
};

} // namespace semantic_an

#endif