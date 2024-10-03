#include "interpreter.hpp"
#include "static_linking_func.hpp"

using namespace aef_expr;

const std::vector<std::string> sl_func::name_static_func_sm = {"exp_data"};
static const std::vector<std::string> vec_name_config_var = {
    PRJ_VAR_NAME_DFLAGS_C,     PRJ_VAR_NAME_DFLAGS_L, PRJ_VAR_NAME_RFLAGS_C,  PRJ_VAR_NAME_RFLAGS_L,
    PRJ_VAR_NAME_LANG,         PRJ_VAR_NAME_PTH_C,    PRJ_VAR_NAME_PTH_L,     PRJ_VAR_NAME_VER,
    PRJ_VAR_NAME_STD_C,        PRJ_VAR_NAME_STD_CPP,  PRJ_VAR_NAME_SRC_FILES, PRJ_VAR_NAME_UTEMPLATES,
    PRJ_VAR_NAME_UITTEMPLATES, TRG_VAR_NAME_NPROJECT, TRG_VAR_NAME_VER,       TRG_VAR_NAME_CFG,
    TRG_VAR_NAME_TYPE_T,       TRG_VAR_NAME_LLIBS};

// ??????
static void
update_cfg_struct(const std::string &name_var, var::scope &curr_scope) {
    if (curr_scope.what_type("DECL_CONFIG_VAR") == 1 && curr_scope.get_var_value<int>(DECL_VAR_STRUCT) > 0) {
        u32t tmp_it = 0;
        for (const auto &it : vec_name_config_var) {
            if ((tmp_it = name_var.find(it)) != UINT32_MAX) {
                if (name_var.size() - it.size() != tmp_it)
                    return;
                std::string tmp_str_postfix = name_var;
                std::string tmp_str_prefix = name_var;
                tmp_str_postfix.erase(0, tmp_it);
                tmp_str_prefix.erase(tmp_it, tmp_str_prefix.size());
                if (tmp_str_postfix == PRJ_VAR_NAME_DFLAGS_C) {
                    if (curr_scope.what_type(tmp_str_prefix) != 5 || curr_scope.what_type(name_var) != 2)
                        return;
                    curr_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).dflags_compiler =
                        curr_scope.get_var_value<std::string>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_DFLAGS_L) {
                    if (curr_scope.what_type(tmp_str_prefix) != 5 || curr_scope.what_type(name_var) != 2)
                        return;
                    curr_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).dflags_linker =
                        curr_scope.get_var_value<std::string>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_RFLAGS_C) {
                    if (curr_scope.what_type(tmp_str_prefix) != 5 || curr_scope.what_type(name_var) != 2)
                        return;
                    curr_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).rflags_compiler =
                        curr_scope.get_var_value<std::string>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_RFLAGS_L) {
                    if (curr_scope.what_type(tmp_str_prefix) != 5 || curr_scope.what_type(name_var) != 2)
                        return;
                    curr_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).rflags_linker =
                        curr_scope.get_var_value<std::string>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_PTH_C) {
                    if (curr_scope.what_type(tmp_str_prefix) != 5 || curr_scope.what_type(name_var) != 2)
                        return;
                    curr_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).path_compiler =
                        curr_scope.get_var_value<std::string>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_PTH_L) {
                    if (curr_scope.what_type(tmp_str_prefix) != 5 || curr_scope.what_type(name_var) != 2)
                        return;
                    curr_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).path_linker =
                        curr_scope.get_var_value<std::string>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_STD_C) {
                    if (curr_scope.what_type(tmp_str_prefix) != 5 || curr_scope.what_type(name_var) != 1)
                        return;
                    curr_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).standart_c =
                        curr_scope.get_var_value<int>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_STD_CPP) {
                    if (curr_scope.what_type(tmp_str_prefix) != 5 || curr_scope.what_type(name_var) != 1)
                        return;
                    curr_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).standart_cpp =
                        curr_scope.get_var_value<int>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_LANG) {
                    if (curr_scope.what_type(tmp_str_prefix) != 5 || curr_scope.what_type(name_var) != 1)
                        return;
                    curr_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).lang =
                        (var::struct_sb::language)curr_scope.get_var_value<int>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_VER) {
                    if (curr_scope.what_type(tmp_str_prefix) != 5 || curr_scope.what_type(name_var) != 2)
                        return;
                    curr_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).version_project =
                        var::struct_sb::version(curr_scope.get_var_value<std::string>(name_var));
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_SRC_FILES) {
                    if (curr_scope.what_type(tmp_str_prefix) != 5 || curr_scope.what_type(name_var) != 4)
                        return;
                    curr_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).src_files =
                        curr_scope.get_var_value<std::vector<std::string>>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_UTEMPLATES) {
                    if (curr_scope.what_type(tmp_str_prefix) != 5 || curr_scope.what_type(name_var) != 4)
                        return;
                    curr_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).vec_templates =
                        curr_scope.get_var_value<std::vector<std::string>>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_UITTEMPLATES) {
                    if (curr_scope.what_type(tmp_str_prefix) != 5 || curr_scope.what_type(name_var) != 1)
                        return;
                    curr_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).use_it_templates =
                        curr_scope.get_var_value<int>(name_var);
                }
                else if (tmp_str_postfix == TRG_VAR_NAME_NPROJECT) {
                    if (curr_scope.what_type(tmp_str_prefix) != 5 || curr_scope.what_type(name_var) != 1)
                        return;
                    curr_scope.get_var_value<var::struct_sb::target>(tmp_str_prefix).prj->name_project =
                        curr_scope.get_var_value<std::string>(name_var);
                }
                else if (tmp_str_postfix == TRG_VAR_NAME_VER) {
                    if (curr_scope.what_type(tmp_str_prefix) != 5 || curr_scope.what_type(name_var) != 1)
                        return;
                    curr_scope.get_var_value<var::struct_sb::target>(tmp_str_prefix).version_target =
                        var::struct_sb::version(curr_scope.get_var_value<std::string>(name_var));
                }
                else if (tmp_str_postfix == TRG_VAR_NAME_CFG) {
                    if (curr_scope.what_type(tmp_str_prefix) != 5 || curr_scope.what_type(name_var) != 1)
                        return;
                    curr_scope.get_var_value<var::struct_sb::target>(tmp_str_prefix).target_cfg =
                        (var::struct_sb::configuration)curr_scope.get_var_value<int>(name_var);
                }
                else if (tmp_str_postfix == TRG_VAR_NAME_TYPE_T) {
                    if (curr_scope.what_type(tmp_str_prefix) != 5 || curr_scope.what_type(name_var) != 1)
                        return;
                    curr_scope.get_var_value<var::struct_sb::target>(tmp_str_prefix).target_t =
                        (var::struct_sb::type_target)curr_scope.get_var_value<int>(name_var);
                }
                else if (tmp_str_postfix == TRG_VAR_NAME_LLIBS) {
                    if (curr_scope.what_type(tmp_str_prefix) != 5 || curr_scope.what_type(name_var) != 1)
                        return;
                    curr_scope.get_var_value<var::struct_sb::target>(tmp_str_prefix).target_vec_libs =
                        curr_scope.get_var_value<std::vector<std::string>>(name_var);
                }
                return;
            }
        }
    }
}

