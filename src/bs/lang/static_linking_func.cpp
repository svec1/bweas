#include "static_linking_func.hpp"

#include "../tools/bwfile.hpp"
#include "interpreter.hpp"

#include <algorithm>
#include <array>

std::map<std::string, std::string> var::struct_sb::project::preset_ext_fields;

static const std::array<std::string, 18> vec_name_config_var = {
    PRJ_VAR_NAME_DFLAGS_C,     PRJ_VAR_NAME_DFLAGS_L, PRJ_VAR_NAME_RFLAGS_C,  PRJ_VAR_NAME_RFLAGS_L,
    PRJ_VAR_NAME_LANG,         PRJ_VAR_NAME_PTH_C,    PRJ_VAR_NAME_PTH_L,     PRJ_VAR_NAME_VER,
    PRJ_VAR_NAME_STD_C,        PRJ_VAR_NAME_STD_CPP,  PRJ_VAR_NAME_SRC_FILES, PRJ_VAR_NAME_UTEMPLATES,
    PRJ_VAR_NAME_UITTEMPLATES, TRG_VAR_NAME_NPROJECT, TRG_VAR_NAME_VER,       TRG_VAR_NAME_CFG,
    TRG_VAR_NAME_TYPE_T,       TRG_VAR_NAME_LLIBS};

static bweas::logger _log{"STATUS"};

// ??????
static void update_cfg_struct(const std::string &name_var, var::scope &current_scope) {
    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<i32t>(DECL_VAR_STRUCT)) {
        u32t tmp_it = 0;
        for (const auto &it : vec_name_config_var) {
            if ((tmp_it = name_var.find(it)) != SIZE_MAX) {
                if (name_var.size() - it.size() != tmp_it)
                    return;
                std::string tmp_str_postfix = name_var;
                std::string tmp_str_prefix  = name_var;
                tmp_str_postfix.erase(0, tmp_it);
                tmp_str_prefix.erase(tmp_it, tmp_str_prefix.size());
                if (tmp_str_postfix == PRJ_VAR_NAME_DFLAGS_C) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 2)
                        return;
                    current_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).dflags_compiler =
                        current_scope.get_var_value<std::string>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_DFLAGS_L) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 2)
                        return;
                    current_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).dflags_linker =
                        current_scope.get_var_value<std::string>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_RFLAGS_C) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 2)
                        return;
                    current_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).rflags_compiler =
                        current_scope.get_var_value<std::string>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_RFLAGS_L) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 2)
                        return;
                    current_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).rflags_linker =
                        current_scope.get_var_value<std::string>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_PTH_C) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 2)
                        return;
                    current_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).path_compiler =
                        current_scope.get_var_value<std::string>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_PTH_L) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 2)
                        return;
                    current_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).path_linker =
                        current_scope.get_var_value<std::string>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_STD_C) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 1)
                        return;
                    current_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).standart_c =
                        current_scope.get_var_value<i32t>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_STD_CPP) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 1)
                        return;
                    current_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).standart_cpp =
                        current_scope.get_var_value<i32t>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_LANG) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 1)
                        return;
                    current_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).language =
                        current_scope.get_var_value<std::string>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_VER) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 2)
                        return;
                    current_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).version_project =
                        var::struct_sb::version(current_scope.get_var_value<std::string>(name_var));
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_SRC_FILES) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 4)
                        return;
                    current_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).src_files =
                        current_scope.get_var_value<std::vector<std::string>>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_UTEMPLATES) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 4)
                        return;
                    current_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).vec_templates =
                        current_scope.get_var_value<std::vector<std::string>>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_UITTEMPLATES) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 1)
                        return;
                    current_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).use_it_templates =
                        current_scope.get_var_value<i32t>(name_var);
                }
                else if (tmp_str_postfix == TRG_VAR_NAME_NPROJECT) {
                    if (current_scope.what_type(tmp_str_prefix) != 6 || current_scope.what_type(name_var) != 2)
                        return;
                    current_scope.get_var_value<var::struct_sb::target>(tmp_str_prefix).prj->name_project =
                        current_scope.get_var_value<std::string>(name_var);
                }
                else if (tmp_str_postfix == TRG_VAR_NAME_VER) {
                    if (current_scope.what_type(tmp_str_prefix) != 6 || current_scope.what_type(name_var) != 2)
                        return;
                    current_scope.get_var_value<var::struct_sb::target>(tmp_str_prefix).version_target =
                        var::struct_sb::version(current_scope.get_var_value<std::string>(name_var));
                }
                else if (tmp_str_postfix == TRG_VAR_NAME_CFG) {
                    if (current_scope.what_type(tmp_str_prefix) != 6 || current_scope.what_type(name_var) != 1)
                        return;
                    current_scope.get_var_value<var::struct_sb::target>(tmp_str_prefix).target_cfg =
                        (var::struct_sb::configuration)current_scope.get_var_value<i32t>(name_var);
                }
                else if (tmp_str_postfix == TRG_VAR_NAME_TYPE_T) {
                    if (current_scope.what_type(tmp_str_prefix) != 6 || current_scope.what_type(name_var) != 1)
                        return;
                    current_scope.get_var_value<var::struct_sb::target>(tmp_str_prefix).target_t =
                        (var::struct_sb::type_target)current_scope.get_var_value<i32t>(name_var);
                }
                else if (tmp_str_postfix == TRG_VAR_NAME_NGENERATOR) {
                    if (current_scope.what_type(tmp_str_prefix) != 6 || current_scope.what_type(name_var) != 2)
                        return;
                    current_scope.get_var_value<var::struct_sb::target>(tmp_str_prefix).name_generator =
                        current_scope.get_var_value<std::string>(name_var);
                }
                else if (tmp_str_postfix == TRG_VAR_NAME_LLIBS) {
                    if (current_scope.what_type(tmp_str_prefix) != 6 || current_scope.what_type(name_var) != 1)
                        return;
                    current_scope.get_var_value<var::struct_sb::target>(tmp_str_prefix).target_vec_libs =
                        current_scope.get_var_value<std::vector<std::string>>(name_var);
                }
                else {
                    if (current_scope.what_type(tmp_str_prefix) != 5 && current_scope.what_type(name_var) != 2)
                        return;
                    auto prj      = current_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix);
                    auto it_field = prj.custom_ext_fields.find(tmp_str_postfix);
                    if (it_field != prj.custom_ext_fields.end())
                        it_field->second = current_scope.get_var_value<std::string>(name_var);
                }
                return;
            }
        }
    }
}

