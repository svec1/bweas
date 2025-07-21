//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses />
// ------------------------------------------
//

#include <lang/static_linking_func.hpp>

#include <bwlogger.hpp>
#include <lang/interpreter.hpp>
#include <tools/bwfile.hpp>

#include <algorithm>

using namespace bweas;

map<string, string> sc::project::preset_ext_fields;

static const array<string, 18> vec_name_config_var = {
    PRJ_VAR_NAME_DFLAGS_C,    PRJ_VAR_NAME_DFLAGS_L,  PRJ_VAR_NAME_RFLAGS_C, PRJ_VAR_NAME_RFLAGS_L,
    PRJ_VAR_NAME_LANG,        PRJ_VAR_NAME_PTH_C,     PRJ_VAR_NAME_PTH_L,    PRJ_VAR_NAME_STD_C,
    PRJ_VAR_NAME_STD_CPP,     PRJ_VAR_NAME_SRC_FILES, PRJ_VAR_NAME_LIBS,     PRJ_VAR_NAME_INCLUDE_PATHS,
    TRG_VAR_NAME_VER,         TRG_VAR_NAME_CFG,       TRG_VAR_NAME_TYPE,     TRG_VAR_NAME_TEMPLATES,
    TRG_VAR_NAME_DEPENDENCIES};

static bweas::logger _log{""};

// ??????
static void update_target_cfg_struct(const string &name_var, scope &current_scope) {
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
                    current_scope.get_var_value<sc::target>(tmp_str_prefix).prj.dflags_compiler =
                        current_scope.get_var_value<string>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_DFLAGS_L) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 2)
                        return;
                    current_scope.get_var_value<sc::target>(tmp_str_prefix).prj.dflags_linker =
                        current_scope.get_var_value<string>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_RFLAGS_C) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 2)
                        return;
                    current_scope.get_var_value<sc::target>(tmp_str_prefix).prj.rflags_compiler =
                        current_scope.get_var_value<string>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_RFLAGS_L) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 2)
                        return;
                    current_scope.get_var_value<sc::target>(tmp_str_prefix).prj.rflags_linker =
                        current_scope.get_var_value<string>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_PTH_C) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 2)
                        return;
                    current_scope.get_var_value<sc::target>(tmp_str_prefix).prj.path_compiler =
                        current_scope.get_var_value<string>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_PTH_L) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 2)
                        return;
                    current_scope.get_var_value<sc::target>(tmp_str_prefix).prj.path_linker =
                        current_scope.get_var_value<string>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_STD_C) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 1)
                        return;
                    current_scope.get_var_value<sc::target>(tmp_str_prefix).prj.standart_c =
                        current_scope.get_var_value<pdiff>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_STD_CPP) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 1)
                        return;
                    current_scope.get_var_value<sc::target>(tmp_str_prefix).prj.standart_cpp =
                        current_scope.get_var_value<pdiff>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_LANG) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 1)
                        return;
                    current_scope.get_var_value<sc::target>(tmp_str_prefix).prj.language =
                        current_scope.get_var_value<string>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_SRC_FILES) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 4)
                        return;
                    current_scope.get_var_value<sc::target>(tmp_str_prefix).prj.src_files =
                        current_scope.get_var_value<vec<string>>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_LIBS) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 4)
                        return;
                    current_scope.get_var_value<sc::target>(tmp_str_prefix).prj.libs =
                        current_scope.get_var_value<vec<string>>(name_var);
                }
                else if (tmp_str_postfix == PRJ_VAR_NAME_INCLUDE_PATHS) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 4)
                        return;
                    current_scope.get_var_value<sc::target>(tmp_str_prefix).prj.include_paths =
                        current_scope.get_var_value<vec<string>>(name_var);
                }
                else if (tmp_str_postfix == TRG_VAR_NAME_TEMPLATES) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 4)
                        return;
                    current_scope.get_var_value<sc::target>(tmp_str_prefix).templates =
                        current_scope.get_var_value<vec<string>>(name_var);
                }
                else if (tmp_str_postfix == TRG_VAR_NAME_DEPENDENCIES) {
                    if (current_scope.what_type(tmp_str_prefix) != 5 || current_scope.what_type(name_var) != 4)
                        return;
                    current_scope.get_var_value<sc::target>(tmp_str_prefix).dependencies =
                        current_scope.get_var_value<vec<string>>(name_var);
                }
                else if (tmp_str_postfix == TRG_VAR_NAME_VER) {
                    if (current_scope.what_type(tmp_str_prefix) != 6 || current_scope.what_type(name_var) != 2)
                        return;
                    current_scope.get_var_value<sc::target>(tmp_str_prefix).ver =
                        sc::version(current_scope.get_var_value<string>(name_var));
                }
                else if (tmp_str_postfix == TRG_VAR_NAME_CFG) {
                    if (current_scope.what_type(tmp_str_prefix) != 6 || current_scope.what_type(name_var) != 1)
                        return;
                    current_scope.get_var_value<sc::target>(tmp_str_prefix).cfg =
                        (sc::target_cfg)current_scope.get_var_value<pdiff>(name_var);
                }
                else if (tmp_str_postfix == TRG_VAR_NAME_TYPE) {
                    if (current_scope.what_type(tmp_str_prefix) != 6 || current_scope.what_type(name_var) != 1)
                        return;
                    current_scope.get_var_value<sc::target>(tmp_str_prefix).type =
                        (sc::target_type)current_scope.get_var_value<pdiff>(name_var);
                }
                else if (tmp_str_postfix == TRG_VAR_NAME_GENERATOR) {
                    if (current_scope.what_type(tmp_str_prefix) != 6 || current_scope.what_type(name_var) != 2)
                        return;
                    current_scope.get_var_value<sc::target>(tmp_str_prefix).name_generator =
                        current_scope.get_var_value<string>(name_var);
                }
                else {
                    if (current_scope.what_type(tmp_str_prefix) != 5 && current_scope.what_type(name_var) != 2)
                        return;
                    auto prj      = current_scope.get_var_value<sc::target>(tmp_str_prefix).prj;
                    auto it_field = prj.custom_ext_fields.find(tmp_str_postfix);
                    if (it_field != prj.custom_ext_fields.end())
                        it_field->second = current_scope.get_var_value<string>(name_var);
                }
                return;
            }
        }
    }
}