void
sl_func::set(const std::vector<subexpressions> &sub_expr, var::scope &curr_scope) {
    if (sub_expr.size() == 0)
        return;
    u32t index_var = curr_scope.what_type(sub_expr[0].token_of_subexpr[0].token_val);
    if (sub_expr.size() == 1) {
        if (index_var == 0)
            (void)curr_scope.create_var<std::string>(sub_expr[0].token_of_subexpr[0].token_val);
    }
    else if (sub_expr.size() == 2) {
        if (sub_expr[1].subexpr_t == subexpressions::type_subexpr::INT) {
            if (index_var == 1)
                curr_scope.get_var_value<int>(sub_expr[0].token_of_subexpr[0].token_val) =
                    std::stoi(sub_expr[1].token_of_subexpr[0].token_val.c_str());
            else if (index_var == 3)
                curr_scope.get_var_value<std::vector<int>>(sub_expr[0].token_of_subexpr[0].token_val) = {
                    std::stoi(sub_expr[1].token_of_subexpr[0].token_val.c_str())};
            else if (index_var == 0)
                (void)curr_scope.create_var(sub_expr[0].token_of_subexpr[0].token_val,
                                            std::stoi(sub_expr[1].token_of_subexpr[0].token_val.c_str()));
            else
                throw semantic_an::rt_semantic_excp(parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                                                        " Var - [" + sub_expr[0].token_of_subexpr[0].token_val +
                                                        "] is " + var::type_var_to_str(index_var) + "\n",
                                                    "003");
        }
        else if (sub_expr[1].subexpr_t == subexpressions::type_subexpr::STRING) {
            if (index_var == 2)
                curr_scope.get_var_value<std::string>(sub_expr[0].token_of_subexpr[0].token_val) =
                    sub_expr[1].token_of_subexpr[0].token_val;
            else if (index_var == 4)
                curr_scope.get_var_value<std::vector<std::string>>(sub_expr[0].token_of_subexpr[0].token_val) = {
                    sub_expr[1].token_of_subexpr[0].token_val};
            else if (index_var == 0)
                (void)curr_scope.create_var(sub_expr[0].token_of_subexpr[0].token_val,
                                            sub_expr[1].token_of_subexpr[0].token_val);
            else
                throw semantic_an::rt_semantic_excp(parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                                                        " Var - [" + sub_expr[0].token_of_subexpr[0].token_val +
                                                        "] is " + var::type_var_to_str(index_var) + "\n",
                                                    "003");
        }
    }
    else if (sub_expr.size() > 2) {
        if (sub_expr[1].subexpr_t == subexpressions::type_subexpr::INT) {
            std::vector<int> vec_int_params;
            vec_int_params.push_back(std::stoi(sub_expr[1].token_of_subexpr[0].token_val.c_str()));
            for (u32t i = 2; i < sub_expr.size(); ++i) {
                if (sub_expr[i].subexpr_t == subexpressions::type_subexpr::INT)
                    vec_int_params.push_back(std::stoi(sub_expr[i].token_of_subexpr[0].token_val.c_str()));
            }
            if (index_var == 0)
                (void)curr_scope.create_var<std::vector<int>>(sub_expr[0].token_of_subexpr[0].token_val,
                                                              vec_int_params);
            else if (index_var == 3)
                curr_scope.get_var_value<std::vector<int>>(sub_expr[0].token_of_subexpr[0].token_val) = vec_int_params;
            else if (index_var == 1)
                throw semantic_an::rt_semantic_excp(parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                                                        " Var - [" + sub_expr[0].token_of_subexpr[0].token_val +
                                                        "] is int\n",
                                                    "002");
            else
                throw semantic_an::rt_semantic_excp(parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                                                        " Var - [" + sub_expr[0].token_of_subexpr[0].token_val +
                                                        "] is " + var::type_var_to_str(index_var) + "\n",
                                                    "003");
        }
        else if (sub_expr[1].subexpr_t == subexpressions::type_subexpr::STRING) {
            std::vector<std::string> vec_str_params;
            vec_str_params.push_back(sub_expr[1].token_of_subexpr[0].token_val);
            for (u32t i = 2; i < sub_expr.size(); ++i) {
                if (sub_expr[i].subexpr_t == subexpressions::type_subexpr::STRING)
                    vec_str_params.push_back(sub_expr[i].token_of_subexpr[0].token_val);
            }
            if (index_var == 0)
                (void)curr_scope.create_var<std::vector<std::string>>(sub_expr[0].token_of_subexpr[0].token_val,
                                                                      vec_str_params);
            else if (index_var == 4)
                curr_scope.get_var_value<std::vector<std::string>>(sub_expr[0].token_of_subexpr[0].token_val) =
                    vec_str_params;
            else if (index_var == 2)
                throw semantic_an::rt_semantic_excp(parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                                                        " Var - [" + sub_expr[0].token_of_subexpr[0].token_val +
                                                        "] is string\n",
                                                    "002");
            else
                throw semantic_an::rt_semantic_excp(parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                                                        " Var - [" + sub_expr[0].token_of_subexpr[0].token_val +
                                                        "] is " + var::type_var_to_str(index_var) + "\n",
                                                    "003");
        }
    }

    // ??????
    // if a variable is abstract ref to cfg structs
    update_cfg_struct(sub_expr[0].token_of_subexpr[0].token_val, curr_scope);
}