void sl_func::set(const expressions &expr_s, var::scope &current_scope) {
    if (expr_s.size() == 0)
        return;
    u32t index_var = current_scope.what_type(expr_s[0].value);
    if (expr_s.size() == 1) {
        if (index_var == 0)
            (void)current_scope.create_var<std::string>(expr_s[0].value);
    }
    else if (expr_s.size() == 2) {
        if (expr_s[1].type == expression::expression_t::NUMBER) {
            if (index_var == 1)
                current_scope.get_var_value<i32t>(expr_s[0].value) = std::stoi(expr_s[1].value);
            else if (index_var == 3)
                current_scope.get_var_value<std::vector<i32t>>(expr_s[0].value) = {std::stoi(expr_s[1].value)};
            else if (index_var == 0)
                (void)current_scope.create_var<i32t>(expr_s[0].value, std::stoll(expr_s[1].value));
        }
        else if (expr_s[1].type == expression::expression_t::STRING) {
            if (index_var == 2)
                current_scope.get_var_value<std::string>(expr_s[0].value) = expr_s[1].value;
            else if (index_var == 4)
                current_scope.get_var_value<std::vector<std::string>>(expr_s[0].value) = {expr_s[1].value};
            else if (index_var == 0)
                (void)current_scope.create_var<std::string>(expr_s[0].value, expr_s[1].value);
        }
    }
    else if (expr_s.size() > 2) {
        if (expr_s[1].type == expression::expression_t::NUMBER) {
            std::vector<i32t> vec_int_params;
            vec_int_params.push_back(std::stoi(expr_s[1].value));
            for (u32t i = 2; i < expr_s.size(); ++i) {
                if (expr_s[i].type == expression::expression_t::NUMBER)
                    vec_int_params.push_back(std::stoi(expr_s[i].value));
            }
            if (index_var == 0)
                (void)current_scope.create_var<std::vector<i32t>>(expr_s[0].value, vec_int_params);
            else if (index_var == 3)
                current_scope.get_var_value<std::vector<i32t>>(expr_s[0].value) = vec_int_params;
        }
        else if (expr_s[1].type == expression::expression_t::STRING) {
            std::vector<std::string> vec_str_params;
            vec_str_params.push_back(expr_s[1].value);
            for (u32t i = 2; i < expr_s.size(); ++i) {
                if (expr_s[i].type == expression::expression_t::STRING)
                    vec_str_params.push_back(expr_s[i].value);
            }
            if (index_var == 0)
                (void)current_scope.create_var<std::vector<std::string>>(expr_s[0].value, vec_str_params);
            else if (index_var == 4)
                current_scope.get_var_value<std::vector<std::string>>(expr_s[0].value) = vec_str_params;
        }
    }

    // ??????
    // if a variable is abstract ref to cfg structs
    update_cfg_struct(expr_s[0].value, current_scope);
}

void sl_func::file(const expressions &expr_s, var::scope &current_scope) {
    if (std::stoi(expr_s[1].value) == 0) {
        std::vector<std::string> path_files;
        for (u32t i = 2; i < expr_s.size(); ++i) {
            if (std::filesystem::exists(expr_s[i].value)) {
                path_files.push_back(std::filesystem::absolute(expr_s[i].value).string());
                continue;
            }
            std::string tmp_path = expr_s[i].value;
            std::string mask     = expr_s[i].value;
            u32t it              = tmp_path.find_last_of("/\\");
            if (it != tmp_path.npos) {
                tmp_path.erase(it);
                mask = std::filesystem::absolute(mask).string();
                if (!std::filesystem::is_directory(tmp_path))
                    _log << bwtools::fatal
                         << (log_message(log_type::fatal) << "Directory does not exist: " << tmp_path);
            }
            else {
                tmp_path = std::filesystem::current_path().string();
                mask     = tmp_path + "/" + mask;
            }
            std::vector<std::string> tmp_path_files, tmp_path_files_o;
            for (const auto &it_path : std::filesystem::directory_iterator(tmp_path))
                tmp_path_files.push_back(it_path.path().string());
            tmp_path_files_o = bwfile::file_slc_mask(mask, tmp_path_files);
            for (const auto &path_file : tmp_path_files_o)
                path_files.push_back(path_file);
        }

        if (path_files.size() == 1)
            current_scope.create_var<std::string>(expr_s[0].value, path_files[0]);
        else
            current_scope.create_var<std::vector<std::string>>(expr_s[0].value, path_files);
    }
}

