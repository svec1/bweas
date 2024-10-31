#ifndef BWLUA__H
#define BWLUA__H

#include "../../mdef.hpp"

#include <any>
#include <exception>
#include <mutex>
#include <string>
#include <tuple>
#include <type_traits>
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
#define LUA_FUNCTION_NFOUND "Function not found! "
#define LUA_FUNCTION_NRET_NUM "The function does not return a number! "
#define LUA_FUNCTION_NRET_STR "The function does not return a string! "
#define LUA_FUNCTION_NRET_BOOL "The function does not return a boolean! "
#define LUA_FUNCTION_PARAM_VOID "Function cannot accept void"
#define LUA_FUNCTION_PARAM_UNK "Unknown type for parameter"

#define LUA_VARIABLE_NFOUND_STR "String variable not found! "
#define LUA_VARIABLE_NFOUND_NUM "Number variable not found! "
#define LUA_VARIABLE_NFOUND_BOOL "Boolean variable not found! "
#define LUA_VARIABLE_UNK "Unknown type for variable! "

namespace bwlua {
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
    struct nil {
        // nothing
    };

  public:
    void create(std::string src) {
        std::lock_guard<std::mutex> guard(lmutex);
        create__nmutex(src);
    }
    void close() {
        std::lock_guard<std::mutex> guard(lmutex);
        lua_close(L);
    }

    template <typename T, typename... Types> T call_function(std::string name_func, Types... param) {
        std::lock_guard<std::mutex> guard(lmutex);
        return call_function__nmutex<T, Types...>(name_func, param...);
    }
    template <typename T> T get_var(std::string name_var) {
        std::lock_guard<std::mutex> guard(lmutex);
        return get_var__nmutex<T>(name_var);
    }
    void execute_str(std::string str_code) {
        std::lock_guard<std::mutex> guard(lmutex);
        execute_str__nmutex(str_code);
    }
    void run_script() {
        std::lock_guard<std::mutex> guard(lmutex);
        run_script__nmutex();
    }

    bool is_created() {
        std::lock_guard<std::mutex> guard(lmutex);
        if (!L)
            return 0;
        return 1;
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
        lua_close(L);
    }
    void run_script__nmutex() {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK)
            LUA_EXCEPTION()
    }
    template <typename T, typename... Types> T call_function__nmutex(std::string name_func, Types... param) {
        lua_getglobal(L, name_func.c_str());

        std::tuple<Types...> params(param...);
        size_t count_params = std::tuple_size<decltype(params)>::value;

        pre_init_stack(params, std::index_sequence_for<Types...>{}, name_func);

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
            if (lua_type(L, 1) != LUA_TNUMBER)
                throw std::runtime_error(LUA_FUNCTION_NRET_STR + name_func);

            res = lua_tonumber(L, 1);
        }
        else if constexpr (std::is_same_v<T, int>) {
            if (lua_type(L, 1) != LUA_TSTRING)
                throw std::runtime_error(LUA_FUNCTION_NRET_NUM + name_func);

            res = lua_tostring(L, 1);
        }
        else if constexpr (std::is_same_v<T, bool>) {
            if (lua_type(L, 1) != LUA_TBOOLEAN)
                throw std::runtime_error(LUA_FUNCTION_NRET_BOOL + name_func);

            res = lua_toboolean(L, 1);
        }

        if constexpr (std::is_same_v<T, void> || std::is_same_v<T, nil>)
            return;
        else
            return res;
    }

    // execute lua code as a string
    void execute_str__nmutex(std::string str_code) {
        if (luaL_dostring(L, str_code.c_str()) != LUA_OK)
            LUA_EXCEPTION()
    }

    // gets the value of a variable of the given type
    // Possible types: string, number, boolean
    template <typename T> T get_var__nmutex(std::string name_var) {
        lua_getglobal(L, name_var.c_str());
        if constexpr (std::is_same_v<T, std::string>) {
            if (!lua_isstring(L, 1))
                throw std::runtime_error(LUA_VARIABLE_NFOUND_STR + name_var);

            return lua_tostring(L, 1);
        }
        else if constexpr (std::is_same_v<T, int>) {
            if (!lua_isnumber(L, 1))
                throw std::runtime_error(LUA_VARIABLE_NFOUND_NUM + name_var);

            return lua_tonumber(L, 1);
        }
        else if constexpr (std::is_same_v<T, bool>) {
            if (!lua_isboolean(L, 1))
                throw std::runtime_error(LUA_VARIABLE_NFOUND_BOOL + name_var);

            return lua_toboolean(L, 1);
        }
        else
            throw std::runtime_error(LUA_VARIABLE_UNK + name_var);
    }

  private:
    template <typename Tuple, size_t... ind>
    void pre_init_stack(const Tuple &tp, std::index_sequence<ind...>, const std::string &name_func) {
        ((push_stack_param<std::tuple_element_t<ind, Tuple>>(std::get<ind>(tp), name_func)), ...);
    }
    template <typename T> void push_stack_param(T param, const std::string &name_func) {
        if constexpr (std::is_same_v<T, const char *>)
            lua_pushstring(L, param);
        else if constexpr (std::is_same_v<T, int>)
            lua_pushnumber(L, param);
        else if constexpr (std::is_same_v<T, bool>)
            lua_pushboolean(L, param);
        else if constexpr (std::is_same_v<T, void>)
            throw std::runtime_error(LUA_FUNCTION_PARAM_VOID + name_func);
        else if constexpr (!std::is_same_v<T, nil>)
            throw std::runtime_error(LUA_FUNCTION_PARAM_UNK + name_func);
    }

  private:
    lua_State *L = NULL;
    std::mutex lmutex;
};
} // namespace bwlua

#endif