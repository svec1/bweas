//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses />
// ------------------------------------------
//

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

    void set_scope(scope *external_scope);

    scope &get_scope();
    const scope &get_scope() const;

    void create_function(const decl_func &func);
    void create_function(string_v name_func, decl_func::func_t func, vec<param> expected_params);

  private:
    scope global_scope;
};

#endif
