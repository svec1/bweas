#ifndef BWLUA__H

// Header-library bwlua - svec
// This library is a wrapper around luajit
// **************************************
#define BWLUA__H

#include <any>
#include <exception>
#include <mutex>
#include <string>
#include <tuple>
#include <vector>

extern "C"
{
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

#define LUA_EXCEPTION()                                                                                                \
    {                                                                                                                  \
        std::string _err("", 256);                                                                                     \
        sprintf(_err.data(), "Lua error: %s", lua_tostring(L, -1));                                                    \
        lua_pop(L, 1);                                                                                                 \
        throw std::runtime_error(_err);                                                                                \
    }

// All error lines
#define LUA_FUNCTION_NFOUND "Function not found! "
#define LUA_FUNCTION_NRET_NUM "The function does not return a number! "
#define LUA_FUNCTION_NRET_STR "The function does not return a string! "
#define LUA_FUNCTION_NRET_BOOL "The function does not return a boolean! "
#define LUA_FUNCTION_NRET_TABLE "The function does not return a table! "
#define LUA_FUNCTION_NRET_TABLE_NUM "The function does not return a table of numbers! "
#define LUA_FUNCTION_NRET_TABLE_STR "The function does not return a table of strings! "
#define LUA_FUNCTION_NRET_TABLE_UNK "The function returns a table with unexpected type! "
#define LUA_FUNCTION_PARAM_VOID "Function cannot accept void "
#define LUA_FUNCTION_PARAM_UNK "Unknown type for parameter "

#define LUA_VARIABLE_NFOUND_STR "String variable not found! "
#define LUA_VARIABLE_NFOUND_NUM "Number variable not found! "
#define LUA_VARIABLE_NFOUND_BOOL "Boolean variable not found! "
#define LUA_VARIABLE_UNK "Unknown type for variable! "

#define LUA_NFOUND_TABLE "Table not found!"

#define LUA_TABLE_FIELD_UNK "Unknown type for field of table! "
#define LUA_PUSH_TYPE_UNK "Unknown type for pushing on stack! "

namespace bwlua {

// A wrapper class for luajit.
// It contains basic functions for interacting with lua scripts.
// The class is thread safe provided that you do not use __mutex functions.
// All functions without the __nmutex prefix block the RAII-style mutex.
// If an error occurs in any function, an exception is thrown, which at best should be handled!
// If a function (any) sees that lua_state is not defined, it will not do anything.
class lua {
  public:
    lua() = default;
    lua(std::string src) {
        create(src);
    }

    ~lua() {
        close();
    }

  public:
    // Used to call a function that does not take parameters, however you can also use it to indicate its return type
    // (i.e. it does not return anything)
    struct nil {
        // nothing
    };

    template <typename> struct is_vector : std::false_type {};
    template <typename U, typename A> struct is_vector<std::vector<U, A>> : std::true_type {};

    template <typename> struct is_pair : std::false_type {};
    template <typename K, typename V> struct is_pair<std::pair<K, V>> : std::true_type {};

    template <typename> struct is_vec_pairs : std::false_type {};
    template <typename A, typename K, typename V>
    struct is_vec_pairs<std::vector<std::pair<K, V>, A>> : std::true_type {};

    template <typename T> using array = std::vector<T>;
    template <typename Key, typename Value> using table = std::vector<std::pair<Key, Value>>;

    typedef ptrdiff_t integer;
    typedef double number;

  public:
    // Safely initializes Lua state
    void create(std::string src) {
        std::lock_guard<std::mutex> guard(lmutex);
        create__nmutex(src);
    }

    // Safely resets Lua state
    void close() {
        std::lock_guard<std::mutex> guard(lmutex);
        close__nmutex();
    }

    // Safely calls a function, provided that it exists and all parameters match.
    // The first parameter of the template is the return type of the function. The remaining parameters are the types of
    // function parameters accepted
    template <typename T, typename... Types> T call_function(std::string name_func, Types... param) {
        std::lock_guard<std::mutex> guard(lmutex);
        return call_function__nmutex<T, Types...>(name_func, param...);
    }

    // Safely returns the value of a variable from the passed type in the template, if it exists.
    template <typename T> T get_var(std::string name_var) {
        std::lock_guard<std::mutex> guard(lmutex);
        return get_var__nmutex<T>(name_var);
    }

    // Safely creates a variable with the specified name and value
    template <typename T> void create_var(std::string name_var, T &&value = {}) {
        std::lock_guard<std::mutex> guard(lmutex);
        create_var__nmutex(name_var, value);
    }

    // Safely interprets (executes) the code presented in str_code.
    void execute_str(std::string str_code) {
        std::lock_guard<std::mutex> guard(lmutex);
        execute_str__nmutex(str_code);
    }

    // Safely interprets (executes) the code that was provided when initializing the lua state
    void run_script() {
        std::lock_guard<std::mutex> guard(lmutex);
        run_script__nmutex();
    }

    // Safely creates gloabal table
    void create_global_table() {
        std::lock_guard<std::mutex> guard(lmutex);
        create_global_table__nmutex();
    }

    // Safely delete last table
    void delete_last_table() {
        std::lock_guard<std::mutex> guard(lmutex);
        delete_last_table__nmutex();
    }

    // Returns true if lua state is initialized
    inline bool is_created() {
        std::lock_guard<std::mutex> guard(lmutex);
        return is_created__nmutex();
    }

    // Returns true if exists function(global) in lua state
    inline bool is_function(std::string name_func) {
        std::lock_guard<std::mutex> guard(lmutex);
        return is_function__nmutex(name_func);
    }

    // Returns true if exists variable(global) in lua state
    inline bool is_var(std::string name_var) {
        std::lock_guard<std::mutex> guard(lmutex);
        return is_var__nmutex(name_var);
    }

  public:
    std::string get_stack__nmutex() {
        std::string str;
        for (size_t i = 1; i < lua_gettop(L); ++i) {
            if (lua_isboolean(L, -i) || lua_isnumber(L, -i))
                str += "[NUMBER]: " + std::to_string(lua_tonumber(L, -i));
            else if (lua_isstring(L, -i))
                str += "[STRING]: " + std::string(lua_tostring(L, -i));
            else if (lua_istable(L, -i))
                str += "[TABLE]";
            else if (lua_isfunction(L, -i))
                str += "[FUNCTION]";
            else
                str += "???";
            str += "\n";
        }

        return str;
    }

  public:
    void create__nmutex(std::string src) {
        if (L)
            lua_close(L);
        L = luaL_newstate();
        luaL_openlibs(L);
        if (luaL_loadstring(L, src.c_str()) != LUA_OK || lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK)
            LUA_EXCEPTION()
    }
    void close__nmutex() {
        if (L) {
            lua_close(L);
            L = NULL;
        }
    }
    void run_script__nmutex() {
        if (!is_created__nmutex())
            return;
        else if (lua_pcall(L, 0, 0, 0) != LUA_OK)
            LUA_EXCEPTION()
    }
    template <typename T, typename... Types> T call_function__nmutex(std::string name_func, Types... param) {
        if (!is_created__nmutex())
            return std::any_cast<T>(std::any{});
        lua_getglobal(L, name_func.c_str());

        std::tuple<Types...> params(param...);
        size_t count_params = std::tuple_size<decltype(params)>::value;

        pre_init_stack(params, std::index_sequence_for<Types...>{});

        if (!lua_isfunction(L, 1))
            throw std::runtime_error(LUA_FUNCTION_NFOUND + name_func);

        std::any res;
        if constexpr (!std::is_same_v<T, void> && !std::is_same_v<T, nil>) {
            if (lua_pcall(L, count_params, 1, 0) != LUA_OK)
                LUA_EXCEPTION()
        }
        else if (lua_pcall(L, count_params, 0, 0) != LUA_OK)
            LUA_EXCEPTION()

        if constexpr (std::is_same_v<T, std::string>) {
            if (lua_type(L, 1) != LUA_TSTRING)
                throw std::runtime_error(LUA_FUNCTION_NRET_STR + name_func);

            res = std::string(lua_tostring(L, 1));
        }
        else if constexpr (std::is_same_v<T, const char *>) {
            if (lua_type(L, 1) != LUA_TSTRING)
                throw std::runtime_error(LUA_FUNCTION_NRET_STR + name_func);

            res = lua_tostring(L, 1);
        }
        else if constexpr (std::is_same_v<T, integer>) {
            if (lua_type(L, 1) != LUA_TNUMBER)
                throw std::runtime_error(LUA_FUNCTION_NRET_NUM + name_func);

            res = lua_tointeger(L, 1);
        }
        else if (std::is_same_v<T, number>) {
            if (lua_type(L, 1) != LUA_TNUMBER)
                throw std::runtime_error(LUA_FUNCTION_NRET_NUM + name_func);

            res = lua_tonumber(L, 1);
        }
        else if constexpr (std::is_same_v<T, bool>) {
            if (lua_type(L, 1) != LUA_TBOOLEAN)
                throw std::runtime_error(LUA_FUNCTION_NRET_BOOL + name_func);

            res = (bool)lua_toboolean(L, 1);
        }
        else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
            if (lua_type(L, 1) != LUA_TTABLE)
                throw std::runtime_error(LUA_FUNCTION_NRET_TABLE + name_func);

            std::vector<std::string> tmp_list;

            lua_pushnil(L);
            while (lua_next(L, -2) != 0) {
                if (lua_type(L, 1) != LUA_TSTRING)
                    throw std::runtime_error(LUA_FUNCTION_NRET_TABLE_STR + name_func);
                tmp_list.push_back(lua_tostring(L, -1));
                lua_pop(L, 1);
            }

            res = tmp_list;
        }
        else if constexpr (std::is_same_v<T, std::vector<integer>> || std::is_same_v<T, std::vector<bool>> ||
                           std::is_same_v<T, std::vector<number>>) {
            if (lua_type(L, 1) != LUA_TTABLE)
                throw std::runtime_error(LUA_FUNCTION_NRET_TABLE + name_func);

            std::vector<int> tmp_list;

            lua_pushnil(L);
            while (lua_next(L, -2) != 0) {
                if (lua_type(L, 1) != LUA_TNUMBER)
                    throw std::runtime_error(LUA_FUNCTION_NRET_TABLE_NUM + name_func);
                tmp_list.push_back(lua_tonumber(L, -1));
                lua_pop(L, 1);
            }

            res = tmp_list;
        }
        else if constexpr (std::is_same_v<T, std::vector<std::any>>) {
            if (lua_type(L, 1) != LUA_TTABLE)
                throw std::runtime_error(LUA_FUNCTION_NRET_TABLE + name_func);

            std::vector<std::any> tmp_list;

            lua_pushnil(L);
            while (lua_next(L, -2) != 0) {
                if (lua_type(L, 1) == LUA_TNUMBER)
                    tmp_list.push_back(lua_tonumber(L, -1));
                else if (lua_type(L, 1) == LUA_TSTRING)
                    tmp_list.push_back(lua_tostring(L, -1));
                else if (lua_type(L, 1) == LUA_TBOOLEAN)
                    tmp_list.push_back(lua_toboolean(L, -1));
                else if (lua_type(L, 1) != LUA_TNIL)
                    throw std::runtime_error(LUA_FUNCTION_NRET_TABLE_UNK + name_func);
                lua_pop(L, 1);
            }

            res = tmp_list;
        }

        lua_pop(L, 1);

        if constexpr (std::is_same_v<T, void> || std::is_same_v<T, nil>)
            return;
        else
            return std::any_cast<T>(res);
    }

    // execute lua code as a string
    void execute_str__nmutex(std::string str_code) {
        if (!is_created__nmutex())
            return;
        if (luaL_dostring(L, str_code.c_str()) != LUA_OK)
            LUA_EXCEPTION()
    }

    // creates a new global table(mainly for isolate subsequent code)
    void create_global_table__nmutex() {
        if (!is_created__nmutex())
            return;
        lua_newtable(L);

        lua_pushvalue(L, -1);
        lua_setfield(L, LUA_REGISTRYINDEX, "_G");
    }

    // deletes the last table (mainly used to delete an isolation(env) table)
    void delete_last_table__nmutex() {
        if (!is_created__nmutex())
            return;
        else if (lua_istable(L, -1))
            lua_pop(L, 1);
    }

    // gets the value of a variable of the given type
    // Possible types: string, number, boolean
    template <typename T> T get_var__nmutex(std::string name_var) {
        lua_getglobal(L, name_var.c_str());
        return get_symbol<T>();
    }

    template <typename T> void create_var__nmutex(std::string name_var, T &&value = {}) {
        try {
            push_stack_param(value);
        }
        catch (std::runtime_error &what) {
            throw std::runtime_error((what.what() + name_var).c_str());
        }
        lua_setglobal(L, name_var.c_str());
    }

    inline bool is_created__nmutex() {
        if (!L)
            return 0;
        return 1;
    }

    inline bool is_function__nmutex(std::string name_func) {
        if (!is_created__nmutex())
            return 0;
        else {
            lua_getglobal(L, name_func.c_str());

            if (!lua_isfunction(L, 1))
                return 0;
        }
        return 1;
    }

    inline bool is_var__nmutex(std::string name_var) {
        if (!is_created__nmutex())
            return 0;
        else {
            lua_getglobal(L, name_var.c_str());

            if (lua_isnil(L, 1))
                return 0;
        }
        return 1;
    }

  private:
    template <typename Tuple, size_t... ind> void pre_init_stack(const Tuple &tp, std::index_sequence<ind...>) {
        (((void)push_stack_param<std::tuple_element_t<ind, Tuple>>(std::get<ind>(tp))), ...);
    }
    template <typename T> void push_stack_param(T param) {
        if constexpr (std::is_same_v<T, const char *>)
            lua_pushstring(L, param);
        else if constexpr (std::is_same_v<T, integer>)
            lua_pushinteger(L, param);
        else if constexpr (std::is_same_v<T, number>)
            lua_pushnumber(L, param);
        else if constexpr (std::is_same_v<T, bool>)
            lua_pushboolean(L, param);
        else if constexpr (std::is_same_v<T, std::string>)
            lua_pushstring(L, param.c_str());
        else if constexpr (is_pair<T>::value) {
            lua_createtable(L, 0, 2);
            push_stack_param(param.first);
            push_stack_param(param.second);
            lua_settable(L, -3);
        }
        else if constexpr (is_vector<T>::value) {
            lua_createtable(L, 0, param.size());
            for (size_t i = 0; i < param.size(); ++i) {
                push_stack_param(param[i]);
                lua_rawseti(L, -2, i + 1);
            }
        }
        else if constexpr (std::is_same_v<T, std::any>) {
            if (param.type() == typeid(const char *))
                lua_pushstring(L, std::any_cast<const char *>(param));
            else if (param.type() == typeid(integer))
                lua_pushinteger(L, std::any_cast<integer>(param));
            else if (param.type() == typeid(number))
                lua_pushnumber(L, std::any_cast<number>(param));
            else if (param.type() == typeid(bool))
                lua_pushboolean(L, std::any_cast<bool>(param));
            else if (param.type() == typeid(std::string))
                lua_pushstring(L, std::any_cast<std::string>(param).c_str());
            else
                throw std::runtime_error(LUA_PUSH_TYPE_UNK);
        }
        else if constexpr (std::is_same_v<T, nil>)
            lua_pushnil(L);
        else
            throw std::runtime_error(LUA_PUSH_TYPE_UNK);
    }

    template <typename T> T get_symbol(int idx = -1) {
        std::any value;
        if constexpr (std::is_same_v<T, const char *>) {
            if (!lua_isstring(L, idx))
                throw std::runtime_error(LUA_VARIABLE_NFOUND_STR);

            value = lua_tostring(L, idx);
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            if (!lua_isstring(L, idx))
                throw std::runtime_error(LUA_VARIABLE_NFOUND_STR);

            value = std::string(lua_tostring(L, idx));
        }
        else if constexpr (std::is_same_v<T, integer>) {
            if (!lua_isnumber(L, idx))
                throw std::runtime_error(LUA_VARIABLE_NFOUND_NUM);

            value = lua_tointeger(L, idx);
        }
        else if constexpr (std::is_same_v<T, number>) {
            if (!lua_isnumber(L, idx))
                throw std::runtime_error(LUA_VARIABLE_NFOUND_NUM);

            value = lua_tonumber(L, idx);
        }
        else if constexpr (std::is_same_v<T, bool>) {
            if (!lua_isboolean(L, idx))
                throw std::runtime_error(LUA_VARIABLE_NFOUND_BOOL);

            value = (bool)lua_toboolean(L, idx);
        }
        else if constexpr (std::is_same_v<T, std::any>) {
            if (lua_isnumber(L, idx))
                value = lua_tonumber(L, idx);
            else if (lua_isstring(L, idx))
                value = std::string(lua_tostring(L, idx));
            else if (lua_isboolean(L, idx))
                value = lua_toboolean(L, idx);
            else
                throw std::runtime_error(LUA_VARIABLE_UNK);
        }
        else if constexpr (is_pair<T>::value) {
            if (!lua_istable(L, idx))
                throw std::runtime_error(LUA_NFOUND_TABLE);

            lua_pushnil(L);
            if (lua_next(L, -2) == 0)
                return T();
            std::tuple_element<0, T>::type key = get_symbol<std::tuple_element<0, T>::type>(-2);
            value = std::make_pair<std::tuple_element<0, T>::type, std::tuple_element<1, T>::type>(
                std::move(key), get_symbol<std::tuple_element<1, T>::type>());
        }
        else if constexpr (is_vector<T>::value) {
            if (!lua_istable(L, idx))
                throw std::runtime_error(LUA_NFOUND_TABLE);

            T vec;
            if constexpr (is_pair<T::value_type>::value) {
                lua_pushnil(L);
                while (lua_next(L, -2)) {
                    vec.push_back(get_symbol<T::value_type>());
                    lua_pop(L, 1);
                }
            }
            else {
                size_t i = 1;
                while (1) {
                    lua_rawgeti(L, idx, i);
                    if (lua_isnil(L, idx))
                        break;
                    vec.push_back(get_symbol<T::value_type>());
                    ++i;
                }
            }
            value = vec;
        }
        else
            throw std::runtime_error(LUA_VARIABLE_UNK);

        if (idx == -1)
            lua_pop(L, 1);
        if constexpr (std::is_same_v<T, std::any>)
            return value;
        return std::any_cast<T>(value);
    }

  private:
    lua_State *L = NULL;
    std::mutex lmutex;
};
} // namespace bwlua

#endif