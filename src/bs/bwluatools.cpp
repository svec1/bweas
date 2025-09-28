//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#include <bwluatools.hpp>

#include <utils/file_utils.hpp>

using namespace bweas;
using namespace bweas::utils;

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
        vec_args.emplace_back(lua_tools::array<any>{arg.value, (lua::integer)arg.type});
    return vec_args;
}

lua_tools::array<any> lua_tools::conv_to_table(const sc::template_command::return_value &returnable) {
    return lua_tools::array<any>{returnable.value, (pdiff)returnable.type};
}

lua_tools::table<string_v, any> lua_tools::conv_to_table(const sc::template_command &tmp_c) {
    return lua_tools::table<string_v, any>{{NAME_FIELD_TEMPLATE_COMMAND_NAME, tmp_c.name},
                                           {NAME_FIELD_TEMPLATE_COMMAND_NAME_CCMP, tmp_c.name_call_component},
                                           {NAME_FIELD_TEMPLATE_COMMAND_NAME_ACCEPTS_ARGS, tmp_c.name_accept_params},
                                           {NAME_FIELD_TEMPLATE_COMMAND_NAME_ARGS, conv_to_table(tmp_c.args)},
                                           {NAME_FIELD_TEMPLATE_COMMAND_RET, conv_to_table(tmp_c.returnable)}};
}

lua_tools::table<string_v, string> lua_tools::conv_to_table(const sc::call_component &ccmp) {
    return lua_tools::table<string_v, string>{{NAME_FIELD_CALL_COMPONENT_NAME, ccmp.name},
                                              {NAME_FIELD_CALL_COMPONENT_NAME_PROGRAM, ccmp.name_program},
                                              {NAME_FIELD_CALL_COMPONENT_PATTERN_FILES, ccmp.pattern_ret_files}};
}

lua_tools::table<string_v, any> lua_tools::conv_to_table(const sc::target &trg_o) {
    return lua_tools::table<string_v, any>{{TRG_NAME_FIELD_EXTENSION, conv_to_table(trg_o.ext)},
                                           {TRG_NAME_FIELD_NTARGET, trg_o.name},
                                           {TRG_VAR_NAME_VER, trg_o.ver.get_str_version()},
                                           {TRG_VAR_NAME_TEMPLATES, trg_o.templates},
                                           {TRG_VAR_NAME_DEPENDENCIES, trg_o.dependencies}};
}
lua_tools::table<string, lua_tools::array<string>> lua_tools::conv_to_table(
    const bweas::dependency_finder::dependency_map &dfiles) {
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

    trg.ext       = conv_to_extension(std::any_cast<lua_tools::table<string, any>>(trg_o_t[TRG_NAME_FIELD_EXTENSION]));
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
    _returnable.type  = (sc::template_command::return_value::e_type)std::any_cast<lua_tools::integer>(returnable[1]);

    return _returnable;
}

sc::template_command lua_tools::conv_to_template(lua_tools::table<string, any> &tcmd) {
    sc::template_command _tcmd;

    _tcmd.name                = std::any_cast<string>(tcmd[NAME_FIELD_CALL_COMPONENT_NAME]);
    _tcmd.name_call_component = std::any_cast<string>(tcmd[NAME_FIELD_TEMPLATE_COMMAND_NAME_CCMP]);
    _tcmd.name_accept_params  = std::any_cast<vec<string>>(tcmd[NAME_FIELD_TEMPLATE_COMMAND_NAME_ACCEPTS_ARGS]);
    _tcmd.args                = conv_to_args(
        std::any_cast<lua_tools::array<lua_tools::array<any>>>(tcmd[NAME_FIELD_TEMPLATE_COMMAND_NAME_ARGS]));
    _tcmd.returnable =
        conv_to_return_value(std::any_cast<lua_tools::array<any>>(tcmd[NAME_FIELD_TEMPLATE_COMMAND_RET]));

    return _tcmd;
}

sc::call_component lua_tools::conv_to_call_components(lua_tools::table<string, any> &ccmp) {
    sc::call_component _ccmp;

    _ccmp.name              = std::any_cast<string>(ccmp[NAME_FIELD_CALL_COMPONENT_NAME]);
    _ccmp.name_program      = std::any_cast<string>(ccmp[NAME_FIELD_CALL_COMPONENT_NAME_PROGRAM]);
    _ccmp.pattern_ret_files = std::any_cast<string>(ccmp[NAME_FIELD_CALL_COMPONENT_PATTERN_FILES]);

    return _ccmp;
}
