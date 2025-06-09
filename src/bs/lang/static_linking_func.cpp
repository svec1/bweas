#include "static_linking_func.hpp"

#include "../tools/bwfile.hpp"
#include "interpreter.hpp"

#include <algorithm>
#include <array>

map<string, string> var::struct_sb::project::preset_ext_fields;

static const std::array<string, 18> vec_name_config_var = {
    PRJ_VAR_NAME_DFLAGS_C,  PRJ_VAR_NAME_DFLAGS_L,   PRJ_VAR_NAME_RFLAGS_C, PRJ_VAR_NAME_RFLAGS_L, PRJ_VAR_NAME_LANG,
    PRJ_VAR_NAME_PTH_C,     PRJ_VAR_NAME_PTH_L,      PRJ_VAR_NAME_VER,      PRJ_VAR_NAME_STD_C,    PRJ_VAR_NAME_STD_CPP,
    PRJ_VAR_NAME_SRC_FILES, PRJ_VAR_NAME_UTEMPLATES, TRG_VAR_NAME_NPROJECT, TRG_VAR_NAME_VER,      TRG_VAR_NAME_CFG,
    TRG_VAR_NAME_TYPE_T,    TRG_VAR_NAME_LLIBS};

static bweas::logger _log{"STATUS"};

// ??????
static void update_cfg_struct(const string &name_var, var::scope &current_scope) {
    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<pdiff>(DECL_VAR_STRUCT)) {
        size_t tmp_it = 0;
        for (const auto &it : vec_name_config_var) {
            if ((tmp_it = name_var.find(it)) != SIZE_MAX) {
                if (name_var.size() - it.size() != tmp_it)
                    return;
                string tmp_str_postfix = name_var;
                string tmp_str_prefix  = name_var;
                tmp_str_postfix.erase(0, tmp_it);
                tmp_str_prefix.erase(tmp_it, tmp_str_prefix.size());
                if (tmp_str_postfix == PRJ_VAR_NAME_DFLAGS_C) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 2)
                        return;
                    current_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).dflags_compiler =
                        current_scope.get_var_value<string>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_DFLAGS_L) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 2)
                        return;
                    current_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).dflags_linker =
                        current_scope.get_var_value<string>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_RFLAGS_C) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 2)
                        return;
                    current_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).rflags_compiler =
                        current_scope.get_var_value<string>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_RFLAGS_L) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 2)
                        return;
                    current_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).rflags_linker =
                        current_scope.get_var_value<string>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_PTH_C) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 2)
                        return;
                    current_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).path_compiler =
                        current_scope.get_var_value<string>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_PTH_L) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 2)
                        return;
                    current_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).path_linker =
                        current_scope.get_var_value<string>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_STD_C) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 1)
                        return;
                    current_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).standart_c =
                        current_scope.get_var_value<pdiff>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_STD_CPP) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 1)
                        return;
                    current_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).standart_cpp =
                        current_scope.get_var_value<pdiff>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_LANG) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 1)
                        return;
                    current_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).language =
                        current_scope.get_var_value<string>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_VER) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 2)
                        return;
                    current_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).version_project =
                        var::struct_sb::version(current_scope.get_var_value<string>(name_var));
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_SRC_FILES) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 4)
                        return;
                    current_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).src_files =
                        current_scope.get_var_value<vec<string>>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_UTEMPLATES) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 4)
                        return;
                    current_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix).vec_templates =
                        current_scope.get_var_value<vec<string>>(name_var);
                }
                else if (tmp_str_postfix == TRG_VAR_NAME_NPROJECT) {
                    if (current_scope.what_type(tmp_str_prefix) != 6 || current_scope.what_type(name_var) != 2)
                        return;
                    current_scope.get_var_value<var::struct_sb::target>(tmp_str_prefix).prj->name_project =
                        current_scope.get_var_value<string>(name_var);
                }
                else if (tmp_str_postfix == TRG_VAR_NAME_VER) {
                    if (current_scope.what_type(tmp_str_prefix) != 6 || current_scope.what_type(name_var) != 2)
                        return;
                    current_scope.get_var_value<var::struct_sb::target>(tmp_str_prefix).version_target =
                        var::struct_sb::version(current_scope.get_var_value<string>(name_var));
                }
                else if (tmp_str_postfix == TRG_VAR_NAME_CFG) {
                    if (current_scope.what_type(tmp_str_prefix) != 6 || current_scope.what_type(name_var) != 1)
                        return;
                    current_scope.get_var_value<var::struct_sb::target>(tmp_str_prefix).target_cfg =
                        (var::struct_sb::configuration)current_scope.get_var_value<pdiff>(name_var);
                }
                else if (tmp_str_postfix == TRG_VAR_NAME_TYPE_T) {
                    if (current_scope.what_type(tmp_str_prefix) != 6 || current_scope.what_type(name_var) != 1)
                        return;
                    current_scope.get_var_value<var::struct_sb::target>(tmp_str_prefix).target_t =
                        (var::struct_sb::type_target)current_scope.get_var_value<pdiff>(name_var);
                }
                else if (tmp_str_postfix == TRG_VAR_NAME_NGENERATOR) {
                    if (current_scope.what_type(tmp_str_prefix) != 6 || current_scope.what_type(name_var) != 2)
                        return;
                    current_scope.get_var_value<var::struct_sb::target>(tmp_str_prefix).name_generator =
                        current_scope.get_var_value<string>(name_var);
                }
                else if (tmp_str_postfix == TRG_VAR_NAME_LLIBS) {
                    if (current_scope.what_type(tmp_str_prefix) != 6 || current_scope.what_type(name_var) != 1)
                        return;
                    current_scope.get_var_value<var::struct_sb::target>(tmp_str_prefix).target_vec_libs =
                        current_scope.get_var_value<vec<string>>(name_var);
                }
                else {
                    if (current_scope.what_type(tmp_str_prefix) != 5 && current_scope.what_type(name_var) != 2)
                        return;
                    auto prj      = current_scope.get_var_value<var::struct_sb::project>(tmp_str_prefix);
                    auto it_field = prj.custom_ext_fields.find(tmp_str_postfix);
                    if (it_field != prj.custom_ext_fields.end())
                        it_field->second = current_scope.get_var_value<string>(name_var);
                }
                return;
            }
        }
    }
}

