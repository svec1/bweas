//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses />
// ------------------------------------------
//

#include <lang/static_linking_func.hpp>

#include <bwlogger.hpp>
#include <lang/interpreter.hpp>
#include <utils/file_utils.hpp>

#include <algorithm>

using namespace bweas;
using namespace bweas::utils;

static const array<string, 18> vec_name_config_var = {
    PRJ_VAR_NAME_DFLAGS_C,  PRJ_VAR_NAME_DFLAGS_L, PRJ_VAR_NAME_RFLAGS_C,      PRJ_VAR_NAME_RFLAGS_L,
    PRJ_VAR_NAME_PTH_C,     PRJ_VAR_NAME_PTH_L,    PRJ_VAR_NAME_STD_C,         PRJ_VAR_NAME_STD_CPP,
    PRJ_VAR_NAME_SRC_FILES, PRJ_VAR_NAME_LIBS,     PRJ_VAR_NAME_INCLUDE_PATHS, TRG_VAR_NAME_VER,
    TRG_VAR_NAME_CFG,       TRG_VAR_NAME_TYPE,     TRG_VAR_NAME_TEMPLATES,     TRG_VAR_NAME_DEPENDENCIES};

static vec<string> name_created_targets;

static bweas::logger _log{""};

// ??????
static void update_target_cfg_struct(const string &name_var, scope &current_scope) {
    if (current_scope.what_type(DECL_VAR_STRUCT) != 1 || !current_scope.get_var_value<pdiff>(DECL_VAR_STRUCT) ||
        name_var == DECL_VAR_STRUCT)
        return;

    size_t tmp_it = 0;
    for (const auto &it : vec_name_config_var) {
        if ((tmp_it = name_var.find(it)) != name_var.npos) {
            if (name_var.size() - it.size() != tmp_it)
                return;

            string tmp_str_postfix = name_var;
            string tmp_str_prefix  = name_var;
            tmp_str_postfix.erase(0, tmp_it);
            tmp_str_prefix.erase(tmp_it, tmp_str_prefix.size());

            if (current_scope.what_type(tmp_str_prefix) != 5)
                return;

            sc::target &trg = current_scope.get_var_value<sc::target>(tmp_str_prefix);

            if (tmp_str_postfix == TRG_VAR_NAME_TEMPLATES && current_scope.what_type(name_var) == 4)
                trg.templates = current_scope.get_var_value<vec<string>>(name_var);
            else if (tmp_str_postfix == TRG_VAR_NAME_DEPENDENCIES && current_scope.what_type(name_var) == 4)
                trg.dependencies = current_scope.get_var_value<vec<string>>(name_var);
            else if (tmp_str_postfix == TRG_VAR_NAME_VER && current_scope.what_type(name_var) == 2)
                trg.ver = sc::version(current_scope.get_var_value<string>(name_var));
            else if (tmp_str_postfix == TRG_VAR_NAME_TYPE && current_scope.what_type(name_var) == 1)
                trg.type = (sc::target::e_type)current_scope.get_var_value<pdiff>(name_var);
            else if (tmp_str_postfix == TRG_VAR_NAME_CFG && current_scope.what_type(name_var) == 1)
                trg.cfg = (sc::target::e_cfg)current_scope.get_var_value<pdiff>(name_var);

            return;
        }
        else {
            auto suitable_longest = [&](const string &str1, const string &str2) {
                if ((!name_var.find(str1) && !name_var.find(str2)) || (name_var.find(str1) && name_var.find(str2)))
                    return str1.size() < str2.size();
                return !name_var.find(str1) && name_var.find(str2) ? false : true;
            };

            string target;
            if (auto it = std::max_element(name_created_targets.begin(), name_created_targets.end(), suitable_longest);
                it != name_created_targets.end())
                target = *it;
            else
                return;

            string tmp_str_postfix = name_var;
            tmp_str_postfix.erase(0, target.size() + 1);

            if (current_scope.what_type(target) != 5)
                return;

            sc::profile::fields &ext_fields = current_scope.get_var_value<sc::target>(target).ext.get_fields();

            for (auto &[key, value] : ext_fields)
                if (std::holds_alternative<string>(value)) {
                    if (key == tmp_str_postfix && current_scope.what_type(name_var) == 2)
                        value = current_scope.get_var_value<string>(name_var);
                }
                else if (key == tmp_str_postfix && current_scope.what_type(name_var) == 4)
                    value = current_scope.get_var_value<vec<string>>(name_var);

            return;
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
            _log << (log_message(log_type::fatal) << "A trivial type was expected: " << expr_s[1].value);
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
            _log << (log_message(log_type::fatal) << "A trivial type was expected: " << expr_s[1].value);
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
                    _log << (log_message(log_type::fatal) << "Directory does not exist: " << tmp_path);
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

            tmp_path_files_o = file_utils::file_slc_mask(mask, tmp_path_files);
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
    if (current_scope.what_type(expr_s[1].value) != 9)
        _log << (log_message(log_type::fatal) << "Expected var profile: " << expr_s[1].value);

    sc::target &trg_ref = current_scope.create_var<sc::target>(expr_s[0].value);

    trg_ref.name = expr_s[0].value;
    trg_ref.ext  = current_scope.get_var_value<sc::profile>(expr_s[1].value);
    trg_ref.cfg  = sc::to_target_cfg(expr_s[2].value);
    trg_ref.type = sc::to_target_type(trg_ref.fields<string>("target_type"));

    trg_ref.ext.set_fields((size_t)trg_ref.cfg);

    for (size_t i = 3; i < expr_s.size(); ++i)
        trg_ref.fields<vec<string>>("source_files").push_back(expr_s[i].value);

    // ??????? - I don't know if this helper function is needed
    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<pdiff>(DECL_VAR_STRUCT) > 0) {
        if (!current_scope.try_create_var<string>(trg_ref.name + TRG_VAR_NAME_VER, trg_ref.ver.get_str_version()))
            trg_ref.ver = sc::version(current_scope.get_var_value<string>(trg_ref.name + TRG_VAR_NAME_VER));
        if (!current_scope.try_create_var<pdiff>(trg_ref.name + TRG_VAR_NAME_CFG, (pdiff)trg_ref.cfg))
            trg_ref.cfg = (sc::target::e_cfg)current_scope.get_var_value<pdiff>(trg_ref.name + TRG_VAR_NAME_CFG);
        if (!current_scope.try_create_var<vec<string>>(trg_ref.name + TRG_VAR_NAME_TEMPLATES, trg_ref.templates))
            trg_ref.templates = current_scope.get_var_value<vec<string>>(trg_ref.name + TRG_VAR_NAME_TEMPLATES);
        if (!current_scope.try_create_var<vec<string>>(trg_ref.name + TRG_VAR_NAME_DEPENDENCIES, trg_ref.dependencies))
            trg_ref.dependencies = current_scope.get_var_value<vec<string>>(trg_ref.name + TRG_VAR_NAME_DEPENDENCIES);
        if (!current_scope.try_create_var<pdiff>(trg_ref.name + TRG_VAR_NAME_TYPE, (pdiff)trg_ref.type))
            trg_ref.type = (sc::target::e_type)current_scope.get_var_value<pdiff>(trg_ref.name + TRG_VAR_NAME_TYPE);

        sc::profile::fields &ext_fields = trg_ref.ext.get_fields();

        for (auto &[key, value] : ext_fields)
            if (std::holds_alternative<string>(value)) {
                if (!current_scope.try_create_var<string>(trg_ref.name + "_" + key, std::get<string>(value)))
                    value = current_scope.get_var_value<string>(trg_ref.name + "_" + key);
            }
            else if (!current_scope.try_create_var<vec<string>>(trg_ref.name + "_" + key, std::get<vec<string>>(value)))
                value = current_scope.get_var_value<vec<string>>(trg_ref.name + "_" + key);
    }

    name_created_targets.push_back(trg_ref.name);
}

void sl_func::add_dependencies_target(const expressions &expr_s, scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 5)
        _log << (log_message(log_type::fatal) << "Expected var target: " << expr_s[0].value);

    sc::target &trg_ref = current_scope.get_var_value<sc::target>(expr_s[0].value);

    for (size_t i = 1; i < expr_s.size(); ++i) {
        if (current_scope.what_type(expr_s[i].value) != 5)
            _log << (log_message(log_type::fatal) << "There is no such target object: " << expr_s[i].value);
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
    _log << (log_message(log_type::msg) << str_out);
}

void sl_func::debug_struct(const expressions &expr_s, scope &current_scope) {
    if (current_scope.what_type(expr_s[0].value) != 5)
        _log << (log_message(log_type::fatal) << "Expected var target: " << expr_s[0].value);

    string str_out;
    sc::target &trg_ref             = current_scope.get_var_value<sc::target>(expr_s[0].value);
    sc::profile::fields &ext_fields = trg_ref.ext.get_fields();

    str_out = "Name Target: " + trg_ref.name + "\nVersion: " + trg_ref.ver.get_str_version() +
              "\nType Build: " + target_type_str(trg_ref.type) + "\nConfiguration: " + target_cfg_str(trg_ref.cfg);

    str_out += "\nDependencies: \n" + std::format("{}", trg_ref.dependencies);
    str_out += "\nTemplates: \n" + std::format("{}", trg_ref.templates);

    str_out += "\n\nExtension fields: \n";
    for (const auto &[key, value] : ext_fields)
        if (std::holds_alternative<string>(value))
            str_out += std::format("{}: {}\n", key, std::get<string>(value));
        else
            str_out += std::format("{}: {}\n", key, std::get<vec<string>>(value));

    _log << (log_message(log_type::msg) << str_out);
}

void sl_func::add_param_template(const expressions &expr_s, scope &current_scope) {
    if (current_scope.what_type(expr_s[1].value) != 2)
        _log << (log_message(log_type::fatal)
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
                _log << (log_message(log_type::fatal) << "Undefined call component: " << tcmd_tmp.name_call_component);
        }
        current_scope.create_var<sc::template_command>(expr_s[0].value, tcmd_tmp);
    }
    catch (std::runtime_error &excp) {
        _log << (log_message(log_type::fatal) << excp.what());
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
        _log << (log_message(log_type::fatal) << "Expected var target: " << expr_s[0].value);

    sc::target &trg_ref = current_scope.get_var_value<sc::target>(expr_s[0].value);

    for (size_t i = 1; i < expr_s.size(); ++i) {
        if (current_scope.what_type(expr_s[i].value) != 6)
            _log << (log_message(log_type::fatal) << "This template does not exist: " << expr_s[i].value);

        trg_ref.templates.push_back(expr_s[i].value);
    }

    if (current_scope.what_type(DECL_VAR_STRUCT) == 1 && current_scope.get_var_value<pdiff>(DECL_VAR_STRUCT) > 0) {
        if (current_scope.what_type(trg_ref.name + TRG_VAR_NAME_TEMPLATES) == 4)
            current_scope.get_var_value<vec<string>>(trg_ref.name + TRG_VAR_NAME_TEMPLATES) = trg_ref.templates;
        else
            current_scope.create_var<vec<string>>(trg_ref.name + TRG_VAR_NAME_TEMPLATES, trg_ref.templates);
    }
}