void sl_func::project(const expressions &expr_s, var::scope &current_scope) {
    var::struct_sb::project &prj_ref = current_scope.create_var<var::struct_sb::project>(expr_s[0].value);
    prj_ref.name_project             = expr_s[0].value;
    prj_ref.language                 = expr_s[1].value;
    for (u32t i = 2; i < expr_s.size(); ++i)
        prj_ref.src_files.push_back(expr_s[i].value);

    // ??????? - I don't know if this helper function is needed
    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<i32t>(DECL_VAR_STRUCT) > 0) {
        if (!current_scope.try_create_var<std::string>(prj_ref.name_project + PRJ_VAR_NAME_LANG, prj_ref.language))
            prj_ref.language = current_scope.get_var_value<std::string>(prj_ref.name_project + PRJ_VAR_NAME_LANG);
        if (!current_scope.try_create_var<std::string>(prj_ref.name_project + PRJ_VAR_NAME_VER,
                                                       prj_ref.version_project.get_str_version()))
            prj_ref.version_project = var::struct_sb::version(
                current_scope.get_var_value<std::string>(prj_ref.name_project + PRJ_VAR_NAME_VER));

        if (!current_scope.try_create_var<std::string>(prj_ref.name_project + PRJ_VAR_NAME_DFLAGS_C,
                                                       prj_ref.dflags_compiler))
            prj_ref.dflags_compiler =
                current_scope.get_var_value<std::string>(prj_ref.name_project + PRJ_VAR_NAME_DFLAGS_C);
        if (!current_scope.try_create_var<std::string>(prj_ref.name_project + PRJ_VAR_NAME_DFLAGS_L,
                                                       prj_ref.dflags_linker))
            prj_ref.dflags_linker =
                current_scope.get_var_value<std::string>(prj_ref.name_project + PRJ_VAR_NAME_DFLAGS_L);
        if (!current_scope.try_create_var<std::string>(prj_ref.name_project + PRJ_VAR_NAME_RFLAGS_C,
                                                       prj_ref.rflags_compiler))
            prj_ref.rflags_compiler =
                current_scope.get_var_value<std::string>(prj_ref.name_project + PRJ_VAR_NAME_RFLAGS_C);
        if (!current_scope.try_create_var<std::string>(prj_ref.name_project + PRJ_VAR_NAME_RFLAGS_L,
                                                       prj_ref.rflags_linker))
            prj_ref.rflags_linker =
                current_scope.get_var_value<std::string>(prj_ref.name_project + PRJ_VAR_NAME_RFLAGS_L);
        if (!current_scope.try_create_var<std::string>(prj_ref.name_project + PRJ_VAR_NAME_PTH_C,
                                                       prj_ref.path_compiler))
            prj_ref.path_compiler = current_scope.get_var_value<std::string>(prj_ref.name_project + PRJ_VAR_NAME_PTH_C);
        if (!current_scope.try_create_var<std::string>(prj_ref.name_project + PRJ_VAR_NAME_PTH_L, prj_ref.path_linker))
            prj_ref.path_linker = current_scope.get_var_value<std::string>(prj_ref.name_project + PRJ_VAR_NAME_PTH_L);
        if (!current_scope.try_create_var<i32t>(prj_ref.name_project + PRJ_VAR_NAME_STD_C, prj_ref.standart_c))
            prj_ref.standart_c = current_scope.get_var_value<i32t>(prj_ref.name_project + PRJ_VAR_NAME_STD_C);
        if (!current_scope.try_create_var<i32t>(prj_ref.name_project + PRJ_VAR_NAME_STD_CPP, prj_ref.standart_cpp))
            prj_ref.standart_cpp = current_scope.get_var_value<i32t>(prj_ref.name_project + PRJ_VAR_NAME_STD_CPP);
        if (!current_scope.try_create_var<std::vector<std::string>>(prj_ref.name_project + PRJ_VAR_NAME_SRC_FILES,
                                                                    prj_ref.src_files))
            prj_ref.src_files =
                current_scope.get_var_value<std::vector<std::string>>(prj_ref.name_project + PRJ_VAR_NAME_SRC_FILES);
        if (!current_scope.try_create_var<std::vector<std::string>>(prj_ref.name_project + PRJ_VAR_NAME_UTEMPLATES,
                                                                    prj_ref.vec_templates))
            prj_ref.vec_templates =
                current_scope.get_var_value<std::vector<std::string>>(prj_ref.name_project + PRJ_VAR_NAME_UTEMPLATES);
        if (!current_scope.try_create_var<i32t>(prj_ref.name_project + PRJ_VAR_NAME_UITTEMPLATES,
                                                prj_ref.use_it_templates))
            prj_ref.use_it_templates =
                current_scope.get_var_value<i32t>(prj_ref.name_project + PRJ_VAR_NAME_UITTEMPLATES);
    }
}
void sl_func::executable(const expressions &expr_s, var::scope &current_scope) {
    if (current_scope.what_type(expr_s[2].value) != 5)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Expected var project: " << expr_s[2].value);

    var::struct_sb::target &trg_ref = current_scope.create_var<var::struct_sb::target>(expr_s[0].value);

    trg_ref.name_target = expr_s[0].value;
    trg_ref.prj         = std::shared_ptr<var::struct_sb::project>(
        (var::struct_sb::project *)&current_scope.get_var_value<var::struct_sb::project>(expr_s[2].value),
        [](const var::struct_sb::project *) {});
    trg_ref.target_cfg     = (var::struct_sb::configuration)std::stoi(expr_s[1].value);
    trg_ref.version_target = var::struct_sb::version(0, 0, 0);

    // ??????? - I don't know if this helper function is needed
    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<i32t>(DECL_VAR_STRUCT) > 0) {
        if (!current_scope.try_create_var<std::string>(trg_ref.name_target + TRG_VAR_NAME_NPROJECT,
                                                       trg_ref.prj->name_project))
            trg_ref.prj->name_project =
                current_scope.get_var_value<std::string>(trg_ref.name_target + TRG_VAR_NAME_NPROJECT);
        if (!current_scope.try_create_var<std::string>(trg_ref.name_target + TRG_VAR_NAME_VER,
                                                       trg_ref.version_target.get_str_version()))
            trg_ref.version_target = var::struct_sb::version(
                current_scope.get_var_value<std::string>(trg_ref.name_target + TRG_VAR_NAME_VER));
        if (!current_scope.try_create_var<i32t>(trg_ref.name_target + TRG_VAR_NAME_CFG, (i32t)trg_ref.target_cfg))
            trg_ref.target_cfg = (var::struct_sb::configuration)current_scope.get_var_value<i32t>(trg_ref.name_target +
                                                                                                  TRG_VAR_NAME_CFG);
        if (!current_scope.try_create_var<i32t>(trg_ref.name_target + TRG_VAR_NAME_TYPE_T, (i32t)trg_ref.target_t))
            trg_ref.target_t = (var::struct_sb::type_target)current_scope.get_var_value<i32t>(trg_ref.name_target +
                                                                                              TRG_VAR_NAME_TYPE_T);
        if (!current_scope.try_create_var<std::string>(trg_ref.name_target + TRG_VAR_NAME_NGENERATOR,
                                                       trg_ref.name_generator))
            trg_ref.name_generator =
                current_scope.get_var_value<std::string>(trg_ref.name_target + TRG_VAR_NAME_NGENERATOR);
        if (!current_scope.try_create_var<std::vector<std::string>>(trg_ref.name_target + TRG_VAR_NAME_LLIBS,
                                                                    trg_ref.target_vec_libs))
            trg_ref.target_vec_libs =
                current_scope.get_var_value<std::vector<std::string>>(trg_ref.name_target + TRG_VAR_NAME_LLIBS);
    }
}

