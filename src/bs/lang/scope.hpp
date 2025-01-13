#ifndef _SCOPE__H
#define _SCOPE__H

#include "../../kernel/high_level/bwtype.h"

#include "../tools/bwexception.hpp"
#include "../tools/call_cmd.hpp"
#include "static_struct.hpp"
#include "var.hpp"

#include <string>
#include <vector>

namespace var {

inline std::string type_var_to_str(u32t ind) {
    if (ind == 1)
        return "int";
    else if (ind == 2)
        return "string";
    else if (ind == 3)
        return "vector<int>";
    else if (ind == 4)
        return "vector<string>";
    else if (ind == 5)
        return "project";
    else if (ind == 6)
        return "target";
    else if (ind == 7)
        return "template command";
    else if (ind == 8)
        return "call component";
    else if (ind == 9)
        return "global external args";
    return "undef";
}

class scope_excp : public ::bwexception::bweas_exception {
  public:
    scope_excp(std::string _what_hp, std::string number_err) : what_hp(_what_hp), bweas_exception("SCOP" + number_err) {
    }
    ~scope_excp() noexcept override final = default;

  public:
    const char *what() const noexcept override final {
        return what_hp.c_str();
    }

  private:
    std::string what_hp;
};

class scope {
  public:
    inline scope();

    inline scope(const scope &) = default;
    inline scope(scope &&) = default;

    ~scope() = default;

  public:
    template <typename T> inline T &create_var(std::string name_var, T val = T{});
    template <typename T> inline bool try_create_var(std::string name_var, T val = T{});

    template <typename T> inline void delete_var(std::string name_var);

    template <typename T> inline T &get_var_value(std::string name_var);
    template <typename T> inline std::vector<std::pair<std::string, T>> &get_vector_variables_t();

    inline bool is_exist(std::string name_var);

    inline void clear();

    // 1 - int
    // 2 - string
    // 3 - vector<int>
    // 4 - vector<string>
    // 5 - project
    // 6 - target
    // 7 - template command
    // 8 - call component
    // 9 - global external args
    // 0 - undefined
    inline u32t what_type(std::string name_var);

  public:
    tools::call_cmd_manager call_cmd;

  private:
    static inline bool init_glob{0};

    var::datatype_var<i32t> int_v;
    var::datatype_var<std::string> str_v;
    var::datatype_var<struct_sb::project> prj_v;
    var::datatype_var<struct_sb::target> trg_v;
    var::datatype_var<std::vector<i32t>> vec_int_v;
    var::datatype_var<std::vector<std::string>> vec_str_v;

