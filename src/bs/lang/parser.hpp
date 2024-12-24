#ifndef _PARSER__H
#define _PARSER__H

#include "expression.hpp"
#include "lexer.hpp"
#include "scope.hpp"

#include <sstream>
#include <vector>

namespace parser {

// abstract expression of functions
using abstract_expr_func = std::vector<aef_expr::expression>;

// auxiliary functions for translating some structural types into an info string

namespace utility {

extern std::string type_token_str(token_expr::token_type token_t);
extern std::string type_ret_subexpr(aef_expr::subexpressions::ret_type_subexpr subexpr_ret_t);
extern std::string build_pos_tokenb_str(const token_expr::token &tk);
extern std::string build_pos_subexpr_str(const aef_expr::subexpressions &sub_expr);
extern std::string type_sybexpr_str(aef_expr::subexpressions::type_subexpr sub_expr_t);
extern std::string tree_build_visually_str(const abstract_expr_func &aef);

extern aef_expr::param_type type_param_in_str(std::string str);

} // namespace utility

class parser_excp : public bw_excp::bweas_exception {
  public:
    parser_excp(std::string _what_hp, std::string number_err)
        : what_hp(_what_hp), bweas_exception("PARS" + number_err) {
    }
    ~parser_excp() noexcept override final = default;

  public:
    const char *what() const noexcept override final {
        return what_hp.c_str();
    }

  private:
    std::string what_hp;
};

class pars_an {
  public:
    pars_an();
    pars_an(const std::vector<token_expr::token> &tk_s);

    pars_an(pars_an &&) = delete;
    pars_an(const pars_an &) = delete;

    pars_an &operator=(pars_an &&) = delete;

    ~pars_an() = default;

  public:
    void set_tokens(const std::vector<token_expr::token> &tk_s);
    abstract_expr_func get_exprs();
    void clear_aef();

    // Parsing
    // -------
    // ID(PARAM1, PARAM2, ...)
    //
    // -----------------------
    // It requires that a certain sequence of tokens begin with a keyword or function identifier,
    // that the enumeration of parameters begins with the token "(" and ends with ")",
    // when enumerating parameters, the separator is used ","
    abstract_expr_func &analysis();

  private:
    // Checks the correctness of the sequence of tokens in the current parameter
    void check_valid_subexpr_first_pass(const aef_expr::subexpressions &sub_expr);

    // Checks the syntactic correctness of the scripting language capabilities (sets rules),
    // and determines the syntactic type of the parameter.
    // It prohibits:
    //  1. Arithmetic operations
    //  2. Comparing strings or two different tokens by type (Number and String)
    void check_valid_subexpr_second_pass(aef_expr::subexpressions &sub_expr);

    // Tries to convert separate sequences of tokens into one
    void try_parse_subexpr(aef_expr::subexpressions &sub_expr);

  private:
    static inline bool init_glob{0};

    std::vector<token_expr::token> tokens;
    abstract_expr_func expr_s;
};

} // namespace parser

#endif