void sl_func::set(const expressions &expr_s, var::scope &current_scope) {
    if (expr_s.size() == 0)
        return;
    size_t index_var = current_scope.what_type(expr_s[0].value);
    if (expr_s.size() == 1) {
        if (index_var == 0)
            (void)current_scope.create_var<string>(expr_s[0].value);
    }
    else if (expr_s.size() == 2) {
        if (expr_s[1].type == expression::expression_t::NUMBER) {
            if (index_var == 1)
                current_scope.get_var_value<pdiff>(expr_s[0].value) = std::stoi(expr_s[1].value);
            else if (index_var == 3)
                current_scope.get_var_value<vec<pdiff>>(expr_s[0].value) = {std::stoi(expr_s[1].value)};
            else if (index_var == 0)
                (void)current_scope.create_var<pdiff>(expr_s[0].value, std::stoll(expr_s[1].value));
        }
        else if (expr_s[1].type == expression::expression_t::STRING) {
            if (index_var == 2)
                current_scope.get_var_value<string>(expr_s[0].value) = expr_s[1].value;
            else if (index_var == 4)
                current_scope.get_var_value<vec<string>>(expr_s[0].value) = {expr_s[1].value};
            else if (index_var == 0)
                (void)current_scope.create_var<string>(expr_s[0].value, expr_s[1].value);
        }
    }
    else if (expr_s.size() > 2) {
        if (expr_s[1].type == expression::expression_t::NUMBER) {
            vec<pdiff> vec_int_params;
            vec_int_params.push_back(std::stoi(expr_s[1].value));
            for (size_t i = 2; i < expr_s.size(); ++i) {
                if (expr_s[i].type == expression::expression_t::NUMBER)
                    vec_int_params.push_back(std::stoi(expr_s[i].value));
            }
            if (index_var == 0)
                (void)current_scope.create_var<vec<pdiff>>(expr_s[0].value, vec_int_params);
            else if (index_var == 3)
                current_scope.get_var_value<vec<pdiff>>(expr_s[0].value) = vec_int_params;
        }
        else if (expr_s[1].type == expression::expression_t::STRING) {
            vec<string> vec_str_params;
            vec_str_params.push_back(expr_s[1].value);
            for (size_t i = 2; i < expr_s.size(); ++i) {
                if (expr_s[i].type == expression::expression_t::STRING)
                    vec_str_params.push_back(expr_s[i].value);
            }
            if (index_var == 0)
                (void)current_scope.create_var<vec<string>>(expr_s[0].value, vec_str_params);
            else if (index_var == 4)
                current_scope.get_var_value<vec<string>>(expr_s[0].value) = vec_str_params;
        }
    }

    // ??????
    // if a variable is abstract ref to cfg structs
    update_cfg_struct(expr_s[0].value, current_scope);
}

