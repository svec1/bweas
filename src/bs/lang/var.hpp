#ifndef _VAR__H
#define _VAR__H

#include "../../kernel/low_level/type.h"

#include <vector>
#include <string>
#include <memory>

namespace var{
    template<typename T>
    class datatype_var{
        public:
            datatype_var() = default;

            datatype_var(const datatype_var&) = default;
            datatype_var(datatype_var&&) = default;

            datatype_var<T>& operator=(const datatype_var<T>& dt_var);
                
            ~datatype_var() = default;

        public:
            const std::pair<std::string, T>& create_var_r(std::string name_var, T value = {});
            u32t create_var(std::string name_var, T value = NULL);

            u32t delete_var(std::string name_var);

            // ret: 0 - succes; 1 - name_var doesn't exist; 2 - name_var_src doesn't exist
            u32t copy(std::string name_var, std::string name_var_src);

            T get_value(std::string name_var);

            const std::pair<std::string, T>& get_ref(std::string name_var);
            T& get_val_ref(std::string name_var);

            bool is_exist_var(std::string name_var);

            void clear();

        private:

            u32t find_var_ind(std::string name_var);

            std::vector<std::pair<std::string, T>> variable_s;
    };
    template<typename T>
    datatype_var<T>& datatype_var<T>::operator=(const datatype_var<T>& dt_var){
        variable_s = dt_var.variable_s;
        return *this;
    }
    template<typename T>
    u32t datatype_var<T>::find_var_ind(std::string name_var){
        for(u32t i = 0; i < variable_s.size(); ++i){
            if(variable_s[i].first == name_var) return i;
        }
        return UINT32_MAX;
    }

    template<typename T>
    bool datatype_var<T>::is_exist_var(std::string name_var){
        if(find_var_ind(name_var) != UINT32_MAX) return 1;
        return 0;
    }

    template<typename T>
    const std::pair<std::string, T>& datatype_var<T>::create_var_r(std::string name_var, T value){
        if(is_exist_var(name_var)) return NULL;
        variable_s.emplace(name_var, value);
        return variable_s[variable_s.size()-1];
    }
    
    template<typename T>
    u32t datatype_var<T>::create_var(std::string name_var, T value){
        if(is_exist_var(name_var)) return 1;
        variable_s.emplace_back(name_var, value);
        return 0;
    }

    template<typename T>
    u32t datatype_var<T>::delete_var(std::string name_var){
        if(!is_exist_var(name_var)) return 1;
        variable_s.erase(variable_s.begin()+find_var_ind(name_var));
        return 0;
    }
    
    template<typename T>
    u32t datatype_var<T>::copy(std::string name_var, std::string name_var_src){
        if(!is_exist_var(name_var)) return 1;
        else if(!is_exist_var(name_var_src)) return 2;

        variable_s[find_var_ind(name_var)].second = variable_s[find_var_ind(name_var_src)].second;
        return 0;
    }
    
    template<typename T>
    T datatype_var<T>::get_value(std::string name_var){
        if(!is_exist_var(name_var)) return T();
        return variable_s[find_var_ind(name_var)].second;
    }

    template<typename T>
    const std::pair<std::string, T>& datatype_var<T>::get_ref(std::string name_var){
        return variable_s[find_var_ind(name_var)];
    }

    template<typename T>
    T& datatype_var<T>::get_val_ref(std::string name_var){
        return variable_s[find_var_ind(name_var)].second;
    }

    template<typename T>
    void datatype_var<T>::clear(){
        variable_s.clear();
    }

}

#endif