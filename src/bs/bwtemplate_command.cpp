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
    template_command tcmd_tmp;
    tcmd_tmp.name = template_name;

    string tmp_param;

    std::regex template_command_syntax(
        R"(^\s*(\w+|[^'"]+:[^'"]+)\(\s*(\w+(?:\s*,\s*\w+)*\s*)\)\s*->\s+(?:(\w+)|\[(\w+)\]):((?:\s+\w+|\s+<\'[^'"]*\'>|\s+<(?:\'[^'"]+\')?\{\w+\}>|\s+<(?:\'[^'"]+\')?\w+>|\s+<(?:\'[^'"]+\')?\[(?:\w+(?::[^'"]+)?)\]>\s*)+)\s*$)");

    std::smatch args_match;
    if (std::regex_match(template_str, args_match, template_command_syntax)) {
        tcmd_tmp.name_call_component = args_match[1].str();
        if (args_match[3].str().empty())
            tcmd_tmp.returnable = template_command::return_value(
                args_match[4].str(), template_command::return_value::e_type::extension_field);
        else
            tcmd_tmp.returnable =
                template_command::return_value(args_match[3].str(), template_command::return_value::e_type::object);

        string str_params = args_match[2].str();
        std::regex params(R"(\w+)");
        for (auto it_match = std::sregex_iterator(str_params.begin(), str_params.end(), params);
             it_match != std::sregex_iterator(); ++it_match)
            tcmd_tmp.name_accept_params.push_back(it_match->str());

        string values = args_match[5].str();
        std::regex args(
            R"(((?:\w+|<\'[^'"]*\'>|<(?:\'[^'"]+\')?\{\w+\}>|<(?:\'[^'"]+\')?\w+>|<(?:\'[^'"]+\')?\[(?:\w+(?::[^'"]+)?)\]>)+))");
        for (auto it_match = std::sregex_iterator(values.begin(), values.end(), args);
             it_match != std::sregex_iterator(); ++it_match) {
            template_command::arg arg_tmp;
            string value = (*it_match)[1].str();

            if (value[0] == '<') {
                value.erase(0, 1);
                if (std::isalpha(value[0])) {
                    arg_tmp.type = template_command::arg::e_type::extglobal;
                    value.erase(value.size() - 1, 1);
                }
                else if (value[0] == '\'') {
                    if (value.find("{") != value.npos)
                        arg_tmp.type = template_command::arg::e_type::internal;
                    else if (value.find("[") != value.npos)
                        arg_tmp.type = template_command::arg::e_type::trgfield;
                    else if (value.find("\'>") == value.npos)
                        arg_tmp.type = template_command::arg::e_type::extglobal;
                    else {
                        arg_tmp.type = template_command::arg::e_type::string;
                        goto value_arg_handler;
                    }

                    size_t offset_to_start_arg = value.find_last_of("\'");

                    arg_tmp.prefix = value.substr(1, offset_to_start_arg - 1);
                    value.erase(0, offset_to_start_arg + 1);
                }
                else if (value[0] == '{')
                    arg_tmp.type = template_command::arg::e_type::internal;
                else if (value[0] == '[')
                    arg_tmp.type = template_command::arg::e_type::trgfield;
                else
                    throw std::runtime_error("Unexpected type of arg(" + value + "): " + template_str);

            value_arg_handler:
                value.erase(0, 1);
                value.erase(value.size() - 2, 2);
                if (arg_tmp.type == template_command::arg::e_type::internal &&
                    std::find(tcmd_tmp.name_accept_params.begin(), tcmd_tmp.name_accept_params.end(), value) ==
                        tcmd_tmp.name_accept_params.end())
                    throw std::runtime_error("Template argument does not exist internally(" + value +
                                             "): " + template_str);
            }
            else
                arg_tmp.type = template_command::arg::e_type::features;
            arg_tmp.value = value;

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
            return _template.returnable.type == sc::template_command::return_value::e_type::object &&
                   _template.returnable.value == target_type_str(target_t);
        });

    if (it_template == vec_templates_tmp.end())
        _log << (log_message(log_type::fatal) << "There is no template that returns a target with the given type: "
                                              << sc::target_type_str(target_t));

    target_queue_templates.push_back(*it_template);
    for (size_t i = 0; i < it_template->name_accept_params.size(); ++i)
        recovery_queue_target_templates(vec_templates_tmp, target_queue_templates, it_template->name_accept_params[i]);

    for (size_t i = 0; i < templates_target.size(); ++i) {
        for (const auto &_template : templates)
            if (templates_target[i] == _template.name &&
                (_template.returnable.type == sc::template_command::return_value::e_type::extension_field &&
                 _template.returnable.value == sc::profile::FIELD_SOURCE_FILES))
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
            return _template.returnable.value == name_internal_param;
        });
    if (it == vec_templates.end())
        return;

    target_queue_templates.push_back(*it);
    for (size_t i = 0; i < it->name_accept_params.size(); ++i)
        recovery_queue_target_templates(vec_templates, target_queue_templates, it->name_accept_params[i]);
}
