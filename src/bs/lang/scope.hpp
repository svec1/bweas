#ifndef SCOPE_HPP
#define SCOPE_HPP

#include <string>
#include <vector>

#include <bwlogger.hpp>

#include <lang/expression.hpp>
#include <lang/static_struct.hpp>
#include <lang/var.hpp>

#include <tools/call_cmd.hpp>

extern string get_current_loc();

using namespace bweas;

namespace var {

inline string type_var_to_str(size_t ind) {
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
    else if (ind == 10)
        return "function";
    return "undef";
}

class scope {
  public:
    scope(bweas::logger &__log) : _log(__log) {
    }

    inline scope(const scope &) = default;
    inline scope(scope &&)      = default;

    ~scope() = default;

  public:
    template <typename T> inline T &create_var(string name_var, T val = T{});
    template <typename T> inline bool try_create_var(string name_var, T val = T{});

    template <typename T> inline void delete_var(string name_var);

    template <typename T> inline T &get_var_value(string name_var);
    template <typename T> inline vec<pair<string, T>> &get_vector_variables_t();

    inline bool is_exist(string name_var);

    inline void clear();

    // 1  - int
    // 2  - string
    // 3  - vector<int>
    // 4  - vector<string>
    // 5  - project
    // 6  - target
    // 7  - template command
    // 8  - call component
    // 9  - global external args
    // 10 - function
    // 0  - undefined
    inline size_t what_type(string name_var);

  public:
    call_cmd_manager call_cmd;

  private:
    logger &_log;

    var::datatype_var<decl_func> funcs_v;

    var::datatype_var<pdiff> int_v;
    var::datatype_var<string> str_v;
    var::datatype_var<struct_sb::project> prj_v;
    var::datatype_var<struct_sb::target> trg_v;
    var::datatype_var<vec<pdiff>> vec_int_v;
    var::datatype_var<vec<string>> vec_str_v;

