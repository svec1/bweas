//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWLANG__H
#define BWLANG__H

#include "bw_defs.hpp"

#include <lang/static_linking_func.hpp>

namespace bweas {

// A wrapper around the bwlang interpreter that installs all
// the standard bweas functions and also provides interaction with the global scope
class bwlang {
  public:
    bwlang(std::string_view bwconf_file = MAIN_FILE);

    bwlang(bwlang &&)            = delete;
    bwlang(const bwlang &)       = delete;
    bwlang &operator=(bwlang &&) = delete;

  private:
    // Sets standard functions and keyword operators corresponding to the bweas specification
    void init_scope();

  public:
    // Starts the internal interpreter
    void execute();

    // Loads external functions (passed into this function) into the interpreter (semantic analyzer)
    inline void init_external_funcs(std::vector<decl_func> funcs);

    // Sets custom extension fields for projects
    inline void set_custom_ext_fields_project(std::map<std::string, std::string> custom_ext_fields);

    template <typename T> inline bool create_global_var(std::string name_var, T val = {}) {
        return _interpreter.get_current_scope().try_create_var(name_var, val);
    }
    template <typename T> inline T get_global_var(std::string name_var) {
        return _interpreter.get_current_scope().get_var_value<T>(name_var);
    }

    var::scope &get_global_scope();
    template <typename T> std::vector<std::pair<std::string, T>> &get_class_variables();

    bw_context get_context();

  private:
    std::vector<var::struct_sb::target_out> get_targets();
    std::vector<var::struct_sb::template_command> get_templates();
    std::vector<var::struct_sb::call_component> get_call_components();
    std::vector<std::pair<std::string, std::string>> get_global_external_args();