void sl_func::link_lib(const expressions &expr_s, var::scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 6)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Expected var target: " << expr_s[2].value);

    var::struct_sb::target &trg_ref = current_scope.get_var_value<var::struct_sb::target>(expr_s[0].value);

    for (u32t i = 1; i < expr_s.size(); ++i)
        trg_ref.target_vec_libs.push_back(expr_s[i].value);

    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<i32t>(DECL_VAR_STRUCT) > 0) {
        if (current_scope.what_type(trg_ref.name_target + TRG_VAR_NAME_LLIBS) == 4) {
            current_scope.get_var_value<std::vector<std::string>>(trg_ref.name_target + TRG_VAR_NAME_LLIBS) =
                trg_ref.target_vec_libs;
        }
        else
            current_scope.create_var<std::vector<std::string>>(trg_ref.name_target + TRG_VAR_NAME_LLIBS,
                                                               trg_ref.target_vec_libs);
    }
}
void sl_func::exp_data(const expressions &expr_s, var::scope &current_scope) {
    interpreter tmp_interpreter(expr_s[0].value);
    tmp_interpreter.set_external_scope(&current_scope);
}
void sl_func::cmd(const expressions &expr_s, var::scope &current_scope) {
    u32t val = std::stoi(expr_s[0].value);
    if (!val)
        current_scope.call_cmd.add_call_before(expr_s[1].value);
    else
        current_scope.call_cmd.add_call_after(expr_s[1].value);
}
void sl_func::debug(const expressions &expr_s, var::scope &current_scope) {
    std::string str_out;
    for (u32t i = 0; i < expr_s.size(); ++i)
        str_out += expr_s[i].value + " ";
    _log << bwtools::message << (log_message(log_type::msg) << str_out);
}

void sl_func::debug_struct(const expressions &expr_s, var::scope &current_scope) {
    std::string str_out;
    u32t ind = current_scope.what_type(expr_s[0].value);
    if (ind == 5) {
        var::struct_sb::project &prj_ref = current_scope.get_var_value<var::struct_sb::project>(expr_s[0].value);
        str_out = "Name Project: " + prj_ref.name_project + "\nVersion: " + prj_ref.version_project.get_str_version() +
                  "\nLang: " + prj_ref.language + "\nCompiler: " + prj_ref.path_compiler +
                  "\nLinker: " + prj_ref.path_linker + "\nDebug Flags Compiler: " + prj_ref.dflags_compiler +
                  "\nRelease Flags Compiler: " + prj_ref.rflags_compiler +
                  "\nDebug Flags Linker: " + prj_ref.dflags_linker +
                  "\nRelease Flags Compiler: " + prj_ref.rflags_linker +
                  "\nStandart C: " + std::to_string(prj_ref.standart_c) +
                  "\nStandart C++: " + std::to_string(prj_ref.standart_cpp) + "\n------\nSRC_FILES: \n";
        for (u32t i = 0; i < prj_ref.src_files.size(); ++i) {
            str_out += prj_ref.src_files[i];
            if (i != prj_ref.src_files.size() - 1)
                str_out += ", ";
        }
        str_out += "\n\nWill templates be used? - " + std::to_string(prj_ref.use_it_templates) + "\nTemplates: \n";
        for (u32t i = 0; i < prj_ref.vec_templates.size(); ++i) {
            str_out += prj_ref.vec_templates[i];
            if (i != prj_ref.vec_templates.size() - 1)
                str_out += ", ";
        }
    }
    else if (ind == 6) {
        var::struct_sb::target &trg_ref = current_scope.get_var_value<var::struct_sb::target>(expr_s[0].value);
        str_out = "Name Target: " + trg_ref.name_target + "\nVersion: " + trg_ref.version_target.get_str_version() +
                  "\nName Project: " + trg_ref.prj->name_project + "\nType Build: " + target_t_str(trg_ref.target_t) +
                  "\nConfiguration: " + cfg_str(trg_ref.target_cfg) + "\nLibs: \n";
        for (u32t i = 0; i < trg_ref.target_vec_libs.size(); ++i) {
            str_out += trg_ref.target_vec_libs[i];
            if (i != trg_ref.target_vec_libs.size() - 1)
                str_out += ", ";
        }
    }
    _log << bwtools::message << (log_message(log_type::msg) << str_out);
}

