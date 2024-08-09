#include "static_linking_func.hpp"
#include "interpreter.hpp"

const std::vector<std::string> sl_func::name_static_func_sm = {"exp_data"};

void sl_func::set(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope){
    if(sub_expr.size() == 0)
        return;
    u32t index_var = curr_scope.what_type(sub_expr[0].token_of_subexpr[0].token_val);
    if(sub_expr.size() == 1){
        if(index_var == 0)
            (void)curr_scope.create_var<std::string>(sub_expr[0].token_of_subexpr[0].token_val);
    }
    else if(sub_expr.size() == 2){
        if(sub_expr[1].subexpr_t == parser::subexpressions::type_subexpr::INT){
            if(index_var == 1)
                curr_scope.get_var_value<int>(sub_expr[0].token_of_subexpr[0].token_val) = std::stoi(sub_expr[1].token_of_subexpr[0].token_val.c_str());
            else if(index_var == 0)
                (void)curr_scope.create_var(sub_expr[0].token_of_subexpr[0].token_val, std::stoi(sub_expr[1].token_of_subexpr[0].token_val.c_str()));
        }
        else if(sub_expr[1].subexpr_t == parser::subexpressions::type_subexpr::STRING){
            if(index_var == 2)
                curr_scope.get_var_value<std::string>(sub_expr[0].token_of_subexpr[0].token_val) = sub_expr[1].token_of_subexpr[0].token_val;
            else if(index_var == 0)
                (void)curr_scope.create_var(sub_expr[0].token_of_subexpr[0].token_val, sub_expr[1].token_of_subexpr[0].token_val);
        }
    }
    else if(sub_expr.size()>2){
        if(sub_expr[1].subexpr_t == parser::subexpressions::type_subexpr::INT){
            std::vector<int> vec_int_params;
            vec_int_params.push_back(std::stoi(sub_expr[1].token_of_subexpr[0].token_val.c_str()));
            for(u32t i = 2; i < sub_expr.size(); ++i){
                if(sub_expr[i].subexpr_t == parser::subexpressions::type_subexpr::INT)
                    vec_int_params.push_back(std::stoi(sub_expr[i].token_of_subexpr[0].token_val.c_str()));
            }
            if(index_var == 0)
                (void)curr_scope.create_var<std::vector<int>>(sub_expr[0].token_of_subexpr[0].token_val, vec_int_params);
            else if(index_var == 3)
                curr_scope.get_var_value<std::vector<int>>(sub_expr[0].token_of_subexpr[0].token_val) = vec_int_params;
        }
        else if(sub_expr[1].subexpr_t == parser::subexpressions::type_subexpr::STRING){
            std::vector<std::string> vec_str_params;
            vec_str_params.push_back(sub_expr[1].token_of_subexpr[0].token_val);
            for(u32t i = 2; i < sub_expr.size(); ++i){
                if(sub_expr[i].subexpr_t == parser::subexpressions::type_subexpr::STRING)
                    vec_str_params.push_back(sub_expr[i].token_of_subexpr[0].token_val);
            }
            if(index_var == 0)
                (void)curr_scope.create_var<std::vector<std::string>>(sub_expr[0].token_of_subexpr[0].token_val, vec_str_params);
            else if(index_var == 4)
                curr_scope.get_var_value<std::vector<std::string>>(sub_expr[0].token_of_subexpr[0].token_val) = vec_str_params;
        }
    }
}

void sl_func::project(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope){
    var::struct_sb::project& prj_ref = curr_scope.create_var<var::struct_sb::project>(sub_expr[0].token_of_subexpr[0].token_val);
    prj_ref.name_project = sub_expr[0].token_of_subexpr[0].token_val;
    prj_ref.lang = (var::struct_sb::language)std::stoi(sub_expr[1].token_of_subexpr[0].token_val);
    for(u32t i = 2; i < sub_expr.size(); ++i)
        prj_ref.src_files.push_back(sub_expr[i].token_of_subexpr[0].token_val);
}
void sl_func::executable(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope){
    if(curr_scope.what_type(sub_expr[1].token_of_subexpr[0].token_val) != 5)
        throw semantic_an::semantic_excp(parser::build_pos_tokenb_str(sub_expr[1].token_of_subexpr[0]) +
                                            " Var id of struct(expected project): " + sub_expr[1].token_of_subexpr[0].token_val + "\n",
                                         "011");
    var::struct_sb::target& trg_ref = curr_scope.create_var<var::struct_sb::target>(sub_expr[0].token_of_subexpr[0].token_val);
    
    trg_ref.name_target = sub_expr[0].token_of_subexpr[0].token_val;
    trg_ref.prj = std::make_shared<var::struct_sb::project>(curr_scope.get_var_value<var::struct_sb::project>(sub_expr[1].token_of_subexpr[0].token_val));
    trg_ref.version_target = var::struct_sb::version(0,0,0);
}