void sl_func::file(const expressions &expr_s, var::scope &current_scope) {
    if (std::stoi(expr_s[1].value) == 0) {
        vec<string> path_files;
        for (size_t i = 2; i < expr_s.size(); ++i) {
            if (std::filesystem::exists(expr_s[i].value)) {
                path_files.push_back(std::filesystem::absolute(expr_s[i].value).string());
                continue;
            }
            string tmp_path = expr_s[i].value;
            string mask     = expr_s[i].value;
            size_t it       = tmp_path.find_last_of("/\\");
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
            vec<string> tmp_path_files, tmp_path_files_o;
            for (const auto &it_path : std::filesystem::directory_iterator(tmp_path))
                tmp_path_files.push_back(it_path.path().string());
            tmp_path_files_o = bwfile::file_slc_mask(mask, tmp_path_files);
            for (const auto &path_file : tmp_path_files_o)
                path_files.push_back(path_file);
        }

        if (path_files.size() == 1)
            current_scope.create_var<string>(expr_s[0].value, path_files[0]);
        else
            current_scope.create_var<vec<string>>(expr_s[0].value, path_files);
    }
}

void sl_func::project(const expressions &expr_s, var::scope &current_scope) {
    var::struct_sb::project &prj_ref = current_scope.create_var<var::struct_sb::project>(expr_s[0].value);
    prj_ref.name_project             = expr_s[0].value;
    prj_ref.language                 = expr_s[1].value;
    for (size_t i = 2; i < expr_s.size(); ++i)
        prj_ref.src_files.push_back(expr_s[i].value);

    // ??????? - I don't know if this helper function is needed
    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<pdiff>(DECL_VAR_STRUCT) > 0) {
        if (!current_scope.try_create_var<string>(prj_ref.name_project + PRJ_VAR_NAME_LANG, prj_ref.language))
            prj_ref.language = current_scope.get_var_value<string>(prj_ref.name_project + PRJ_VAR_NAME_LANG);
        if (!current_scope.try_create_var<string>(prj_ref.name_project + PRJ_VAR_NAME_VER,
                                                  prj_ref.version_project.get_str_version()))
            prj_ref.version_project =
                var::struct_sb::version(current_scope.get_var_value<string>(prj_ref.name_project + PRJ_VAR_NAME_VER));

        if (!current_scope.try_create_var<string>(prj_ref.name_project + PRJ_VAR_NAME_DFLAGS_C,
                                                  prj_ref.dflags_compiler))
            prj_ref.dflags_compiler = current_scope.get_var_value<string>(prj_ref.name_project + PRJ_VAR_NAME_DFLAGS_C);
        if (!current_scope.try_create_var<string>(prj_ref.name_project + PRJ_VAR_NAME_DFLAGS_L, prj_ref.dflags_linker))
            prj_ref.dflags_linker = current_scope.get_var_value<string>(prj_ref.name_project + PRJ_VAR_NAME_DFLAGS_L);
        if (!current_scope.try_create_var<string>(prj_ref.name_project + PRJ_VAR_NAME_RFLAGS_C,
                                                  prj_ref.rflags_compiler))
            prj_ref.rflags_compiler = current_scope.get_var_value<string>(prj_ref.name_project + PRJ_VAR_NAME_RFLAGS_C);
        if (!current_scope.try_create_var<string>(prj_ref.name_project + PRJ_VAR_NAME_RFLAGS_L, prj_ref.rflags_linker))
            prj_ref.rflags_linker = current_scope.get_var_value<string>(prj_ref.name_project + PRJ_VAR_NAME_RFLAGS_L);
        if (!current_scope.try_create_var<string>(prj_ref.name_project + PRJ_VAR_NAME_PTH_C, prj_ref.path_compiler))
            prj_ref.path_compiler = current_scope.get_var_value<string>(prj_ref.name_project + PRJ_VAR_NAME_PTH_C);
        if (!current_scope.try_create_var<string>(prj_ref.name_project + PRJ_VAR_NAME_PTH_L, prj_ref.path_linker))
            prj_ref.path_linker = current_scope.get_var_value<string>(prj_ref.name_project + PRJ_VAR_NAME_PTH_L);
        if (!current_scope.try_create_var<pdiff>(prj_ref.name_project + PRJ_VAR_NAME_STD_C, prj_ref.standart_c))
            prj_ref.standart_c = current_scope.get_var_value<pdiff>(prj_ref.name_project + PRJ_VAR_NAME_STD_C);
        if (!current_scope.try_create_var<pdiff>(prj_ref.name_project + PRJ_VAR_NAME_STD_CPP, prj_ref.standart_cpp))
            prj_ref.standart_cpp = current_scope.get_var_value<pdiff>(prj_ref.name_project + PRJ_VAR_NAME_STD_CPP);
        if (!current_scope.try_create_var<vec<string>>(prj_ref.name_project + PRJ_VAR_NAME_SRC_FILES,
                                                       prj_ref.src_files))
            prj_ref.src_files = current_scope.get_var_value<vec<string>>(prj_ref.name_project + PRJ_VAR_NAME_SRC_FILES);
        if (!current_scope.try_create_var<vec<string>>(prj_ref.name_project + PRJ_VAR_NAME_UTEMPLATES,
                                                       prj_ref.vec_templates))
            prj_ref.vec_templates =
                current_scope.get_var_value<vec<string>>(prj_ref.name_project + PRJ_VAR_NAME_UTEMPLATES);
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
    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<pdiff>(DECL_VAR_STRUCT) > 0) {
        if (!current_scope.try_create_var<string>(trg_ref.name_target + TRG_VAR_NAME_NPROJECT,
                                                  trg_ref.prj->name_project))
            trg_ref.prj->name_project =
                current_scope.get_var_value<string>(trg_ref.name_target + TRG_VAR_NAME_NPROJECT);
        if (!current_scope.try_create_var<string>(trg_ref.name_target + TRG_VAR_NAME_VER,
                                                  trg_ref.version_target.get_str_version()))
            trg_ref.version_target =
                var::struct_sb::version(current_scope.get_var_value<string>(trg_ref.name_target + TRG_VAR_NAME_VER));
        if (!current_scope.try_create_var<pdiff>(trg_ref.name_target + TRG_VAR_NAME_CFG, (pdiff)trg_ref.target_cfg))
            trg_ref.target_cfg = (var::struct_sb::configuration)current_scope.get_var_value<pdiff>(trg_ref.name_target +
                                                                                                   TRG_VAR_NAME_CFG);
        if (!current_scope.try_create_var<pdiff>(trg_ref.name_target + TRG_VAR_NAME_TYPE_T, (pdiff)trg_ref.target_t))
            trg_ref.target_t = (var::struct_sb::type_target)current_scope.get_var_value<pdiff>(trg_ref.name_target +
                                                                                               TRG_VAR_NAME_TYPE_T);
        if (!current_scope.try_create_var<string>(trg_ref.name_target + TRG_VAR_NAME_NGENERATOR,
                                                  trg_ref.name_generator))
            trg_ref.name_generator = current_scope.get_var_value<string>(trg_ref.name_target + TRG_VAR_NAME_NGENERATOR);
        if (!current_scope.try_create_var<vec<string>>(trg_ref.name_target + TRG_VAR_NAME_LLIBS,
                                                       trg_ref.target_vec_libs))
            trg_ref.target_vec_libs =
                current_scope.get_var_value<vec<string>>(trg_ref.name_target + TRG_VAR_NAME_LLIBS);
    }
}