void
sl_func::project(const std::vector<subexpressions> &sub_expr, var::scope &curr_scope) {
    var::struct_sb::project &prj_ref =
        curr_scope.create_var<var::struct_sb::project>(sub_expr[0].token_of_subexpr[0].token_val);
    prj_ref.name_project = sub_expr[0].token_of_subexpr[0].token_val;
    prj_ref.lang = (var::struct_sb::language)std::stoi(sub_expr[1].token_of_subexpr[0].token_val);
    for (u32t i = 2; i < sub_expr.size(); ++i)
        prj_ref.src_files.push_back(sub_expr[i].token_of_subexpr[0].token_val);

    // ??????? - I don't know if this helper function is needed
    if (curr_scope.what_type("DECL_CONFIG_VAR") == 1 && curr_scope.get_var_value<int>(DECL_VAR_STRUCT) > 0) {
        if (!curr_scope.try_create_var<int>(prj_ref.name_project + PRJ_VAR_NAME_LANG, (int)prj_ref.lang))
            prj_ref.lang =
                (var::struct_sb::language)curr_scope.get_var_value<int>(prj_ref.name_project + PRJ_VAR_NAME_LANG);
        if (!curr_scope.try_create_var<std::string>(prj_ref.name_project + PRJ_VAR_NAME_VER,
                                                    prj_ref.version_project.get_str_version()))
            prj_ref.version_project =
                var::struct_sb::version(curr_scope.get_var_value<std::string>(prj_ref.name_project + PRJ_VAR_NAME_VER));

        if (!curr_scope.try_create_var<std::string>(prj_ref.name_project + PRJ_VAR_NAME_DFLAGS_C,
                                                    prj_ref.dflags_compiler))
            prj_ref.dflags_compiler =
                curr_scope.get_var_value<std::string>(prj_ref.name_project + PRJ_VAR_NAME_DFLAGS_C);
        if (!curr_scope.try_create_var<std::string>(prj_ref.name_project + PRJ_VAR_NAME_DFLAGS_L,
                                                    prj_ref.dflags_linker))
            prj_ref.dflags_linker = curr_scope.get_var_value<std::string>(prj_ref.name_project + PRJ_VAR_NAME_DFLAGS_L);
        if (!curr_scope.try_create_var<std::string>(prj_ref.name_project + PRJ_VAR_NAME_RFLAGS_C,
                                                    prj_ref.rflags_compiler))
            prj_ref.rflags_compiler =
                curr_scope.get_var_value<std::string>(prj_ref.name_project + PRJ_VAR_NAME_RFLAGS_C);
        if (!curr_scope.try_create_var<std::string>(prj_ref.name_project + PRJ_VAR_NAME_RFLAGS_L,
                                                    prj_ref.rflags_linker))
            prj_ref.rflags_linker = curr_scope.get_var_value<std::string>(prj_ref.name_project + PRJ_VAR_NAME_RFLAGS_L);
        if (!curr_scope.try_create_var<std::string>(prj_ref.name_project + PRJ_VAR_NAME_PTH_C, prj_ref.path_compiler))
            prj_ref.path_compiler = curr_scope.get_var_value<std::string>(prj_ref.name_project + PRJ_VAR_NAME_PTH_C);
        if (!curr_scope.try_create_var<std::string>(prj_ref.name_project + PRJ_VAR_NAME_PTH_L, prj_ref.path_linker))
            prj_ref.path_linker = curr_scope.get_var_value<std::string>(prj_ref.name_project + PRJ_VAR_NAME_PTH_L);
        if (!curr_scope.try_create_var<int>(prj_ref.name_project + PRJ_VAR_NAME_STD_C, prj_ref.standart_c))
            prj_ref.standart_c = curr_scope.get_var_value<int>(prj_ref.name_project + PRJ_VAR_NAME_STD_C);
        if (!curr_scope.try_create_var<int>(prj_ref.name_project + PRJ_VAR_NAME_STD_CPP, prj_ref.standart_cpp))
            prj_ref.standart_cpp = curr_scope.get_var_value<int>(prj_ref.name_project + PRJ_VAR_NAME_STD_CPP);
        if (!curr_scope.try_create_var<std::vector<std::string>>(prj_ref.name_project + PRJ_VAR_NAME_SRC_FILES,
                                                                 prj_ref.src_files))
            prj_ref.src_files =
                curr_scope.get_var_value<std::vector<std::string>>(prj_ref.name_project + PRJ_VAR_NAME_SRC_FILES);
        if (!curr_scope.try_create_var<std::vector<std::string>>(prj_ref.name_project + PRJ_VAR_NAME_UTEMPLATES,
                                                                 prj_ref.vec_templates))
            prj_ref.vec_templates =
                curr_scope.get_var_value<std::vector<std::string>>(prj_ref.name_project + PRJ_VAR_NAME_UTEMPLATES);
        if (!curr_scope.try_create_var<int>(prj_ref.name_project + PRJ_VAR_NAME_UITTEMPLATES, prj_ref.use_it_templates))
            prj_ref.use_it_templates = curr_scope.get_var_value<int>(prj_ref.name_project + PRJ_VAR_NAME_UITTEMPLATES);
    }
}
void
sl_func::executable(const std::vector<subexpressions> &sub_expr, var::scope &curr_scope) {
    if (curr_scope.what_type(sub_expr[2].token_of_subexpr[0].token_val) != 5)
        throw semantic_an::rt_semantic_excp(
            parser::build_pos_tokenb_str(sub_expr[2].token_of_subexpr[0]) +
                " Var id of struct(expected project): " + sub_expr[2].token_of_subexpr[0].token_val + "\n",
            "001");
    var::struct_sb::target &trg_ref =
        curr_scope.create_var<var::struct_sb::target>(sub_expr[0].token_of_subexpr[0].token_val);

    trg_ref.name_target = sub_expr[0].token_of_subexpr[0].token_val;
    trg_ref.prj = std::make_shared<var::struct_sb::project>(
        curr_scope.get_var_value<var::struct_sb::project>(sub_expr[2].token_of_subexpr[0].token_val));
    trg_ref.target_cfg = (var::struct_sb::configuration)std::stoi(sub_expr[1].token_of_subexpr[0].token_val);
    trg_ref.version_target = var::struct_sb::version(0, 0, 0);

    // ??????? - I don't know if this helper function is needed
    if (curr_scope.what_type("DECL_CONFIG_VAR") == 1 && curr_scope.get_var_value<int>(DECL_VAR_STRUCT) > 0) {
        if (!curr_scope.try_create_var<std::string>(trg_ref.name_target + TRG_VAR_NAME_NPROJECT,
                                                    trg_ref.prj->name_project))
            trg_ref.prj->name_project =
                curr_scope.get_var_value<std::string>(trg_ref.name_target + TRG_VAR_NAME_NPROJECT);
        if (!curr_scope.try_create_var<std::string>(trg_ref.name_target + TRG_VAR_NAME_VER,
                                                    trg_ref.version_target.get_str_version()))
            trg_ref.version_target =
                var::struct_sb::version(curr_scope.get_var_value<std::string>(trg_ref.name_target + TRG_VAR_NAME_VER));
        if (!curr_scope.try_create_var<int>(trg_ref.name_target + TRG_VAR_NAME_CFG, (int)trg_ref.target_cfg))
            trg_ref.target_cfg =
                (var::struct_sb::configuration)curr_scope.get_var_value<int>(trg_ref.name_target + TRG_VAR_NAME_CFG);
        if (!curr_scope.try_create_var<int>(trg_ref.name_target + TRG_VAR_NAME_TYPE_T, (int)trg_ref.target_t))
            trg_ref.target_t =
                (var::struct_sb::type_target)curr_scope.get_var_value<int>(trg_ref.name_target + TRG_VAR_NAME_TYPE_T);
        if (!curr_scope.try_create_var<std::vector<std::string>>(trg_ref.name_target + TRG_VAR_NAME_LLIBS,
                                                                 trg_ref.target_vec_libs))
            trg_ref.target_vec_libs =
                curr_scope.get_var_value<std::vector<std::string>>(trg_ref.name_target + TRG_VAR_NAME_LLIBS);
    }
}

