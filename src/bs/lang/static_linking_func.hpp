#ifndef _STATIC_LINKING_FUNC__H
#define _STATIC_LINKING_FUNC__H

#include <bwlogger.hpp>
#include <lang/expression.hpp>

// all static global functions of the build system
// ----------------------------------------------
// - set, project, executable, link_lib, exp_data, cmd,
// debug, debug_struct, flags_compiler, flags_linker, path_compiler,
// path_linker, standart_c, standart_cpp, lang, generator,
// add_param_template, use_tamplates, use_it_template
namespace sl_func {

extern "C++"
{

    // A function that creates or modifies (if it already exists) a variable.
    // Cannot work with variables that are structural (project, target)
    void set(const expressions &expr_s, var::scope &current_scope);

    // A function that finds (or performs other actions) files,
    // defining paths for them, to prevent problems with the subsequent build of targets.
    // Functions:
    //  1. PATH
    //  2. FIND
    //  3. COPY
    void file(const expressions &expr_s, var::scope &current_scope);

    // Creates a project with a specific language and source files
    void project(const expressions &expr_s, var::scope &current_scope);

    // Creates a target with a specific configuration (RELEASE/DEBUG)
    // and a main project
    void executable(const expressions &expr_s, var::scope &current_scope);

    // Adds the specified libraries to the list of dependencies of the target (library)
    void link_lib(const expressions &expr_s, var::scope &current_scope);

    // Exports all created variables or structures (by creating another
    // interpreter for the file)
    // from a file in another project using the same build system
    void exp_data(const expressions &expr_s, var::scope &current_scope);

    // Executes the passed string in cmd
    void cmd(const expressions &expr_s, var::scope &current_scope);

    // Outputs the passed text to the console
    void debug(const expressions &expr_s, var::scope &current_scope);

    // Outputs the passed info of struct(project, target) to the console
    void debug_struct(const expressions &expr_s, var::scope &current_scope);

    // Sets compiler flags for the passed project depending on the configuration
    void flags_compiler(const expressions &expr_s, var::scope &current_scope);

    // Sets linker flags for the passed project depending on the configuration
    void flags_linker(const expressions &expr_s, var::scope &current_scope);

    // Sets the path to the compiler in project which was passed
    void path_compiler(const expressions &expr_s, var::scope &current_scope);

    // Sets the path to the linker in project which was passed
    void path_linker(const expressions &expr_s, var::scope &current_scope);

    // Sets the standard of the C language in project which was passed
    void standart_c(const expressions &expr_s, var::scope &current_scope);

    // Sets the standard of the C++ language in project which was passed
    void standart_cpp(const expressions &expr_s, var::scope &current_scope);

    // Sets the language of the project that was passed
    void lang(const expressions &expr_s, var::scope &current_scope);

    // Sets the base generator for the target that was passed.
    void generator(const expressions &expr_s, var::scope &current_scope);

    // Creates a parameter declaration for a command template (variable)
    // that refers to another variable. Therefore, the parameter for the template
    // command will depend on the passed variable
    void add_param_template(const expressions &expr_s, var::scope &current_scope);

    // Creates a template that can be used to generate commands
    // ----------
    // #### NAME_CALL_COMPONENT(ACP1, ...) -> return NAME_smt: <param1>,<param2>,<{ACP1}>, <[FIELD_TARGET_OR_PROJECT]>,
    // FEATURES_CURRENT_GENERATOR ... Available components: COMPILER C/C++, LINKER C/C++, ARCHIVER, INTERPRETER,
    // [user]... NAME_ret is file or list of files
    void create_templates(const expressions &expr_s, var::scope &current_scope);

    // Creates a component of call
    // ----------
    // Creates a call component with the name of the program that will be called
    // when using this component, and also determines the pattern of files in the output of the program
    void create_call_component(const expressions &expr_s, var::scope &current_scope);

    // Adds the name of the template that will be used when generating commands
    void use_templates(const expressions &expr_s, var::scope &current_scope);
}

} // namespace sl_func

#endif
