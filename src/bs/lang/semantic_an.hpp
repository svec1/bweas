#ifndef _SEMANTIC_AN__H
#define _SEMANTIC_AN__H

#include "parser.hpp"
#include "scope.hpp"

#include <unordered_map>
#include <set>

// Semantic analysis:
//  - through the vector of expressions and
//    compiles a table of symbols-function for subsequent linking
//
//    ^^^
//    zero pass
//
// - linking all representations of functions
//   in expressions with functions that describe their behavior.
//   Checks the correspondence of the transmitted types of indicators
//   with the expected types of the function.
//
//   ^^^
//   first pass
//
// - creates a *table* of symbols (creates, in the scope 
//   of the current file that is being processed, all
//   variables that were declared independently, through the set command)
//   and checks the correspondence between indicators.
//
//   ^^^
//   second pass
//
namespace semantic_an{

    using namespace parser;
    using table_decl_symbol = std::set<std::string>;
    using table_func = std::unordered_map<std::string, parser::notion_func>;

    extern var::scope global_scope;

    class semantic_analyzer{
        public:
            semantic_analyzer();
            semantic_analyzer(semantic_analyzer&&) = delete;
            semantic_analyzer(const semantic_analyzer&) = delete;
            semantic_analyzer& operator=(semantic_analyzer&&) = delete;

            ~semantic_analyzer() = default;

        public:
            void analysis(abstract_expr_func& expr_s, var::scope& global_scope);

            void load_external_func_table(const table_func& notion_external_func);

        private:
            // Adding a function definition to the functions table
            void add_func_flink(std::string name_token_func,
                                void(*func_ref)(const std::vector<subexpressions>&, var::scope& curr_scope),
                                std::vector<params> expected_args);
            
            void smt_zero_pass(const abstract_expr_func& expr_s);
            void smt_first_pass(abstract_expr_func& expr_s);

            // The set command (initialization or assignment) is called here
            void smt_second_pass(abstract_expr_func& expr_s, var::scope& curr_scope);

        private:
            // Parses a subexpression if it has not taken the type 
            // INT, STRING, or VAR_STRUCT_ID after parsing at the AEF construction stage
            void parse_subexpr_param(subexpressions& sub_expr, std::vector<subexpressions>& sub_exprs,
                                     u32t& pos_sub_expr_in_vec, var::scope& curr_scope,
                                     params expected_param);
    
        private:
            static inline bool init_glob{0};

            table_decl_symbol table_symbol;    
            
            // functions tables
            table_func notion_all_func;
            table_func notion_external_func;
    };

}

#endif