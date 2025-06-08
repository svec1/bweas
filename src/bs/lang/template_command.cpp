#include <algorithm>
#include <array>
#include <regex>

#include <bwtools.hpp>

#include "template_command.hpp"

template_command template_command::create_template_command(string_v template_name, const string &template_str) {
    static const std::array<string, 17> name_field_target = {
        NAME_FIELD_TARGET_NAME,        NAME_FIELD_TARGET_LIBS,        NAME_FIELD_TARGET_TYPE,
        NAME_FIELD_TARGET_CFG,         NAME_FIELD_TARGET_VER,         NAME_FIELD_PROJECT_NAME,
        NAME_FIELD_PROJECT_VER,        NAME_FIELD_PROJECT_LANG,       NAME_FIELD_PROJECT_PCOMPILER,
        NAME_FIELD_PROJECT_PLINKER,    NAME_FIELD_PROJECT_RFCOMPILER, NAME_FIELD_PROJECT_RFLINKER,
        NAME_FIELD_PROJECT_DFCOMPILER, NAME_FIELD_PROJECT_DFLINKER,   NAME_FIELD_PROJECT_STD_C,
        NAME_FIELD_PROJECT_STD_CPP,    NAME_FIELD_PROJECT_SRC_FILES};

    template_command tcmd_tmp;
    tcmd_tmp.name = template_name;

    string tmp_param;

    std::regex template_command_syntax(
        R"(^(\w+)\(\s*(\w+(?:\s*,\s*\w+)*\s*)\)\s*->\s*(\w+):\s*((?:\w+(?::\w+)?\s*|<\'[-+=\w]*\'>\s*|<\{\w+\}>\s*|<\w+>\s*|<\[(?:\w+(?::\w+)?)\]>\s*)+)$)");

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
        std::regex args(R"(\s*(\w+(?::\w+)?|<\'[-+=\w]*\'>|<\{\w+\}>|<\w+>|<\[(?:\w+(?::\w+)?)\]>)(?=\s|$))");
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
                    else if (str_arg[0] == '[') {
                        arg_tmp.arg_t = template_command::arg::type::trgfield;

                        string internal_arg = str_arg;
                        if (tmp_param.find(":") != tmp_param.npos)
                            internal_arg.erase(internal_arg.find(":"));

                        if (std::find(name_field_target.begin(), name_field_target.end(), internal_arg) ==
                            name_field_target.end())
                            throw std::runtime_error("Field does not exist in target structure(" + str_arg +
                                                     "): " + template_str);
                    }
                    else
                        throw std::runtime_error("Unexpected type of arg(" + str_arg + "): " + template_str);

                    str_arg.erase(0, 1);
                    str_arg.erase(str_arg.size() - 2, 2);
                    if (arg_tmp.arg_t == template_command::arg::type::internal &&
                        std::find(tcmd_tmp.name_accept_params.begin(), tcmd_tmp.name_accept_params.end(), str_arg) ==
                            tcmd_tmp.name_accept_params.end())
                        throw std::runtime_error("Template argument does not exist internally(" + str_arg +
                                                 "): " + template_str);
                }
            }
            else
                arg_tmp.arg_t = template_command::arg::type::features;
            arg_tmp.str_arg = str_arg;

            tcmd_tmp.args.push_back(arg_tmp);
        }
    }
    else
        throw std::runtime_error(
            "Invalid syntax. Expected: call_component(PARAM1 PARAM2 ...) -> returnable: ARG_FEATURE "
            "<ARG_EXTERNAL> <'ARG_STRING'> <{ARG_PARAM}> <[ARG_TARGET_FIELD]>");

    return tcmd_tmp;
}
