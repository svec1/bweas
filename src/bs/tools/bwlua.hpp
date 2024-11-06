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
    void create_global_table(){
        std::lock_guard<std::mutex> guard(lmutex);
        create_global_table__nmutex();
    }

    // Safely delete last table
    void delete_last_table(){
        std::lock_guard<std::mutex> guard(lmutex);
        delete_last_table__nmutex();
    }

    // Returns true if lua state is initialized
    inline bool is_created() {
        std::lock_guard<std::mutex> guard(lmutex);
        is_created__nmutex();
    }

    // Returns true if exists function(global) in lua state
    inline bool is_function(std::string name_func){
        std::lock_guard<std::mutex> guard(lmutex);
        is_function__nmutex(name_func);
    }

    // Returns true if exists variable(global) in lua state
    inline bool is_var(std::string name_var){
        std::lock_guard<std::mutex> guard(lmutex);
        is_var__nmutex(name_var);
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
        if(L)
            lua_close(L);
    }
    void run_script__nmutex() {
        if(!is_created__nmutex())
            return;
        else if (lua_pcall(L, 0, 0, 0) != LUA_OK)
            LUA_EXCEPTION()
    }
    template <typename T, typename... Types> T call_function__nmutex(std::string name_func, Types... param) {
        if(!is_created__nmutex())
            return std::any_cast<T>(std::any{});
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
        else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
            if (lua_type(L, 1) != LUA_TTABLE)
                throw std::runtime_error(LUA_FUNCTION_NRET_TABLE + name_func);

            std::vector<std::string> tmp_list;

            lua_pushnil(L);
            while(lua_next(L, -2) != 0){
                if(lua_type(L, 1) != LUA_TSTRING)
                    throw std::runtime_error(LUA_FUNCTION_NRET_TABLE_STR + name_func);
                tmp_list.push_back(lua_tostring(L, -1));
                lua_pop(L, 1);
            }

            res = tmp_list;
        }
        else if constexpr (std::is_same_v<T, std::vector<int>> || std::is_same_v<T, std::vector<bool>>) {
            if (lua_type(L, 1) != LUA_TTABLE)
                throw std::runtime_error(LUA_FUNCTION_NRET_TABLE + name_func);

            std::vector<int> tmp_list;

            lua_pushnil(L);
            while(lua_next(L, -2) != 0){
                if(lua_type(L, 1) != LUA_TNUMBER)
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
            while(lua_next(L, -2) != 0){
                if(lua_type(L, 1) == LUA_TNUMBER)
                    tmp_list.push_back(lua_tonumber(L, -1));
                else if(lua_type(L, 1) == LUA_TSTRING)
                    tmp_list.push_back(lua_tostring(L, -1));
                else if(lua_type(L, 1) == LUA_TBOOLEAN)
                    tmp_list.push_back(lua_toboolean(L, -1));
                else if(lua_type(L, 1) != LUA_TNIL)
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
        if(!is_created__nmutex())
            return;
        if (luaL_dostring(L, str_code.c_str()) != LUA_OK)
            LUA_EXCEPTION()
    }

    // creates a new global table(mainly for isolate subsequent code)
    void create_global_table__nmutex(){
        if(!is_created__nmutex())
            return;
        lua_newtable(L);

        lua_pushvalue(L, -1);
        lua_setfield(L, LUA_REGISTRYINDEX, "_G");
    }

    // deletes the last table (mainly used to delete an isolation(env) table)
    void delete_last_table__nmutex(){
        if(!is_created__nmutex())
            return;
        else if(lua_istable(L, -1))
            lua_pop(L, 1);
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

    inline bool is_created__nmutex() {
        if (!L)
            return 0;
        return 1;
    }

    inline bool is_function__nmutex(std::string name_func){
        if(!is_created__nmutex())
            return 0;
        else{
            lua_getglobal(L, name_func.c_str());

            if(!lua_isfunction(L, 1))
                return 0;
        }
        return 1;
    }

    inline bool is_var__nmutex(std::string name_var){
        if(!is_created__nmutex())
            return 0;
        else{
            lua_getglobal(L, name_var.c_str());

            if(lua_isnil(L, 1))
                return 0;
        }
        return 1;
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