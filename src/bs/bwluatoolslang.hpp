#ifndef BWLUATOOLSLANG__H
#define BWLUATOOLSLANG__H

#include "bwgntools.hpp"
#include "lang/static_struct.hpp"
#include "tools/bwfile.hpp"
#include "tools/bwlua.hpp"

namespace bweas {

namespace luatools_lang {

static inline bwlua::lua::table<std::string, std::any> conv_to_table(const var::struct_sb::project &prj) {
    return bwlua::lua::table<std::string, std::any>{
        {PRJ_VAR_NAME, prj.name_project},
        {PRJ_VAR_NAME_VER, prj.version_project.get_str_version()},
        {PRJ_VAR_NAME_LANG, lang_str(prj.lang)},
        {PRJ_VAR_NAME_PTH_C, prj.path_compiler},
        {PRJ_VAR_NAME_PTH_L, prj.path_linker},
        {PRJ_VAR_NAME_RFLAGS_C, prj.rflags_compiler},
        {PRJ_VAR_NAME_RFLAGS_L, prj.rflags_linker},
        {PRJ_VAR_NAME_DFLAGS_C, prj.dflags_compiler},
        {PRJ_VAR_NAME_DFLAGS_L, prj.dflags_linker},
        {PRJ_VAR_NAME_STD_C, prj.standart_c},
        {PRJ_VAR_NAME_STD_CPP, prj.standart_cpp},
        {PRJ_VAR_NAME_UITTEMPLATES, (bwlua::lua::integer)prj.use_it_templates},
        {PRJ_VAR_NAME_SRC_FILES, prj.src_files},
        {PRJ_VAR_NAME_UTEMPLATES, prj.vec_templates}};
}

static inline bwlua::lua::array<std::any> conv_to_table(
    const std::vector<var::struct_sb::template_command::arg> &args) {
    bwlua::lua::array<std::any> vec_args;
    for (const auto &arg : args)
        vec_args.emplace_back(
            bwlua::lua::keyValue<std::string, bwlua::lua::integer>{arg.str_arg, (bwlua::lua::integer)arg.arg_t});
    return vec_args;
}

static inline bwlua::lua::table<std::string, std::any> conv_to_table(const var::struct_sb::template_command &tmp_c) {
    return bwlua::lua::table<std::string, std::any>{
        {NAME_FIELD_TEMPLATE_COMMAND_NAME, tmp_c.name},
        {NAME_FIELD_TEMPLATE_COMMAND_NAME_CCMP, tmp_c.name_call_component},
        {NAME_FIELD_TEMPLATE_COMMAND_NAME_ACCEPTS_ARGS, tmp_c.name_accept_params},
        {NAME_FIELD_TEMPLATE_COMMAND_NAME_ARGS, conv_to_table(tmp_c.args)},
        {NAME_FIELD_TEMPLATE_COMMAND_RET, tmp_c.returnable}};
}

static inline bwlua::lua::table<std::string, std::any> conv_to_table(const var::struct_sb::call_component &ccmp) {
    return bwlua::lua::table<std::string, std::any>{{NAME_FIELD_CALL_COMPONENT_NAME, ccmp.name},
                                                    {NAME_FIELD_CALL_COMPONENT_NAME_PROGRAM, ccmp.name_program},
                                                    {NAME_FIELD_CALL_COMPONENT_PATTERN_FILES, ccmp.pattern_ret_files}};
}

static inline bwlua::lua::table<std::string, std::any> conv_to_table(const var::struct_sb::target &trg_o) {
    return bwlua::lua::table<std::string, std::any>{{TRG_NAME_FIELD_PROJECT, conv_to_table(*trg_o.prj)},
                                                    {TRG_VAR_NAME_TYPE_T, target_t_str(trg_o.target_t)},
                                                    {TRG_VAR_NAME_CFG, cfg_str(trg_o.target_cfg)},
                                                    {TRG_NAME_FIELD_NTARGET, trg_o.name_target},
                                                    {TRG_VAR_NAME_VER, trg_o.version_target.get_str_version()},
                                                    {TRG_VAR_NAME_LLIBS, trg_o.target_vec_libs}};
}

static inline bwlua::lua::table<std::string, std::any> conv_to_table(const var::struct_sb::target_out &trg_o) {
    return bwlua::lua::table<std::string, std::any>{{TRG_NAME_FIELD_PROJECT, conv_to_table(trg_o.prj)},
                                                    {TRG_VAR_NAME_TYPE_T, target_t_str(trg_o.target_t)},
                                                    {TRG_VAR_NAME_CFG, cfg_str(trg_o.target_cfg)},
                                                    {TRG_NAME_FIELD_NTARGET, trg_o.name_target},
                                                    {TRG_VAR_NAME_VER, trg_o.version_target.get_str_version()},
                                                    {TRG_VAR_NAME_LLIBS, trg_o.target_vec_libs}};
}

static inline var::struct_sb::project conv_to_project(bwlua::lua::table<std::string, std::any> &prj_t) {
    var::struct_sb::project prj;

    prj.name_project = std::any_cast<std::string>(prj_t[PRJ_VAR_NAME]);
    prj.version_project = std::any_cast<std::string>(prj_t[PRJ_VAR_NAME_VER]);
    prj.lang = var::struct_sb::to_lang(std::any_cast<std::string>(prj_t[PRJ_VAR_NAME_LANG]));
    prj.path_compiler = std::any_cast<std::string>(prj_t[PRJ_VAR_NAME_PTH_C]);
    prj.path_linker = std::any_cast<std::string>(prj_t[PRJ_VAR_NAME_PTH_L]);
    prj.rflags_compiler = std::any_cast<std::string>(prj_t[PRJ_VAR_NAME_RFLAGS_C]);
    prj.rflags_linker = std::any_cast<std::string>(prj_t[PRJ_VAR_NAME_RFLAGS_L]);
    prj.dflags_compiler = std::any_cast<std::string>(prj_t[PRJ_VAR_NAME_DFLAGS_C]);
    prj.dflags_linker = std::any_cast<std::string>(prj_t[PRJ_VAR_NAME_DFLAGS_L]);
    prj.standart_c = std::any_cast<i32t>(prj_t[PRJ_VAR_NAME_STD_C]);
    prj.standart_cpp = std::any_cast<i32t>(prj_t[PRJ_VAR_NAME_STD_CPP]);
    prj.use_it_templates = std::any_cast<bool>(prj_t[PRJ_VAR_NAME_UITTEMPLATES]);
    prj.src_files = std::any_cast<std::vector<std::string>>(prj_t[PRJ_VAR_NAME_SRC_FILES]);
    prj.vec_templates = std::any_cast<std::vector<std::string>>(prj_t[PRJ_VAR_NAME_UTEMPLATES]);

    return prj;
}

static inline var::struct_sb::target_out conv_to_target(bwlua::lua::table<std::string, std::any> &trg_o_t) {
    var::struct_sb::target_out trg;

    trg.prj = conv_to_project(std::any_cast<bwlua::lua::table<std::string, std::any>>(trg_o_t[TRG_NAME_FIELD_PROJECT]));
    trg.target_t = var::struct_sb::to_type_target(std::any_cast<std::string>(trg_o_t[TRG_VAR_NAME_TYPE_T]));
    trg.target_cfg = var::struct_sb::to_cfg(std::any_cast<std::string>(trg_o_t[TRG_VAR_NAME_CFG]));
    trg.name_target = std::any_cast<std::string>(trg_o_t[TRG_NAME_FIELD_NTARGET]);
    trg.version_target = std::any_cast<std::string>(trg_o_t[TRG_VAR_NAME_VER]);
    trg.target_vec_libs = std::any_cast<std::vector<std::string>>(trg_o_t[TRG_VAR_NAME_LLIBS]);

    return trg;
}

static inline std::vector<var::struct_sb::template_command::arg> conv_to_args(
    bwlua::lua::array<bwlua::lua::array<std::any>> &args) {
    std::vector<var::struct_sb::template_command::arg> _args;

    for (const auto &arg : args)
        _args.emplace_back(std::any_cast<std::string>(arg[0]),
                           std::any_cast<var::struct_sb::template_command::arg::type>(arg[1]));

    return _args;
}

static inline var::struct_sb::template_command conv_to_template(bwlua::lua::table<std::string, std::any> &tcmd) {
    var::struct_sb::template_command _tcmd;

    _tcmd.name = std::any_cast<std::string>(tcmd[NAME_FIELD_CALL_COMPONENT_NAME]);
    _tcmd.name_call_component = std::any_cast<std::string>(tcmd[NAME_FIELD_TEMPLATE_COMMAND_NAME_CCMP]);
    _tcmd.name_accept_params =
        std::any_cast<std::vector<std::string>>(tcmd[NAME_FIELD_TEMPLATE_COMMAND_NAME_ACCEPTS_ARGS]);
    _tcmd.args = conv_to_args(
        std::any_cast<bwlua::lua::array<bwlua::lua::array<std::any>>>(tcmd[NAME_FIELD_TEMPLATE_COMMAND_NAME_ARGS]));
    _tcmd.returnable = std::any_cast<std::string>(tcmd[NAME_FIELD_TEMPLATE_COMMAND_RET]);

    return _tcmd;
}

static inline var::struct_sb::call_component conv_to_call_components(bwlua::lua::table<std::string, std::any> &ccmp) {
    var::struct_sb::call_component _ccmp;

    _ccmp.name = std::any_cast<std::string>(ccmp[NAME_FIELD_CALL_COMPONENT_NAME]);
    _ccmp.name_program = std::any_cast<std::string>(ccmp[NAME_FIELD_CALL_COMPONENT_NAME_PROGRAM]);
    _ccmp.pattern_ret_files = std::any_cast<std::string>(ccmp[NAME_FIELD_CALL_COMPONENT_PATTERN_FILES]);

    return _ccmp;
}

static inline int get_var(lua_State *L) {
    std::string name_var = bwlua::tools::pop_stack<std::string>(L);
    bwlua::lua::integer var_t = bwlua::tools::pop_stack<bwlua::lua::integer>(L);
    var::scope *ref = (var::scope *)bwlua::tools::pop_stack<bwlua::lua::integer>(L);
    try {
        if (var_t == 1)
            bwlua::tools::push_stack(L, (bwlua::lua::integer)ref->get_var_value<i32t>(name_var));
        else if (var_t == 2)
            bwlua::tools::push_stack(L, ref->get_var_value<std::string>(name_var));
        else if (var_t == 3)
            bwlua::tools::push_stack(L, ref->get_var_value<std::vector<i32t>>(name_var));
        else if (var_t == 4)
            bwlua::tools::push_stack(L, ref->get_var_value<std::vector<std::string>>(name_var));
        else if (var_t == 5)
            bwlua::tools::push_stack(L, conv_to_table(ref->get_var_value<var::struct_sb::project>(name_var)));
        else if (var_t == 6)
            bwlua::tools::push_stack(L, conv_to_table(ref->get_var_value<var::struct_sb::target>(name_var)));
        else if (var_t == 7)
            bwlua::tools::push_stack(L, conv_to_table(ref->get_var_value<var::struct_sb::template_command>(name_var)));
        else if (var_t == 8)
            bwlua::tools::push_stack(L, conv_to_table(ref->get_var_value<var::struct_sb::call_component>(name_var)));
        else if (var_t == 9)
            bwlua::tools::push_stack(L, ref->get_var_value<std::pair<std::string, std::string>>(name_var));
    }
    catch (...) {
        bwlua::tools::push_stack(L, "\"" + name_var + "\" variable was not found.");
    };

    return 1;
}

static inline int set_var(lua_State *L) {
    std::string name_var = bwlua::tools::pop_stack<std::string>(L);
    std::any value = bwlua::tools::pop_stack<std::any>(L);
    var::scope *ref = (var::scope *)bwlua::tools::pop_stack<bwlua::lua::integer>(L);
    try {
        if (value.type() == typeid(i32t) && !ref->try_create_var(name_var, std::any_cast<i32t>(value)))
            ref->get_var_value<i32t>(name_var) = std::any_cast<i32t>(value);
        else if (value.type() == typeid(std::string) &&
                 !ref->try_create_var(name_var, std::any_cast<std::string>(value)))
            ref->get_var_value<std::string>(name_var) = std::any_cast<std::string>(value);
        else if (value.type() == typeid(std::vector<std::any>)) {
            std::vector<std::any> any_vec = std::any_cast<std::vector<std::any>>(value);
            auto el_tmp = std::any_cast<std::any>(any_vec[0]);
            if (el_tmp.type() == typeid(i32t)) {
                std::vector<i32t> tmp_vec;
                for (const auto &it : any_vec)
                    tmp_vec.push_back(std::any_cast<i32t>(it));
                if (!ref->try_create_var(name_var, tmp_vec))
                    ref->get_var_value<std::vector<i32t>>(name_var) = tmp_vec;
            }
            else if (el_tmp.type() == typeid(std::string)) {
                std::vector<std::string> tmp_vec;
                for (const auto &it : any_vec)
                    tmp_vec.push_back(std::any_cast<std::string>(it));
                if (!ref->try_create_var(name_var, tmp_vec))
                    ref->get_var_value<std::vector<std::string>>(name_var) = tmp_vec;
            }
        }
    }
    catch (...) {
    }

    return 0;
}

static inline int get_name_output_file_lua(lua_State *L) {
    bwlua::tools::push_stack(
        L,
        generator::tools::get_name_output_file(lua_tostring(L, -3), lua_tointeger(L, -2), lua_tostring(L, -1)).c_str());
    return 1;
}
static inline int file_slc_mask_lua(lua_State *L) {
    commands cmd_s = bwlua::tools::pop_stack<commands>(L);
    cmd_s = bwfile::file_slc_mask(lua_tostring(L, -2), cmd_s);
    bwlua::tools::push_stack(L, cmd_s);
    return 1;
}

} // namespace luatools_lang

} // namespace bweas

#endif