  private:
    interpreter _interpreter;
    bw_context context;
};
bwlang::bwlang(std::string_view bwconf_file) : _interpreter(bwconf_file) {
    init_scope();
}

void bwlang::init_scope() {
    _interpreter.get_current_scope().create_var<i32t>("DEBUG", 0);
    _interpreter.get_current_scope().create_var<i32t>("RELEASE", 1);

    _interpreter.create_function(
        "set", sl_func::set,
        {param_type::NCHECK_VAR_ID, param_type::ANY_VALUE_WITHOUT_FUTUREID_NEXT, param_type::NEXT_TOO});
    _interpreter.create_function("file", sl_func::file,
                                 {{param_type::FUTURE_VAR_ID, "{NULL}"},
                                  param_type::LNUM_OR_ID_VAR,
                                  param_type::LSTR_OR_ID_VAR,
                                  param_type::NEXT_TOO});

    _interpreter.create_function(
        "project", sl_func::project,
        {param_type::FUTURE_VAR_ID, param_type::LNUM_OR_ID_VAR, param_type::LSTR_OR_ID_VAR, param_type::NEXT_TOO});
    _interpreter.create_function("executable", sl_func::executable,
                                 {param_type::FUTURE_VAR_ID, param_type::LNUM_OR_ID_VAR, param_type::VAR_STRUCT_ID});

    _interpreter.create_function("link_lib", sl_func::link_lib,
                                 {param_type::VAR_STRUCT_ID, param_type::LSTR_OR_ID_VAR, param_type::NEXT_TOO});
    _interpreter.create_function("exp_data", sl_func::exp_data, {param_type::LSTR_OR_ID_VAR});
    _interpreter.create_function("cmd", sl_func::cmd, {param_type::LNUM_OR_ID_VAR, param_type::LSTR_OR_ID_VAR});
    _interpreter.create_function("debug", sl_func::debug, {param_type::LSTR_OR_ID_VAR, param_type::NEXT_TOO});
    _interpreter.create_function("debug_struct", sl_func::debug_struct, {param_type::VAR_STRUCT_ID});

    _interpreter.create_function(
        "flags_compiler", sl_func::flags_compiler,
        {param_type::VAR_STRUCT_ID, param_type::LNUM_OR_ID_VAR, param_type::LSTR_OR_ID_VAR, param_type::NEXT_TOO});
    _interpreter.create_function(
        "flags_linker", sl_func::flags_linker,
        {param_type::VAR_STRUCT_ID, param_type::LNUM_OR_ID_VAR, param_type::LSTR_OR_ID_VAR, param_type::NEXT_TOO});

    _interpreter.create_function("path_compiler", sl_func::path_compiler,
                                 {param_type::VAR_STRUCT_ID, param_type::LSTR_OR_ID_VAR});
    _interpreter.create_function("path_linker", sl_func::path_linker,
                                 {param_type::VAR_STRUCT_ID, param_type::LSTR_OR_ID_VAR});
    _interpreter.create_function("standart_c", sl_func::standart_c,
                                 {param_type::VAR_STRUCT_ID, param_type::LNUM_OR_ID_VAR});
    _interpreter.create_function("standart_cpp", sl_func::standart_cpp,
                                 {param_type::VAR_STRUCT_ID, param_type::LNUM_OR_ID_VAR});
    _interpreter.create_function("lang", sl_func::lang, {param_type::VAR_STRUCT_ID, param_type::LSTR_OR_ID_VAR});

    _interpreter.create_function("generator", sl_func::generator,
                                 {param_type::VAR_STRUCT_ID, param_type::LSTR_OR_ID_VAR});

    _interpreter.create_function("create_templates", sl_func::create_templates,
                                 {param_type::FUTURE_VAR_ID, param_type::LSTR_OR_ID_VAR});
    _interpreter.create_function("create_call_component", sl_func::create_call_component,
                                 {param_type::FUTURE_VAR_ID, param_type::LSTR_OR_ID_VAR, param_type::LSTR_OR_ID_VAR});
    _interpreter.create_function("add_param_template", sl_func::add_param_template,
                                 {param_type::FUTURE_VAR_ID, param_type::VAR_ID});
    _interpreter.create_function("use_templates", sl_func::use_templates,
                                 {param_type::VAR_STRUCT_ID, param_type::LSTR_OR_ID_VAR, param_type::NEXT_TOO});
    _interpreter.create_function("use_it_template", sl_func::use_it_template,
                                 {param_type::VAR_STRUCT_ID, param_type::LNUM_OR_ID_VAR});
}

void bwlang::execute() {
    _interpreter.interpret();
}
void bwlang::init_external_funcs(std::vector<decl_func> funcs) {
    for (const auto &func : funcs)
        _interpreter.create_function(func);
}
void bwlang::set_custom_ext_fields_project(std::map<std::string, std::string> custom_ext_fields) {
    var::struct_sb::project::preset_ext_fields.merge(custom_ext_fields);
}

bw_context bwlang::get_context() {
    context.out_targets          = get_targets();
    context.templates            = get_templates();
    context.call_components      = get_call_components();
    context.global_external_args = get_global_external_args();

    return context;
}

std::vector<var::struct_sb::target_out> bwlang::get_targets() {
    std::vector<var::struct_sb::target> targets = _interpreter.export_targets();

    std::vector<var::struct_sb::target_out> targets_o;
    var::struct_sb::target_out target_tmp;

    for (u32t i = 0; i < targets.size(); ++i) {
        target_tmp.name_target     = targets[i].name_target;
        target_tmp.target_t        = targets[i].target_t;
        target_tmp.target_cfg      = targets[i].target_cfg;
        target_tmp.version_target  = targets[i].version_target;
        target_tmp.name_generator  = targets[i].name_generator;
        target_tmp.target_vec_libs = targets[i].target_vec_libs;
        target_tmp.prj             = *targets[i].prj;

        targets_o.push_back(target_tmp);
    }

    return targets_o;
}

std::vector<var::struct_sb::template_command> bwlang::get_templates() {
    std::vector<std::pair<std::string, var::struct_sb::template_command>> templates =
        _interpreter.get_current_scope().get_vector_variables_t<var::struct_sb::template_command>();

    std::vector<var::struct_sb::template_command> templates_out;
    for (const auto &_template : templates)
        templates_out.emplace_back(_template.second);

    return templates_out;
}

std::vector<var::struct_sb::call_component> bwlang::get_call_components() {
    std::vector<std::pair<std::string, var::struct_sb::call_component>> call_components =
        _interpreter.get_current_scope().get_vector_variables_t<var::struct_sb::call_component>();

    std::vector<var::struct_sb::call_component> call_components_out;
    for (const auto &_template : call_components)
        call_components_out.emplace_back(_template.second);

    return call_components_out;
}

std::vector<std::pair<std::string, std::string>> bwlang::get_global_external_args() {
    std::vector<std::pair<std::string, std::pair<std::string, std::string>>> global_external_args =
        _interpreter.get_current_scope().get_vector_variables_t<std::pair<std::string, std::string>>();

    std::vector<std::pair<std::string, std::string>> global_external_args_out;
    for (const auto &global_external_arg : global_external_args)
        global_external_args_out.emplace_back(global_external_arg.second);

    return global_external_args_out;
}

var::scope &bwlang::get_global_scope() {
    return _interpreter.get_current_scope();
}
template <typename T> std::vector<std::pair<std::string, T>> &bwlang::get_class_variables() {
    return _interpreter.get_current_scope().get_vector_variables_t<T>();
}

} // namespace bweas

#endif
