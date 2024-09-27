#ifndef _SCOPE__H
#define _SCOPE__H

#include "../../mdef.hpp"

#include "../tools/call_cmd.hpp"
#include "static_struct.hpp"
#include "var.hpp"

#include <string>
#include <vector>

namespace var {

inline std::string
type_var_to_str(u32t ind) {
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
    return "undef";
}

class scope {
  public:
    inline scope();

    inline scope(const scope &) = default;
    inline scope(scope &&) = default;

    ~scope() = default;

  public:
    template <typename T>
    inline T &
    create_var(std::string name_var, T val = T{});
    template <typename T>
    inline bool
    try_create_var(std::string name_var, T val = T{});

    template <typename T>
    inline void
    delete_var(std::string name_var);
    template <typename T>

    inline T &
    get_var_value(std::string name_var);

    template <typename T>
    inline std::vector<std::pair<std::string, T>> &
    get_vector_variables_t();

    inline bool
    is_exist(std::string name_var);

    inline void
    clear();

    // 1 - int
    // 2 - string
    // 3 - vector<int>
    // 4 - vector<string>
    // 5 - project
    // 6 - target
    // 0 - undefined
    inline u32t
    what_type(std::string name_var);

  public:
    tools::call_cmd_manager call_cmd;

  private:
    static inline bool init_glob{0};