    var::datatype_var<struct_sb::template_command> tcmd_v;
    var::datatype_var<struct_sb::call_component> ccmp_v;
    var::datatype_var<std::pair<std::string, std::string>> global_ext_args_v;
};

inline scope::scope() {
    if (!init_glob) {
        init_glob = 1;
        assist.add_err("SCOP000", "The variable does not exist");
        assist.add_err("SCOP001", "The variable has already been created(does exist)");
        assist.add_err("SCOP002", "Unknown data type");
    }
}

template <typename T> inline T &scope::create_var(std::string name_var, T val) {
    if constexpr (std::is_same_v<T, i32t>) {
        if (int_v.create_var(name_var, val))
            throw scope_excp("int(" + std::to_string(val) + ") <- " + name_var, "001");
        return int_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, std::string>) {
        if (str_v.create_var(name_var, val))
            throw scope_excp("string(" + val + ") <- " + name_var, "001");
        return str_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, struct_sb::project>) {
        if (prj_v.create_var(name_var, val))
            throw scope_excp("project(" + val.name_project + ") <- " + name_var, "001");
        return prj_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, struct_sb::target>) {
        if (trg_v.create_var(name_var, val))
            throw scope_excp("target(" + val.name_target + ") <- " + name_var, "001");
        return trg_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, std::vector<i32t>>) {
        if (vec_int_v.create_var(name_var, val))
            throw scope_excp("vector<int>(" + std::to_string(val[0]) + ", ..." + ") <- " + name_var, "001");
        return vec_int_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
        if (vec_str_v.create_var(name_var, val))
            throw scope_excp("vector<string>(" + val[0] + ", ..." + ") <- " + name_var, "001");
        return vec_str_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, struct_sb::template_command>) {
        if (tcmd_v.create_var(name_var, val))
            throw scope_excp("template_command(" + val.name + ", ..." + ") <- " + name_var, "001");
        return tcmd_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, struct_sb::call_component>) {
        if (ccmp_v.create_var(name_var, val))
            throw scope_excp("call_component(" + val.name + ", ..." + ") <- " + name_var, "001");
        return ccmp_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, std::pair<std::string, std::string>>) {
        if (global_ext_args_v.create_var(name_var, val))
            throw scope_excp("global_external_args(" + val.first + ", ..." + ") <- " + name_var, "001");
        return global_ext_args_v.get_val_ref(name_var);
    }
}

template <typename T> inline bool scope::try_create_var(std::string name_var, T val) {
    bool creates = 0;
    if constexpr (std::is_same_v<T, i32t>)
        creates = int_v.create_var(name_var, val);
    else if constexpr (std::is_same_v<T, std::string>)
        creates = str_v.create_var(name_var, val);
    else if constexpr (std::is_same_v<T, struct_sb::project>)
        creates = prj_v.create_var(name_var, val);
    else if constexpr (std::is_same_v<T, struct_sb::target>)
        creates = trg_v.create_var(name_var, val);
    else if constexpr (std::is_same_v<T, std::vector<i32t>>)
        creates = vec_int_v.create_var(name_var, val);
    else if constexpr (std::is_same_v<T, std::vector<std::string>>)
        creates = vec_str_v.create_var(name_var, val);
    else if constexpr (std::is_same_v<T, struct_sb::template_command>)
        creates = tcmd_v.create_var(name_var, val);
    else if constexpr (std::is_same_v<T, struct_sb::call_component>)
        creates = ccmp_v.create_var(name_var, val);
    else if constexpr (std::is_same_v<T, std::pair<std::string, std::string>>)
        creates = global_ext_args_v.create_var(name_var, val);

    return !creates;
}

template <typename T> inline void scope::delete_var(std::string name_var) {
    if constexpr (std::is_same_v<T, i32t>) {
        if (int_v.delete_var(name_var))
            throw scope_excp("int() <- " + name_var, "000");
    }
    else if constexpr (std::is_same_v<T, std::string>) {
        if (str_v.delete_var(name_var))
            throw scope_excp("string() <- " + name_var, "000");
    }
    else if constexpr (std::is_same_v<T, struct_sb::project>) {
        if (prj_v.delete_var(name_var))
            throw scope_excp("project() <- " + name_var, "000");
    }
    else if constexpr (std::is_same_v<T, struct_sb::target>) {
        if (trg_v.delete_var(name_var))
            throw scope_excp("target() <- " + name_var, "000");
    }
    else if constexpr (std::is_same_v<T, std::vector<i32t>>) {
        if (vec_int_v.delete_var(name_var))
            throw scope_excp("vector<int>() <- " + name_var, "000");
    }
    else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
        if (vec_str_v.delete_var(name_var))
            throw scope_excp("vector<string>() <- " + name_var, "000");
    }
    else if constexpr (std::is_same_v<T, struct_sb::template_command>) {
        if (tcmd_v.delete_var(name_var))
            throw scope_excp("template_command() <- " + name_var, "000");
    }
    else if constexpr (std::is_same_v<T, struct_sb::call_component>) {
        if (ccmp_v.delete_var(name_var))
            throw scope_excp("call_component() <- " + name_var, "000");
    }
    else if constexpr (std::is_same_v<T, std::pair<std::string, std::string>>) {
        if (global_ext_args_v.delete_var(name_var))
            throw scope_excp("global_external_args() <- " + name_var, "000");
    }
}