void
sl_func::link_lib(const std::vector<subexpressions> &sub_expr, var::scope &curr_scope) {
    if (curr_scope.what_type(sub_expr[0].token_of_subexpr[0].token_val) != 6)
        throw interpreter::realtime_excp(
            parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                " Var id of struct(expected target): " + sub_expr[0].token_of_subexpr[0].token_val + "\n",
            "003");
    var::struct_sb::target &trg_ref =
        curr_scope.get_var_value<var::struct_sb::target>(sub_expr[0].token_of_subexpr[0].token_val);
    for (u32t i = 1; i < sub_expr.size(); ++i)
        trg_ref.target_vec_libs.push_back(sub_expr[i].token_of_subexpr[0].token_val);

    if (curr_scope.what_type("DECL_CONFIG_VAR") == 1 && curr_scope.get_var_value<int>(DECL_VAR_STRUCT) > 0) {
        if (curr_scope.what_type(trg_ref.name_target + TRG_VAR_NAME_LLIBS) == 4) {
            curr_scope.get_var_value<std::vector<std::string>>(trg_ref.name_target + TRG_VAR_NAME_LLIBS) =
                trg_ref.target_vec_libs;
        }
        else
            curr_scope.create_var<std::vector<std::string>>(trg_ref.name_target + TRG_VAR_NAME_LLIBS,
                                                            trg_ref.target_vec_libs);
    }
}
void
sl_func::exp_data(const std::vector<subexpressions> &sub_expr, var::scope &curr_scope) {
    interpreter::interpreter_exec::config tmp_conf;
    tmp_conf.filename_interp = sub_expr[0].token_of_subexpr[0].token_val.c_str();
    tmp_conf.import_module = 0;
    tmp_conf.use_external_scope = 1;
    interpreter::interpreter_exec tmp_interpreter(tmp_conf);
    tmp_interpreter.set_external_scope(&curr_scope);
    tmp_interpreter.build_aef();
}
void
sl_func::cmd(const std::vector<subexpressions> &sub_expr, var::scope &curr_scope) {
    u32t val = std::stoi(sub_expr[0].token_of_subexpr[0].token_val.c_str());
    if (!val)
        curr_scope.call_cmd.add_call_before(sub_expr[1].token_of_subexpr[0].token_val);
    else
        curr_scope.call_cmd.add_call_after(sub_expr[1].token_of_subexpr[0].token_val);
}
void
sl_func::debug(const std::vector<subexpressions> &sub_expr, var::scope &curr_scope) {
    std::string str_out;
    for (u32t i = 0; i < sub_expr.size(); ++i)
        str_out += sub_expr[i].token_of_subexpr[0].token_val + " ";
    assist << str_out;
}

