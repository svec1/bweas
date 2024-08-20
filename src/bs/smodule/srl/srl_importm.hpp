#ifndef SRL_IMPORTM__H
#define SRL_IMPORTM__H

#include "../../../mdef.hpp"
#include "../../lang/expression.hpp"

#include <unordered_map>

namespace srl {

using param = aef_expr::params;
using func_define = std::unordered_map<std::string, std::vector<param>>; // <name_func>,<params>
using funcs_module = std::vector<func_define>;                           // <func>...s
using module = std::unordered_map<std::string, funcs_module>;            // <name_module>, <<func>...s>
using modules = std::vector<module>;                                     // <module>...s

enum class token_type {
    START_MODULE,
    END_MODULE,

    NAME_MODULE,
    NAME_FUNC_OF_MODULE,

    START_ENUM_PARAMS,
    NEXT_ENUM_PARAMS,
    PARAM
};

struct token {
    token(std::string _val, u32t _lines, u32t _symbols, token_type _token_t, param _token_p = param::SIZE_ENUM_PARAMS)
        : val(_val), lines(_lines), symbols(_symbols), token_t(_token_t), token_p(_token_p) {
    }
    token_type token_t;
    param token_p;

    std::string val;

    u32t lines = 0, symbols = 0;
};

extern void
init_err();

extern std::pair<std::vector<srl::token>, std::vector<std::string>>
parser(std::string symbols);
extern modules
build_modules(std::vector<srl::token> tokens);

} // namespace srl

#endif