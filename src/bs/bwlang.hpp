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
    lang(context *const __context);

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

    template <typename T> inline bool create_global_var(string name_var, T val = {}) {
        return _interpreter.get_scope().try_create_var(name_var, val);
    }
    template <typename T> inline T get_global_var(string name_var) {
        return _interpreter.get_scope().get_var_value<T>(name_var);
    }

    scope &get_global_scope() &;
    template <typename T> container_vars<T>::container_type &get_container_vars() &;

    void init_context();

  private:
    vec<sc::target> get_targets();
    vec<sc::template_command> get_templates();
    vec<sc::call_component> get_call_components();
    vec<pair<string, string>> get_global_external_args();

  private:
    context *const _context;
    interpreter _interpreter;
};
bweas::lang::lang(context *const __context) : _context(__context), _interpreter(_context->path_bweas_config) {
    init_scope();
}

void bweas::lang::init_scope() {
    _interpreter.get_scope().create_var<pdiff>("DEBUG", 0);
    _interpreter.get_scope().create_var<pdiff>("RELEASE", 1);
    _interpreter.get_scope().create_var<pdiff>("FALSE", 0);
    _interpreter.get_scope().create_var<pdiff>("TRUE", 1);
    _interpreter.get_scope().create_var<pdiff>("EXECUTABLE", 0);
    _interpreter.get_scope().create_var<pdiff>("LIBRARY", 1);
    _interpreter.get_scope().create_var<string>("BWEAS_CONFIG_PATH", fs::current_path().string());

    _interpreter.create_function(
        "set", sl_func::set,
        {param_type::NCHECK_VAR_ID, param_type::ANY_VALUE_WITHOUT_FUTUREID_NEXT, param_type::NEXT_TOO});
    _interpreter.create_function(
        "file", sl_func::file,
        {{param_type::FUTURE_VAR_ID, "{NULL}"}, param_type::LIT_NUM, param_type::LIT_STR, param_type::NEXT_TOO});

    _interpreter.create_function("create_target", sl_func::create_target,
                                 {param_type::FUTURE_VAR_ID,
                                  param_type::VAR_STRUCT_ID,
                                  param_type::LIT_NUM,
                                  {param_type::LIT_STR, "{NULL}"},
                                  param_type::NEXT_TOO});
    _interpreter.create_function("add_dependencies_target", sl_func::add_dependencies_target,
                                 {param_type::VAR_STRUCT_ID, param_type::VAR_STRUCT_ID, param_type::NEXT_TOO});

    _interpreter.create_function("exp_data", sl_func::exp_data, {param_type::LIT_STR});
    _interpreter.create_function("debug", sl_func::debug, {param_type::LIT_STR, param_type::NEXT_TOO});
    _interpreter.create_function("debug_struct", sl_func::debug_struct, {param_type::VAR_STRUCT_ID});

    _interpreter.create_function("create_template", sl_func::create_template,
                                 {param_type::FUTURE_VAR_ID, param_type::LIT_STR});
    _interpreter.create_function("create_call_component", sl_func::create_call_component,
                                 {param_type::FUTURE_VAR_ID, param_type::LIT_STR, param_type::LIT_STR});
    _interpreter.create_function("add_param_template", sl_func::add_param_template,
                                 {param_type::FUTURE_VAR_ID, param_type::VAR_ID});
    _interpreter.create_function("use_templates", sl_func::use_templates,
                                 {param_type::VAR_STRUCT_ID, param_type::VAR_STRUCT_ID, param_type::NEXT_TOO});
}

void bweas::lang::execute() {
    _interpreter.interpret();
}
void bweas::lang::init_external_funcs(vec<decl_func> funcs) {
    for (const auto &func : funcs)
        _interpreter.create_function(func);
}

void bweas::lang::init_context() {
    _context->targets              = get_targets();
    _context->templates            = get_templates();
    _context->call_components      = get_call_components();
    _context->global_external_args = get_global_external_args();
}

vec<bweas::sc::target> bweas::lang::get_targets() {
    const container_vars<sc::target>::container_type &container_targets =
        _interpreter.get_scope().get_container_vars<sc::target>();

    vec<sc::target> targets;
    for (auto it = container_targets.begin(); it != container_targets.end(); ++it)
        targets.push_back(it->second);

    return targets;
}

vec<bweas::sc::template_command> bweas::lang::get_templates() {
    container_vars<sc::template_command>::container_type container_templates =
        _interpreter.get_scope().get_container_vars<sc::template_command>();

    vec<sc::template_command> templates;
    for (auto it = container_templates.begin(); it != container_templates.end(); ++it)
        templates.emplace_back(it->second);

    return templates;
}

vec<bweas::sc::call_component> bweas::lang::get_call_components() {
    container_vars<sc::call_component>::container_type container_call_components =
        _interpreter.get_scope().get_container_vars<sc::call_component>();

    vec<sc::call_component> call_components;
    for (auto it = container_call_components.begin(); it != container_call_components.end(); ++it)
        call_components.emplace_back(it->second);

    return call_components;
}

vec<pair<string, string>> bweas::lang::get_global_external_args() {
    container_vars<pair<string, string>>::container_type container_global_external_args =
        _interpreter.get_scope().get_container_vars<pair<string, string>>();

    vec<pair<string, string>> global_external_args;
    for (auto it = container_global_external_args.begin(); it != container_global_external_args.end(); ++it)
        global_external_args.emplace_back(it->second);

    return global_external_args;
}

scope &bweas::lang::get_global_scope() & {
    return _interpreter.get_scope();
}
template <typename T> container_vars<T>::container_type &bweas::lang::get_container_vars() & {
    return _interpreter.get_scope().get_container_vars<T>();
}

#endif