    var::datatype_var<int> int_v;
    var::datatype_var<std::string> str_v;
    var::datatype_var<struct_sb::project> prj_v;
    var::datatype_var<struct_sb::target> trg_v;
    var::datatype_var<std::vector<int>> vec_int_v;
    var::datatype_var<std::vector<std::string>> vec_str_v;
};

inline scope::scope() {
    if (!init_glob) {
        init_glob = 1;
        assist.add_err("SCOP000", "The variable does not exist");
        assist.add_err("SCOP001", "The variable has already been created(does exist)");
        assist.add_err("SCOP002", "Unknown data type");
    }
}

template <>
inline int &
scope::create_var<int>(std::string name_var, int val) {
    if (int_v.create_var(name_var, val))
        assist.call_err("SCOP001", "int(" + std::to_string(val) + ") <- " + name_var);
    return int_v.get_val_ref(name_var);
}
template <>
inline std::string &
scope::create_var<std::string>(std::string name_var, std::string val) {
    if (str_v.create_var(name_var, val))
        assist.call_err("SCOP001", "string(" + val + ") <- " + name_var);
    return str_v.get_val_ref(name_var);
}
template <>
inline struct_sb::project &
scope::create_var<struct_sb::project>(std::string name_var, struct_sb::project val) {
    if (prj_v.create_var(name_var, val))
        assist.call_err("SCOP001", "project(" + val.name_project + ") <- " + name_var);
    return prj_v.get_val_ref(name_var);
}
template <>
inline struct_sb::target &
scope::create_var<struct_sb::target>(std::string name_var, struct_sb::target val) {
    if (trg_v.create_var(name_var, val))
        assist.call_err("SCOP001", "target(" + val.name_target + ") <- " + name_var);
    return trg_v.get_val_ref(name_var);
}
template <>
inline std::vector<int> &
scope::create_var<std::vector<int>>(std::string name_var, std::vector<int> val) {
    if (vec_int_v.create_var(name_var, val))
        assist.call_err("SCOP001", "vector<int>(" + std::to_string(val[0]) + ", ..." + ") <- " + name_var);
    return vec_int_v.get_val_ref(name_var);
}
template <>
inline std::vector<std::string> &
scope::create_var<std::vector<std::string>>(std::string name_var, std::vector<std::string> val) {
    if (vec_str_v.create_var(name_var, val))
        assist.call_err("SCOP001", "vector<string>(" + val[0] + ", ..." + ") <- " + name_var);
    return vec_str_v.get_val_ref(name_var);
}
template <>
inline bool
scope::try_create_var<int>(std::string name_var, int val) {
    if (int_v.create_var(name_var, val))
        return 0;
    return 1;
}
template <>
inline bool
scope::try_create_var<std::string>(std::string name_var, std::string val) {
    if (str_v.create_var(name_var, val))
        return 0;
    return 1;
}
template <>
inline bool
scope::try_create_var<struct_sb::project>(std::string name_var, struct_sb::project val) {
    if (prj_v.create_var(name_var, val))
        return 0;
    return 1;
}
template <>
inline bool
scope::try_create_var<struct_sb::target>(std::string name_var, struct_sb::target val) {
    if (trg_v.create_var(name_var, val))
        return 0;
    return 1;
}
template <>
inline bool
scope::try_create_var<std::vector<int>>(std::string name_var, std::vector<int> val) {
    if (vec_int_v.create_var(name_var, val))
        return 0;
    return 1;
}
template <>
inline bool
scope::try_create_var<std::vector<std::string>>(std::string name_var, std::vector<std::string> val) {
    if (vec_str_v.create_var(name_var, val))
        return 0;
    return 1;
}

template <>
inline void
scope::delete_var<int>(std::string name_var) {
    if (int_v.delete_var(name_var))
        assist.call_err("SCOP000", "int() <- " + name_var);
}
template <>
inline void
scope::delete_var<std::string>(std::string name_var) {
    if (str_v.delete_var(name_var))
        assist.call_err("SCOP000", "string() <- " + name_var);
}
template <>
inline void
scope::delete_var<struct_sb::project>(std::string name_var) {
    if (prj_v.delete_var(name_var))
        assist.call_err("SCOP000", "project() <- " + name_var);
}
template <>
inline void
scope::delete_var<struct_sb::target>(std::string name_var) {
    if (trg_v.delete_var(name_var))
        assist.call_err("SCOP000", "target() <- " + name_var);
}
template <>
inline void
scope::delete_var<std::vector<int>>(std::string name_var) {
    if (vec_int_v.delete_var(name_var))
        assist.call_err("SCOP000", "vector<int>() <- " + name_var);
}
template <>
inline void
scope::delete_var<std::vector<std::string>>(std::string name_var) {
    if (vec_str_v.delete_var(name_var))
        assist.call_err("SCOP000", "vector<std::string>() <- " + name_var);
}
template <>
inline int &
scope::get_var_value<int>(std::string name_var) {
    if (!int_v.is_exist_var(name_var))
        assist.call_err("SCOP000", "int() <- " + name_var);
    return int_v.get_val_ref(name_var);
}
template <>
inline std::string &
scope::get_var_value<std::string>(std::string name_var) {
    if (!str_v.is_exist_var(name_var))
        assist.call_err("SCOP000", "string() <- " + name_var);
    return str_v.get_val_ref(name_var);
}
template <>
inline struct_sb::project &
scope::get_var_value<struct_sb::project>(std::string name_var) {
    if (!prj_v.is_exist_var(name_var))
        assist.call_err("SCOP000", "project() <- " + name_var);
    return prj_v.get_val_ref(name_var);
}
template <>
inline struct_sb::target &
scope::get_var_value<struct_sb::target>(std::string name_var) {
    if (!trg_v.is_exist_var(name_var))
        assist.call_err("SCOP000", "target() <- " + name_var);
    return trg_v.get_val_ref(name_var);
}
template <>
inline std::vector<int> &
scope::get_var_value<std::vector<int>>(std::string name_var) {
    if (!vec_int_v.is_exist_var(name_var))
        assist.call_err("SCOP000", "vector<int>() <- " + name_var);
    return vec_int_v.get_val_ref(name_var);
}
template <>
inline std::vector<std::string> &
scope::get_var_value<std::vector<std::string>>(std::string name_var) {
    if (!vec_str_v.is_exist_var(name_var))
        assist.call_err("SCOP000", "vector<string>() <- " + name_var);
    return vec_str_v.get_val_ref(name_var);
}

template <>
inline std::vector<std::pair<std::string, int>> &
scope::get_vector_variables_t() {
    return int_v.get_vector_variables();
}
template <>
inline std::vector<std::pair<std::string, std::string>> &
scope::get_vector_variables_t() {
    return str_v.get_vector_variables();
}
template <>
inline std::vector<std::pair<std::string, std::vector<int>>> &
scope::get_vector_variables_t() {
    return vec_int_v.get_vector_variables();
}
template <>
inline std::vector<std::pair<std::string, std::vector<std::string>>> &
scope::get_vector_variables_t() {
    return vec_str_v.get_vector_variables();
}
template <>
inline std::vector<std::pair<std::string, var::struct_sb::project>> &
scope::get_vector_variables_t() {
    return prj_v.get_vector_variables();
}
template <>
inline std::vector<std::pair<std::string, var::struct_sb::target>> &
scope::get_vector_variables_t() {
    return trg_v.get_vector_variables();
}

inline bool
scope::is_exist(std::string name_var) {
    if (int_v.is_exist_var(name_var) || str_v.is_exist_var(name_var) || vec_int_v.is_exist_var(name_var) ||
        vec_str_v.is_exist_var(name_var) || prj_v.is_exist_var(name_var) || trg_v.is_exist_var(name_var))
        return 1;
    return 0;
}
inline u32t
scope::what_type(std::string name_var) {
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
    else
        return 0;
}

inline void
scope::clear() {
    int_v.clear();
    str_v.clear();
    vec_int_v.clear();
    vec_str_v.clear();
    prj_v.clear();
    trg_v.clear();
}

} // namespace var

#endif