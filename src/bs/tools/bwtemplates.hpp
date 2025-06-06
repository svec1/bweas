#ifndef BWTEMPLATES_HPP
#define BWTEMPLATES_HPP

#include <bwaliases.hpp>

#include <algorithm>

// enum of the name field of target struct
static constexpr auto NAME_FIELD_TARGET_NAME = "TARGET_NAME";
static constexpr auto NAME_FIELD_TARGET_LIBS = "TARGET_LIBS";
static constexpr auto NAME_FIELD_TARGET_TYPE = "TARGET_TYPE";
static constexpr auto NAME_FIELD_TARGET_CFG  = "TARGET_CFG";
static constexpr auto NAME_FIELD_TARGET_VER  = "TARGET_VER";

// enum of the name field of project struct
static constexpr auto NAME_FIELD_PROJECT_NAME       = "T_PROJECT_NAME";
static constexpr auto NAME_FIELD_PROJECT_VER        = "T_PROJECT_VER";
static constexpr auto NAME_FIELD_PROJECT_LANG       = "T_PROJECT_LANG";
static constexpr auto NAME_FIELD_PROJECT_PCOMPILER  = "T_PROJECT_PATH_COMPILER";
static constexpr auto NAME_FIELD_PROJECT_PLINKER    = "T_PROJECT_PATH_LINKER";
static constexpr auto NAME_FIELD_PROJECT_RFCOMPILER = "T_PROJECT_RFLAGS_COMPILER";
static constexpr auto NAME_FIELD_PROJECT_RFLINKER   = "T_PROJECT_RFLAGS_LINKER";
static constexpr auto NAME_FIELD_PROJECT_DFCOMPILER = "T_PROJECT_DFLAGS_COMPILER";
static constexpr auto NAME_FIELD_PROJECT_DFLINKER   = "T_PROJECT_DFLAGS_LINKER";
static constexpr auto NAME_FIELD_PROJECT_STD_C      = "T_PROJECT_STANDART_C";
static constexpr auto NAME_FIELD_PROJECT_STD_CPP    = "T_PROJECT_STANDART_CPP";
static constexpr auto NAME_FIELD_PROJECT_SRC_FILES  = "T_PROJECT_SRC_FILES";

static constexpr auto FEATURE_FIELD_BS_CURRENT_IF = "FBS_CURRENT_INPUT_FILE";
static constexpr auto FEATURE_FIELD_BS_CURRENT_OF = "FBS_CURRENT_OUTPUT_FILE";

class template_command {
  public:
    template_command() = default;

  public:
    struct arg {
      public:
        enum class type {
            extglobal = 0,
            trgfield,
            string,
            internal,
            features
        };

      public:
        arg()            = default;
        arg(const arg &) = default;
        arg(string _arg, type _arg_t) : str_arg(_arg), arg_t(_arg_t) {
        }

      public:
        string str_arg;
        type arg_t;
    };

  public:
    static inline template_command create_template_command(string_v template_name, const string &template_str);

  public:
    string name;

    string name_call_component;
    vec<string> name_accept_params;
    string returnable;

    vec<arg> args;
};

