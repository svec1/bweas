//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses />
// ------------------------------------------
//

#ifndef _STATIC_LINKING_FUNC__H
#define _STATIC_LINKING_FUNC__H

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
    void set(const expressions &expr_s, scope &current_scope);

    // A function that finds (or performs other actions) files,
    // defining paths for them, to prevent problems with the subsequent build of targets.
    // Functions:
    //  1. PATH
    //  2. FIND
    //  3. COPY
    void file(const expressions &expr_s, scope &current_scope);

    // Creates a project with a specific language and source files
    void project(const expressions &expr_s, scope &current_scope);

    // Creates a target with a specific configuration (RELEASE/DEBUG)
    // and a main project
    void create_target(const expressions &expr_s, scope &current_scope);

    // Adds the specified libraries to the list of dependencies of the target (library)
    void add_dependencies_target(const expressions &expr_s, scope &current_scope);

    // Exports all created variables or structures (by creating another
    // interpreter for the file)
    // from a file in another project using the same build system
    void exp_data(const expressions &expr_s, scope &current_scope);

    // Outputs the passed text to the console
    void debug(const expressions &expr_s, scope &current_scope);

    // Outputs the passed info of struct(project, target) to the console
    void debug_struct(const expressions &expr_s, scope &current_scope);

    // Sets compiler flags for the passed project depending on the configuration
    void flags_compiler(const expressions &expr_s, scope &current_scope);

    // Sets linker flags for the passed project depending on the configuration
    void flags_linker(const expressions &expr_s, scope &current_scope);

    // Sets the path to the compiler in project which was passed
    void path_compiler(const expressions &expr_s, scope &current_scope);

    // Sets the path to the linker in project which was passed
    void path_linker(const expressions &expr_s, scope &current_scope);

    // Sets the standard of the C language in project which was passed
    void standart_c(const expressions &expr_s, scope &current_scope);

    // Sets the standard of the C++ language in project which was passed
    void standart_cpp(const expressions &expr_s, scope &current_scope);

    // Sets include directories for header files
    void include_directories(const expressions &expr_s, scope &current_scope);

    // Sets the language of the project that was passed
    void lang(const expressions &expr_s, scope &current_scope);

    // Sets the base generator for the target that was passed.
    void generator(const expressions &expr_s, scope &current_scope);

    // Creates a parameter declaration for a command template (variable)
    // that refers to another variable. Therefore, the parameter for the template
    // command will depend on the passed variable
    void add_param_template(const expressions &expr_s, scope &current_scope);

    // Creates a template that can be used to generate commands
    void create_template(const expressions &expr_s, scope &current_scope);

    // Creates a component of call
    // ----------
    // Creates a call component with the name of the program that will be called
    // when using this component, and also determines the pattern of files in the output of the program
    void create_call_component(const expressions &expr_s, scope &current_scope);

    // Adds the name of the template that will be used when generating commands
    void use_templates(const expressions &expr_s, scope &current_scope);
}

} // namespace sl_func

#endif
