//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include <bwluatools.hpp>

#include <bwgntools.hpp>
#include <lang/scope.hpp>
#include <tools/bwfile.hpp>

using namespace bweas;

lua_tools::table<string, any> lua_tools::conv_to_table(const sc::profile &ext) {
    lua_tools::table<string, any> _ext;

    for (const auto &[key, value] : ext.get_fields())
        if (std::holds_alternative<string>(value))
            _ext[key] = std::get<string>(value);
        else
            _ext[key] = std::get<vec<string>>(value);
    return _ext;
}

lua_tools::array<any> lua_tools::conv_to_table(const vec<sc::template_command::arg> &args) {
    lua_tools::array<any> vec_args;
    for (const auto &arg : args)
        vec_args.emplace_back(lua_tools::array<any>{arg.value, (bwlua::lua::integer)arg.type});
    return vec_args;
}

lua_tools::array<any> lua_tools::conv_to_table(const sc::template_command::return_value &returnable) {
    return lua_tools::array<any>{returnable.value, (pdiff)returnable.type};
}

lua_tools::table<string_v, any> lua_tools::conv_to_table(const sc::template_command &tmp_c) {
    return table<string_v, any>{{NAME_FIELD_TEMPLATE_COMMAND_NAME, tmp_c.name},
                                {NAME_FIELD_TEMPLATE_COMMAND_NAME_CCMP, tmp_c.name_call_component},
                                {NAME_FIELD_TEMPLATE_COMMAND_NAME_ACCEPTS_ARGS, tmp_c.name_accept_params},
                                {NAME_FIELD_TEMPLATE_COMMAND_NAME_ARGS, conv_to_table(tmp_c.args)},
                                {NAME_FIELD_TEMPLATE_COMMAND_RET, conv_to_table(tmp_c.returnable)},
                                {NAME_FIELD_TEMPLATE_COMMAND_IFILES, tmp_c.ifiles},
                                {NAME_FIELD_TEMPLATE_COMMAND_SINGLE_GENERATES, (pdiff)tmp_c.single_generates}};
}

lua_tools::table<string_v, string> lua_tools::conv_to_table(const sc::call_component &ccmp) {
    return lua_tools::table<string_v, string>{{NAME_FIELD_CALL_COMPONENT_NAME, ccmp.name},
                                              {NAME_FIELD_CALL_COMPONENT_NAME_PROGRAM, ccmp.name_program},
                                              {NAME_FIELD_CALL_COMPONENT_PATTERN_FILES, ccmp.pattern_ret_files}};
}

lua_tools::table<string_v, any> lua_tools::conv_to_table(const sc::target &trg_o) {
    return lua_tools::table<string_v, any>{
        {TRG_NAME_FIELD_EXTENSION, conv_to_table(trg_o.ext)}, {TRG_VAR_NAME_TYPE, target_type_str(trg_o.type)},
        {TRG_VAR_NAME_CFG, target_cfg_str(trg_o.cfg)},        {TRG_NAME_FIELD_NTARGET, trg_o.name},
        {TRG_VAR_NAME_VER, trg_o.ver.get_str_version()},      {TRG_VAR_NAME_TEMPLATES, trg_o.templates},
        {TRG_VAR_NAME_DEPENDENCIES, trg_o.dependencies}};
}
lua_tools::table<string, lua_tools::array<string>> lua_tools::conv_to_table(
    const bweas::depends_files::depends_map &dfiles) {
    lua_tools::table<string, lua_tools::array<string>> _dfiles;
    for (const auto &dfile : dfiles)
        _dfiles.insert({dfile.first, {dfile.second.begin(), dfile.second.end()}});
    return _dfiles;
}

sc::profile lua_tools::conv_to_extension(lua_tools::table<string, any> ext) {
    sc::profile _ext;
    sc::profile::fields &ext_fields = _ext.get_fields();

    for (const auto &[key, value] : ext)
        if (value.type() == typeid(std::string))
            ext_fields[key] = std::any_cast<string>(value);
        else
            ext_fields[key] = std::any_cast<vec<string>>(value);
    return _ext;
}

sc::target lua_tools::conv_to_target(lua_tools::table<string, any> &trg_o_t) {
    sc::target trg;

    trg.ext       = conv_to_extension(std::any_cast<bwlua::lua::table<string, any>>(trg_o_t[TRG_NAME_FIELD_EXTENSION]));
    trg.type      = sc::to_target_type(std::any_cast<string>(trg_o_t[TRG_VAR_NAME_TYPE]));
    trg.cfg       = sc::to_target_cfg(std::any_cast<string>(trg_o_t[TRG_VAR_NAME_CFG]));
    trg.name      = std::any_cast<string>(trg_o_t[TRG_NAME_FIELD_NTARGET]);
    trg.ver       = std::any_cast<string>(trg_o_t[TRG_VAR_NAME_VER]);
    trg.templates = std::any_cast<vec<string>>(trg_o_t[TRG_VAR_NAME_TEMPLATES]);
    trg.dependencies = std::any_cast<vec<string>>(trg_o_t[TRG_VAR_NAME_DEPENDENCIES]);

    return trg;
}

vec<sc::template_command::arg> lua_tools::conv_to_args(lua_tools::array<lua_tools::array<any>> args) {
    vec<sc::template_command::arg> _args;

    for (const auto &arg : args)
        _args.emplace_back(std::any_cast<string>(arg[0]), std::any_cast<sc::template_command::arg::e_type>(arg[1]));

    return _args;
}
sc::template_command::return_value lua_tools::conv_to_return_value(lua_tools::array<any> returnable) {
    sc::template_command::return_value _returnable;

    _returnable.value = std::any_cast<string>(returnable[0]);
    _returnable.type  = (sc::template_command::return_value::e_type)std::any_cast<integer>(returnable[1]);

    return _returnable;
}

