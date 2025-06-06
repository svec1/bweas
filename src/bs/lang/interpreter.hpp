#ifndef INTERPRETER__H
#define INTERPRETER__H

#include <lang/semantic_an.hpp>

class interpreter {
  public:
    interpreter(string_v name_file);

    interpreter(interpreter &&)            = delete;
    interpreter(const interpreter &)       = delete;
    interpreter &operator=(interpreter &&) = delete;

    ~interpreter() = default;

  public:
    // Interpretation of functions (not declaring, and which are not
    // explicitly marked as called in the semantic analysis)
    void interpret();

    void set_external_scope(var::scope *_external_scope);
    var::scope &get_current_scope();

    vec<var::struct_sb::target> export_targets();

    void create_function(const decl_func &func);
    void create_function(string_v name_func, decl_func::func_t func, vec<param> expected_params);

  private:
    var::scope global_scope;
    var::scope *external_scope = (var::scope *)&global_scope;

    semantic_analyzer smt_analyzer;
};

#endif