template_command template_command::create_template_command(string_v template_name, const string &template_str) {
    static const std::array<string, 17> name_field_target = {
        NAME_FIELD_TARGET_NAME,        NAME_FIELD_TARGET_LIBS,        NAME_FIELD_TARGET_TYPE,
        NAME_FIELD_TARGET_CFG,         NAME_FIELD_TARGET_VER,         NAME_FIELD_PROJECT_NAME,
        NAME_FIELD_PROJECT_VER,        NAME_FIELD_PROJECT_LANG,       NAME_FIELD_PROJECT_PCOMPILER,
        NAME_FIELD_PROJECT_PLINKER,    NAME_FIELD_PROJECT_RFCOMPILER, NAME_FIELD_PROJECT_RFLINKER,
        NAME_FIELD_PROJECT_DFCOMPILER, NAME_FIELD_PROJECT_DFLINKER,   NAME_FIELD_PROJECT_STD_C,
        NAME_FIELD_PROJECT_STD_CPP,    NAME_FIELD_PROJECT_SRC_FILES};

    template_command tcmd_tmp;
    template_command::arg arg_tmp;
    tcmd_tmp.name = template_name;

    string tmp_param;

    bool defined_template = 0;

    bool op_close_acp_param      = 0;
    bool op_minus_and_next_arrow = 0;

    bool colon = 0;

    bool beg_param = 0, end_param = 0;

    // To access internal parameters passed to the template
    // <{SMT_PARAMETR}>
    bool used_internal_param           = 0;
    bool was_close_used_internal_param = 0;

    bool op_comma = 0;

    // Regular line
    // <'string'>
    bool op_quote        = 0;
    bool was_close_quote = 0;

    // To access the fields of the current target during assembly, to obtain its information
    // <[NAME_FIELD_TARGET_STRCTURE]>
    bool op_target_hand_field     = 0;
    bool was_close_op_target_hand = 0;

    bool is_name_features_bs = 0;

    bool was_sep = 0;

    for (size_t i = 0; i < template_str.size(); ++i) {
        if ((template_str[i] == ' ' || template_str[i] == '\n' || template_str[i] == '\t') && !op_quote) {
            // te xt <- it is forbidden
            if (!tmp_param.empty())
                was_sep = 1;
            else if (tmp_param.empty() && was_sep) // after processing of operator
                was_sep = 0;
            continue;
        }
        else if ((op_quote && template_str[i] != '\'') ||
                 (is_name_features_bs &&
                  (template_str[i] == ':' || isdigit(template_str[i]) || isalpha(template_str[i]))))
            goto curr_sym;

        if (template_str[i] == '(') {
            if (defined_template)
                throw std::runtime_error("Unexpected operator of parameter enum of template: " + template_str);
            else if (tmp_param.empty())
                throw std::runtime_error("The name of the call component is empty(" + tmp_param + "): " + template_str);
            else if (!tcmd_tmp.name_call_component.empty())
                throw std::runtime_error("The name of the call component that already exists(" + tmp_param +
                                         "): " + template_str);

            tcmd_tmp.name_call_component = tmp_param;
            tmp_param.clear();

            defined_template   = 1;
            op_close_acp_param = 1;

            continue;
        }
        else if (template_str[i] == ')') {
            if (!op_close_acp_param)
                throw std::runtime_error("Unexpected operator(" + tmp_param + "): " + template_str);
            else if (tmp_param.empty())
                throw std::runtime_error("The name of the parameters of call component is empty(" + tmp_param +
                                         "): " + template_str);
            else if (tmp_param == "NULL")
                goto next_op_mn;
            else if (std::find(tcmd_tmp.name_accept_params.begin(), tcmd_tmp.name_accept_params.end(), tmp_param) !=
                     tcmd_tmp.name_accept_params.end())
                throw std::runtime_error("The parameter was already specified in the internal parameters(" + tmp_param +
                                         "): " + template_str);

            tcmd_tmp.name_accept_params.push_back(tmp_param);

        next_op_mn:
            tmp_param.clear();

            op_minus_and_next_arrow = 1;
            op_close_acp_param      = 0;

            continue;
        }
        else if (template_str[i] == '-') {
            if (!op_minus_and_next_arrow || i == template_str.size() - 1 || template_str[i + 1] != '>')
                throw std::runtime_error("Unexpected operator - \'-\'(" + tmp_param + "): " + template_str);

            colon                   = 1;
            op_minus_and_next_arrow = 0;

            ++i;
            continue;
        }
        else if (template_str[i] == ':') {
            if (op_quote || op_target_hand_field)
                goto curr_sym;
            if (!colon)
                throw std::runtime_error("Unexpected operator - \':\'(" + tmp_param + "): " + template_str);
            else if (tmp_param.empty())
                throw std::runtime_error("The name of the returned value of call component is empty(" + tmp_param +
                                         "): " + template_str);

            beg_param = 1;
            colon     = 0;

            tcmd_tmp.returnable = tmp_param;
            tmp_param.clear();

            continue;
        }
        else if (beg_param) {
            if (isalpha(template_str[i])) {
                beg_param = 0;

                op_comma            = 1;
                is_name_features_bs = 1;
                goto curr_sym;
            }
            else if (template_str[i] != '<')
                throw std::runtime_error("The enumeration of parameters is expected(" + tmp_param +
                                         "): " + template_str);

            beg_param = 0;
            end_param = 1;

            continue;
        }
        else if (end_param && template_str[i] == '>') {
            if ((was_close_used_internal_param || was_close_quote || was_close_op_target_hand) && tmp_param == "NULL") {
                was_close_used_internal_param = 0;
                goto next_arg;
            }

            if (used_internal_param || op_quote || op_target_hand_field)
                throw std::runtime_error(
                    "The construction of using internal arguments, just string and handle to field"
                    " of target must comply with this structure - {OPEN_OPERATOR}SOMETHING{CLOSE_OPERATOR}(" +
                    tmp_param + "): " + template_str);
            else if (was_close_used_internal_param &&
                     std::find(tcmd_tmp.name_accept_params.begin(), tcmd_tmp.name_accept_params.end(), tmp_param) ==
                         tcmd_tmp.name_accept_params.end())
                throw std::runtime_error("Template argument does not exist internally(" + tmp_param +
                                         "): " + template_str);
            else if (was_close_quote) {
                arg_tmp.arg_t   = template_command::arg::type::string;
                was_close_quote = 0;
            }
            else if (was_close_op_target_hand) {
                if (std::find(name_field_target.begin(), name_field_target.end(), tmp_param) ==
                    name_field_target.end()) {
                    if (tmp_param.find(":") != tmp_param.npos) {
                        string str_tmp{tmp_param};
                        str_tmp.erase(str_tmp.find(":"));

                        if (std::find(name_field_target.begin(), name_field_target.end(), str_tmp) !=
                            name_field_target.end())
                            goto proc_hand_target_field;
                    }
                    throw std::runtime_error("Field does not exist in target structure(" + tmp_param +
                                             "): " + template_str);
                }
            proc_hand_target_field:
                arg_tmp.arg_t            = template_command::arg::type::trgfield;
                was_close_op_target_hand = 0;
            }
            else if (was_close_used_internal_param) {
                arg_tmp.arg_t                 = template_command::arg::type::internal;
                was_close_used_internal_param = 0;
            }
            else
                arg_tmp.arg_t = template_command::arg::type::extglobal;
            arg_tmp.str_arg = tmp_param;

            if (tmp_param.empty())
                continue;

            tcmd_tmp.args.push_back(arg_tmp);

        next_arg:
            tmp_param.clear();

            end_param = 0;
            op_comma  = 1;
            continue;
        }
        else if (end_param && template_str[i] == '\'') {
            if ((!tmp_param.empty() || !end_param) && !op_quote)
                throw std::runtime_error("The symbol - \' must be after the beginning of the argument(" + tmp_param +
                                         "): " + template_str);
            else if (!end_param && op_quote)
                throw std::runtime_error("The symbol - \' must be at end of the argument(" + tmp_param +
                                         "): " + template_str);

            if (!op_quote)
                op_quote = 1;
            else {
                op_quote        = 0;
                was_close_quote = 1;
            }

            continue;
        }
        else if (end_param && template_str[i] == '[') {
            if (!tmp_param.empty() || !end_param || op_target_hand_field)
                throw std::runtime_error("The symbol - \'[\' must be after the beginning of the argument(" + tmp_param +
                                         "): " + template_str);

            op_target_hand_field = 1;

            continue;
        }
        else if (end_param && template_str[i] == ']') {
            if (!end_param)
                throw std::runtime_error("The symbol - \']\' must be at end of the argument(" + tmp_param +
                                         "): " + template_str);

            op_target_hand_field     = 0;
            was_close_op_target_hand = 1;

            continue;
        }
        else if (end_param && template_str[i] == '{') {
            if (!tmp_param.empty() || !end_param)
                throw std::runtime_error("The symbol - \'}\' must be after the beginning of the argument(" + tmp_param +
                                         "): " + template_str);

            used_internal_param = 1;

            continue;
        }
        else if (end_param && template_str[i] == '}') {
            if (!end_param)
                throw std::runtime_error("The symbol - \'}\' must be at the end of the argument(" + tmp_param +
                                         "): " + template_str);

            used_internal_param           = 0;
            was_close_used_internal_param = 1;

            continue;
        }
        else if (op_close_acp_param && template_str[i] == ',') {
            if (tmp_param == "NULL")
                throw std::runtime_error("If the parameter is zero, there should be no other parameters(" + tmp_param +
                                         "): " + template_str);

            tcmd_tmp.name_accept_params.push_back(tmp_param);
            tmp_param.clear();
            continue;
        }
        else if (op_comma) {
            if (template_str[i] != ',' && !is_name_features_bs)
                throw std::runtime_error("The operator is expected - \',\'" + template_str);
            else if (template_str[i] == ',' && is_name_features_bs) {
                arg_tmp.arg_t   = template_command::arg::type::features;
                arg_tmp.str_arg = tmp_param;
                tcmd_tmp.args.push_back(arg_tmp);
                tmp_param.clear();

                beg_param           = 1;
                is_name_features_bs = 0;
                op_comma            = 0;

                continue;
            }
            else if (is_name_features_bs) {
                if (!isalpha(template_str[i]) && template_str[i] != '_')
                    throw std::runtime_error("The symbol - \'" + std::to_string(template_str[i]) +
                                             "\' was not expected: " + template_str);
                goto curr_sym;
            }
            beg_param = 1;
            op_comma  = 0;

            continue;
        }
    curr_sym:

        if (op_minus_and_next_arrow)
            throw std::runtime_error("The operator is expected - \'->\':" + template_str);
        else if (was_sep) {
            if (beg_param)
                throw std::runtime_error("Pattern parameter start operator expected(" + tmp_param +
                                         "): " + template_str);
            else if (end_param)
                throw std::runtime_error("Pattern parameter end operator expected(" + tmp_param + "): " + template_str);
            else if (is_name_features_bs || op_comma)
                throw std::runtime_error("Expected comma operator(" + tmp_param + "): " + template_str);
        }
        tmp_param += template_str[i];
    }

    if (beg_param)
        throw std::runtime_error("Template argument expected: " + template_str);
    else if (is_name_features_bs) {
        arg_tmp.arg_t   = template_command::arg::type::features;
        arg_tmp.str_arg = tmp_param;
        tcmd_tmp.args.push_back(arg_tmp);
    }

    return tcmd_tmp;
}

#endif