void sl_func::flags_compiler(const expressions &expr_s, var::scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 5)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Expected var project: " << expr_s[0].value);

    var::struct_sb::project &prj_ref = current_scope.get_var_value<var::struct_sb::project>(expr_s[0].value);
    var::struct_sb::configuration tmp_cfg;

    if ((tmp_cfg = (var::struct_sb::configuration)std::stoi(expr_s[1].value)) == var::struct_sb::configuration::RELEASE)
        for (u32t i = 2; i < expr_s.size(); ++i) {
            prj_ref.rflags_compiler += expr_s[i].value + " ";
        }
    else
        for (u32t i = 2; i < expr_s.size(); ++i) {
            prj_ref.dflags_compiler += expr_s[i].value + " ";
        }

    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<i32t>(DECL_VAR_STRUCT) > 0)
        if (tmp_cfg == var::struct_sb::configuration::DEBUG)
            if (current_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_DFLAGS_C) == 2) {
                current_scope.get_var_value<std::string>(prj_ref.name_project + PRJ_VAR_NAME_DFLAGS_C) =
                    prj_ref.dflags_compiler;
            }
            else
                current_scope.create_var<std::string>(prj_ref.name_project + PRJ_VAR_NAME_DFLAGS_C,
                                                      prj_ref.dflags_compiler);
        else if (tmp_cfg == var::struct_sb::configuration::RELEASE)
            if (current_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_RFLAGS_C) == 2) {
                current_scope.get_var_value<std::string>(prj_ref.name_project + PRJ_VAR_NAME_RFLAGS_C) =
                    prj_ref.rflags_compiler;
            }
            else
                current_scope.create_var<std::string>(prj_ref.name_project + PRJ_VAR_NAME_RFLAGS_C,
                                                      prj_ref.rflags_compiler);
}
void sl_func::flags_linker(const expressions &expr_s, var::scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 5)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Expected var project: " << expr_s[0].value);

    var::struct_sb::project &prj_ref = current_scope.get_var_value<var::struct_sb::project>(expr_s[0].value);
    var::struct_sb::configuration tmp_cfg;

    if ((tmp_cfg = (var::struct_sb::configuration)std::stoi(expr_s[1].value)) == var::struct_sb::configuration::RELEASE)
        for (u32t i = 2; i < expr_s.size(); ++i) {
            prj_ref.rflags_linker += expr_s[i].value + " ";
        }
    else
        for (u32t i = 2; i < expr_s.size(); ++i) {
            prj_ref.dflags_linker += expr_s[i].value + " ";
        }

    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<i32t>(DECL_VAR_STRUCT) > 0)
        if (tmp_cfg == var::struct_sb::configuration::DEBUG)
            if (current_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_DFLAGS_L) == 2) {
                current_scope.get_var_value<std::string>(prj_ref.name_project + PRJ_VAR_NAME_DFLAGS_L) =
                    prj_ref.dflags_linker;
            }
            else
                current_scope.create_var<std::string>(prj_ref.name_project + PRJ_VAR_NAME_DFLAGS_L,
                                                      prj_ref.dflags_linker);
        else if (tmp_cfg == var::struct_sb::configuration::RELEASE)
            if (current_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_RFLAGS_L) == 2) {
                current_scope.get_var_value<std::string>(prj_ref.name_project + PRJ_VAR_NAME_RFLAGS_L) =
                    prj_ref.rflags_linker;
            }
            else
                current_scope.create_var<std::string>(prj_ref.name_project + PRJ_VAR_NAME_RFLAGS_L,
                                                      prj_ref.dflags_linker);
}
void sl_func::path_compiler(const expressions &expr_s, var::scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 5)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Expected var project: " << expr_s[0].value);

    var::struct_sb::project &prj_ref = current_scope.get_var_value<var::struct_sb::project>(expr_s[0].value);
    prj_ref.path_compiler            = expr_s[1].value;

    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<i32t>(DECL_VAR_STRUCT) > 0)
        if (current_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_PTH_C) == 2) {
            current_scope.get_var_value<std::string>(prj_ref.name_project + PRJ_VAR_NAME_PTH_C) = prj_ref.path_compiler;
        }
        else
            current_scope.create_var<std::string>(prj_ref.name_project + PRJ_VAR_NAME_PTH_C, prj_ref.path_compiler);
}
void sl_func::path_linker(const expressions &expr_s, var::scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 5)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Expected var project: " << expr_s[0].value);

    var::struct_sb::project &prj_ref = current_scope.get_var_value<var::struct_sb::project>(expr_s[0].value);
    prj_ref.path_linker              = expr_s[1].value;

    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<i32t>(DECL_VAR_STRUCT) > 0)
        if (current_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_PTH_C) == 2) {
            current_scope.get_var_value<std::string>(prj_ref.name_project + PRJ_VAR_NAME_PTH_C) = prj_ref.path_linker;
        }
        else
            current_scope.create_var<std::string>(prj_ref.name_project + PRJ_VAR_NAME_PTH_C, prj_ref.path_linker);
}
void sl_func::standart_c(const expressions &expr_s, var::scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 5)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Expected var project: " << expr_s[0].value);

    var::struct_sb::project &prj_ref = current_scope.get_var_value<var::struct_sb::project>(expr_s[0].value);
    prj_ref.standart_c               = std::stoi(expr_s[1].value);

    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<i32t>(DECL_VAR_STRUCT) > 0)
        if (current_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_STD_C) == 1) {
            current_scope.get_var_value<i32t>(prj_ref.name_project + PRJ_VAR_NAME_STD_C) = prj_ref.standart_c;
        }
        else
            current_scope.create_var<i32t>(prj_ref.name_project + PRJ_VAR_NAME_STD_C, prj_ref.standart_c);
}
void sl_func::standart_cpp(const expressions &expr_s, var::scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 5)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Expected var project: " << expr_s[0].value);

    var::struct_sb::project &prj_ref = current_scope.get_var_value<var::struct_sb::project>(expr_s[0].value);
    prj_ref.standart_cpp             = std::stoi(expr_s[1].value);

    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<i32t>(DECL_VAR_STRUCT) > 0)
        if (current_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_STD_CPP) == 1) {
            current_scope.get_var_value<i32t>(prj_ref.name_project + PRJ_VAR_NAME_STD_CPP) = prj_ref.standart_c;
        }
        else
            current_scope.create_var<i32t>(prj_ref.name_project + PRJ_VAR_NAME_STD_CPP, prj_ref.standart_c);
}
void sl_func::lang(const expressions &expr_s, var::scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 5)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Expected var project: " << expr_s[0].value);

    var::struct_sb::project &prj_ref = current_scope.get_var_value<var::struct_sb::project>(expr_s[0].value);
    prj_ref.language                 = expr_s[1].value;

    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<i32t>(DECL_VAR_STRUCT) > 0)
        if (current_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_LANG) == 1)
            current_scope.get_var_value<std::string>(prj_ref.name_project + PRJ_VAR_NAME_LANG) = prj_ref.language;
        else
            current_scope.create_var<std::string>(prj_ref.name_project + PRJ_VAR_NAME_LANG, prj_ref.language);
}