void sl_func::set(const expressions &expr_s, scope &current_scope) {
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
        else
            _log << bwtools::fatal
                 << (log_message(log_type::fatal) << "A trivial type was expected: " << expr_s[1].value);
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
        else
            _log << bwtools::fatal
                 << (log_message(log_type::fatal) << "A trivial type was expected: " << expr_s[1].value);
    }

    // ??????
    // if a variable is abstract ref to cfg structs
    update_target_cfg_struct(expr_s[0].value, current_scope);
}

void sl_func::file(const expressions &expr_s, scope &current_scope) {
    if (std::stoi(expr_s[1].value) == 0) {
        vec<string> path_files;
        for (size_t i = 2; i < expr_s.size(); ++i) {
            if (fs::exists(expr_s[i].value)) {
                path_files.push_back(fs::absolute(expr_s[i].value).string());
                continue;
            }
            string tmp_path = expr_s[i].value;
            string mask     = expr_s[i].value;
            size_t it       = tmp_path.find_last_of("/\\");
            if (it != tmp_path.npos) {
                tmp_path.erase(it);
                mask = fs::absolute(mask).string();
                if (!fs::is_directory(tmp_path))
                    _log << bwtools::fatal
                         << (log_message(log_type::fatal) << "Directory does not exist: " << tmp_path);
            }
            else {
                tmp_path = fs::current_path().string();
                mask     = tmp_path + "/" + mask;
            }
            vec<string> tmp_path_files, tmp_path_files_o;
            for (const auto &it_path : fs::directory_iterator(tmp_path))
                if (fs::is_regular_file(it_path.path()))
                    tmp_path_files.push_back(fs::absolute(it_path.path()).string());

            string mask_regex;

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

void sl_func::create_target(const expressions &expr_s, scope &current_scope) {
    sc::target &trg_ref  = current_scope.create_var<sc::target>(expr_s[0].value);
    sc::project &prj_ref = trg_ref.prj;

    trg_ref.name     = expr_s[0].value;
    trg_ref.type     = (sc::target_type)std::stoi(expr_s[1].value);
    prj_ref.language = "C++";

    for (size_t i = 2; i < expr_s.size(); ++i)
        trg_ref.prj.src_files.push_back(expr_s[i].value);

    // ??????? - I don't know if this helper function is needed
    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<pdiff>(DECL_VAR_STRUCT) > 0) {
        if (!current_scope.try_create_var<string>(trg_ref.name + TRG_VAR_NAME_VER, trg_ref.ver.get_str_version()))
            trg_ref.ver = sc::version(current_scope.get_var_value<string>(trg_ref.name + TRG_VAR_NAME_VER));
        if (!current_scope.try_create_var<pdiff>(trg_ref.name + TRG_VAR_NAME_CFG, (pdiff)trg_ref.cfg))
            trg_ref.cfg = (sc::target_cfg)current_scope.get_var_value<pdiff>(trg_ref.name + TRG_VAR_NAME_CFG);
        if (!current_scope.try_create_var<pdiff>(trg_ref.name + TRG_VAR_NAME_TYPE, (pdiff)trg_ref.type))
            trg_ref.type = (sc::target_type)current_scope.get_var_value<pdiff>(trg_ref.name + TRG_VAR_NAME_TYPE);
        if (!current_scope.try_create_var<string>(trg_ref.name + TRG_VAR_NAME_GENERATOR, trg_ref.name_generator))
            trg_ref.name_generator = current_scope.get_var_value<string>(trg_ref.name + TRG_VAR_NAME_GENERATOR);
        if (!current_scope.try_create_var<vec<string>>(trg_ref.name + TRG_VAR_NAME_TEMPLATES, trg_ref.templates))
            trg_ref.templates = current_scope.get_var_value<vec<string>>(trg_ref.name + TRG_VAR_NAME_TEMPLATES);
        if (!current_scope.try_create_var<vec<string>>(trg_ref.name + TRG_VAR_NAME_DEPENDENCIES, trg_ref.dependencies))
            trg_ref.dependencies = current_scope.get_var_value<vec<string>>(trg_ref.name + TRG_VAR_NAME_DEPENDENCIES);

        if (!current_scope.try_create_var<string>(trg_ref.name + PRJ_VAR_NAME_LANG, prj_ref.language))
            prj_ref.language = current_scope.get_var_value<string>(trg_ref.name + PRJ_VAR_NAME_LANG);
        if (!current_scope.try_create_var<string>(trg_ref.name + PRJ_VAR_NAME_DFLAGS_C, prj_ref.dflags_compiler))
            prj_ref.dflags_compiler = current_scope.get_var_value<string>(trg_ref.name + PRJ_VAR_NAME_DFLAGS_C);
        if (!current_scope.try_create_var<string>(trg_ref.name + PRJ_VAR_NAME_DFLAGS_L, prj_ref.dflags_linker))
            prj_ref.dflags_linker = current_scope.get_var_value<string>(trg_ref.name + PRJ_VAR_NAME_DFLAGS_L);
        if (!current_scope.try_create_var<string>(trg_ref.name + PRJ_VAR_NAME_RFLAGS_C, prj_ref.rflags_compiler))
            prj_ref.rflags_compiler = current_scope.get_var_value<string>(trg_ref.name + PRJ_VAR_NAME_RFLAGS_C);
        if (!current_scope.try_create_var<string>(trg_ref.name + PRJ_VAR_NAME_RFLAGS_L, prj_ref.rflags_linker))
            prj_ref.rflags_linker = current_scope.get_var_value<string>(trg_ref.name + PRJ_VAR_NAME_RFLAGS_L);
        if (!current_scope.try_create_var<string>(trg_ref.name + PRJ_VAR_NAME_PTH_C, prj_ref.path_compiler))
            prj_ref.path_compiler = current_scope.get_var_value<string>(trg_ref.name + PRJ_VAR_NAME_PTH_C);
        if (!current_scope.try_create_var<string>(trg_ref.name + PRJ_VAR_NAME_PTH_L, prj_ref.path_linker))
            prj_ref.path_linker = current_scope.get_var_value<string>(trg_ref.name + PRJ_VAR_NAME_PTH_L);
        if (!current_scope.try_create_var<pdiff>(trg_ref.name + PRJ_VAR_NAME_STD_C, prj_ref.standart_c))
            prj_ref.standart_c = current_scope.get_var_value<pdiff>(trg_ref.name + PRJ_VAR_NAME_STD_C);
        if (!current_scope.try_create_var<pdiff>(trg_ref.name + PRJ_VAR_NAME_STD_CPP, prj_ref.standart_cpp))
            prj_ref.standart_cpp = current_scope.get_var_value<pdiff>(trg_ref.name + PRJ_VAR_NAME_STD_CPP);
        if (!current_scope.try_create_var<vec<string>>(trg_ref.name + PRJ_VAR_NAME_SRC_FILES, prj_ref.src_files))
            prj_ref.src_files = current_scope.get_var_value<vec<string>>(trg_ref.name + PRJ_VAR_NAME_SRC_FILES);
        if (!current_scope.try_create_var<vec<string>>(trg_ref.name + PRJ_VAR_NAME_LIBS, prj_ref.libs))
            prj_ref.libs = current_scope.get_var_value<vec<string>>(trg_ref.name + PRJ_VAR_NAME_LIBS);
        if (!current_scope.try_create_var<vec<string>>(trg_ref.name + PRJ_VAR_NAME_INCLUDE_PATHS,
                                                       prj_ref.include_paths))
            prj_ref.include_paths = current_scope.get_var_value<vec<string>>(trg_ref.name + PRJ_VAR_NAME_INCLUDE_PATHS);
    }
}

void sl_func::add_dependencies_target(const expressions &expr_s, scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 5)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Expected var target: " << expr_s[0].value);

    sc::target &trg_ref = current_scope.get_var_value<sc::target>(expr_s[0].value);

    for (size_t i = 1; i < expr_s.size(); ++i) {
        if (current_scope.what_type(expr_s[i].value) != 5)
            _log << bwtools::fatal
                 << (log_message(log_type::fatal) << "There is no such target object: " << expr_s[i].value);
        trg_ref.dependencies.push_back(expr_s[i].value);
    }

    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<pdiff>(DECL_VAR_STRUCT) > 0) {
        if (current_scope.what_type(trg_ref.name + TRG_VAR_NAME_DEPENDENCIES) == 4) {
            current_scope.get_var_value<vec<string>>(trg_ref.name + TRG_VAR_NAME_DEPENDENCIES) = trg_ref.dependencies;
        }
        else
            current_scope.create_var<vec<string>>(trg_ref.name + TRG_VAR_NAME_DEPENDENCIES, trg_ref.dependencies);
    }
}
void sl_func::exp_data(const expressions &expr_s, scope &current_scope) {
    interpreter tmp_interpreter(expr_s[0].value);
    tmp_interpreter.set_scope(&current_scope);
    tmp_interpreter.interpret();
}
void sl_func::debug(const expressions &expr_s, scope &current_scope) {
    string str_out;
    for (size_t i = 0; i < expr_s.size(); ++i)
        str_out += expr_s[i].value + " ";
    _log << bwtools::message << (log_message(log_type::msg) << str_out);
}

