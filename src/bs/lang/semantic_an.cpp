#include "../../mdef.hpp"

#include "static_linking_func.hpp"
#include "semantic_an.hpp"

using namespace semantic_an;

semantic_analyzer::semantic_analyzer(){
    if(!init_glob){
        init_glob = 1;
        assist.add_err("SMT000", "The function symbol was not found when creating the symbol table");
        assist.add_err("SMT001", "The function values passed do not correspond to the expected ones");
        assist.add_err("SMT002", "Incorrect definition of function parameter types was observed");
        assist.add_err("SMT003", "When going through the aef, at one of the stages of semantic analysis, an undefined error was noticed");
        assist.add_err("SMT004", "There is no variable with the expected type");
        assist.add_err("SMT005", "Arithmetic operations are prohibited at the external level");
        assist.add_err("SMT006", "It's not possible to compare non-numeric values");
        assist.add_err("SMT007", "It's not possible to use a structural variable in the set command");
        assist.add_err("SMT008", "It's impossible to determine the type of a non-existent variable");
        assist.add_err("SMT009", "Undefined error, it's impossible to determine the type of token");
        assist.add_err("SMT010", "A new symbol was expected");
        assist.add_err("SMT011", "Expected an existing symbol");
    }
}

void semantic_analyzer::add_func_flink(
    std::string name_token_func,
    void(*func_ref)(const std::vector<subexpressions>&, var::scope& curr_scope),
    std::vector<params> expected_args){
    
    parser::notion_func nfunc;
    nfunc.func_ref = func_ref;
    nfunc.expected_args = expected_args;

    defining_call_func(name_token_func, nfunc);

    std::pair<std::string, parser::notion_func> n_link_func_spec = {name_token_func, nfunc};

    notion_all_func.insert(n_link_func_spec);
}


void semantic_analyzer::analysis(abstract_expr_func &expr_s, var::scope& global_scope){
    smt_zero_pass(expr_s);
    smt_first_pass(expr_s);
    smt_second_pass(expr_s, global_scope);
}

void semantic_analyzer::load_external_func_table(const table_func& notion_external_func){
    this->notion_external_func = notion_external_func;
}
void semantic_analyzer::load_list_func_with_semantic_an(const std::vector<std::string>& list_name_func){
    this->name_func_with_semantic_an = list_name_func;
}

void semantic_analyzer::smt_zero_pass(const abstract_expr_func &expr_s){
    parser::notion_func nfunc;
    for(u32t i = 0; i < expr_s.size(); ++i){
        if(notion_all_func.find(expr_s[i].expr_func.func_t.token_val) != notion_all_func.end()) continue;
        if(expr_s[i].expr_func.func_t.token_val=="set")
            add_func_flink(expr_s[i].expr_func.func_t.token_val,
                       sl_func::set,
                       {params::FUTURE_VAR_ID, params::ANY_VALUE_WITHOUT_FUTUREID_NEXT, params::NEXT_TOO});
        else if(expr_s[i].expr_func.func_t.token_val=="project")
            add_func_flink(expr_s[i].expr_func.func_t.token_val,
                       sl_func::project,
                       {params::FUTURE_VAR_ID, params::LNUM_OR_ID_VAR, params::LSTR_OR_ID_VAR, params::NEXT_TOO});
        else if(expr_s[i].expr_func.func_t.token_val=="executable")
            add_func_flink(expr_s[i].expr_func.func_t.token_val,
                       sl_func::executable,
                       {params::FUTURE_VAR_ID, params::VAR_STRUCT_ID});
        else if(expr_s[i].expr_func.func_t.token_val=="link_lib")
            add_func_flink(expr_s[i].expr_func.func_t.token_val,
                       sl_func::link_lib,
                       {params::VAR_STRUCT_ID, params::LSTR_OR_ID_VAR, params::NEXT_TOO});
        else if(expr_s[i].expr_func.func_t.token_val=="exp_data")
            add_func_flink(expr_s[i].expr_func.func_t.token_val,
                       sl_func::exp_data,
                       {params::LSTR_OR_ID_VAR});
        else if(expr_s[i].expr_func.func_t.token_val=="cmd")
            add_func_flink(expr_s[i].expr_func.func_t.token_val,
                       sl_func::cmd,
                       {params::LNUM_OR_ID_VAR, params::LSTR_OR_ID_VAR});
        else if(expr_s[i].expr_func.func_t.token_val=="debug")
            add_func_flink(expr_s[i].expr_func.func_t.token_val,
                       sl_func::debug,
                       {params::LSTR_OR_ID_VAR});
        else if(expr_s[i].expr_func.func_t.token_val=="flags_compiler")
            add_func_flink(expr_s[i].expr_func.func_t.token_val,
                       sl_func::flags_compiler,
                       {params::VAR_STRUCT_ID, params::LNUM_OR_ID_VAR, params::LSTR_OR_ID_VAR, params::NEXT_TOO});
        else if(expr_s[i].expr_func.func_t.token_val=="flags_linker")
            add_func_flink(expr_s[i].expr_func.func_t.token_val,
                       sl_func::flags_linker,
                       {params::VAR_STRUCT_ID, params::LNUM_OR_ID_VAR, params::LSTR_OR_ID_VAR, params::NEXT_TOO});
        else if(expr_s[i].expr_func.func_t.token_val=="path_compiler")
            add_func_flink(expr_s[i].expr_func.func_t.token_val,
                       sl_func::path_compiler,
                       {params::VAR_STRUCT_ID, params::LSTR_OR_ID_VAR});
        else if(expr_s[i].expr_func.func_t.token_val=="path_linker")
            add_func_flink(expr_s[i].expr_func.func_t.token_val,
                       sl_func::path_linker,
                       {params::VAR_STRUCT_ID, params::LSTR_OR_ID_VAR});
        else if(expr_s[i].expr_func.func_t.token_val=="standart_c")
            add_func_flink(expr_s[i].expr_func.func_t.token_val,
                       sl_func::standart_c,
                       {params::VAR_STRUCT_ID, params::LNUM_OR_ID_VAR});
        else if(expr_s[i].expr_func.func_t.token_val=="standart_cpp")
            add_func_flink(expr_s[i].expr_func.func_t.token_val,
                       sl_func::standart_cpp,
                       {params::VAR_STRUCT_ID, params::LNUM_OR_ID_VAR});
        else if(expr_s[i].expr_func.func_t.token_val=="lang")
            add_func_flink(expr_s[i].expr_func.func_t.token_val,
                       sl_func::lang,
                       {params::VAR_STRUCT_ID, params::LNUM_OR_ID_VAR});
        else if(expr_s[i].expr_func.func_t.token_val=="add_param_template")
            add_func_flink(expr_s[i].expr_func.func_t.token_val,
                       sl_func::add_param_template,
                       {params::FUTURE_VAR_ID, params::VAR_ID});
        else if(expr_s[i].expr_func.func_t.token_val=="use_templates")
            add_func_flink(expr_s[i].expr_func.func_t.token_val,
                       sl_func::use_templates,
                       {params::VAR_STRUCT_ID, params::LSTR_OR_ID_VAR, params::NEXT_TOO});
        else if(expr_s[i].expr_func.func_t.token_val=="use_it_template")
            add_func_flink(expr_s[i].expr_func.func_t.token_val,
                       sl_func::use_it_template,
                       {params::VAR_STRUCT_ID, params::LNUM_OR_ID_VAR});
        else{
            if(notion_external_func.find(expr_s[i].expr_func.func_t.token_val) != notion_external_func.end()){
                std::pair<std::string, parser::notion_func> n_link_func_spec = 
                    {expr_s[i].expr_func.func_t.token_val, notion_external_func[expr_s[i].expr_func.func_t.token_val]};
                defining_call_func(n_link_func_spec.first, n_link_func_spec.second);
                notion_all_func.insert(n_link_func_spec);
                continue;
            }

            assist.call_err("SMT000", build_pos_tokenb_str(expr_s[i].expr_func.func_t));
        }
    }
}