void sl_func::link_lib(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope){    
    if(curr_scope.what_type(sub_expr[0].token_of_subexpr[0].token_val) != 6)
        throw interpreter::realtime_excp(parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                                            " Var id of struct(expected target): " + sub_expr[0].token_of_subexpr[0].token_val + "\n",
                                         "003");
    var::struct_sb::target& trg_ref = curr_scope.get_var_value<var::struct_sb::target>(sub_expr[0].token_of_subexpr[0].token_val);
    for(u32t i = 1; i < sub_expr.size(); ++i)
        trg_ref.target_vec_libs.push_back(sub_expr[i].token_of_subexpr[0].token_val);
}
void sl_func::exp_data(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope){
    interpreter::interpreter_exec::config tmp_conf;
    tmp_conf.filename_interp = sub_expr[0].token_of_subexpr[0].token_val.c_str();
    tmp_conf.import_module = 0;
    tmp_conf.use_external_scope = 1;
    interpreter::interpreter_exec tmp_interpreter(tmp_conf);
    tmp_interpreter.set_external_scope(&curr_scope);
    tmp_interpreter.build_aef();
}
void sl_func::cmd(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope){
    u32t val = std::stoi(sub_expr[0].token_of_subexpr[0].token_val.c_str());
    if(!val)
        curr_scope.call_cmd.add_call_before(sub_expr[1].token_of_subexpr[0].token_val);
    else
        curr_scope.call_cmd.add_call_after(sub_expr[1].token_of_subexpr[0].token_val);
}
void sl_func::debug(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope){
    assist << sub_expr[0].token_of_subexpr[0].token_val;
}