void sl_func::debug_struct(const expressions &expr_s, scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 5)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Expected var target: " << expr_s[0].value);

    string str_out;
    sc::target &trg_ref  = current_scope.get_var_value<sc::target>(expr_s[0].value);
    sc::project &prj_ref = trg_ref.prj;

    str_out = "Name Target: " + trg_ref.name + "\nVersion: " + trg_ref.ver.get_str_version() +
              "\nType Build: " + target_type_str(trg_ref.type) + "\nConfiguration: " + target_cfg_str(trg_ref.cfg) +
              "\nDependencies: \n";
    for (size_t i = 0; i < trg_ref.dependencies.size(); ++i) {
        str_out += trg_ref.dependencies[i];
        if (i != trg_ref.dependencies.size() - 1)
            str_out += ", ";
    }

    str_out += "\nTemplates: \n";
    for (size_t i = 0; i < trg_ref.templates.size(); ++i) {
        str_out += trg_ref.templates[i];
        if (i != trg_ref.templates.size() - 1)
            str_out += ", ";
    }

    str_out += "\nLang: " + prj_ref.language + "\nCompiler: " + prj_ref.path_compiler +
               "\nLinker: " + prj_ref.path_linker + "\nDebug Flags Compiler: " + prj_ref.dflags_compiler +
               "\nRelease Flags Compiler: " + prj_ref.rflags_compiler +
               "\nDebug Flags Linker: " + prj_ref.dflags_linker + "\nRelease Flags Compiler: " + prj_ref.rflags_linker +
               "\nStandart C: " + std::to_string(prj_ref.standart_c) +
               "\nStandart C++: " + std::to_string(prj_ref.standart_cpp) + "\n------\nSRC_FILES: \n";
    for (size_t i = 0; i < prj_ref.src_files.size(); ++i) {
        str_out += prj_ref.src_files[i];
        if (i != prj_ref.src_files.size() - 1)
            str_out += ", ";
    }
    str_out += "\n\nInclude paths: \n";
    for (size_t i = 0; i < prj_ref.include_paths.size(); ++i) {
        str_out += prj_ref.include_paths[i];
        if (i != prj_ref.include_paths.size() - 1)
            str_out += ", ";
    }
    _log << bwtools::message << (log_message(log_type::msg) << str_out);
}

