#ifndef BWLANG__H
#define BWLANG__H

#include "lang/interpreter.hpp"

namespace bweas {

// A wrapper around the bwlang interpreter that installs all
// the standard bweas functions and also provides interaction with the global scope
class bwlang {
  public:
    bwlang(std::string bwconf_file = "");

    bwlang(bwlang &&) = delete;
    bwlang(const bwlang &) = delete;
    bwlang &operator=(bwlang &&) = delete;

  private:
    // Sets standard functions and keyword operators corresponding to the bweas specification
    void set_bwstd_funcs();

  public:
    void set_config();
    void set_bwconf_file(std::string bwconf_file);

  public:
    // Starts the internal interpreter
    void execute();

    // Loads external functions (passed into this function) into the interpreter (semantic analyzer)
    inline void load_external_tfuncs(const semantic_an::table_func &tfuncs);

    template <typename T> inline bool create_global_var(std::string name_var, T val = {}) {
        return interpreter.get_current_scope().try_create_var(name_var, val);
    }
    template <typename T> inline T get_global_var(std::string name_var) {
        return interpreter.get_current_scope().get_var_value<T>(name_var);
    }

    var::scope &get_global_scope();
    template <typename T> std::vector<std::pair<std::string, T>> &get_class_variables();

    std::vector<var::struct_sb::target_out> get_targets();
    std::vector<var::struct_sb::template_command> get_templates();
    std::vector<var::struct_sb::call_component> get_call_components();
    std::vector<std::pair<std::string, std::string>> get_global_external_args();

  public:
    interpreter::interpreter_exec::config cfg_intp;

