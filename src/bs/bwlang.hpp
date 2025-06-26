//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWLANG_HPP
#define BWLANG_HPP

#include <bw_defs.hpp>

#include <lang/static_linking_func.hpp>

namespace bweas {
class lang;
}

// A wrapper around the lang interpreter that installs all
// the standard bweas functions and also provides interaction with the global scope
class bweas::lang {
  public:
    lang(string_v bwconf_file = CONFIG_FILE);

    lang(lang &&)            = delete;
    lang(const lang &)       = delete;
    lang &operator=(lang &&) = delete;

  private:
    // Sets standard functions and keyword operators corresponding to the bweas specification
    void init_scope();

  public:
    // Starts the internal interpreter
    void execute();

    // Loads external functions (passed into this function) into the interpreter (semantic analyzer)
    inline void init_external_funcs(vec<decl_func> funcs);

    // Sets custom extension fields for projects
    inline void set_custom_ext_fields_project(map<string, string> custom_ext_fields);

    template <typename T> inline bool create_global_var(string name_var, T val = {}) {
        return _interpreter.get_scope().try_create_var(name_var, val);
    }
    template <typename T> inline T get_global_var(string name_var) {
        return _interpreter.get_scope().get_var_value<T>(name_var);
    }

    scope &get_global_scope();
    template <typename T> vec<pair<string, T>> &get_class_variables();

    bw_context get_context();

  private:
    vec<sc::target_out> get_targets();
    vec<sc::template_command> get_templates();
    vec<sc::call_component> get_call_components();
    vec<pair<string, string>> get_global_external_args();

  private:
    interpreter _interpreter;
    bw_context context;
};
lang::lang(string_v bwconf_file) : _interpreter(bwconf_file) {
    init_scope();
}

void lang::init_scope() {
    _interpreter.get_scope().create_var<pdiff>("DEBUG", 0);
    _interpreter.get_scope().create_var<pdiff>("RELEASE", 1);

    _interpreter.create_function(
        "set", sl_func::set,
        {param_type::NCHECK_VAR_ID, param_type::ANY_VALUE_WITHOUT_FUTUREID_NEXT, param_type::NEXT_TOO});
    _interpreter.create_function("file", sl_func::file,
                                 {{param_type::FUTURE_VAR_ID, "{NULL}"},
                                  param_type::LNUM_OR_ID_VAR,
                                  param_type::LSTR_OR_ID_VAR,
                                  param_type::NEXT_TOO});

    _interpreter.create_function("project", sl_func::project,
                                 {param_type::FUTURE_VAR_ID,
                                  {param_type::LNUM_OR_ID_VAR, "1"},
                                  param_type::LSTR_OR_ID_VAR,
                                  param_type::NEXT_TOO});
    _interpreter.create_function("executable", sl_func::executable,
                                 {param_type::FUTURE_VAR_ID, param_type::LNUM_OR_ID_VAR, param_type::VAR_STRUCT_ID});

    _interpreter.create_function("link_lib", sl_func::link_lib,
                                 {param_type::VAR_STRUCT_ID, param_type::LSTR_OR_ID_VAR, param_type::NEXT_TOO});
    _interpreter.create_function("exp_data", sl_func::exp_data, {param_type::LSTR_OR_ID_VAR});
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

    _interpreter.create_function("include_directories", sl_func::include_directories,
                                 {param_type::VAR_STRUCT_ID, param_type::LSTR_OR_ID_VAR, param_type::NEXT_TOO});

    _interpreter.create_function("lang", sl_func::lang, {param_type::VAR_STRUCT_ID, param_type::LSTR_OR_ID_VAR});

    _interpreter.create_function("generator", sl_func::generator,
                                 {param_type::VAR_STRUCT_ID, param_type::LSTR_OR_ID_VAR});

    _interpreter.create_function("create_template", sl_func::create_template,
                                 {param_type::FUTURE_VAR_ID, param_type::LSTR_OR_ID_VAR});
    _interpreter.create_function("create_call_component", sl_func::create_call_component,
                                 {param_type::FUTURE_VAR_ID, param_type::LSTR_OR_ID_VAR, param_type::LSTR_OR_ID_VAR});
    _interpreter.create_function("add_param_template", sl_func::add_param_template,
                                 {param_type::FUTURE_VAR_ID, param_type::VAR_ID});
    _interpreter.create_function("use_templates", sl_func::use_templates,
                                 {param_type::VAR_STRUCT_ID, param_type::LSTR_OR_ID_VAR, param_type::NEXT_TOO});
}

void lang::execute() {
    _interpreter.interpret();
}
void lang::init_external_funcs(vec<decl_func> funcs) {
    for (const auto &func : funcs)
        _interpreter.create_function(func);
}
void lang::set_custom_ext_fields_project(map<string, string> custom_ext_fields) {
    sc::project::preset_ext_fields.merge(custom_ext_fields);
}

bw_context lang::get_context() {
    context.out_targets          = get_targets();
    context.templates            = get_templates();
    context.call_components      = get_call_components();
    context.global_external_args = get_global_external_args();

    return context;
}

vec<sc::target_out> lang::get_targets() {
    vec<sc::target> targets = _interpreter.export_targets();

    vec<sc::target_out> targets_o;
    sc::target_out target_tmp;

    for (size_t i = 0; i < targets.size(); ++i) {
        target_tmp.name            = targets[i].name;
        target_tmp.type            = targets[i].type;
        target_tmp.cfg             = targets[i].cfg;
        target_tmp.version         = targets[i].version;
        target_tmp.name_generator  = targets[i].name_generator;
        target_tmp.target_vec_libs = targets[i].target_vec_libs;
        target_tmp.prj             = *targets[i].prj;

        targets_o.push_back(target_tmp);
    }

    return targets_o;
}

vec<sc::template_command> lang::get_templates() {
    vec<pair<string, sc::template_command>> templates =
        _interpreter.get_scope().get_vector_variables_t<sc::template_command>();

    vec<sc::template_command> templates_out;
    for (const auto &_template : templates)
        templates_out.emplace_back(_template.second);

    return templates_out;
}

vec<sc::call_component> lang::get_call_components() {
    vec<pair<string, sc::call_component>> call_components =
        _interpreter.get_scope().get_vector_variables_t<sc::call_component>();

    vec<sc::call_component> call_components_out;
    for (const auto &_template : call_components)
        call_components_out.emplace_back(_template.second);

    return call_components_out;
}

vec<pair<string, string>> lang::get_global_external_args() {
    vec<pair<string, pair<string, string>>> global_external_args =
        _interpreter.get_scope().get_vector_variables_t<pair<string, string>>();

    vec<pair<string, string>> global_external_args_out;
    for (const auto &global_external_arg : global_external_args)
        global_external_args_out.emplace_back(global_external_arg.second);

    return global_external_args_out;
}

scope &lang::get_global_scope() {
    return _interpreter.get_scope();
}
template <typename T> vec<pair<string, T>> &lang::get_class_variables() {
    return _interpreter.get_scope().get_vector_variables_t<T>();
}

#endif