void sl_func::flags_compiler(const expressions &expr_s, scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 5)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Expected var target: " << expr_s[0].value);

    sc::target &trg_ref  = current_scope.get_var_value<sc::target>(expr_s[0].value);
    sc::project &prj_ref = trg_ref.prj;
    sc::target_cfg tmp_cfg;

    if ((tmp_cfg = (sc::target_cfg)std::stoi(expr_s[1].value)) == sc::target_cfg::RELEASE)
        for (size_t i = 2; i < expr_s.size(); ++i) {
            prj_ref.rflags_compiler += expr_s[i].value + " ";
        }
    else
        for (size_t i = 2; i < expr_s.size(); ++i) {
            prj_ref.dflags_compiler += expr_s[i].value + " ";
        }

    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<pdiff>(DECL_VAR_STRUCT) > 0) {
        if (tmp_cfg == sc::target_cfg::DEBUG) {
            if (current_scope.what_type(trg_ref.name + PRJ_VAR_NAME_DFLAGS_C) == 2) {
                current_scope.get_var_value<string>(trg_ref.name + PRJ_VAR_NAME_DFLAGS_C) = prj_ref.dflags_compiler;
            }
            else
                current_scope.create_var<string>(trg_ref.name + PRJ_VAR_NAME_DFLAGS_C, prj_ref.dflags_compiler);
        }
        else if (tmp_cfg == sc::target_cfg::RELEASE) {
            if (current_scope.what_type(trg_ref.name + PRJ_VAR_NAME_RFLAGS_C) == 2) {
                current_scope.get_var_value<string>(trg_ref.name + PRJ_VAR_NAME_RFLAGS_C) = prj_ref.rflags_compiler;
            }
            else
                current_scope.create_var<string>(trg_ref.name + PRJ_VAR_NAME_RFLAGS_C, prj_ref.rflags_compiler);
        }
    }
}
void sl_func::flags_linker(const expressions &expr_s, scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 5)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Expected var target: " << expr_s[0].value);

    sc::target &trg_ref  = current_scope.get_var_value<sc::target>(expr_s[0].value);
    sc::project &prj_ref = trg_ref.prj;
    sc::target_cfg tmp_cfg;

    if ((tmp_cfg = (sc::target_cfg)std::stoi(expr_s[1].value)) == sc::target_cfg::RELEASE)
        for (size_t i = 2; i < expr_s.size(); ++i) {
            prj_ref.rflags_linker += expr_s[i].value + " ";
        }
    else
        for (size_t i = 2; i < expr_s.size(); ++i) {
            prj_ref.dflags_linker += expr_s[i].value + " ";
        }

    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<pdiff>(DECL_VAR_STRUCT) > 0) {
        if (tmp_cfg == sc::target_cfg::DEBUG) {
            if (current_scope.what_type(trg_ref.name + PRJ_VAR_NAME_DFLAGS_L) == 2)
                current_scope.get_var_value<string>(trg_ref.name + PRJ_VAR_NAME_DFLAGS_L) = prj_ref.dflags_linker;
            else
                current_scope.create_var<string>(trg_ref.name + PRJ_VAR_NAME_DFLAGS_L, prj_ref.dflags_linker);
        }
        else if (tmp_cfg == sc::target_cfg::RELEASE) {
            if (current_scope.what_type(trg_ref.name + PRJ_VAR_NAME_RFLAGS_L) == 2)
                current_scope.get_var_value<string>(trg_ref.name + PRJ_VAR_NAME_RFLAGS_L) = prj_ref.rflags_linker;
            else
                current_scope.create_var<string>(trg_ref.name + PRJ_VAR_NAME_RFLAGS_L, prj_ref.dflags_linker);
        }
    }
}
void sl_func::path_compiler(const expressions &expr_s, scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 5)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Expected var target: " << expr_s[0].value);

    sc::target &trg_ref   = current_scope.get_var_value<sc::target>(expr_s[0].value);
    sc::project &prj_ref  = trg_ref.prj;
    prj_ref.path_compiler = expr_s[1].value;

    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<pdiff>(DECL_VAR_STRUCT) > 0) {
        if (current_scope.what_type(trg_ref.name + PRJ_VAR_NAME_PTH_C) == 2)
            current_scope.get_var_value<string>(trg_ref.name + PRJ_VAR_NAME_PTH_C) = prj_ref.path_compiler;
        else
            current_scope.create_var<string>(trg_ref.name + PRJ_VAR_NAME_PTH_C, prj_ref.path_compiler);
    }
}
void sl_func::path_linker(const expressions &expr_s, scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 5)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Expected var target: " << expr_s[0].value);

    sc::target &trg_ref  = current_scope.get_var_value<sc::target>(expr_s[0].value);
    sc::project &prj_ref = trg_ref.prj;
    prj_ref.path_linker  = expr_s[1].value;

    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<pdiff>(DECL_VAR_STRUCT) > 0) {
        if (current_scope.what_type(trg_ref.name + PRJ_VAR_NAME_PTH_C) == 2)
            current_scope.get_var_value<string>(trg_ref.name + PRJ_VAR_NAME_PTH_C) = prj_ref.path_linker;
        else
            current_scope.create_var<string>(trg_ref.name + PRJ_VAR_NAME_PTH_C, prj_ref.path_linker);
    }
}
void sl_func::standart_c(const expressions &expr_s, scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 5)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Expected var target: " << expr_s[0].value);

    sc::target &trg_ref  = current_scope.get_var_value<sc::target>(expr_s[0].value);
    sc::project &prj_ref = trg_ref.prj;
    prj_ref.standart_c   = std::stoi(expr_s[1].value);

    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<pdiff>(DECL_VAR_STRUCT) > 0) {
        if (current_scope.what_type(trg_ref.name + PRJ_VAR_NAME_STD_C) == 1)
            current_scope.get_var_value<pdiff>(trg_ref.name + PRJ_VAR_NAME_STD_C) = prj_ref.standart_c;
        else
            current_scope.create_var<pdiff>(trg_ref.name + PRJ_VAR_NAME_STD_C, prj_ref.standart_c);
    }
}
void sl_func::standart_cpp(const expressions &expr_s, scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 5)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Expected var target: " << expr_s[0].value);

    sc::target &trg_ref  = current_scope.get_var_value<sc::target>(expr_s[0].value);
    sc::project &prj_ref = trg_ref.prj;
    prj_ref.standart_cpp = std::stoi(expr_s[1].value);

    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<pdiff>(DECL_VAR_STRUCT) > 0) {
        if (current_scope.what_type(trg_ref.name + PRJ_VAR_NAME_STD_CPP) == 1)
            current_scope.get_var_value<pdiff>(trg_ref.name + PRJ_VAR_NAME_STD_CPP) = prj_ref.standart_c;
        else
            current_scope.create_var<pdiff>(trg_ref.name + PRJ_VAR_NAME_STD_CPP, prj_ref.standart_c);
    }
}
void sl_func::include_directories(const expressions &expr_s, scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 5)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Expected var target: " << expr_s[0].value);

    sc::target &trg_ref  = current_scope.get_var_value<sc::target>(expr_s[0].value);
    sc::project &prj_ref = trg_ref.prj;
    for (size_t i = 1; i < expr_s.size(); ++i)
        prj_ref.include_paths.push_back(fs::absolute(expr_s[i].value).c_str());

    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<pdiff>(DECL_VAR_STRUCT) > 0) {
        if (current_scope.what_type(trg_ref.name + PRJ_VAR_NAME_INCLUDE_PATHS) == 4)
            current_scope.get_var_value<vec<string>>(trg_ref.name + PRJ_VAR_NAME_INCLUDE_PATHS) = prj_ref.include_paths;
        else
            current_scope.create_var<vec<string>>(trg_ref.name + PRJ_VAR_NAME_INCLUDE_PATHS, prj_ref.include_paths);
    }
}
void sl_func::lang(const expressions &expr_s, scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 5)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Expected var target: " << expr_s[0].value);

    sc::target &trg_ref  = current_scope.get_var_value<sc::target>(expr_s[0].value);
    sc::project &prj_ref = trg_ref.prj;
    prj_ref.language     = expr_s[1].value;

    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<pdiff>(DECL_VAR_STRUCT) > 0) {
        if (current_scope.what_type(trg_ref.name + PRJ_VAR_NAME_LANG) == 1)
            current_scope.get_var_value<string>(trg_ref.name + PRJ_VAR_NAME_LANG) = prj_ref.language;
        else
            current_scope.create_var<string>(trg_ref.name + PRJ_VAR_NAME_LANG, prj_ref.language);
    }
}

