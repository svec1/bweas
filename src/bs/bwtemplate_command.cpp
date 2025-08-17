//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses />
// ------------------------------------------
//

#include <algorithm>

#include <bwlogger.hpp>
#include <bwstructs_context.hpp>

using namespace bweas;
using namespace sc;

bweas::logger _log{"BWCOMMAND_TEMPLATE"};

template_command template_command::create_template_command(string_v template_name, const string &template_str) {
    static const array<string, 18> name_field_target = {
        NAME_FIELD_TARGET_NAME,        NAME_FIELD_TARGET_TEMPLATES,  NAME_FIELD_TARGET_DEPENDENCIES,
        NAME_FIELD_TARGET_TYPE,        NAME_FIELD_TARGET_CFG,        NAME_FIELD_TARGET_VER,
        NAME_FIELD_PROJECT_LANG,       NAME_FIELD_PROJECT_PCOMPILER, NAME_FIELD_PROJECT_PLINKER,
        NAME_FIELD_PROJECT_RFCOMPILER, NAME_FIELD_PROJECT_RFLINKER,  NAME_FIELD_PROJECT_DFCOMPILER,
        NAME_FIELD_PROJECT_DFLINKER,   NAME_FIELD_PROJECT_STD_C,     NAME_FIELD_PROJECT_STD_CPP,
        NAME_FIELD_PROJECT_SRC_FILES,  NAME_FIELD_PROJECT_LIBS,      NAME_FIELD_PROJECT_INCLUDE_PATHS};

    template_command tcmd_tmp;
    tcmd_tmp.name = template_name;

    string tmp_param;

    std::regex template_command_syntax(
        R"(^\s*(\w+|[-+\.\/\*=\w+]+:.+)\(\s*(\w+(?:\s*,\s*\w+)*\s*)\)\s*->\s*(\w+):\s*((?:\w+\s*|<\'[-+\.\/\*=\w]*\'>\s*|<\{\w+\}>\s*|<\w+>\s*|<\[(?:\w+(?::[-+\.\/\*=\w+]+)?)\]>\s*)+)$)");

    std::smatch args_match;
    if (std::regex_match(template_str, args_match, template_command_syntax)) {
        tcmd_tmp.name_call_component = args_match[1].str();
        tcmd_tmp.returnable          = args_match[3].str();

        string str_params = args_match[2].str();
        std::regex params(R"(\w+)");
        for (auto it_match = std::sregex_iterator(str_params.begin(), str_params.end(), params);
             it_match != std::sregex_iterator(); ++it_match)
            tcmd_tmp.name_accept_params.push_back(it_match->str());

        string str_args = args_match[4].str();
        std::regex args(R"(\s*(\w+|<\'[-+\.\/\*=\w]*\'>|<\{\w+\}>|<\w+>|<\[(?:\w+(?::[-+\.\/\*=\w+]+)?)\]>)(?=\s|$))");
        for (auto it_match = std::sregex_iterator(str_args.begin(), str_args.end(), args);
             it_match != std::sregex_iterator(); ++it_match) {
            template_command::arg arg_tmp;
            string str_arg = (*it_match)[1].str();

            if (str_arg[0] == '<') {
                str_arg.erase(0, 1);
                if (std::isalpha(str_arg[0])) {
                    arg_tmp.arg_t = template_command::arg::type::extglobal;
                    str_arg.erase(str_arg.size() - 1, 1);
                }
                else {
                    if (str_arg[0] == '\'')
                        arg_tmp.arg_t = template_command::arg::type::string;
                    else if (str_arg[0] == '{')
                        arg_tmp.arg_t = template_command::arg::type::internal;
                    else if (str_arg[0] == '[')
                        arg_tmp.arg_t = template_command::arg::type::trgfield;
                    else
                        throw std::runtime_error("Unexpected type of arg(" + str_arg + "): " + template_str);

                    str_arg.erase(0, 1);
                    str_arg.erase(str_arg.size() - 2, 2);
                    if (arg_tmp.arg_t == template_command::arg::type::internal &&
                        std::find(tcmd_tmp.name_accept_params.begin(), tcmd_tmp.name_accept_params.end(), str_arg) ==
                            tcmd_tmp.name_accept_params.end())
                        throw std::runtime_error("Template argument does not exist internally(" + str_arg +
                                                 "): " + template_str);
                    else if (arg_tmp.arg_t == template_command::arg::type::trgfield) {
                        string target_field_arg = str_arg;
                        if (target_field_arg.find(":") != target_field_arg.npos)
                            target_field_arg.erase(target_field_arg.find(":"));

                        if (std::find(name_field_target.begin(), name_field_target.end(), target_field_arg) ==
                            name_field_target.end())
                            throw std::runtime_error("Field does not exist in target structure(" + target_field_arg +
                                                     "): " + template_str);
                    }
                }
            }
            else
                arg_tmp.arg_t = template_command::arg::type::features;
            arg_tmp.str_arg = str_arg;

            tcmd_tmp.args.push_back(arg_tmp);
        }
    }
    else
        _log << (log_message(log_type::fatal) << "Invalid syntax. Expected: call_component(PARAM1, PARAM2, ...) -> "
                                                 "returnable: ARG_FEATURE "
                                                 "<ARG_EXTERNAL> <'ARG_STRING'> <{ARG_PARAM}> <[ARG_TARGET_FIELD]>:\n"
                                              << template_str);

    return tcmd_tmp;
}
vec<template_command> template_command::create_queue_target_templates(const vec<template_command> &templates,
                                                                      const vec<string> &templates_target,
                                                                      target::e_type target_t) {

    vec<template_command> vec_templates_tmp;
    vec<template_command> target_queue_templates;

    for (size_t i = 0; i < templates_target.size(); ++i) {
        for (const auto &_template : templates)
            if (templates_target[i] == _template.name)
                vec_templates_tmp.push_back(_template);
    }

    const auto &it_template =
        find_if(vec_templates_tmp.begin(), vec_templates_tmp.end(), [target_t](const sc::template_command &_template) {
            return _template.returnable == target_type_str(target_t);
        });

    if (it_template == vec_templates_tmp.end())
        _log << (log_message(log_type::fatal) << "There is no template that returns a target with the given type: "
                                              << sc::target_type_str(target_t));

    target_queue_templates.push_back(*it_template);
    for (size_t i = 0; i < it_template->name_accept_params.size(); ++i)
        recovery_queue_target_templates(vec_templates_tmp, target_queue_templates, it_template->name_accept_params[i]);

    for (size_t i = 0; i < templates_target.size(); ++i) {
        for (const auto &_template : templates)
            if (templates_target[i] == _template.name && _template.returnable == NAME_FIELD_PROJECT_SRC_FILES)
                target_queue_templates.push_back(_template);
    }

    std::reverse(target_queue_templates.begin(), target_queue_templates.end());

    return target_queue_templates;
}

void template_command::recovery_queue_target_templates(vec<template_command> &vec_templates,
                                                       vec<template_command> &target_queue_templates,
                                                       const string &name_internal_param) {
    const auto &it =
        find_if(vec_templates.begin(), vec_templates.end(), [name_internal_param](const template_command &_template) {
            return _template.returnable == name_internal_param;
        });
    if (it == vec_templates.end())
        return;

    target_queue_templates.push_back(*it);
    for (size_t i = 0; i < it->name_accept_params.size(); ++i)
        recovery_queue_target_templates(vec_templates, target_queue_templates, it->name_accept_params[i]);
}
