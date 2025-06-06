#ifndef _VAR__H
#define _VAR__H

#include <bwaliases.hpp>

#include <memory>
#include <string>
#include <vector>

namespace var {
template <typename T> class datatype_var {
  public:
    datatype_var() = default;

    datatype_var(const datatype_var &) = default;
    datatype_var(datatype_var &&)      = default;

    datatype_var<T> &operator=(const datatype_var<T> &dt_var);

    ~datatype_var() = default;

  public:
    const pair<string, T> &create_var_r(string name_var, T value = {});
    uint create_var(string name_var, T value = NULL);

    uint delete_var(string name_var);

    // ret: 0 - succes; 1 - name_var doesn't exist; 2 - name_var_src doesn't exist
    uint copy(string name_var, string name_var_src);

    T get_value(string name_var);

    const pair<string, T> &get_ref(string name_var);
    T &get_val_ref(string name_var);

    vec<pair<string, T>> &get_vector_variables();

    bool is_exist_var(string name_var);

    void clear();

  private:
    uint find_var_ind(string name_var);

    vec<pair<string, T>> variable_s;
};

template <typename T> datatype_var<T> &datatype_var<T>::operator=(const datatype_var<T> &dt_var) {
    variable_s = dt_var.variable_s;
    return *this;
}
template <typename T> uint datatype_var<T>::find_var_ind(string name_var) {
    for (uint i = 0; i < variable_s.size(); ++i) {
        if (variable_s[i].first == name_var)
            return i;
    }
    return UINT32_MAX;
}

template <typename T> bool datatype_var<T>::is_exist_var(string name_var) {
    if (find_var_ind(name_var) != UINT32_MAX)
        return 1;
    return 0;
}

template <typename T> const pair<string, T> &datatype_var<T>::create_var_r(string name_var, T value) {
    if (is_exist_var(name_var))
        return NULL;
    variable_s.emplace(name_var, value);
    return variable_s[variable_s.size() - 1];
}

template <typename T> uint datatype_var<T>::create_var(string name_var, T value) {
    if (is_exist_var(name_var))
        return 1;
    variable_s.emplace_back(name_var, value);
    return 0;
}

template <typename T> uint datatype_var<T>::delete_var(string name_var) {
    if (!is_exist_var(name_var))
        return 1;
    variable_s.erase(variable_s.begin() + find_var_ind(name_var));
    return 0;
}

template <typename T> uint datatype_var<T>::copy(string name_var, string name_var_src) {
    if (!is_exist_var(name_var))
        return 1;
    else if (!is_exist_var(name_var_src))
        return 2;

    variable_s[find_var_ind(name_var)].second = variable_s[find_var_ind(name_var_src)].second;
    return 0;
}

template <typename T> T datatype_var<T>::get_value(string name_var) {
    if (!is_exist_var(name_var))
        return T();
    return variable_s[find_var_ind(name_var)].second;
}

template <typename T> const pair<string, T> &datatype_var<T>::get_ref(string name_var) {
    return variable_s[find_var_ind(name_var)];
}

template <typename T> T &datatype_var<T>::get_val_ref(string name_var) {
    return variable_s[find_var_ind(name_var)].second;
}

template <typename T> vec<pair<string, T>> &datatype_var<T>::get_vector_variables() {
    return variable_s;
}

template <typename T> void datatype_var<T>::clear() {
    variable_s.clear();
}

} // namespace var

#endif