template <typename T> inline T &scope::get_var_value(std::string name_var) {
    if constexpr (std::is_same_v<T, i32t>) {
        if (!int_v.is_exist_var(name_var))
            throw scope_excp("int() <- " + name_var, "000");
        return int_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, std::string>) {
        if (!str_v.is_exist_var(name_var))
            throw scope_excp("string() <- " + name_var, "000");
        return str_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, struct_sb::project>) {
        if (!prj_v.is_exist_var(name_var))
            throw scope_excp("project() <- " + name_var, "000");
        return prj_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, struct_sb::target>) {
        if (!trg_v.is_exist_var(name_var))
            throw scope_excp("target() <- " + name_var, "000");
        return trg_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, std::vector<i32t>>) {
        if (!vec_int_v.is_exist_var(name_var))
            throw scope_excp("vector<int>() <- " + name_var, "000");
        return vec_int_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
        if (!vec_str_v.is_exist_var(name_var))
            throw scope_excp("vector<string>() <- " + name_var, "000");
        return vec_str_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, struct_sb::template_command>) {
        if (!tcmd_v.is_exist_var(name_var))
            throw scope_excp("template_command() <- " + name_var, "000");
        return tcmd_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, struct_sb::call_component>) {
        if (!ccmp_v.is_exist_var(name_var))
            throw scope_excp("call_component() <- " + name_var, "000");
        return ccmp_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, std::pair<std::string, std::string>>) {
        if (!global_ext_args_v.is_exist_var(name_var))
            throw scope_excp("global_external_args() <- " + name_var, "000");
        return global_ext_args_v.get_val_ref(name_var);
    }
}

template <typename T> inline std::vector<std::pair<std::string, T>> &scope::get_vector_variables_t() {
    if constexpr (std::is_same_v<T, i32t>)
        return int_v.get_vector_variables();
    else if constexpr (std::is_same_v<T, std::string>)
        return str_v.get_vector_variables();
    else if constexpr (std::is_same_v<T, struct_sb::project>)
        return prj_v.get_vector_variables();
    else if constexpr (std::is_same_v<T, struct_sb::target>)
        return trg_v.get_vector_variables();
    else if constexpr (std::is_same_v<T, std::vector<i32t>>)
        return vec_int_v.get_vector_variables();
    else if constexpr (std::is_same_v<T, std::vector<std::string>>)
        return vec_str_v.get_vector_variables();
    else if constexpr (std::is_same_v<T, struct_sb::template_command>)
        return tcmd_v.get_vector_variables();
    else if constexpr (std::is_same_v<T, struct_sb::call_component>)
        return ccmp_v.get_vector_variables();
    else if constexpr (std::is_same_v<T, std::pair<std::string, std::string>>)
        return global_ext_args_v.get_vector_variables();
}

inline bool scope::is_exist(std::string name_var) {
    if (int_v.is_exist_var(name_var) || str_v.is_exist_var(name_var) || vec_int_v.is_exist_var(name_var) ||
        vec_str_v.is_exist_var(name_var) || prj_v.is_exist_var(name_var) || trg_v.is_exist_var(name_var) ||
        tcmd_v.is_exist_var(name_var) || ccmp_v.is_exist_var(name_var) || global_ext_args_v.is_exist_var(name_var))
        return 1;
    return 0;
}
inline u32t scope::what_type(std::string name_var) {
    if (int_v.is_exist_var(name_var))
        return 1;
    else if (str_v.is_exist_var(name_var))
        return 2;
    else if (vec_int_v.is_exist_var(name_var))
        return 3;
    else if (vec_str_v.is_exist_var(name_var))
        return 4;
    else if (prj_v.is_exist_var(name_var))
        return 5;
    else if (trg_v.is_exist_var(name_var))
        return 6;
    else if (tcmd_v.is_exist_var(name_var))
        return 7;
    else if (ccmp_v.is_exist_var(name_var))
        return 8;
    else if (global_ext_args_v.is_exist_var(name_var))
        return 9;
    else
        return 0;
}

inline void scope::clear() {
    int_v.clear();
    str_v.clear();
    vec_int_v.clear();
    vec_str_v.clear();
    prj_v.clear();
    trg_v.clear();
    tcmd_v.clear();
    ccmp_v.clear();
    global_ext_args_v.clear();
}

} // namespace var

#endif