void
sl_func::debug_struct(const std::vector<aef_expr::subexpressions> &sub_expr, var::scope &curr_scope) {
    std::string out_str;
    u32t ind = curr_scope.what_type(sub_expr[0].token_of_subexpr[0].token_val);
    if (ind == 5) {
        var::struct_sb::project &prj_ref =
            curr_scope.get_var_value<var::struct_sb::project>(sub_expr[0].token_of_subexpr[0].token_val);
        out_str = "Name Project: " + prj_ref.name_project + "\nVersion: " + prj_ref.version_project.get_str_version() +
                  "\nLang: " + lang_str(prj_ref.lang) + "\nCompiler: " + prj_ref.path_compiler +
                  "\nLinker: " + prj_ref.path_linker + "\nDebug Flags Compiler: " + prj_ref.dflags_compiler +
                  "\nRelease Flags Compiler: " + prj_ref.rflags_compiler +
                  "\nDebug Flags Linker: " + prj_ref.dflags_linker +
                  "\nRelease Flags Compiler: " + prj_ref.rflags_linker +
                  "\nStandart C: " + std::to_string(prj_ref.standart_c) +
                  "\nStandart C++: " + std::to_string(prj_ref.standart_cpp) + "\n------\nSRC_FILES: \n";
        for (u32t i = 0; i < prj_ref.src_files.size(); ++i) {
            out_str += prj_ref.src_files[i];
            if (i != prj_ref.src_files.size() - 1)
                out_str += ", ";
        }
        out_str += "\n\nWill templates be used? - " + std::to_string(prj_ref.use_it_templates) + "\nTemplates: \n";
        for (u32t i = 0; i < prj_ref.vec_templates.size(); ++i) {
            out_str += prj_ref.vec_templates[i];
            if (i != prj_ref.vec_templates.size() - 1)
                out_str += ", ";
        }
    }
    else if (ind == 6) {
        var::struct_sb::target &trg_ref =
            curr_scope.get_var_value<var::struct_sb::target>(sub_expr[0].token_of_subexpr[0].token_val);
        out_str = "Name Target: " + trg_ref.name_target + "\nVersion: " + trg_ref.version_target.get_str_version() +
                  "\nName Project: " + trg_ref.prj->name_project + "\nType Build: " + target_t_str(trg_ref.target_t) +
                  "\nConfiguration: " + cfg_str(trg_ref.target_cfg) + "\nLibs: \n";
        for (u32t i = 0; i < trg_ref.target_vec_libs.size(); ++i) {
            out_str += trg_ref.target_vec_libs[i];
            if (i != trg_ref.target_vec_libs.size() - 1)
                out_str += ", ";
        }
    }
    assist << out_str;
}