void semantic_analyzer::smt_first_pass(abstract_expr_func &expr_s){
    for(u32t i = 0; i < expr_s.size(); ++i){
        expr_s[i].expr_func.func_n = notion_all_func[expr_s[i].expr_func.func_t.token_val];
        if(expr_s[i].expr_func.func_n.expected_args.size() != expr_s[i].sub_expr_s.size()){
            if(expr_s[i].expr_func.func_n.expected_args[expr_s[i].expr_func.func_n.expected_args.size()-1] == params::NEXT_TOO &&
              (expr_s[i].expr_func.func_n.expected_args.size()-1 == expr_s[i].sub_expr_s.size() ||
               expr_s[i].expr_func.func_n.expected_args.size() < expr_s[i].sub_expr_s.size()))
               goto check_valied_exp_type;
            assist.call_err("SMT001", build_pos_tokenb_str(expr_s[i].expr_func.func_t));
        }
        else if(expr_s[i].sub_expr_s.size() == 0) continue;
        
check_valied_exp_type:
        for(u32t j = 0; j < expr_s[i].sub_expr_s.size(); ++j){
            if((expr_s[i].expr_func.func_n.expected_args[j] == params::FUTURE_VAR_ID ||
                expr_s[i].expr_func.func_n.expected_args[j] == params::VAR_ID ||
                expr_s[i].expr_func.func_n.expected_args[j] == params::VAR_STRUCT_ID) &&
                expr_s[i].sub_expr_s[j].subexpr_t != subexpressions::type_subexpr::ID) 
               assist.call_err("SMT001", build_pos_subexpr_str(expr_s[i].sub_expr_s[j]) + " Expected: [VAR ID]\n");
            else if(expr_s[i].expr_func.func_n.expected_args[j] == params::ANY_VALUE_WITHOUT_FUTUREID_NEXT &&
               expr_s[i].sub_expr_s[j].subexpr_t != subexpressions::type_subexpr::STRING &&
               expr_s[i].sub_expr_s[j].subexpr_t != subexpressions::type_subexpr::STRING_ADD &&
               expr_s[i].sub_expr_s[j].subexpr_t != subexpressions::type_subexpr::INT &&
               expr_s[i].sub_expr_s[j].subexpr_t != subexpressions::type_subexpr::INT_COMPARE &&
               expr_s[i].sub_expr_s[j].subexpr_t != subexpressions::type_subexpr::ID) 
               assist.call_err("SMT001", build_pos_subexpr_str(expr_s[i].sub_expr_s[j]) + " Expected: [STRING] OR [NUMBER] OR [VAR ID]\n");
            else if(expr_s[i].expr_func.func_n.expected_args[j] == params::LIT_STR &&
               expr_s[i].sub_expr_s[j].subexpr_t != subexpressions::type_subexpr::STRING &&
               expr_s[i].sub_expr_s[j].subexpr_t != subexpressions::type_subexpr::STRING_ADD) 
               assist.call_err("SMT001", build_pos_subexpr_str(expr_s[i].sub_expr_s[j]) + " Expected: [STRING]\n");
            else if(expr_s[i].expr_func.func_n.expected_args[j] == params::LIT_NUM &&
               expr_s[i].sub_expr_s[j].subexpr_t != subexpressions::type_subexpr::INT &&
               expr_s[i].sub_expr_s[j].subexpr_t != subexpressions::type_subexpr::INT_COMPARE) 
               assist.call_err("SMT001", build_pos_subexpr_str(expr_s[i].sub_expr_s[j]) + " Expected: [NUMBER]\n");
            else if(expr_s[i].expr_func.func_n.expected_args[j] == params::LSTR_OR_ID_VAR &&
               expr_s[i].sub_expr_s[j].subexpr_t != subexpressions::type_subexpr::STRING &&
               expr_s[i].sub_expr_s[j].subexpr_t != subexpressions::type_subexpr::STRING_ADD &&
               expr_s[i].sub_expr_s[j].subexpr_t != subexpressions::type_subexpr::ID) 
               assist.call_err("SMT001", build_pos_subexpr_str(expr_s[i].sub_expr_s[j]) + " Expected: [STRING] OR [VAR ID]\n");
            else if(expr_s[i].expr_func.func_n.expected_args[j] == params::LNUM_OR_ID_VAR &&
               expr_s[i].sub_expr_s[j].subexpr_t != subexpressions::type_subexpr::INT &&
               expr_s[i].sub_expr_s[j].subexpr_t != subexpressions::type_subexpr::INT_COMPARE &&
               expr_s[i].sub_expr_s[j].subexpr_t != subexpressions::type_subexpr::ID) 
               assist.call_err("SMT001", build_pos_subexpr_str(expr_s[i].sub_expr_s[j]) + " Expected: [NUMBER] OR [VAR ID]\n");
            else if(expr_s[i].expr_func.func_n.expected_args[j] == params::NEXT_TOO){
                if(j == 0 || j != expr_s[i].expr_func.func_n.expected_args.size()-1)
                    assist.call_err("SMT002", build_pos_tokenb_str(expr_s[i].expr_func.func_t));
                for(u32t b = j; b < expr_s[i].sub_expr_s.size(); ++b){
                    if((expr_s[i].expr_func.func_n.expected_args[j-1] == params::FUTURE_VAR_ID ||
                        expr_s[i].expr_func.func_n.expected_args[j-1] == params::VAR_ID ||
                        expr_s[i].expr_func.func_n.expected_args[j-1] == params::VAR_STRUCT_ID) &&
                        expr_s[i].sub_expr_s[b].subexpr_t != subexpressions::type_subexpr::ID)
                        assist.call_err("SMT001", build_pos_subexpr_str(expr_s[i].sub_expr_s[b]) + " Expected: [VAR ID]\n");
                    else if(expr_s[i].expr_func.func_n.expected_args[j-1] == params::ANY_VALUE_WITHOUT_FUTUREID_NEXT){
                        if(expr_s[i].sub_expr_s[j-1].token_of_subexpr[0].token_t == lexer::token_type::LITERAL &&
                            expr_s[i].sub_expr_s[b].subexpr_t != subexpressions::type_subexpr::INT &&
                            expr_s[i].sub_expr_s[b].subexpr_t != subexpressions::type_subexpr::INT_COMPARE)
                            assist.call_err("SMT001", build_pos_subexpr_str(expr_s[i].sub_expr_s[b]) + " Expected: [NUMBER]\n");
                        else if(expr_s[i].sub_expr_s[j-1].token_of_subexpr[0].token_t == lexer::token_type::LITERALS &&
                                expr_s[i].sub_expr_s[b].subexpr_t != subexpressions::type_subexpr::STRING &&
                                expr_s[i].sub_expr_s[b].subexpr_t != subexpressions::type_subexpr::STRING_ADD)
                            assist.call_err("SMT001", build_pos_subexpr_str(expr_s[i].sub_expr_s[b]) + " Expected: [STRING]\n");
                    }
                    else if(expr_s[i].expr_func.func_n.expected_args[j-1] == params::LIT_STR &&
                            expr_s[i].sub_expr_s[b].subexpr_t != subexpressions::type_subexpr::STRING &&
                            expr_s[i].sub_expr_s[b].subexpr_t != subexpressions::type_subexpr::STRING_ADD)
                            assist.call_err("SMT001", build_pos_subexpr_str(expr_s[i].sub_expr_s[b]) + " Expected: [STRING]\n");
                    else if(expr_s[i].expr_func.func_n.expected_args[j-1] == params::LIT_NUM &&
                            expr_s[i].sub_expr_s[b].subexpr_t != subexpressions::type_subexpr::INT &&
                                expr_s[i].sub_expr_s[b].subexpr_t != subexpressions::type_subexpr::INT_COMPARE)
                            assist.call_err("SMT001", build_pos_subexpr_str(expr_s[i].sub_expr_s[b]) + " Expected: [NUMBER]\n");
                    else if(expr_s[i].expr_func.func_n.expected_args[j-1] == params::LSTR_OR_ID_VAR &&
                            expr_s[i].sub_expr_s[b].subexpr_t != subexpressions::type_subexpr::STRING &&
                            expr_s[i].sub_expr_s[b].subexpr_t != subexpressions::type_subexpr::STRING_ADD &&
                            expr_s[i].sub_expr_s[b].subexpr_t != subexpressions::type_subexpr::ID)
                            assist.call_err("SMT001", build_pos_subexpr_str(expr_s[i].sub_expr_s[b]) + " Expected: [STRING] OR [VAR ID]\n");
                    else if(expr_s[i].expr_func.func_n.expected_args[j-1] == params::LNUM_OR_ID_VAR &&
                            expr_s[i].sub_expr_s[b].subexpr_t != subexpressions::type_subexpr::INT &&
                            expr_s[i].sub_expr_s[b].subexpr_t != subexpressions::type_subexpr::INT_COMPARE &&
                            expr_s[i].sub_expr_s[b].subexpr_t != subexpressions::type_subexpr::ID)
                            assist.call_err("SMT001", build_pos_subexpr_str(expr_s[i].sub_expr_s[b]) + " Expected: [NUMBER] OR [VAR ID]\n");
                }
                break;
            }
        }
    }
}
void semantic_analyzer::smt_second_pass(abstract_expr_func &expr_s, var::scope& curr_scope){
    for(u32t i = 0; i < expr_s.size(); ++i){
        if(expr_s[i].expr_func.func_n.func_ref == sl_func::set){
            u32t tmp_index_type_var;
            u32t index_type = curr_scope.what_type(expr_s[i].sub_expr_s[0].token_of_subexpr[0].token_val);
            if(expr_s[i].sub_expr_s.size() == 0 || expr_s[i].sub_expr_s[0].token_of_subexpr.size() == 0) continue;
            if(index_type == 1){
                for(u32t j = 1; j < expr_s[i].sub_expr_s.size(); ++j){
                    for(u32t b = 0; b < expr_s[i].sub_expr_s[j].token_of_subexpr.size(); ++b){
                        if(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_t == lexer::token_type::OPERATOR &&
                           expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val == "+")
                           assist.call_err("SMT005", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]));
                        else if(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_t == lexer::token_type::ID){
                            tmp_index_type_var = curr_scope.what_type(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val);
                            if(tmp_index_type_var == 0)
                                assist.call_err("SMT008", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) + " Expected: exist variable\n");
                            else if(tmp_index_type_var != 1 && tmp_index_type_var != 3)
                                assist.call_err("SMT004", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) + " Expected: [NUMBER] or [VECTOR NUMBER] or [VAR ID->([NUMBER] or [VECTOR NUMBER])]\n");
                        }
                    }
                }
            }
            else if(index_type == 2){
                for(u32t j = 1; j < expr_s[i].sub_expr_s.size(); ++j){
                    for(u32t b = 0; b < expr_s[i].sub_expr_s[j].token_of_subexpr.size(); ++b){
                        if(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_t == lexer::token_type::OPERATOR &&
                           expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val == ">" ||
                           expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val == "<" ||
                           expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val == "=")
                           assist.call_err("SMT006", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]));
                        else if(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_t == lexer::token_type::ID){
                            tmp_index_type_var = curr_scope.what_type(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val);
                            if(tmp_index_type_var == 0)
                                assist.call_err("SMT008", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) + " Expected: exist variable\n");
                            else if(tmp_index_type_var != 2 && tmp_index_type_var != 4)
                                assist.call_err("SMT004", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) + " Expected: [STRING] or [VECTOR STRING] or [VAR ID->([STRING] or [VECTOR STRING])]\n");
                        }
                    }
                }
            }
            else if(index_type == 3){
                for(u32t j = 1; j < expr_s[i].sub_expr_s.size(); ++j){
                    for(u32t b = 0; b < expr_s[i].sub_expr_s[j].token_of_subexpr.size(); ++b){
                        if(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_t == lexer::token_type::OPERATOR &&
                            expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val == ">" ||
                            expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val == "<" ||
                            expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val == "=")
                            assist.call_err("SMT006", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]));
                        else if(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_t == lexer::token_type::ID){
                                 tmp_index_type_var = curr_scope.what_type(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val);
                                 if(tmp_index_type_var == 0)
                                    assist.call_err("SMT008", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) + " Expected: exist variable\n");
                                 else if(tmp_index_type_var != 1 && tmp_index_type_var != 3)
                                    assist.call_err("SMT004", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) + " Expected: [NUMBER] or [VECTOR NUMBER] or [VAR ID->([NUMBER] or [VECTOR NUMBER])]\n");
                        }
                        else if(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_t != lexer::token_type::LITERALS){
                            assist.call_err("SMT004", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) + " Expected: [NUMBER] or [VECTOR NUMBER] or [VAR ID->([NUMBER] or [VECTOR NUMBER])]\n");
                        }
                    }
                }
            }
            else if(index_type == 4){
                for(u32t j = 1; j < expr_s[i].sub_expr_s.size(); ++j){
                    for(u32t b = 0; b < expr_s[i].sub_expr_s[j].token_of_subexpr.size(); ++b){
                        if(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_t == lexer::token_type::OPERATOR &&
                           expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val == ">" ||
                           expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val == "<" ||
                           expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val == "=")
                           assist.call_err("SMT006", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]));
                        else if(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_t == lexer::token_type::ID){
                                 tmp_index_type_var = curr_scope.what_type(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val);
                                 if(tmp_index_type_var == 0)
                                    assist.call_err("SMT008", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) + " Expected: exist variable\n");
                                 else if(tmp_index_type_var != 2 && tmp_index_type_var != 4)
                                    assist.call_err("SMT004", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) + " Expected: [STRING] or [VECTOR STRING] or [VAR ID->([STRING] or [VECTOR STRING])]\n");
                        }
                        else if(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_t != lexer::token_type::LITERAL){
                            assist.call_err("SMT004", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) + " Expected: [STRING] or [VECTOR STRING] or [VAR ID->([STRING] or [VECTOR STRING])]\n");
                        }
                    }
                }
            }
            else if(index_type == 5 || index_type == 6)
                assist.call_err("SMT007", build_pos_tokenb_str(expr_s[i].sub_expr_s[0].token_of_subexpr[0]));
            else if(index_type == 0){
                bool num = 0, str = 0;
                for(u32t j = 1; j < expr_s[i].sub_expr_s.size(); ++j){
                    if(expr_s[i].sub_expr_s[j].token_of_subexpr.size() == 1 &&
                       expr_s[i].sub_expr_s[j].token_of_subexpr[0].token_t == lexer::token_type::ID){
                        if(j == 1){
                            u32t index_type_id = curr_scope.what_type(expr_s[i].sub_expr_s[j].token_of_subexpr[0].token_val);
                            if(index_type_id == 1 || index_type_id == 3)
                               num = 1;
                            else if(index_type_id == 2 || index_type_id == 4)
                               str = 1;
                            else if(index_type_id == 5 || index_type_id == 6)
                               assist.call_err("SMT007", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[0]));
                            else
                               assist.call_err("SMT008", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[0]) + " Expected: exist variable with any type\n");
                        }
                        else{
                            u32t index_type_id = curr_scope.what_type(expr_s[i].sub_expr_s[j].token_of_subexpr[0].token_val);
                            if(num && index_type_id != 1 && index_type_id != 3)
                                assist.call_err("SMT004", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[0]) + " Expected: [NUMBER] or [VECTOR NUMBER] or [VAR ID->([NUMBER] or [VECTOR NUMBER])]\n");
                            else if(str && index_type_id != 2 && index_type_id != 4)
                                assist.call_err("SMT004", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[0]) + " Expected: [STRING] or [VECTOR STRING] or [VAR ID->([STRING] or [VECTOR STRING])]\n");
                        }
                    }
                    else{
                        bool vec = 0;
                        for(u32t b = 0; b < expr_s[i].sub_expr_s[j].token_of_subexpr.size(); ++b){
                            if(j == 1 && b == 0){
                                if(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_t == lexer::token_type::LITERAL ||
                                   (expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_t == lexer::token_type::OPERATOR &&
                                         expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val == ">" ||
                                         expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val == "<" ||
                                         expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val == "="))
                                   num = 1;
                                else if(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_t == lexer::token_type::LITERALS ||
                                        (expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_t == lexer::token_type::OPERATOR &&
                                         expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val == "+"))
                                   str = 1;
                                else if(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_t == lexer::token_type::ID){
                                    u32t index_type_id = curr_scope.what_type(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val);
                                    if(index_type_id == 1 || index_type_id == 3)
                                        num = 1;
                                    else if(index_type_id == 2 || index_type_id == 4)
                                        str = 1;
                                    else if(index_type_id == 5 || index_type_id == 6)
                                        assist.call_err("SMT007", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]));
                                    else
                                        assist.call_err("SMT008", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) + " Expected: exist variable with any type\n");
                                }
                                else
                                   assist.call_err("SMT009", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) + " Expected: LITERAL or LITERALS or ID\n");
                            }
                            else{
                                u32t tmp_index_type_var = 0;
                                if((num || vec) && expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_t == lexer::token_type::OPERATOR &&
                                   expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val == "+")
                                   assist.call_err("SMT005", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]));
                                else if((str || vec) && expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_t == lexer::token_type::OPERATOR &&
                                       (expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val == ">" ||
                                        expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val == "<" ||
                                        expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val == "=")){
                                        assist.call_err("SMT006", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]));
                                }
                                else if(num && expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_t == lexer::token_type::ID){
                                        tmp_index_type_var = curr_scope.what_type(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val);
                                        if(tmp_index_type_var == 0)
                                            assist.call_err("SMT008", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) + " Expected: exist variable\n");
                                        else if(tmp_index_type_var != 1 && tmp_index_type_var != 3)
                                            assist.call_err("SMT004", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) + " Expected: [NUMBER] or [VECTOR NUMBER] or [VAR ID->([NUMBER] or [VECTOR NUMBER])]\n");
                                        else if(tmp_index_type_var == 3)
                                            vec = 1;
                                }
                                else if(str && expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_t == lexer::token_type::ID){
                                        tmp_index_type_var = curr_scope.what_type(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val);
                                        if(tmp_index_type_var == 0)
                                            assist.call_err("SMT008", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) + " Expected: exist variable\n");
                                        else if(tmp_index_type_var != 2 && tmp_index_type_var != 4)
                                            assist.call_err("SMT004", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) + " Expected: [STRING] or [VECTOR STRING] or [VAR ID->([STRING] or [VECTOR STRING])]\n");
                                        else if(tmp_index_type_var == 4)
                                            vec = 1;
                                }
                                else if((str || vec) && expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_t != lexer::token_type::LITERALS &&
                                                        expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_t != lexer::token_type::OPERATOR)
                                    assist.call_err("SMT004", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) + " Expected: [STRING] or [VECTOR STRING] or [VAR ID->([STRING] or [VECTOR STRING])]\n");
                                else if((num || vec) && expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_t != lexer::token_type::LITERAL &&
                                                        expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_t != lexer::token_type::OPERATOR)
                                    assist.call_err("SMT004", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) + " Expected: [NUMBER] or [VECTOR NUMBER] or [VAR ID->([NUMBER] or [VECTOR NUMBER])]\n");
                            }
                        }
                    }
                }
                num = 0, str = 0;
            }
            for(u32t j = 0; j < expr_s[i].sub_expr_s.size(); ++j){
                parse_subexpr_param(expr_s[i].sub_expr_s[j], expr_s[i].sub_expr_s, j, curr_scope, params::SIZE_ENUM_PARAMS);
            }
            expr_s[i].expr_func.func_n.func_ref(expr_s[i].sub_expr_s, curr_scope);
        }
        else{
            params before_nextt_param = params::SIZE_ENUM_PARAMS;
            for(u32t j = 0; j < expr_s[i].sub_expr_s.size(); ++j){
                if (expr_s[i].sub_expr_s.size() == 0 || expr_s[i].sub_expr_s[0].token_of_subexpr.size() == 0) continue;
                else if (before_nextt_param == params::SIZE_ENUM_PARAMS && expr_s[i].expr_func.func_n.expected_args[j] == params::NEXT_TOO) {
                    before_nextt_param = expr_s[i].expr_func.func_n.expected_args[j - 1];
                }
                for(u32t b = 0; b < expr_s[i].sub_expr_s[j].token_of_subexpr.size(); ++b){
                    if (before_nextt_param != params::SIZE_ENUM_PARAMS) {
                        if (before_nextt_param == params::FUTURE_VAR_ID) {
                            u32t index_type = curr_scope.what_type(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val);
                            if (index_type != 0)
                                assist.call_err("SMT010", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) + " Expected: NEW [VAR ID]\n");
                        }
                        else if (before_nextt_param == params::VAR_STRUCT_ID) {
                            u32t index_type = curr_scope.what_type(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val);
                            if(index_type != 5 && index_type != 6)
                                assist.call_err("SMT011", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) + " Expected: [VAR STRUCT ID]\n");
                        }
                        else if (expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_t == lexer::token_type::ID) {
                            u32t index_type = curr_scope.what_type(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val);
                            if (before_nextt_param == params::LNUM_OR_ID_VAR &&
                                index_type != 1 && index_type != 3)
                                assist.call_err("SMT004", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) + " Expected: [VAR ID->([NUMBER] or [VECTOR NUMBER])]\n");
                            else if (before_nextt_param == params::LSTR_OR_ID_VAR &&
                                index_type != 2 && index_type != 4)
                                assist.call_err("SMT004", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) + " Expected: [VAR ID->([STRING] or [VECTOR STRING])]\n");
                            else if (before_nextt_param == params::FUTURE_VAR_ID && index_type != 0)
                                assist.call_err("SMT008", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) + " Expected: exist variable with any type\n");
                        }
                    }
                    else if(expr_s[i].expr_func.func_n.expected_args[j] == params::FUTURE_VAR_ID){
                        u32t index_type = curr_scope.what_type(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val);
                        if(index_type != 0)
                            assist.call_err("SMT010", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) + " Expected: NEW [VAR ID]\n");
                    }
                    else if(expr_s[i].expr_func.func_n.expected_args[j] == params::VAR_STRUCT_ID){
                        u32t index_type = curr_scope.what_type(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val);
                        if(index_type != 5 && index_type != 6)
                            assist.call_err("SMT011", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) + " Expected: [VAR STRUCT ID]\n");
                    }
                    else if(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_t == lexer::token_type::ID){
                        u32t index_type = curr_scope.what_type(expr_s[i].sub_expr_s[j].token_of_subexpr[b].token_val);
                        if(expr_s[i].expr_func.func_n.expected_args[j] == params::LNUM_OR_ID_VAR &&
                           index_type != 1 && index_type != 3)
                           assist.call_err("SMT004", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) + " Expected: [VAR ID->([NUMBER] or [VECTOR NUMBER])]\n");
                        else if(expr_s[i].expr_func.func_n.expected_args[j] == params::LSTR_OR_ID_VAR &&
                           index_type != 2 && index_type != 4)
                           assist.call_err("SMT004", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) + " Expected: [VAR ID->([STRING] or [VECTOR STRING])]\n");
                        else if(expr_s[i].expr_func.func_n.expected_args[j] == params::VAR_ID && index_type == 0)
                            assist.call_err("SMT008", build_pos_tokenb_str(expr_s[i].sub_expr_s[j].token_of_subexpr[b]) + " Expected: exist variable with any type\n");
                        
                    }
                }
            }
            for(u32t j = 0; j < expr_s[i].sub_expr_s.size(); ++j){
                if(before_nextt_param != params::SIZE_ENUM_PARAMS)
                    parse_subexpr_param(expr_s[i].sub_expr_s[j], expr_s[i].sub_expr_s, j, curr_scope, before_nextt_param);
                else
                    parse_subexpr_param(expr_s[i].sub_expr_s[j], expr_s[i].sub_expr_s, j, curr_scope, expr_s[i].expr_func.func_n.expected_args[j]);
            }
            if(expr_s[i].execute_with_semantic_an()){
                wrap_callf_declaration(expr_s[i].expr_func.func_n.func_ref(expr_s[i].sub_expr_s, curr_scope))
                assist.check_safe_call_dll_func();
            }
        }
    }
}
void semantic_analyzer::parse_subexpr_param(subexpressions& sub_expr, std::vector<subexpressions>& sub_exprs,
                         u32t& pos_sub_expr_in_vec, var::scope& curr_scope, params expected_param){
    subexpressions parse_subexpr, tmp_parse_subexpr;
    if(sub_expr.subexpr_t == subexpressions::type_subexpr::INT_COMPARE){
        for(u32t i = 0; i < sub_expr.token_of_subexpr.size(); ++i){
            if(sub_expr.token_of_subexpr[i].token_t == lexer::token_type::ID){
                parse_subexpr.token_of_subexpr.push_back(
                    lexer::token(lexer::token_type::LITERAL,
                    std::to_string(curr_scope.get_var_value<int>(sub_expr.token_of_subexpr[i].token_val.c_str())),
                    sub_expr.token_of_subexpr[i].pos_defined_line,
                    sub_expr.token_of_subexpr[i].pos_beg_defined_sym)
                );
                if (parse_subexpr.token_of_subexpr.size() == 3)
                    goto parse_integer;
                continue;
            }
            parse_subexpr.token_of_subexpr.push_back(sub_expr.token_of_subexpr[i]);
            if(parse_subexpr.token_of_subexpr.size() == 3){
                parse_integer:
                bool condition = 0;
                switch (parse_subexpr.token_of_subexpr[1].token_val[0])
                {
                case '>':
                    if(std::stoi(parse_subexpr.token_of_subexpr[0].token_val.c_str()) >
                        std::stoi(parse_subexpr.token_of_subexpr[2].token_val.c_str()))
                        condition = 1;
                    break;
                case '<':
                    if(std::stoi(parse_subexpr.token_of_subexpr[0].token_val.c_str()) <
                        std::stoi(parse_subexpr.token_of_subexpr[2].token_val.c_str()))
                        condition = 1;
                    break;
                case '=':
                    if(std::stoi(parse_subexpr.token_of_subexpr[0].token_val.c_str()) ==
                        std::stoi(parse_subexpr.token_of_subexpr[2].token_val.c_str()))
                        condition = 1;
                    break;
                }
                tmp_parse_subexpr = parse_subexpr;
                parse_subexpr.token_of_subexpr.clear();
                if(condition)
                    parse_subexpr.token_of_subexpr.push_back(
                        lexer::token(lexer::token_type::LITERAL, "1",
                        tmp_parse_subexpr.token_of_subexpr[0].pos_defined_line,
                        tmp_parse_subexpr.token_of_subexpr[0].pos_beg_defined_sym)
                    );
                else
                    parse_subexpr.token_of_subexpr.push_back(
                        lexer::token(lexer::token_type::LITERAL, "0",
                        tmp_parse_subexpr.token_of_subexpr[0].pos_defined_line,
                        tmp_parse_subexpr.token_of_subexpr[0].pos_beg_defined_sym)
                    );
            }
        }
    }
    else if(sub_expr.subexpr_t == subexpressions::type_subexpr::STRING_ADD){
        for(u32t i = 0; i < sub_expr.token_of_subexpr.size(); ++i){
            if(sub_expr.token_of_subexpr[i].token_t == lexer::token_type::ID){
                parse_subexpr.token_of_subexpr.push_back(
                    lexer::token(lexer::token_type::LITERALS,
                    curr_scope.get_var_value<std::string>(sub_expr.token_of_subexpr[i].token_val),
                    sub_expr.token_of_subexpr[i].pos_defined_line,
                    sub_expr.token_of_subexpr[i].pos_beg_defined_sym)
                );
                if (parse_subexpr.token_of_subexpr.size() == 3)
                    goto parse_string;
                continue;
            }
            parse_subexpr.token_of_subexpr.push_back(sub_expr.token_of_subexpr[i]);
            if(parse_subexpr.token_of_subexpr.size() == 3){
                parse_string:
                tmp_parse_subexpr = parse_subexpr;
                parse_subexpr.token_of_subexpr.clear();
                parse_subexpr.token_of_subexpr.push_back(
                    lexer::token(lexer::token_type::LITERALS, tmp_parse_subexpr.token_of_subexpr[0].token_val+
                                                              tmp_parse_subexpr.token_of_subexpr[2].token_val,
                        tmp_parse_subexpr.token_of_subexpr[0].pos_defined_line,
                        tmp_parse_subexpr.token_of_subexpr[0].pos_beg_defined_sym)
                );
            }
        }
    }
    else if(sub_expr.subexpr_t == subexpressions::type_subexpr::ID && expected_param != params::VAR_ID){
        u32t index_var = curr_scope.what_type(sub_expr.token_of_subexpr[0].token_val);
        if(index_var == 1){
            parse_subexpr.subexpr_t = subexpressions::type_subexpr::INT;
            parse_subexpr.token_of_subexpr.push_back(
                lexer::token(lexer::token_type::LITERAL,
                std::to_string(curr_scope.get_var_value<int>(sub_expr.token_of_subexpr[0].token_val.c_str())),
                sub_expr.token_of_subexpr[0].pos_defined_line,
                sub_expr.token_of_subexpr[0].pos_beg_defined_sym)
            );
        }
        else if(index_var == 2){
            parse_subexpr.subexpr_t = subexpressions::type_subexpr::STRING;
            parse_subexpr.token_of_subexpr.push_back(
                lexer::token(lexer::token_type::LITERALS,
                curr_scope.get_var_value<std::string>(sub_expr.token_of_subexpr[0].token_val),
                sub_expr.token_of_subexpr[0].pos_defined_line,
                sub_expr.token_of_subexpr[0].pos_beg_defined_sym)
            );
        }
        else if(index_var == 3){
            std::vector<subexpressions> new_sub_exprs;
            subexpressions tmp_sub_expr;
            tmp_sub_expr.subexpr_t = subexpressions::type_subexpr::INT;
            const std::vector<int> vec_int_id = curr_scope.get_var_value<std::vector<int>>(sub_expr.token_of_subexpr[0].token_val);
            for(u32t i = 0; i < pos_sub_expr_in_vec; ++i)
                new_sub_exprs.push_back(sub_exprs[i]);
            for(u32t i = 0; i < vec_int_id.size(); ++i){
                tmp_sub_expr.token_of_subexpr.push_back(
                    lexer::token(lexer::token_type::LITERAL,
                    std::to_string(vec_int_id[i]),
                    sub_expr.token_of_subexpr[0].pos_defined_line,
                    sub_expr.token_of_subexpr[0].pos_beg_defined_sym)
                );
                new_sub_exprs.push_back(tmp_sub_expr);
                tmp_sub_expr.token_of_subexpr.clear();
            }
            for(u32t i = pos_sub_expr_in_vec+1; i < sub_exprs.size(); ++i)
                new_sub_exprs.push_back(sub_exprs[i]);
            pos_sub_expr_in_vec += vec_int_id.size()-1;
            sub_exprs = new_sub_exprs;
        }
        else if(index_var == 4){
            std::vector<subexpressions> new_sub_exprs;
            subexpressions tmp_sub_expr;
            tmp_sub_expr.subexpr_t = subexpressions::type_subexpr::STRING;
            const std::vector<std::string> vec_str_id = curr_scope.get_var_value<std::vector<std::string>>(sub_expr.token_of_subexpr[0].token_val);
            for(u32t i = 0; i < pos_sub_expr_in_vec; ++i)
                new_sub_exprs.push_back(sub_exprs[i]);
            for(u32t i = 0; i < vec_str_id.size(); ++i){
                tmp_sub_expr.token_of_subexpr.push_back(
                    lexer::token(lexer::token_type::LITERALS,
                    vec_str_id[i],
                    sub_expr.token_of_subexpr[0].pos_defined_line,
                    sub_expr.token_of_subexpr[0].pos_beg_defined_sym)
                );
                new_sub_exprs.push_back(tmp_sub_expr);
                tmp_sub_expr.token_of_subexpr.clear();
            }
            for(u32t i = pos_sub_expr_in_vec+1; i < sub_exprs.size(); ++i)
                new_sub_exprs.push_back(sub_exprs[i]);
            pos_sub_expr_in_vec += vec_str_id.size()-1;
            sub_exprs = new_sub_exprs;
        }
        if(parse_subexpr.token_of_subexpr.size() > 0)
            sub_expr = parse_subexpr;
        return;
    }
    else
        return;

    if(parse_subexpr.token_of_subexpr[0].token_t == lexer::token_type::LITERAL)
            parse_subexpr.subexpr_t = subexpressions::type_subexpr::INT;
    else
        parse_subexpr.subexpr_t = subexpressions::type_subexpr::STRING;
    sub_expr = parse_subexpr;
}
void semantic_analyzer::defining_call_func(const std::string& name, notion_func& nfunc){
    if(std::find(nfunc.expected_args.begin(), nfunc.expected_args.end(), params::FUTURE_VAR_ID) != nfunc.expected_args.end())
        nfunc.is_declaration_var = 1;
    else if(std::find(name_func_with_semantic_an.begin(), name_func_with_semantic_an.end(), name) != name_func_with_semantic_an.end())
        nfunc.only_with_semantic = 1;
}