sc::template_command lua_tools::conv_to_template(lua_tools::table<string, any> &tcmd) {
    sc::template_command _tcmd;

    _tcmd.name                = std::any_cast<string>(tcmd[NAME_FIELD_CALL_COMPONENT_NAME]);
    _tcmd.name_call_component = std::any_cast<string>(tcmd[NAME_FIELD_TEMPLATE_COMMAND_NAME_CCMP]);
    _tcmd.name_accept_params  = std::any_cast<vec<string>>(tcmd[NAME_FIELD_TEMPLATE_COMMAND_NAME_ACCEPTS_ARGS]);
    _tcmd.args                = conv_to_args(
        std::any_cast<bwlua::lua::array<bwlua::lua::array<any>>>(tcmd[NAME_FIELD_TEMPLATE_COMMAND_NAME_ARGS]));
    _tcmd.returnable       = conv_to_return_value(std::any_cast<array<any>>(tcmd[NAME_FIELD_TEMPLATE_COMMAND_RET]));
    _tcmd.ifiles           = std::any_cast<vec<string>>(tcmd[NAME_FIELD_TEMPLATE_COMMAND_IFILES]);
    _tcmd.single_generates = std::any_cast<pdiff>(tcmd[NAME_FIELD_TEMPLATE_COMMAND_SINGLE_GENERATES]);

    return _tcmd;
}

sc::call_component lua_tools::conv_to_call_components(lua_tools::table<string, any> &ccmp) {
    sc::call_component _ccmp;

    _ccmp.name              = std::any_cast<string>(ccmp[NAME_FIELD_CALL_COMPONENT_NAME]);
    _ccmp.name_program      = std::any_cast<string>(ccmp[NAME_FIELD_CALL_COMPONENT_NAME_PROGRAM]);
    _ccmp.pattern_ret_files = std::any_cast<string>(ccmp[NAME_FIELD_CALL_COMPONENT_PATTERN_FILES]);

    return _ccmp;
}

int lua_tools::get_var(lua_State *L) {
    string name_var           = bwlua::tools::pop_stack<string>(L);
    bwlua::lua::integer var_t = bwlua::tools::pop_stack<integer>(L);
    scope *ref                = (scope *)bwlua::tools::pop_stack<integer>(L);
    try {
        if (var_t == 1)
            bwlua::tools::push_stack(L, (bwlua::lua::integer)ref->get_var_value<pdiff>(name_var));
        else if (var_t == 2)
            bwlua::tools::push_stack(L, ref->get_var_value<string>(name_var));
        else if (var_t == 3)
            bwlua::tools::push_stack(L, ref->get_var_value<vec<pdiff>>(name_var));
        else if (var_t == 4)
            bwlua::tools::push_stack(L, ref->get_var_value<vec<string>>(name_var));
        else if (var_t == 5)
            bwlua::tools::push_stack(L, conv_to_table(ref->get_var_value<sc::target>(name_var)));
        else if (var_t == 6)
            bwlua::tools::push_stack(L, conv_to_table(ref->get_var_value<sc::template_command>(name_var)));
        else if (var_t == 7)
            bwlua::tools::push_stack(L, conv_to_table(ref->get_var_value<sc::call_component>(name_var)));
        else if (var_t == 8)
            bwlua::tools::push_stack(L, ref->get_var_value<std::pair<string, string>>(name_var));
    }
    catch (...) {
        bwlua::tools::push_stack(L, "\"" + name_var + "\" variable was not found.");
    };

    return 1;
}

int lua_tools::set_var(lua_State *L) {
    string name_var = bwlua::tools::pop_stack<string>(L);
    any value       = bwlua::tools::pop_stack<any>(L);
    scope *ref      = (scope *)bwlua::tools::pop_stack<integer>(L);
    try {
        if (value.type() == typeid(pdiff) && !ref->try_create_var(name_var, std::any_cast<pdiff>(value)))
            ref->get_var_value<pdiff>(name_var) = std::any_cast<pdiff>(value);
        else if (value.type() == typeid(string) && !ref->try_create_var(name_var, std::any_cast<string>(value)))
            ref->get_var_value<string>(name_var) = std::any_cast<string>(value);
        else if (value.type() == typeid(vec<any>)) {
            vec<any> any_vec = std::any_cast<vec<any>>(value);
            auto el_tmp      = std::any_cast<any>(any_vec[0]);
            if (el_tmp.type() == typeid(pdiff)) {
                vec<pdiff> tmp_vec;
                for (const auto &it : any_vec)
                    tmp_vec.push_back(std::any_cast<pdiff>(it));
                if (!ref->try_create_var(name_var, tmp_vec))
                    ref->get_var_value<vec<pdiff>>(name_var) = tmp_vec;
            }
            else if (el_tmp.type() == typeid(string)) {
                vec<string> tmp_vec;
                for (const auto &it : any_vec)
                    tmp_vec.push_back(std::any_cast<string>(it));
                if (!ref->try_create_var(name_var, tmp_vec))
                    ref->get_var_value<vec<string>>(name_var) = tmp_vec;
            }
        }
    }
    catch (...) {
    }

    return 0;
}

int lua_tools::get_name_output_file_lua(lua_State *L) {
    bwlua::tools::push_stack(
        L,
        generator_tools::get_name_output_file(lua_tostring(L, -3), lua_tostring(L, -2), lua_tointeger(L, -1)).c_str());
    return 1;
}
int lua_tools::file_slc_mask_lua(lua_State *L) {
    vec<string> files = bwlua::tools::pop_stack<vec<string>>(L);
    bwlua::tools::push_stack(L, bwfile::file_slc_mask(lua_tostring(L, -2), files));
    return 1;
}