void
sl_func::flags_compiler(const std::vector<subexpressions> &sub_expr, var::scope &curr_scope) {
    if (curr_scope.what_type(sub_expr[0].token_of_subexpr[0].token_val) != 5)
        throw interpreter::realtime_excp(
            parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                " Var id of struct(expected project): " + sub_expr[0].token_of_subexpr[0].token_val + "\n",
            "003");
    var::struct_sb::project &prj_ref =
        curr_scope.get_var_value<var::struct_sb::project>(sub_expr[0].token_of_subexpr[0].token_val);
    var::struct_sb::configuration tmp_cfg;
    if ((tmp_cfg = (var::struct_sb::configuration)std::stoi(sub_expr[1].token_of_subexpr[0].token_val.c_str())) ==
        var::struct_sb::configuration::RELEASE)
        for (u32t i = 2; i < sub_expr.size(); ++i) {
            prj_ref.rflags_compiler += sub_expr[i].token_of_subexpr[0].token_val + " ";
        }
    else
        for (u32t i = 2; i < sub_expr.size(); ++i) {
            prj_ref.dflags_compiler += sub_expr[i].token_of_subexpr[0].token_val + " ";
        }

    if (curr_scope.what_type("DECL_CONFIG_VAR") == 1 && curr_scope.get_var_value<int>(DECL_VAR_STRUCT) > 0)
        if (tmp_cfg == var::struct_sb::configuration::DEBUG)
            if (curr_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_DFLAGS_C) == 2) {
                curr_scope.get_var_value<std::string>(prj_ref.name_project + PRJ_VAR_NAME_DFLAGS_C) =
                    prj_ref.dflags_compiler;
            }
            else
                curr_scope.create_var<std::string>(prj_ref.name_project + PRJ_VAR_NAME_DFLAGS_C,
                                                   prj_ref.dflags_compiler);
        else if (tmp_cfg == var::struct_sb::configuration::RELEASE)
            if (curr_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_RFLAGS_C) == 2) {
                curr_scope.get_var_value<std::string>(prj_ref.name_project + PRJ_VAR_NAME_RFLAGS_C) =
                    prj_ref.rflags_compiler;
            }
            else
                curr_scope.create_var<std::string>(prj_ref.name_project + PRJ_VAR_NAME_RFLAGS_C,
                                                   prj_ref.rflags_compiler);
}
void
sl_func::flags_linker(const std::vector<subexpressions> &sub_expr, var::scope &curr_scope) {
    if (curr_scope.what_type(sub_expr[0].token_of_subexpr[0].token_val) != 5)
        throw interpreter::realtime_excp(
            parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                " Var id of struct(expected project): " + sub_expr[0].token_of_subexpr[0].token_val + "\n",
            "003");
    var::struct_sb::project &prj_ref =
        curr_scope.get_var_value<var::struct_sb::project>(sub_expr[0].token_of_subexpr[0].token_val);
    var::struct_sb::configuration tmp_cfg;
    if ((tmp_cfg = (var::struct_sb::configuration)std::stoi(sub_expr[1].token_of_subexpr[0].token_val.c_str())) ==
        var::struct_sb::configuration::RELEASE)
        for (u32t i = 2; i < sub_expr.size(); ++i) {
            prj_ref.rflags_linker += sub_expr[i].token_of_subexpr[0].token_val + " ";
        }
    else
        for (u32t i = 2; i < sub_expr.size(); ++i) {
            prj_ref.dflags_linker += sub_expr[i].token_of_subexpr[0].token_val + " ";
        }

    if (curr_scope.what_type("DECL_CONFIG_VAR") == 1 && curr_scope.get_var_value<int>(DECL_VAR_STRUCT) > 0)
        if (tmp_cfg == var::struct_sb::configuration::DEBUG)
            if (curr_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_DFLAGS_L) == 2) {
                curr_scope.get_var_value<std::string>(prj_ref.name_project + PRJ_VAR_NAME_DFLAGS_L) =
                    prj_ref.dflags_linker;
            }
            else
                curr_scope.create_var<std::string>(prj_ref.name_project + PRJ_VAR_NAME_DFLAGS_L, prj_ref.dflags_linker);
        else if (tmp_cfg == var::struct_sb::configuration::RELEASE)
            if (curr_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_RFLAGS_L) == 2) {
                curr_scope.get_var_value<std::string>(prj_ref.name_project + PRJ_VAR_NAME_RFLAGS_L) =
                    prj_ref.rflags_linker;
            }
            else
                curr_scope.create_var<std::string>(prj_ref.name_project + PRJ_VAR_NAME_RFLAGS_L, prj_ref.dflags_linker);
}
void
sl_func::path_compiler(const std::vector<subexpressions> &sub_expr, var::scope &curr_scope) {
    if (curr_scope.what_type(sub_expr[0].token_of_subexpr[0].token_val) != 5)
        throw interpreter::realtime_excp(
            parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                " Var id of struct(expected project): " + sub_expr[0].token_of_subexpr[0].token_val + "\n",
            "003");
    var::struct_sb::project &prj_ref =
        curr_scope.get_var_value<var::struct_sb::project>(sub_expr[0].token_of_subexpr[0].token_val);
    prj_ref.path_compiler = sub_expr[1].token_of_subexpr[0].token_val;

    if (curr_scope.what_type("DECL_CONFIG_VAR") == 1 && curr_scope.get_var_value<int>(DECL_VAR_STRUCT) > 0)
        if (curr_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_PTH_C) == 2) {
            curr_scope.get_var_value<std::string>(prj_ref.name_project + PRJ_VAR_NAME_PTH_C) = prj_ref.path_compiler;
        }
        else
            curr_scope.create_var<std::string>(prj_ref.name_project + PRJ_VAR_NAME_PTH_C, prj_ref.path_compiler);
}
void
sl_func::path_linker(const std::vector<subexpressions> &sub_expr, var::scope &curr_scope) {
    if (curr_scope.what_type(sub_expr[0].token_of_subexpr[0].token_val) != 5)
        throw interpreter::realtime_excp(
            parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                " Var id of struct(expected project): " + sub_expr[0].token_of_subexpr[0].token_val + "\n",
            "003");
    var::struct_sb::project &prj_ref =
        curr_scope.get_var_value<var::struct_sb::project>(sub_expr[0].token_of_subexpr[0].token_val);
    prj_ref.path_linker = sub_expr[1].token_of_subexpr[0].token_val;

    if (curr_scope.what_type("DECL_CONFIG_VAR") == 1 && curr_scope.get_var_value<int>(DECL_VAR_STRUCT) > 0)
        if (curr_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_PTH_C) == 2) {
            curr_scope.get_var_value<std::string>(prj_ref.name_project + PRJ_VAR_NAME_PTH_C) = prj_ref.path_linker;
        }
        else
            curr_scope.create_var<std::string>(prj_ref.name_project + PRJ_VAR_NAME_PTH_C, prj_ref.path_linker);
}
void
sl_func::standart_c(const std::vector<subexpressions> &sub_expr, var::scope &curr_scope) {
    if (curr_scope.what_type(sub_expr[0].token_of_subexpr[0].token_val) != 5)
        throw interpreter::realtime_excp(
            parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                " Var id of struct(expected project): " + sub_expr[0].token_of_subexpr[0].token_val + "\n",
            "003");
    var::struct_sb::project &prj_ref =
        curr_scope.get_var_value<var::struct_sb::project>(sub_expr[0].token_of_subexpr[0].token_val);
    prj_ref.standart_c = std::stoi(sub_expr[1].token_of_subexpr[0].token_val.c_str());
    if (curr_scope.what_type("DECL_CONFIG_VAR") == 1 && curr_scope.get_var_value<int>(DECL_VAR_STRUCT) > 0)
        if (curr_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_STD_C) == 1) {
            curr_scope.get_var_value<int>(prj_ref.name_project + PRJ_VAR_NAME_STD_C) = prj_ref.standart_c;
        }
        else
            curr_scope.create_var<int>(prj_ref.name_project + PRJ_VAR_NAME_STD_C, prj_ref.standart_c);
}
void
sl_func::standart_cpp(const std::vector<subexpressions> &sub_expr, var::scope &curr_scope) {
    if (curr_scope.what_type(sub_expr[0].token_of_subexpr[0].token_val) != 5)
        throw interpreter::realtime_excp(
            parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                " Var id of struct(expected project): " + sub_expr[0].token_of_subexpr[0].token_val + "\n",
            "003");
    var::struct_sb::project &prj_ref =
        curr_scope.get_var_value<var::struct_sb::project>(sub_expr[0].token_of_subexpr[0].token_val);
    prj_ref.standart_cpp = std::stoi(sub_expr[1].token_of_subexpr[0].token_val.c_str());
    if (curr_scope.what_type("DECL_CONFIG_VAR") == 1 && curr_scope.get_var_value<int>(DECL_VAR_STRUCT) > 0)
        if (curr_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_STD_CPP) == 1) {
            curr_scope.get_var_value<int>(prj_ref.name_project + PRJ_VAR_NAME_STD_CPP) = prj_ref.standart_c;
        }
        else
            curr_scope.create_var<int>(prj_ref.name_project + PRJ_VAR_NAME_STD_CPP, prj_ref.standart_c);
}
void
sl_func::lang(const std::vector<subexpressions> &sub_expr, var::scope &curr_scope) {
    if (curr_scope.what_type(sub_expr[0].token_of_subexpr[0].token_val) != 5)
        throw interpreter::realtime_excp(
            parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                " Var id of struct(expected project): " + sub_expr[0].token_of_subexpr[0].token_val + "\n",
            "003");
    var::struct_sb::project &prj_ref =
        curr_scope.get_var_value<var::struct_sb::project>(sub_expr[0].token_of_subexpr[0].token_val);
    prj_ref.lang = (var::struct_sb::language)std::stoi(sub_expr[1].token_of_subexpr[0].token_val);
    if (curr_scope.what_type("DECL_CONFIG_VAR") == 1 && curr_scope.get_var_value<int>(DECL_VAR_STRUCT) > 0)
        if (curr_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_LANG) == 1) {
            curr_scope.get_var_value<int>(prj_ref.name_project + PRJ_VAR_NAME_LANG) = (int)prj_ref.lang;
        }
        else
            curr_scope.create_var<int>(prj_ref.name_project + PRJ_VAR_NAME_LANG, (int)prj_ref.lang);
}