void sl_func::link_lib(const expressions &expr_s, var::scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 6)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Expected var target: " << expr_s[2].value);

    var::struct_sb::target &trg_ref = current_scope.get_var_value<var::struct_sb::target>(expr_s[0].value);

    for (size_t i = 1; i < expr_s.size(); ++i)
        trg_ref.target_vec_libs.push_back(expr_s[i].value);

    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<pdiff>(DECL_VAR_STRUCT) > 0) {
        if (current_scope.what_type(trg_ref.name_target + TRG_VAR_NAME_LLIBS) == 4) {
            current_scope.get_var_value<vec<string>>(trg_ref.name_target + TRG_VAR_NAME_LLIBS) =
                trg_ref.target_vec_libs;
        }
        else
            current_scope.create_var<vec<string>>(trg_ref.name_target + TRG_VAR_NAME_LLIBS, trg_ref.target_vec_libs);
    }
}
void sl_func::exp_data(const expressions &expr_s, var::scope &current_scope) {
    interpreter tmp_interpreter(expr_s[0].value);
    tmp_interpreter.set_external_scope(&current_scope);
}
void sl_func::cmd(const expressions &expr_s, var::scope &current_scope) {
    size_t val = std::stoi(expr_s[0].value);
    if (!val)
        current_scope.call_cmd.add_call_before(expr_s[1].value);
    else
        current_scope.call_cmd.add_call_after(expr_s[1].value);
}
void sl_func::debug(const expressions &expr_s, var::scope &current_scope) {
    string str_out;
    for (size_t i = 0; i < expr_s.size(); ++i)
        str_out += expr_s[i].value + " ";
    _log << bwtools::message << (log_message(log_type::msg) << str_out);
}