  private:
    interpreter::interpreter_exec interpreter;
};
bwlang::bwlang(std::string bwconf_file) {
    cfg_intp.debug_output = 0;
    cfg_intp.filename_interp = bwconf_file;
    cfg_intp.transmit_smt_name_func_with_smt = 1;
    cfg_intp.use_external_scope = 0;

    interpreter.set_config(cfg_intp);
    set_bwstd_funcs();
}

void bwlang::set_bwstd_funcs() {
    using namespace aef_expr;

    interpreter.set_std_function(
        "set", sl_func::set,
        {param_type::NCHECK_VAR_ID, param_type::ANY_VALUE_WITHOUT_FUTUREID_NEXT, param_type::NEXT_TOO});
    interpreter.set_std_function("file", sl_func::file,
                                 {{param_type::FUTURE_VAR_ID, "{NULL}"},
                                  param_type::LNUM_OR_ID_VAR,
                                  param_type::LSTR_OR_ID_VAR,
                                  param_type::NEXT_TOO});

    interpreter.set_std_function(
        "project", sl_func::project,
        {param_type::FUTURE_VAR_ID, param_type::LNUM_OR_ID_VAR, param_type::LSTR_OR_ID_VAR, param_type::NEXT_TOO});
    interpreter.set_std_function("executable", sl_func::executable,
                                 {param_type::FUTURE_VAR_ID, param_type::LNUM_OR_ID_VAR, param_type::VAR_STRUCT_ID});

    interpreter.set_std_function("link_lib", sl_func::link_lib,
                                 {param_type::VAR_STRUCT_ID, param_type::LSTR_OR_ID_VAR, param_type::NEXT_TOO});
    interpreter.set_std_function("exp_data", sl_func::exp_data, {param_type::LSTR_OR_ID_VAR});
    interpreter.set_std_function("cmd", sl_func::cmd, {param_type::LNUM_OR_ID_VAR, param_type::LSTR_OR_ID_VAR});
    interpreter.set_std_function("debug", sl_func::debug, {param_type::LSTR_OR_ID_VAR, param_type::NEXT_TOO});
    interpreter.set_std_function("debug_struct", sl_func::debug_struct, {param_type::VAR_STRUCT_ID});

    interpreter.set_std_function(
        "flags_compiler", sl_func::flags_compiler,
        {param_type::VAR_STRUCT_ID, param_type::LNUM_OR_ID_VAR, param_type::LSTR_OR_ID_VAR, param_type::NEXT_TOO});
    interpreter.set_std_function(
        "flags_linker", sl_func::flags_linker,
        {param_type::VAR_STRUCT_ID, param_type::LNUM_OR_ID_VAR, param_type::LSTR_OR_ID_VAR, param_type::NEXT_TOO});

    interpreter.set_std_function("path_compiler", sl_func::path_compiler,
                                 {param_type::VAR_STRUCT_ID, param_type::LSTR_OR_ID_VAR});
    interpreter.set_std_function("path_linker", sl_func::path_linker,
                                 {param_type::VAR_STRUCT_ID, param_type::LSTR_OR_ID_VAR});
    interpreter.set_std_function("standart_c", sl_func::standart_c,
                                 {param_type::VAR_STRUCT_ID, param_type::LNUM_OR_ID_VAR});
    interpreter.set_std_function("standart_cpp", sl_func::standart_cpp,
                                 {param_type::VAR_STRUCT_ID, param_type::LNUM_OR_ID_VAR});
    interpreter.set_std_function("lang", sl_func::lang, {param_type::VAR_STRUCT_ID, param_type::LNUM_OR_ID_VAR});

    interpreter.set_std_function("generator", sl_func::generator,
                                 {param_type::VAR_STRUCT_ID, param_type::LSTR_OR_ID_VAR});

    interpreter.set_std_function("create_templates", sl_func::create_templates,
                                 {param_type::FUTURE_VAR_ID, param_type::LSTR_OR_ID_VAR});
    interpreter.set_std_function("create_call_component", sl_func::create_call_component,
                                 {param_type::FUTURE_VAR_ID, param_type::LSTR_OR_ID_VAR, param_type::LSTR_OR_ID_VAR});
    interpreter.set_std_function("add_param_template", sl_func::add_param_template,
                                 {param_type::FUTURE_VAR_ID, param_type::VAR_ID});
    interpreter.set_std_function("use_templates", sl_func::use_templates,
                                 {param_type::VAR_STRUCT_ID, param_type::LSTR_OR_ID_VAR, param_type::NEXT_TOO});
    interpreter.set_std_function("use_it_template", sl_func::use_it_template,
                                 {param_type::VAR_STRUCT_ID, param_type::LNUM_OR_ID_VAR});
}

void bwlang::set_config() {
    interpreter.set_config(cfg_intp);
}
void bwlang::set_bwconf_file(std::string bwconf_file) {
    cfg_intp.filename_interp = bwconf_file;
    interpreter.set_config(cfg_intp);
}
void bwlang::execute() {
    interpreter.build_aef();
    interpreter.interpret();
}
void bwlang::load_external_tfuncs(const semantic_an::table_func &tfuncs) {
    interpreter.load_external_func(tfuncs);
}
std::vector<var::struct_sb::target_out> bwlang::get_targets() {
    std::vector<var::struct_sb::target> targets = interpreter.export_targets();

    std::vector<var::struct_sb::target_out> targets_o;
    var::struct_sb::target_out target_tmp;

    for (u32t i = 0; i < targets.size(); ++i) {
        target_tmp.name_target = targets[i].name_target;
        target_tmp.target_t = targets[i].target_t;
        target_tmp.target_cfg = targets[i].target_cfg;
        target_tmp.version_target = targets[i].version_target;
        target_tmp.name_generator = targets[i].name_generator;
        target_tmp.target_vec_libs = targets[i].target_vec_libs;
        target_tmp.prj = *targets[i].prj;

        targets_o.push_back(target_tmp);
    }

    return targets_o;
}

std::vector<var::struct_sb::template_command> bwlang::get_templates() {
    std::vector<std::pair<std::string, var::struct_sb::template_command>> templates =
        interpreter.get_current_scope().get_vector_variables_t<var::struct_sb::template_command>();

    std::vector<var::struct_sb::template_command> templates_out;
    for (const auto &_template : templates)
        templates_out.emplace_back(_template.second);

    return templates_out;
}

std::vector<var::struct_sb::call_component> bwlang::get_call_components() {
    std::vector<std::pair<std::string, var::struct_sb::call_component>> call_components =
        interpreter.get_current_scope().get_vector_variables_t<var::struct_sb::call_component>();

    std::vector<var::struct_sb::call_component> call_components_out;
    for (const auto &_template : call_components)
        call_components_out.emplace_back(_template.second);

    return call_components_out;
}

std::vector<std::pair<std::string, std::string>> bwlang::get_global_external_args() {
    std::vector<std::pair<std::string, std::pair<std::string, std::string>>> global_external_args =
        interpreter.get_current_scope().get_vector_variables_t<std::pair<std::string, std::string>>();

    std::vector<std::pair<std::string, std::string>> global_external_args_out;
    for (const auto &global_external_arg : global_external_args)
        global_external_args_out.emplace_back(global_external_arg.second);

    return global_external_args_out;
}

var::scope &bwlang::get_global_scope() {
    return interpreter.get_current_scope();
}
template <typename T> std::vector<std::pair<std::string, T>> &bwlang::get_class_variables() {
    return interpreter.get_current_scope().get_vector_variables_t<T>();
}

} // namespace bweas

#endif