void sl_func::generator(const expressions &expr_s, var::scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 6)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Expected var project: " << expr_s[0].value);

    var::struct_sb::target &trg_ref = current_scope.get_var_value<var::struct_sb::target>(expr_s[0].value);
    trg_ref.name_generator          = expr_s[1].value;
}

void sl_func::add_param_template(const expressions &expr_s, var::scope &current_scope) {
    if (current_scope.what_type(expr_s[1].value) != 2)
        _log << bwtools::fatal
             << (log_message(log_type::fatal)
                 << "Global template arguments must be of type string: " << expr_s[0].value);
    current_scope.create_var<std::pair<std::string, std::string>>(
        expr_s[0].value, {expr_s[0].value, current_scope.get_var_value<std::string>(expr_s[1].value)});
}

void sl_func::create_templates(const expressions &expr_s, var::scope &current_scope) {
    std::string tmp_param;

    std::array<std::string, 17> name_field_target = {
        NAME_FIELD_TARGET_NAME,        NAME_FIELD_TARGET_LIBS,        NAME_FIELD_TARGET_TYPE,
        NAME_FIELD_TARGET_CFG,         NAME_FIELD_TARGET_VER,         NAME_FIELD_PROJECT_NAME,
        NAME_FIELD_PROJECT_VER,        NAME_FIELD_PROJECT_LANG,       NAME_FIELD_PROJECT_PCOMPILER,
        NAME_FIELD_PROJECT_PLINKER,    NAME_FIELD_PROJECT_RFCOMPILER, NAME_FIELD_PROJECT_RFLINKER,
        NAME_FIELD_PROJECT_DFCOMPILER, NAME_FIELD_PROJECT_DFLINKER,   NAME_FIELD_PROJECT_STD_C,
        NAME_FIELD_PROJECT_STD_CPP,    NAME_FIELD_PROJECT_SRC_FILES};

    var::struct_sb::template_command tcmd_tmp;
    var::struct_sb::template_command::arg arg_tmp;
    tcmd_tmp.name = expr_s[0].value;

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

    for (u32t i = 0; i < expr_s[1].value.size(); ++i) {

        if ((expr_s[1].value[i] == ' ' || expr_s[1].value[i] == '\n' || expr_s[1].value[i] == '\t') && !op_quote) {
            // te xt <- it is forbidden
            if (!tmp_param.empty())
                was_sep = 1;
            else if (tmp_param.empty() && was_sep) // after processing of operator
                was_sep = 0;
            continue;
        }
        else if ((op_quote && expr_s[1].value[i] != '\'') ||
                 (is_name_features_bs &&
                  (expr_s[1].value[i] == ':' || isdigit(expr_s[1].value[i]) || isalpha(expr_s[1].value[i]))))
            goto curr_sym;

        if (expr_s[1].value[i] == '(') {
            if (defined_template)
                _log << bwtools::fatal
                     << (log_message(log_type::fatal)
                         << "Unexpected operator of parameter enum of template: " << expr_s[1].value);
            else if (tmp_param.empty())
                _log << bwtools::fatal
                     << (log_message(log_type::fatal)
                         << "The name of the call component is empty(" << tmp_param << "): " << expr_s[1].value);
            else if (!tcmd_tmp.name_call_component.empty())
                _log << bwtools::fatal
                     << (log_message(log_type::fatal) << "The name of the call component that already exists("
                                                      << tmp_param << "): " << expr_s[1].value);
            else if (current_scope.what_type(tmp_param) != 8)
                _log << bwtools::fatal
                     << (log_message(log_type::fatal)
                         << "The specified call component does not exist(" << tmp_param << "): " << expr_s[1].value);

            tcmd_tmp.name_call_component = tmp_param;
            tmp_param.clear();

            defined_template   = 1;
            op_close_acp_param = 1;

            continue;
        }
        else if (expr_s[1].value[i] == ')') {
            if (!op_close_acp_param)
                _log << bwtools::fatal
                     << (log_message(log_type::fatal)
                         << "Unexpected operator(" << tmp_param << "): " << expr_s[1].value);
            else if (tmp_param.empty())
                _log << bwtools::fatal
                     << (log_message(log_type::fatal) << "The name of the parameters of call component is empty("
                                                      << tmp_param << "): " << expr_s[1].value);
            else if (tmp_param == "NULL")
                goto next_op_mn;
            else if (find(tcmd_tmp.name_accept_params.begin(), tcmd_tmp.name_accept_params.end(), tmp_param) !=
                     tcmd_tmp.name_accept_params.end())
                _log << bwtools::fatal
                     << (log_message(log_type::fatal)
                         << "The parameter was already specified in the internal parameters(" << tmp_param
                         << "): " << expr_s[1].value);

            tcmd_tmp.name_accept_params.push_back(tmp_param);

        next_op_mn:
            tmp_param.clear();

            op_minus_and_next_arrow = 1;
            op_close_acp_param      = 0;

            continue;
        }
        else if (expr_s[1].value[i] == '-') {
            if (!op_minus_and_next_arrow || i == expr_s[1].value.size() - 1 || expr_s[1].value[i + 1] != '>')
                _log << bwtools::fatal
                     << (log_message(log_type::fatal)
                         << "Unexpected operator - \'-\'(" << tmp_param << "): " << expr_s[1].value);

            colon                   = 1;
            op_minus_and_next_arrow = 0;

            ++i;
            continue;
        }
        else if (expr_s[1].value[i] == ':') {
            if (op_quote || op_target_hand_field)
                goto curr_sym;
            if (!colon)
                _log << bwtools::fatal
                     << (log_message(log_type::fatal)
                         << "Unexpected operator - \':\'(" << tmp_param << "): " << expr_s[1].value);
            else if (tmp_param.empty())
                _log << bwtools::fatal
                     << (log_message(log_type::fatal) << "The name of the returned value of call component is empty("
                                                      << tmp_param << "): " << expr_s[1].value);

            beg_param = 1;
            colon     = 0;

            tcmd_tmp.returnable = tmp_param;
            tmp_param.clear();

            continue;
        }
        else if (beg_param) {
            if (isalpha(expr_s[1].value[i])) {
                beg_param = 0;

                op_comma            = 1;
                is_name_features_bs = 1;
                goto curr_sym;
            }
            else if (expr_s[1].value[i] != '<')
                _log << bwtools::fatal
                     << (log_message(log_type::fatal)
                         << "The enumeration of parameters is expected(" << tmp_param << "): " << expr_s[1].value);

            beg_param = 0;
            end_param = 1;

            continue;
        }
        else if (end_param && expr_s[1].value[i] == '>') {
            if ((was_close_used_internal_param || was_close_quote || was_close_op_target_hand) && tmp_param == "NULL") {
                was_close_used_internal_param = 0;
                goto next_arg;
            }

            if (used_internal_param || op_quote || op_target_hand_field)
                _log << bwtools::fatal
                     << (log_message(log_type::fatal)
                         << "The construction of using internal arguments, just string and handle to field"
                            " of target must comply with this structure - {OPEN_OPERATOR}SOMETHING{CLOSE_OPERATOR}("
                         << tmp_param << "): " << expr_s[1].value);
            else if ((!was_close_used_internal_param && !was_close_quote && !was_close_op_target_hand) &&
                     (current_scope.what_type(tmp_param) != 2 ||
                      current_scope.what_type(current_scope.get_var_value<std::string>(tmp_param)) != 2))
                _log << bwtools::fatal
                     << (log_message(log_type::fatal)
                         << "Template argument does not exist externally(" << tmp_param << "): " << expr_s[1].value);
            else if (was_close_used_internal_param &&
                     find(tcmd_tmp.name_accept_params.begin(), tcmd_tmp.name_accept_params.end(), tmp_param) ==
                         tcmd_tmp.name_accept_params.end())
                _log << bwtools::fatal
                     << (log_message(log_type::fatal)
                         << "Template argument does not exist internally(" << tmp_param << "): " << expr_s[1].value);
            else if (was_close_quote) {
                arg_tmp.arg_t   = var::struct_sb::template_command::arg::type::string;
                was_close_quote = 0;
            }
            else if (was_close_op_target_hand) {
                if (find(name_field_target.begin(), name_field_target.end(), tmp_param) == name_field_target.end()) {
                    if (tmp_param.find(":") != tmp_param.npos) {
                        std::string str_tmp{tmp_param};
                        str_tmp.erase(str_tmp.find(":"));

                        if (find(name_field_target.begin(), name_field_target.end(), str_tmp) !=
                            name_field_target.end())
                            goto proc_hand_target_field;
                    }
                    _log << bwtools::fatal
                         << (log_message(log_type::fatal)
                             << "Field does not exist in target structure(" << tmp_param << "): " << expr_s[1].value);
                }
            proc_hand_target_field:
                arg_tmp.arg_t            = var::struct_sb::template_command::arg::type::trgfield;
                was_close_op_target_hand = 0;
            }
            else if (was_close_used_internal_param) {
                arg_tmp.arg_t                 = var::struct_sb::template_command::arg::type::internal;
                was_close_used_internal_param = 0;
            }
            else
                arg_tmp.arg_t = var::struct_sb::template_command::arg::type::extglobal;
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
        else if (end_param && expr_s[1].value[i] == '\'') {
            if ((!tmp_param.empty() || !end_param) && !op_quote)
                _log << bwtools::fatal
                     << (log_message(log_type::fatal) << "The symbol - \' must be after the beginning of the argument("
                                                      << tmp_param << "): " << expr_s[1].value);
            else if (!end_param && op_quote)
                _log << bwtools::fatal
                     << (log_message(log_type::fatal)
                         << "The symbol - \' must be at end of the argument(" << tmp_param << "): " << expr_s[1].value);

            if (!op_quote)
                op_quote = 1;
            else {
                op_quote        = 0;
                was_close_quote = 1;
            }

            continue;
        }
        else if (end_param && expr_s[1].value[i] == '[') {
            if (!tmp_param.empty() || !end_param || op_target_hand_field)
                _log << bwtools::fatal
                     << (log_message(log_type::fatal)
                         << "The symbol - \'[\' must be after the beginning of the argument(" << tmp_param
                         << "): " << expr_s[1].value);

            op_target_hand_field = 1;

            continue;
        }
        else if (end_param && expr_s[1].value[i] == ']') {
            if (!end_param)
                _log << bwtools::fatal
                     << (log_message(log_type::fatal) << "The symbol - \']\' must be at end of the argument("
                                                      << tmp_param << "): " << expr_s[1].value);

            op_target_hand_field     = 0;
            was_close_op_target_hand = 1;

            continue;
        }
        else if (end_param && expr_s[1].value[i] == '{') {
            if (!tmp_param.empty() || !end_param)
                _log << bwtools::fatal
                     << (log_message(log_type::fatal)
                         << "The symbol - \'}\' must be after the beginning of the argument(" << tmp_param
                         << "): " << expr_s[1].value);

            used_internal_param = 1;

            continue;
        }
        else if (end_param && expr_s[1].value[i] == '}') {
            if (!end_param)
                _log << bwtools::fatal
                     << (log_message(log_type::fatal) << "The symbol - \'}\' must be at the end of the argument("
                                                      << tmp_param << "): " << expr_s[1].value);

            used_internal_param           = 0;
            was_close_used_internal_param = 1;

            continue;
        }
        else if (op_close_acp_param && expr_s[1].value[i] == ',') {
            if (tmp_param == "NULL")
                _log << bwtools::fatal
                     << (log_message(log_type::fatal)
                         << "If the parameter is zero, there should be no other parameters(" << tmp_param
                         << "): " << expr_s[1].value);

            tcmd_tmp.name_accept_params.push_back(tmp_param);
            tmp_param.clear();
            continue;
        }
        else if (op_comma) {
            if (expr_s[1].value[i] != ',' && !is_name_features_bs)
                _log << bwtools::fatal
                     << (log_message(log_type::fatal) << "The operator is expected - \',\'" << expr_s[1].value);
            else if (expr_s[1].value[i] == ',' && is_name_features_bs) {
                arg_tmp.arg_t   = var::struct_sb::template_command::arg::type::features;
                arg_tmp.str_arg = tmp_param;
                tcmd_tmp.args.push_back(arg_tmp);
                tmp_param.clear();

                beg_param           = 1;
                is_name_features_bs = 0;
                op_comma            = 0;

                continue;
            }
            else if (is_name_features_bs) {
                if (!isalpha(expr_s[1].value[i]) && expr_s[1].value[i] != '_')
                    _log << bwtools::fatal
                         << (log_message(log_type::fatal)
                             << "The symbol - \'" << expr_s[1].value[i] << "\' was not expected: " << expr_s[1].value);
                goto curr_sym;
            }
            beg_param = 1;
            op_comma  = 0;

            continue;
        }
    curr_sym:

        if (op_minus_and_next_arrow)
            _log << bwtools::fatal
                 << (log_message(log_type::fatal) << "The operator is expected - \'->\':" << expr_s[1].value);
        else if (was_sep) {
            if (beg_param)
                _log << bwtools::fatal
                     << (log_message(log_type::fatal)
                         << "Pattern parameter start operator expected(" << tmp_param << "): " << expr_s[1].value);
            else if (end_param)
                _log << bwtools::fatal
                     << (log_message(log_type::fatal)
                         << "Pattern parameter end operator expected(" << tmp_param << "): " << expr_s[1].value);
            else if (is_name_features_bs || op_comma)
                _log << bwtools::fatal
                     << (log_message(log_type::fatal)
                         << "Expected comma operator(" << tmp_param << "): " << expr_s[1].value);
        }
        tmp_param += expr_s[1].value[i];
    }

    if (beg_param)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Template argument expected: " << expr_s[1].value);
    else if (is_name_features_bs) {
        arg_tmp.arg_t   = var::struct_sb::template_command::arg::type::features;
        arg_tmp.str_arg = tmp_param;
        tcmd_tmp.args.push_back(arg_tmp);
    }
    const auto &vec_templates_cmd = current_scope.get_vector_variables_t<var::struct_sb::template_command>();

    for (const std::string &acp_param : tcmd_tmp.name_accept_params) {
        bool find = 0;
        for (const auto &tcmd : vec_templates_cmd) {
            if (acp_param == tcmd.second.returnable) {
                find = 1;
                break;
            }
        }

        if (!find)
            _log << bwtools::fatal
                 << (log_message(log_type::fatal) << "There is no command template that returns such a value("
                                                  << acp_param << "): " << expr_s[1].value);
    }

    current_scope.create_var<var::struct_sb::template_command>(expr_s[0].value, tcmd_tmp);
}

void sl_func::create_call_component(const expressions &expr_s, var::scope &current_scope) {
    var::struct_sb::call_component ccmp_tmp;
    ccmp_tmp.name              = expr_s[0].value;
    ccmp_tmp.name_program      = expr_s[1].value;
    ccmp_tmp.pattern_ret_files = expr_s[2].value;

    current_scope.create_var<var::struct_sb::call_component>(expr_s[0].value, ccmp_tmp);
}

void sl_func::use_templates(const expressions &expr_s, var::scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 5)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Expected var project: " << expr_s[0].value);

    var::struct_sb::project &prj_ref = current_scope.get_var_value<var::struct_sb::project>(expr_s[0].value);

    for (u32t i = 1; i < expr_s.size(); ++i) {
        if (current_scope.what_type(expr_s[i].value) != 7)
            _log << bwtools::fatal
                 << (log_message(log_type::fatal) << "This template does not exist: " << expr_s[i].value);

        prj_ref.vec_templates.push_back(expr_s[i].value);
    }

    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<i32t>(DECL_VAR_STRUCT) > 0)
        if (current_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_UTEMPLATES) == 4) {
            current_scope.get_var_value<std::vector<std::string>>(prj_ref.name_project + PRJ_VAR_NAME_UTEMPLATES) =
                prj_ref.vec_templates;
        }
        else
            current_scope.create_var<std::vector<std::string>>(prj_ref.name_project + PRJ_VAR_NAME_UTEMPLATES,
                                                               prj_ref.vec_templates);
}
void sl_func::use_it_template(const expressions &expr_s, var::scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 5)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Expected var project: " << expr_s[0].value);

    var::struct_sb::project &prj_ref = current_scope.get_var_value<var::struct_sb::project>(expr_s[0].value);
    prj_ref.use_it_templates         = std::stoi(expr_s[1].value);

    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<i32t>(DECL_VAR_STRUCT) > 0)
        if (current_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_UITTEMPLATES) == 1) {
            current_scope.get_var_value<i32t>(prj_ref.name_project + PRJ_VAR_NAME_UITTEMPLATES) =
                prj_ref.use_it_templates;
        }
        else
            current_scope.create_var<i32t>(prj_ref.name_project + PRJ_VAR_NAME_UITTEMPLATES, prj_ref.use_it_templates);
}
