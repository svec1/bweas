//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses />
// ------------------------------------------
//

#ifndef _VAR__H
#define _VAR__H

#include <bwaliases.hpp>

template <typename T> class container_vars {
  public:
    container_vars() = default;

    container_vars(const container_vars &) = default;
    container_vars(container_vars &&)      = default;

    container_vars<T> &operator=(const container_vars<T> &dt_var) = default;

    ~container_vars() = default;

  public:
    using container_type      = umap<string, T>;
    using container_pair_type = container_type::value_type;

    const container_pair_type &create_var_r(string name_var, T value = {}) &;
    size_t create_var(string name_var, T value = {}) &;

    size_t delete_var(string name_var) &;

    T get_value(string name_var) const &;

    const container_pair_type &get_pair_ref(string name_var) const &;

    T &get_val_ref(string name_var) &;
    const T &get_val_ref(string name_var) const &;

    container_type &get_container() &;
    const container_type &get_container() const &;

    bool is_exist_var(string name_var) const &;

    void clear() &;

  private:
    container_type container;
};

template <typename T> bool container_vars<T>::is_exist_var(string name_var) const & {
    if (container.find(name_var) != container.end())
        return 1;
    return 0;
}

template <typename T>
const container_vars<T>::container_pair_type &container_vars<T>::create_var_r(string name_var, T value) & {
    if (is_exist_var(name_var))
        return NULL;
    container.emplace_hint(container.end(), name_var, value);
    return *(container.end() - 1);
}

template <typename T> size_t container_vars<T>::create_var(string name_var, T value) & {
    if (is_exist_var(name_var))
        return 1;
    container.emplace_hint(container.end(), name_var, value);
    return 0;
}

template <typename T> size_t container_vars<T>::delete_var(string name_var) & {
    if (auto it = container.find(name_var); it != container.end()) {
        container.erase(it);
        return 1;
    }
    return 0;
}

template <typename T> T container_vars<T>::get_value(string name_var) const & {
    if (auto it = container.find(name_var); it != container.end())
        return it->second;
    return create_var(name_var, T{});
}

template <typename T>
const container_vars<T>::container_pair_type &container_vars<T>::get_pair_ref(string name_var) const & {
    return *container.find(name_var);
}

template <typename T> T &container_vars<T>::get_val_ref(string name_var) & {
    return container.find(name_var)->second;
}

template <typename T> const T &container_vars<T>::get_val_ref(string name_var) const & {
    return container.find(name_var)->second;
}

template <typename T> container_vars<T>::container_type &container_vars<T>::get_container() & {
    return container;
}

template <typename T> const container_vars<T>::container_type &container_vars<T>::get_container() const & {
    return container;
}

template <typename T> void container_vars<T>::clear() & {
    container.clear();
}

#endif