void
sl_func::add_param_template(const std::vector<subexpressions> &sub_expr, var::scope &curr_scope) {
    curr_scope.create_var<std::string>(sub_expr[0].token_of_subexpr[0].token_val,
                                       sub_expr[1].token_of_subexpr[0].token_val);
}

void
sl_func::create_templates(const std::vector<aef_expr::subexpressions> &sub_expr, var::scope &curr_scope) {
    std::string tmp_param;

    var::struct_sb::template_command tcmd_tmp;
    tcmd_tmp.name = sub_expr[0].token_of_subexpr[0].token_val;

    bool beg_param = 0, end_param = 0;
    bool op_comma = 0;

    for (u32t i = 0; i < sub_expr[1].token_of_subexpr[0].token_val.size(); ++i) {

        if (sub_expr[1].token_of_subexpr[0].token_val[i] == ' ' ||
            sub_expr[1].token_of_subexpr[0].token_val[i] == '\n' ||
            sub_expr[1].token_of_subexpr[0].token_val[i] == '\t')
            continue;

        if (sub_expr[1].token_of_subexpr[0].token_val[i] == ':') {
            if (tmp_param.empty())
                throw semantic_an::rt_semantic_excp(
                    parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                        " The name of the call component is empty: " + sub_expr[1].token_of_subexpr[0].token_val + "\n",
                    "004");
            else if (!tcmd_tmp.name_call_component.empty())
                throw semantic_an::rt_semantic_excp(parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                                                        " The name of the call component that already exists: " +
                                                        sub_expr[1].token_of_subexpr[0].token_val + "\n",
                                                    "004");

            beg_param = 1;

            tcmd_tmp.name_call_component = tmp_param;
            tmp_param.clear();

            continue;
        }
        else if (beg_param) {
            if (sub_expr[1].token_of_subexpr[0].token_val[i] != '<')
                throw semantic_an::rt_semantic_excp(parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                                                        " The enumeration of parameters is expected: " +
                                                        sub_expr[1].token_of_subexpr[0].token_val + "\n",
                                                    "004");

            end_param = 1;
            beg_param = 0;

            continue;
        }
        else if (end_param && sub_expr[1].token_of_subexpr[0].token_val[i] == '>') {
            if (tmp_param.empty())
                continue;
            else if (curr_scope.what_type(tmp_param) != 2)
                throw semantic_an::rt_semantic_excp(parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                                                        " Template argument does not exist externally[" + tmp_param +
                                                        "]: " + sub_expr[1].token_of_subexpr[0].token_val + "\n",
                                                    "004");

            tcmd_tmp.name_args.push_back(tmp_param);

            tmp_param.clear();

            op_comma = 1;
            end_param = 0;
            continue;
        }
        else if (op_comma) {
            if (sub_expr[1].token_of_subexpr[0].token_val[i] != ',')
                throw semantic_an::rt_semantic_excp(
                    parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                        " The operator is expected - ',': " + sub_expr[1].token_of_subexpr[0].token_val + "\n",
                    "004");

            beg_param = 1;
            op_comma = 0;

            continue;
        }

        tmp_param += sub_expr[1].token_of_subexpr[0].token_val[i];
    }

    if (beg_param)
        throw semantic_an::rt_semantic_excp(
            parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                " Template argument expected: " + sub_expr[1].token_of_subexpr[0].token_val + "\n",
            "004");

    curr_scope.create_var<var::struct_sb::template_command>(sub_expr[0].token_of_subexpr[0].token_val, tcmd_tmp);
}