void sl_func::debug_struct(const expressions &expr_s, var::scope &current_scope) {
    string str_out;
    size_t ind = current_scope.what_type(expr_s[0].value);
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
        for (size_t i = 0; i < prj_ref.src_files.size(); ++i) {
            str_out += prj_ref.src_files[i];
            if (i != prj_ref.src_files.size() - 1)
                str_out += ", ";
        }
        str_out += "\n\nTemplates: \n";
        for (size_t i = 0; i < prj_ref.vec_templates.size(); ++i) {
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
        for (size_t i = 0; i < trg_ref.target_vec_libs.size(); ++i) {
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
        for (size_t i = 2; i < expr_s.size(); ++i) {
            prj_ref.rflags_compiler += expr_s[i].value + " ";
        }
    else
        for (size_t i = 2; i < expr_s.size(); ++i) {
            prj_ref.dflags_compiler += expr_s[i].value + " ";
        }

    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<pdiff>(DECL_VAR_STRUCT) > 0)
        if (tmp_cfg == var::struct_sb::configuration::DEBUG)
            if (current_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_DFLAGS_C) == 2) {
                current_scope.get_var_value<string>(prj_ref.name_project + PRJ_VAR_NAME_DFLAGS_C) =
                    prj_ref.dflags_compiler;
            }
            else
                current_scope.create_var<string>(prj_ref.name_project + PRJ_VAR_NAME_DFLAGS_C, prj_ref.dflags_compiler);
        else if (tmp_cfg == var::struct_sb::configuration::RELEASE)
            if (current_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_RFLAGS_C) == 2) {
                current_scope.get_var_value<string>(prj_ref.name_project + PRJ_VAR_NAME_RFLAGS_C) =
                    prj_ref.rflags_compiler;
            }
            else
                current_scope.create_var<string>(prj_ref.name_project + PRJ_VAR_NAME_RFLAGS_C, prj_ref.rflags_compiler);
}
void sl_func::flags_linker(const expressions &expr_s, var::scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 5)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Expected var project: " << expr_s[0].value);

    var::struct_sb::project &prj_ref = current_scope.get_var_value<var::struct_sb::project>(expr_s[0].value);
    var::struct_sb::configuration tmp_cfg;

    if ((tmp_cfg = (var::struct_sb::configuration)std::stoi(expr_s[1].value)) == var::struct_sb::configuration::RELEASE)
        for (size_t i = 2; i < expr_s.size(); ++i) {
            prj_ref.rflags_linker += expr_s[i].value + " ";
        }
    else
        for (size_t i = 2; i < expr_s.size(); ++i) {
            prj_ref.dflags_linker += expr_s[i].value + " ";
        }

    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<pdiff>(DECL_VAR_STRUCT) > 0)
        if (tmp_cfg == var::struct_sb::configuration::DEBUG)
            if (current_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_DFLAGS_L) == 2) {
                current_scope.get_var_value<string>(prj_ref.name_project + PRJ_VAR_NAME_DFLAGS_L) =
                    prj_ref.dflags_linker;
            }
            else
                current_scope.create_var<string>(prj_ref.name_project + PRJ_VAR_NAME_DFLAGS_L, prj_ref.dflags_linker);
        else if (tmp_cfg == var::struct_sb::configuration::RELEASE)
            if (current_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_RFLAGS_L) == 2) {
                current_scope.get_var_value<string>(prj_ref.name_project + PRJ_VAR_NAME_RFLAGS_L) =
                    prj_ref.rflags_linker;
            }
            else
                current_scope.create_var<string>(prj_ref.name_project + PRJ_VAR_NAME_RFLAGS_L, prj_ref.dflags_linker);
}
void sl_func::path_compiler(const expressions &expr_s, var::scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 5)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Expected var project: " << expr_s[0].value);

    var::struct_sb::project &prj_ref = current_scope.get_var_value<var::struct_sb::project>(expr_s[0].value);
    prj_ref.path_compiler            = expr_s[1].value;

    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<pdiff>(DECL_VAR_STRUCT) > 0)
        if (current_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_PTH_C) == 2) {
            current_scope.get_var_value<string>(prj_ref.name_project + PRJ_VAR_NAME_PTH_C) = prj_ref.path_compiler;
        }
        else
            current_scope.create_var<string>(prj_ref.name_project + PRJ_VAR_NAME_PTH_C, prj_ref.path_compiler);
}
void sl_func::path_linker(const expressions &expr_s, var::scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 5)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Expected var project: " << expr_s[0].value);

    var::struct_sb::project &prj_ref = current_scope.get_var_value<var::struct_sb::project>(expr_s[0].value);
    prj_ref.path_linker              = expr_s[1].value;

    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<pdiff>(DECL_VAR_STRUCT) > 0)
        if (current_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_PTH_C) == 2) {
            current_scope.get_var_value<string>(prj_ref.name_project + PRJ_VAR_NAME_PTH_C) = prj_ref.path_linker;
        }
        else
            current_scope.create_var<string>(prj_ref.name_project + PRJ_VAR_NAME_PTH_C, prj_ref.path_linker);
}
void sl_func::standart_c(const expressions &expr_s, var::scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 5)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Expected var project: " << expr_s[0].value);

    var::struct_sb::project &prj_ref = current_scope.get_var_value<var::struct_sb::project>(expr_s[0].value);
    prj_ref.standart_c               = std::stoi(expr_s[1].value);

    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<pdiff>(DECL_VAR_STRUCT) > 0)
        if (current_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_STD_C) == 1) {
            current_scope.get_var_value<pdiff>(prj_ref.name_project + PRJ_VAR_NAME_STD_C) = prj_ref.standart_c;
        }
        else
            current_scope.create_var<pdiff>(prj_ref.name_project + PRJ_VAR_NAME_STD_C, prj_ref.standart_c);
}
void sl_func::standart_cpp(const expressions &expr_s, var::scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 5)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Expected var project: " << expr_s[0].value);

    var::struct_sb::project &prj_ref = current_scope.get_var_value<var::struct_sb::project>(expr_s[0].value);
    prj_ref.standart_cpp             = std::stoi(expr_s[1].value);

    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<pdiff>(DECL_VAR_STRUCT) > 0)
        if (current_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_STD_CPP) == 1) {
            current_scope.get_var_value<pdiff>(prj_ref.name_project + PRJ_VAR_NAME_STD_CPP) = prj_ref.standart_c;
        }
        else
            current_scope.create_var<pdiff>(prj_ref.name_project + PRJ_VAR_NAME_STD_CPP, prj_ref.standart_c);
}
void sl_func::lang(const expressions &expr_s, var::scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 5)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Expected var project: " << expr_s[0].value);

    var::struct_sb::project &prj_ref = current_scope.get_var_value<var::struct_sb::project>(expr_s[0].value);
    prj_ref.language                 = expr_s[1].value;

    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<pdiff>(DECL_VAR_STRUCT) > 0)
        if (current_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_LANG) == 1)
            current_scope.get_var_value<string>(prj_ref.name_project + PRJ_VAR_NAME_LANG) = prj_ref.language;
        else
            current_scope.create_var<string>(prj_ref.name_project + PRJ_VAR_NAME_LANG, prj_ref.language);
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
    current_scope.create_var<pair<string, string>>(
        expr_s[0].value, {expr_s[0].value, current_scope.get_var_value<string>(expr_s[1].value)});
}

