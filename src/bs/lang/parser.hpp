#ifndef _PARSER__H
#define _PARSER__H

#include "lexer.hpp"
#include "scope.hpp"

#include <vector>
#include <sstream>

namespace parser{

    enum class params{
        FUTURE_VAR_ID,
        VAR_ID,
        VAR_STRUCT_ID,
        ANY_VALUE_WITHOUT_FUTUREID_NEXT,
        LIT_STR,
        LIT_NUM,
        LSTR_OR_ID_VAR,
        LNUM_OR_ID_VAR,
        NEXT_TOO,
        SIZE_ENUM_PARAMS
    };

    struct subexpressions;
    struct notion_func{
        notion_func() = default;
        notion_func(void(*func)(const std::vector<subexpressions>&, var::scope&), std::vector<params> params)
                    : func_ref(func),  expected_args(params){}
        void(*func_ref)(const std::vector<subexpressions>&, var::scope&);
        std::vector<params> expected_args;
    };

    struct notion_expr_func{
        lexer::token func_t;
        notion_func func_n;
    };

    struct expression{
        notion_expr_func expr_func;
        std::vector<subexpressions> sub_expr_s;
    };
    struct subexpressions{

        enum class type_subexpr{
            INT_COMPARE,
            STRING_ADD,
            INT,
            STRING,
            ID
        };

        std::vector<lexer::token> token_of_subexpr;
        type_subexpr subexpr_t;
    };

    // abstract expression of functions
    using abstract_expr_func = std::vector<expression>;
    
    extern std::string type_token_str(lexer::token_type token_t);
    extern std::string build_pos_tokenb_str(const lexer::token& tk);
    extern std::string build_pos_subexpr_str(const subexpressions& sub_expr);
    extern std::string type_sybexpr_str(subexpressions::type_subexpr sub_expr_t);
    extern std::string tree_build_visually_str(const abstract_expr_func& aef);
    
    extern params type_param_in_str(std::string str);
    
    class pars_an{
        public:
            pars_an();
            pars_an(const std::vector<lexer::token> &tk_s);

            pars_an(pars_an&&) = delete;
            pars_an(const pars_an&) = delete;

            pars_an& operator=(pars_an&&) = delete;

            ~pars_an() = default;

        public:
            void set_tokens(const std::vector<lexer::token> &tk_s);
            abstract_expr_func get_exprs();
            void clear_aef();

            abstract_expr_func analysis();
        
        private:
            void check_valid_subexpr_first_pass(const subexpressions& sub_expr);
            void check_valid_subexpr_second_pass(subexpressions& sub_expr);

            void try_parse_subexpr(subexpressions &sub_expr);

        private:
            static inline bool init_glob{0};

            std::vector<lexer::token> tokens;
            abstract_expr_func expr_s;
    };

}

#endif