void sl_func::flags_compiler(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope){
    if(curr_scope.what_type(sub_expr[0].token_of_subexpr[0].token_val) != 5)
        throw interpreter::realtime_excp(parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                                            " Var id of struct(expected project): " + sub_expr[0].token_of_subexpr[0].token_val + "\n",
                                         "003");
    var::struct_sb::project& prj_ref =  curr_scope.get_var_value<var::struct_sb::project>(sub_expr[0].token_of_subexpr[0].token_val);
    if((var::struct_sb::configuration)std::stoi(sub_expr[1].token_of_subexpr[0].token_val.c_str()) == var::struct_sb::configuration::RELEASE)
        for(u32t i = 2; i < sub_expr.size(); ++i){
            prj_ref.rflags_compiler += sub_expr[i].token_of_subexpr[0].token_val + " ";
        }
    else
        for(u32t i = 2; i < sub_expr.size(); ++i){
            prj_ref.dflags_compiler += sub_expr[i].token_of_subexpr[0].token_val + " ";
        }
}
void sl_func::flags_linker(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope){
    if(curr_scope.what_type(sub_expr[0].token_of_subexpr[0].token_val) != 5)
        throw interpreter::realtime_excp(parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                                            " Var id of struct(expected project): " + sub_expr[0].token_of_subexpr[0].token_val + "\n",
                                         "003");
    var::struct_sb::project& prj_ref =  curr_scope.get_var_value<var::struct_sb::project>(sub_expr[0].token_of_subexpr[0].token_val);
    if((var::struct_sb::configuration)std::stoi(sub_expr[1].token_of_subexpr[0].token_val.c_str()) == var::struct_sb::configuration::RELEASE)
        for(u32t i = 2; i < sub_expr.size(); ++i){
            prj_ref.rflags_linker += sub_expr[i].token_of_subexpr[0].token_val + " ";
        }
    else
        for(u32t i = 2; i < sub_expr.size(); ++i){
            prj_ref.dflags_linker += sub_expr[i].token_of_subexpr[0].token_val + " ";
        }
}
void sl_func::path_compiler(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope){
    if(curr_scope.what_type(sub_expr[0].token_of_subexpr[0].token_val) != 5)
        throw interpreter::realtime_excp(parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                                            " Var id of struct(expected project): " + sub_expr[0].token_of_subexpr[0].token_val + "\n",
                                         "003");
    curr_scope.get_var_value<var::struct_sb::project>(sub_expr[0].token_of_subexpr[0].token_val).path_compiler = sub_expr[1].token_of_subexpr[0].token_val;
}
void sl_func::path_linker(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope){
    if(curr_scope.what_type(sub_expr[0].token_of_subexpr[0].token_val) != 5)
        throw interpreter::realtime_excp(parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                                            " Var id of struct(expected project): " + sub_expr[0].token_of_subexpr[0].token_val + "\n",
                                         "003");
    curr_scope.get_var_value<var::struct_sb::project>(sub_expr[0].token_of_subexpr[0].token_val).path_linker = sub_expr[1].token_of_subexpr[0].token_val;
}
void sl_func::standart_c(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope){
    if(curr_scope.what_type(sub_expr[0].token_of_subexpr[0].token_val) != 5)
        throw interpreter::realtime_excp(parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                                            " Var id of struct(expected project): " + sub_expr[0].token_of_subexpr[0].token_val + "\n",
                                         "003");
    curr_scope.get_var_value<var::struct_sb::project>(sub_expr[0].token_of_subexpr[0].token_val).standart_c = std::stoi(sub_expr[1].token_of_subexpr[0].token_val.c_str());
}
void sl_func::standart_cpp(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope){
    if(curr_scope.what_type(sub_expr[0].token_of_subexpr[0].token_val) != 5)
        throw interpreter::realtime_excp(parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                                            " Var id of struct(expected project): " + sub_expr[0].token_of_subexpr[0].token_val + "\n",
                                         "003");
    curr_scope.get_var_value<var::struct_sb::project>(sub_expr[0].token_of_subexpr[0].token_val).standart_cpp = std::stoi(sub_expr[1].token_of_subexpr[0].token_val.c_str());
}
void sl_func::lang(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope){
    if(curr_scope.what_type(sub_expr[0].token_of_subexpr[0].token_val) != 5)
        throw interpreter::realtime_excp(parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                                            " Var id of struct(expected project): " + sub_expr[0].token_of_subexpr[0].token_val + "\n",
                                         "003");
    curr_scope.get_var_value<var::struct_sb::project>(sub_expr[0].token_of_subexpr[0].token_val).lang = (var::struct_sb::language)std::stoi(sub_expr[1].token_of_subexpr[0].token_val);
} 

void sl_func::add_param_template(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope){
    curr_scope.create_var<std::string>(sub_expr[0].token_of_subexpr[0].token_val, sub_expr[1].token_of_subexpr[0].token_val);
}
void sl_func::use_templates(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope){
    if(curr_scope.what_type(sub_expr[0].token_of_subexpr[0].token_val) != 5)
        throw interpreter::realtime_excp(parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                                            " Var id of struct(expected project): " + sub_expr[0].token_of_subexpr[0].token_val + "\n",
                                         "003");
    var::struct_sb::project& prj_ref = curr_scope.get_var_value<var::struct_sb::project>(sub_expr[0].token_of_subexpr[0].token_val);
    for(u32t i = 1; i < sub_expr.size(); ++i){
        prj_ref.vec_templates.push_back(sub_expr[i].token_of_subexpr[0].token_val);
    }
}
void sl_func::use_it_template(const std::vector<parser::subexpressions>& sub_expr, var::scope& curr_scope){
    if(curr_scope.what_type(sub_expr[0].token_of_subexpr[0].token_val) != 5)
        throw interpreter::realtime_excp(parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                                            " Var id of struct(expected project): " + sub_expr[0].token_of_subexpr[0].token_val + "\n",
                                         "003");
    
    curr_scope.get_var_value<var::struct_sb::project>(sub_expr[0].token_of_subexpr[0].token_val).use_it_templates = std::stoi(sub_expr[1].token_of_subexpr[0].token_val);
}