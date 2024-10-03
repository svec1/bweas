#ifndef _STATIC_LINKING_FUNC__H
#define _STATIC_LINKING_FUNC__H

#include "parser.hpp"

// all static global functions of the build system
// ----------------------------------------------
// - set, project, executable, link_lib, exp_data, cmd,
// debug, debug_struct, flags_compiler, flags_linker, path_compiler,
// path_linker, standart_c, standart_cpp, lang,
// add_param_template, use_tamplates, use_it_template
namespace sl_func {

using func_decl = void (*)(const std::vector<aef_expr::subexpressions> &sub_expr, var::scope &curr_scope);

// list of all static global functions that
// should be called in semantic analysis (second pass)
extern const std::vector<std::string> name_static_func_sm;

// A function that creates or modifies (if it already exists) a variable.
// Cannot work with variables that are structural (project, target)
extern void
set(const std::vector<aef_expr::subexpressions> &sub_expr, var::scope &curr_scope);

// Creates a project with a specific language and source files
extern void
project(const std::vector<aef_expr::subexpressions> &sub_expr, var::scope &curr_scope);

// Creates a target with a specific configuration (RELEASE/DEBUG)
// and a main project
extern void
executable(const std::vector<aef_expr::subexpressions> &sub_expr, var::scope &curr_scope);

// Adds the specified libraries to the list of dependencies of the target (library)
extern void
link_lib(const std::vector<aef_expr::subexpressions> &sub_expr, var::scope &curr_scope);

// Exports all created variables or structures (by creating another
// interpreter for the file)
// from a file in another project using the same build system
extern void
exp_data(const std::vector<aef_expr::subexpressions> &sub_expr, var::scope &curr_scope);

// Executes the passed string in cmd
extern void
cmd(const std::vector<aef_expr::subexpressions> &sub_expr, var::scope &curr_scope);

// Outputs the passed text to the console
extern void
debug(const std::vector<aef_expr::subexpressions> &sub_expr, var::scope &curr_scope);

// Outputs the passed info of struct(project, target) to the console
extern void
debug_struct(const std::vector<aef_expr::subexpressions> &sub_expr, var::scope &curr_scope);

// Sets compiler flags for the passed project depending on the configuration
extern void
flags_compiler(const std::vector<aef_expr::subexpressions> &sub_expr, var::scope &curr_scope);

// Sets linker flags for the passed project depending on the configuration
extern void
flags_linker(const std::vector<aef_expr::subexpressions> &sub_expr, var::scope &curr_scope);

// Sets the path to the compiler in project which was passed
extern void
path_compiler(const std::vector<aef_expr::subexpressions> &sub_expr, var::scope &curr_scope);

// Sets the path to the linker in project which was passed
extern void
path_linker(const std::vector<aef_expr::subexpressions> &sub_expr, var::scope &curr_scope);

// Sets the standard of the C language in project which was passed
extern void
standart_c(const std::vector<aef_expr::subexpressions> &sub_expr, var::scope &curr_scope);

// Sets the standard of the C++ language in project which was passed
extern void
standart_cpp(const std::vector<aef_expr::subexpressions> &sub_expr, var::scope &curr_scope);

// Sets the language of the project that was passed
extern void
lang(const std::vector<aef_expr::subexpressions> &sub_expr, var::scope &curr_scope);

// Creates a parameter declaration for a command template (variable)
// that refers to another variable. Therefore, the parameter for the template
// command will depend on the passed variable
extern void
add_param_template(const std::vector<aef_expr::subexpressions> &sub_expr, var::scope &curr_scope);

// Creates a template that can be used to generate commands
// ----------
// syntax: NAME_CALL_COMPONENT: <param1>,<param2>,<param3>...
// If the call component does not exist, it is created.
// Available components: COMPILER, LINKER, ARCHIVER, INTERPRETER
extern void
create_templates(const std::vector<aef_expr::subexpressions> &sub_expr, var::scope &curr_scope);

// Adds the name of the template that will be used when generating commands
extern void
use_templates(const std::vector<aef_expr::subexpressions> &sub_expr, var::scope &curr_scope);

// Sets the flag of the passed project, whether to use templates for generating commands
extern void
use_it_template(const std::vector<aef_expr::subexpressions> &sub_expr, var::scope &curr_scope);

} // namespace sl_func

#endif