void sl_func::generator(const expressions &expr_s, scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 5)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Expected var target: " << expr_s[0].value);

    sc::target &trg_ref    = current_scope.get_var_value<sc::target>(expr_s[0].value);
    trg_ref.name_generator = expr_s[1].value;
}

void sl_func::add_param_template(const expressions &expr_s, scope &current_scope) {
    if (current_scope.what_type(expr_s[1].value) != 2)
        _log << bwtools::fatal
             << (log_message(log_type::fatal)
                 << "Global template arguments must be of type string: " << expr_s[0].value);
    current_scope.create_var<pair<string, string>>(
        expr_s[0].value, {expr_s[0].value, current_scope.get_var_value<string>(expr_s[1].value)});
}

void sl_func::create_template(const expressions &expr_s, scope &current_scope) {
    try {
        sc::template_command tcmd_tmp = sc::template_command::create_template_command(expr_s[0].value, expr_s[1].value);
        if (current_scope.what_type(tcmd_tmp.name_call_component) != 7) {
            if (tcmd_tmp.name_call_component.find(":") != tcmd_tmp.name_call_component.npos) {
                string program = tcmd_tmp.name_call_component, pattern_file = tcmd_tmp.name_call_component;
                program.erase(program.find(":"));
                pattern_file.erase(0, pattern_file.find(":") + 1);

                tcmd_tmp.name_call_component = expr_s[0].value + "_anon_cc";
                current_scope.create_var<sc::call_component>(
                    tcmd_tmp.name_call_component,
                    sc::call_component{tcmd_tmp.name_call_component, program, pattern_file});
            }
            else
                _log << bwtools::fatal
                     << (log_message(log_type::fatal) << "Undefined call component: " << tcmd_tmp.name_call_component);
        }
        current_scope.create_var<sc::template_command>(expr_s[0].value, tcmd_tmp);
    }
    catch (std::runtime_error &excp) {
        _log << bwtools::fatal << (log_message(log_type::fatal) << excp.what());
    }
}