void sl_func::create_templates(const expressions &expr_s, var::scope &current_scope) {
    try {
        auto tcmd_tmp = var::struct_sb::template_command::create_template_command(expr_s[0].value, expr_s[1].value);
        current_scope.create_var<var::struct_sb::template_command>(expr_s[0].value, tcmd_tmp);
    }
    catch (std::runtime_error &excp) {
        _log << bwtools::fatal << (log_message(log_type::fatal) << excp.what());
    }
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

    for (size_t i = 1; i < expr_s.size(); ++i) {
        if (current_scope.what_type(expr_s[i].value) != 7)
            _log << bwtools::fatal
                 << (log_message(log_type::fatal) << "This template does not exist: " << expr_s[i].value);

        prj_ref.vec_templates.push_back(expr_s[i].value);
    }

    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<pdiff>(DECL_VAR_STRUCT) > 0)
        if (current_scope.what_type(prj_ref.name_project + PRJ_VAR_NAME_UTEMPLATES) == 4) {
            current_scope.get_var_value<vec<string>>(prj_ref.name_project + PRJ_VAR_NAME_UTEMPLATES) =
                prj_ref.vec_templates;
        }
        else
            current_scope.create_var<vec<string>>(prj_ref.name_project + PRJ_VAR_NAME_UTEMPLATES,
                                                  prj_ref.vec_templates);
}