    var::datatype_var<struct_sb::template_command> tcmd_v;
    var::datatype_var<struct_sb::call_component> ccmp_v;
    var::datatype_var<pair<string, string>> global_ext_args_v;
};

template <typename T> inline T &scope::create_var(string name_var, T val) {
    if constexpr (std::is_same_v<T, pdiff>) {
        if (!int_v.create_var(name_var, val))
            return int_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, string>) {
        if (!str_v.create_var(name_var, val))
            return str_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, struct_sb::project>) {
        if (!prj_v.create_var(name_var, val))
            return prj_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, struct_sb::target>) {
        if (!trg_v.create_var(name_var, val))
            return trg_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, vec<pdiff>>) {
        if (!vec_int_v.create_var(name_var, val))
            return vec_int_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, vec<string>>) {
        if (!vec_str_v.create_var(name_var, val))
            return vec_str_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, struct_sb::template_command>) {
        if (!tcmd_v.create_var(name_var, val))
            return tcmd_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, struct_sb::call_component>) {
        if (!ccmp_v.create_var(name_var, val))
            return ccmp_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, pair<string, string>>) {
        if (!global_ext_args_v.create_var(name_var, val))
            return global_ext_args_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, decl_func>) {
        if (!funcs_v.create_var(name_var, val))
            return funcs_v.get_val_ref(name_var);
    }
    else
        static_assert(false, "Unsuitable type.");
#if defined(UNIX)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
    (_log << bwtools::fatal) << (log_message(log_type::fatal)
                                 << get_current_loc() << ": The " << name_var << " variable already exists");
#pragma GCC diagnostic pop
#else
__assume(0);
#endif
}

template <typename T> inline bool scope::try_create_var(string name_var, T val) {
    bool creates = 0;
    if constexpr (std::is_same_v<T, pdiff>)
        creates = int_v.create_var(name_var, val);
    else if constexpr (std::is_same_v<T, string>)
        creates = str_v.create_var(name_var, val);
    else if constexpr (std::is_same_v<T, struct_sb::project>)
        creates = prj_v.create_var(name_var, val);
    else if constexpr (std::is_same_v<T, struct_sb::target>)
        creates = trg_v.create_var(name_var, val);
    else if constexpr (std::is_same_v<T, vec<pdiff>>)
        creates = vec_int_v.create_var(name_var, val);
    else if constexpr (std::is_same_v<T, vec<string>>)
        creates = vec_str_v.create_var(name_var, val);
    else if constexpr (std::is_same_v<T, struct_sb::template_command>)
        creates = tcmd_v.create_var(name_var, val);
    else if constexpr (std::is_same_v<T, struct_sb::call_component>)
        creates = ccmp_v.create_var(name_var, val);
    else if constexpr (std::is_same_v<T, pair<string, string>>)
        creates = global_ext_args_v.create_var(name_var, val);
    else if constexpr (std::is_same_v<T, decl_func>)
        creates = funcs_v.create_var(name_var, val);

    return !creates;
}

template <typename T> inline void scope::delete_var(string name_var) {
    bool err_handling = 0;

    if constexpr (std::is_same_v<T, pdiff>)
        err_handling = int_v.delete_var(name_var);
    else if constexpr (std::is_same_v<T, string>)
        err_handling = str_v.delete_var(name_var);
    else if constexpr (std::is_same_v<T, struct_sb::project>)
        err_handling = prj_v.delete_var(name_var);
    else if constexpr (std::is_same_v<T, struct_sb::target>)
        err_handling = trg_v.delete_var(name_var);
    else if constexpr (std::is_same_v<T, vec<pdiff>>)
        err_handling = vec_int_v.delete_var(name_var);
    else if constexpr (std::is_same_v<T, vec<string>>)
        err_handling = vec_str_v.delete_var(name_var);
    else if constexpr (std::is_same_v<T, struct_sb::template_command>)
        err_handling = tcmd_v.delete_var(name_var);
    else if constexpr (std::is_same_v<T, struct_sb::call_component>)
        err_handling = ccmp_v.delete_var(name_var);
    else if constexpr (std::is_same_v<T, pair<string, string>>)
        err_handling = global_ext_args_v.delete_var(name_var);
    else if constexpr (std::is_same_v<T, decl_func>)
        err_handling = funcs_v.delete_var(name_var);

    if (err_handling)
        (_log << bwtools::error) << (log_message(log_type::error) << "The " << name_var << " variable does not exist");
}

template <typename T> inline T &scope::get_var_value(string name_var) {
    if constexpr (std::is_same_v<T, pdiff>) {
        if (int_v.is_exist_var(name_var))
            return int_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, string>) {
        if (str_v.is_exist_var(name_var))
            return str_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, struct_sb::project>) {
        if (prj_v.is_exist_var(name_var))
            return prj_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, struct_sb::target>) {
        if (trg_v.is_exist_var(name_var))
            return trg_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, vec<pdiff>>) {
        if (vec_int_v.is_exist_var(name_var))
            return vec_int_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, vec<string>>) {
        if (vec_str_v.is_exist_var(name_var))
            return vec_str_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, struct_sb::template_command>) {
        if (tcmd_v.is_exist_var(name_var))
            return tcmd_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, struct_sb::call_component>) {
        if (ccmp_v.is_exist_var(name_var))
            return ccmp_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, pair<string, string>>) {
        if (global_ext_args_v.is_exist_var(name_var))
            return global_ext_args_v.get_val_ref(name_var);
    }
    else if constexpr (std::is_same_v<T, decl_func>) {
        if (funcs_v.is_exist_var(name_var))
            return funcs_v.get_val_ref(name_var);
    }
    else
        static_assert(false, "Unsuitable type.");
#if defined(UNIX)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
    (_log << bwtools::fatal) << (log_message(log_type::fatal)
                                 << get_current_loc() << ": The " << name_var << " variable does not exist");
#pragma GCC diagnostic pop
#else
__assume(0);
#endif
}

template <typename T> inline vec<pair<string, T>> &scope::get_vector_variables_t() {
    if constexpr (std::is_same_v<T, pdiff>)
        return int_v.get_vector_variables();
    else if constexpr (std::is_same_v<T, string>)
        return str_v.get_vector_variables();
    else if constexpr (std::is_same_v<T, struct_sb::project>)
        return prj_v.get_vector_variables();
    else if constexpr (std::is_same_v<T, struct_sb::target>)
        return trg_v.get_vector_variables();
    else if constexpr (std::is_same_v<T, vec<pdiff>>)
        return vec_int_v.get_vector_variables();
    else if constexpr (std::is_same_v<T, vec<string>>)
        return vec_str_v.get_vector_variables();
    else if constexpr (std::is_same_v<T, struct_sb::template_command>)
        return tcmd_v.get_vector_variables();
    else if constexpr (std::is_same_v<T, struct_sb::call_component>)
        return ccmp_v.get_vector_variables();
    else if constexpr (std::is_same_v<T, pair<string, string>>)
        return global_ext_args_v.get_vector_variables();
    else if constexpr (std::is_same_v<T, decl_func>)
        return funcs_v.get_vector_variables();
    else
        static_assert(false, "Unsuitable type.");
#if defined(UNIX)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
    __builtin_unreachable();
#pragma GCC diagnostic pop
#else
    __assume(0);
#endif
}

inline bool scope::is_exist(string name_var) {
    if (int_v.is_exist_var(name_var) || str_v.is_exist_var(name_var) || vec_int_v.is_exist_var(name_var) ||
        vec_str_v.is_exist_var(name_var) || prj_v.is_exist_var(name_var) || trg_v.is_exist_var(name_var) ||
        tcmd_v.is_exist_var(name_var) || ccmp_v.is_exist_var(name_var) || global_ext_args_v.is_exist_var(name_var) ||
        funcs_v.is_exist_var(name_var))
        return 1;
    return 0;
}
inline size_t scope::what_type(string name_var) {
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
    else if (funcs_v.is_exist_var(name_var))
        return 10;
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
    funcs_v.clear();
}

} // namespace var

#endif
