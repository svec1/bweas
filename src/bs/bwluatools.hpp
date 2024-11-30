#ifndef BWLUATOOLS__H
#define BWLUATOOLS__H

#include "lang/static_struct.hpp"
#include "tools/bwlua.hpp"

#include <any>
#include <map>
#include <string>

namespace bweas {
namespace luatools_bwstruct {
static inline bwlua::lua::table<std::string, std::any> conv_to_table(const var::struct_sb::project &prj) {
    return bwlua::lua::table<std::string, std::any>{
        {PRJ_VAR_NAME, prj.name_project},           {PRJ_VAR_NAME_VER, prj.version_project.get_str_version()},
        {PRJ_VAR_NAME_LANG, lang_str(prj.lang)},    {PRJ_VAR_NAME_PTH_C, prj.path_compiler},
        {PRJ_VAR_NAME_PTH_L, prj.path_linker},      {PRJ_VAR_NAME_RFLAGS_C, prj.rflags_compiler},
        {PRJ_VAR_NAME_RFLAGS_L, prj.rflags_linker}, {PRJ_VAR_NAME_DFLAGS_C, prj.dflags_compiler},
        {PRJ_VAR_NAME_DFLAGS_L, prj.dflags_linker}, {PRJ_VAR_NAME_STD_C, prj.standart_c},
        {PRJ_VAR_NAME_STD_CPP, prj.standart_cpp},   {PRJ_VAR_NAME_UITTEMPLATES, prj.use_it_templates},
        {PRJ_VAR_NAME_SRC_FILES, prj.src_files},    {PRJ_VAR_NAME_UTEMPLATES, prj.vec_templates}};
}

static inline bwlua::lua::table<std::string, std::any> conv_to_table(const var::struct_sb::template_command &tmp_c) {
    return bwlua::lua::table<std::string, std::any>{
        {NAME_FIELD_TEMPLATE_COMMAND_NAME, tmp_c.name},
        {NAME_FIELD_TEMPLATE_COMMAND_NAME_CCMP, tmp_c.name_call_component},
        {NAME_FIELD_TEMPLATE_COMMAND_NAME_ACCEPTS_ARGS, tmp_c.name_accept_params},
        {NAME_FIELD_TEMPLATE_COMMAND_NAME_ARGS, tmp_c.name_args},
        {NAME_FIELD_TEMPLATE_COMMAND_RET, tmp_c.returnable}};
}

static inline bwlua::lua::table<std::string, std::any> conv_to_table(const var::struct_sb::target_out &trg_o) {
    return bwlua::lua::table<std::string, std::any>{{TRG_NAME_FIELD_PROJECT, conv_to_table(trg_o.prj)},
                                                    {TRG_VAR_NAME_TYPE_T, target_t_str(trg_o.target_t)},
                                                    {TRG_VAR_NAME_CFG, cfg_str(trg_o.target_cfg)},
                                                    {TRG_NAME_FIELD_NTARGET, trg_o.name_target},
                                                    {TRG_VAR_NAME_VER, trg_o.version_target.get_str_version()},
                                                    {TRG_VAR_NAME_LLIBS, trg_o.target_vec_libs}};
}
} // namespace luatools_bwstruct
} // namespace bweas

#endif