void sl_func::create_call_component(const expressions &expr_s, scope &current_scope) {
    sc::call_component ccmp_tmp;
    ccmp_tmp.name              = expr_s[0].value;
    ccmp_tmp.name_program      = expr_s[1].value;
    ccmp_tmp.pattern_ret_files = expr_s[2].value;

    current_scope.create_var<sc::call_component>(expr_s[0].value, ccmp_tmp);
}

void sl_func::use_templates(const expressions &expr_s, scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 5)
        _log << bwtools::fatal << (log_message(log_type::fatal) << "Expected var target: " << expr_s[0].value);

    sc::target &trg_ref = current_scope.get_var_value<sc::target>(expr_s[0].value);

    for (size_t i = 1; i < expr_s.size(); ++i) {
        if (current_scope.what_type(expr_s[i].value) != 6)
            _log << bwtools::fatal
                 << (log_message(log_type::fatal) << "This template does not exist: " << expr_s[i].value);

        trg_ref.templates.push_back(expr_s[i].value);
    }

    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<pdiff>(DECL_VAR_STRUCT) > 0) {
        if (current_scope.what_type(trg_ref.name + TRG_VAR_NAME_TEMPLATES) == 4)
            current_scope.get_var_value<vec<string>>(trg_ref.name + TRG_VAR_NAME_TEMPLATES) = trg_ref.templates;
        else
            current_scope.create_var<vec<string>>(trg_ref.name + TRG_VAR_NAME_TEMPLATES, trg_ref.templates);
    }
}
