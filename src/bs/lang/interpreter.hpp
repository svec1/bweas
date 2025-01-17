#ifndef INTERPRETER__H
#define INTERPRETER__H

#include "semantic_an.hpp"

namespace interpreter {

#define debug_init_time() clock_t beg;

#define debug_mark_time_func(func, func_name)                                                                          \
    beg = clock();                                                                                                     \
    func;                                                                                                              \
    assist << std::string("Run time " func_name ": " + std::to_string((double)(clock() - beg) / CLOCKS_PER_SEC));

class realtime_excp : public ::bwexception::bweas_exception {
  public:
    realtime_excp(std::string _what_hp, std::string number_err)
        : what_hp(_what_hp), bweas_exception("RTT" + number_err) {
    }
    ~realtime_excp() noexcept override final = default;

  public:
    const char *what() const noexcept override final {
        return what_hp.c_str();
    }

  private:
    std::string what_hp;
};

class interpreter_exec {
  public:
    struct config {
        // output:
        // - tree after semanticanalyzer
        // - run time every of components interpreter(lexer, parser...)
        bool debug_output{0};

        // whether to use external scope
        bool use_external_scope{0};

        // whether to pass the list of external functions that
        // should be called in semantic analysis to semantic analysis
        bool transmit_smt_name_func_with_smt{0};

        std::string filename_interp;
    };

  public:
    interpreter_exec();
    interpreter_exec(config conf);

    interpreter_exec(interpreter_exec &&) = delete;
    interpreter_exec(const interpreter_exec &) = delete;
    interpreter_exec &operator=(interpreter_exec &&) = delete;

    ~interpreter_exec() = default;

  public:
    // Compiles an abstract expression of function
    void build_aef();

    // Interpretation of functions (not declaring, and which are not
    // explicitly marked as called in the semantic analysis)
    void interpret();

    void set_config(config conf);
    void set_external_scope(var::scope *_external_scope);

    std::vector<var::struct_sb::target> export_targets();

    var::scope &get_current_scope();

    void load_external_func(const semantic_an::table_func &tfuncs);
    void set_std_function(std::string name_token_func, aef_expr::notion_func::func_t func_ref,
                          std::vector<aef_expr::param> expected_param);

  private:
    static inline bool init_glob{0};

    config interp_conf;

    var::scope global_scope;
    var::scope *external_scope{&global_scope};

    lexer::lex_an lexer;
    parser::pars_an parser;
    semantic_an::semantic_analyzer smt_analyzer;

    parser::abstract_expr_func aef;
    semantic_an::table_func external_func_table;

    u32t curr_expr{0};
};
} // namespace interpreter

#endif