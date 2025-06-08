#ifndef BWTEMPLATES_HPP
#define BWTEMPLATES_HPP

#include <bwaliases.hpp>

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
    static template_command create_template_command(string_v template_name, const string &template_str);

  public:
    string name;

    string name_call_component;
    vec<string> name_accept_params;
    string returnable;

    vec<arg> args;
};

#endif