void
sl_func::use_templates(const std::vector<subexpressions> &sub_expr, var::scope &curr_scope) {
    if (curr_scope.what_type(sub_expr[0].token_of_subexpr[0].token_val) != 5)
        throw interpreter::realtime_excp(
            parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                " Var id of struct(expected project): " + sub_expr[0].token_of_subexpr[0].token_val + "\n",
            "003");
    var::struct_sb::project &prj_ref =
        curr_scope.get_var_value<var::struct_sb::project>(sub_expr[0].token_of_subexpr[0].token_val);
    for (u32t i = 1; i < sub_expr.size(); ++i) {
        prj_ref.vec_templates.push_back(sub_expr[i].token_of_subexpr[0].token_val);
    }

    if (curr_scope.what_type("DECL_CONFIG_VAR") == 1 && curr_scope.get_var_value<int>(DECL_VAR_STRUCT) > 0)
        if (curr_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_UTEMPLATES) == 1) {
            curr_scope.get_var_value<std::vector<std::string>>(prj_ref.name_project + PRJ_VAR_NAME_UTEMPLATES) =
                prj_ref.vec_templates;
        }
        else
            curr_scope.create_var<std::vector<std::string>>(prj_ref.name_project + PRJ_VAR_NAME_UTEMPLATES,
                                                            prj_ref.vec_templates);
}
void
sl_func::use_it_template(const std::vector<subexpressions> &sub_expr, var::scope &curr_scope) {
    if (curr_scope.what_type(sub_expr[0].token_of_subexpr[0].token_val) != 5)
        throw interpreter::realtime_excp(
            parser::build_pos_tokenb_str(sub_expr[0].token_of_subexpr[0]) +
                " Var id of struct(expected project): " + sub_expr[0].token_of_subexpr[0].token_val + "\n",
            "003");

    var::struct_sb::project &prj_ref =
        curr_scope.get_var_value<var::struct_sb::project>(sub_expr[0].token_of_subexpr[0].token_val);
    prj_ref.use_it_templates = std::stoi(sub_expr[1].token_of_subexpr[0].token_val);
    if (curr_scope.what_type("DECL_CONFIG_VAR") == 1 && curr_scope.get_var_value<int>(DECL_VAR_STRUCT) > 0)
        if (curr_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_UITTEMPLATES) == 1) {
            curr_scope.get_var_value<int>(prj_ref.name_project + PRJ_VAR_NAME_UITTEMPLATES) = prj_ref.use_it_templates;
        }
        else
            curr_scope.create_var<int>(prj_ref.name_project + PRJ_VAR_NAME_UITTEMPLATES, prj_ref.use_it_templates);
}