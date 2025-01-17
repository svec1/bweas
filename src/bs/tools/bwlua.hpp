#ifndef BWLUA__H

// Header-library bwlua - svec
// This library is a wrapper around luajit
// **************************************
#define BWLUA__H

#include <any>
#include <map>
#include <mutex>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include <lua.hpp>

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

#define LUA_SYMBOL_NE_LUA_STATE "Object - symbol points to non-existent lua_State"
#define LUA_SYMBOL_WRONG_TYPE_VAR "Object - symbol must be of type variable or nil for interaction with variables"
#define LUA_SYMBOL_WRONG_TYPE_FUNC "Object - symbol must be of type function for call of function"
#define LUA_SYMBOL_WRONG_TYPE_VAR_VALUE "Object - symbol must be of type variable to get the value"
#define LUA_SYMBOL_NREG_TYPE_CFUNC "When registering a function, a function of type cfunc is expected."

#define LUA_TABLE_KEYCMP "Unexpected key type table with type key - std::any! "

#define LUA_EXCEPTION()                                                                                                \
    {                                                                                                                  \
        std::string _err("", 256);                                                                                     \
        sprintf(_err.data(), "Lua error: %s", lua_tostring(L, -1));                                                    \
        lua_pop(L, 1);                                                                                                 \
        throw std::runtime_error(_err);                                                                                \
    }

namespace bwlua {

// Special tools - Wrappers over the lua class,
// for simple interaction with the lua stack and others.
namespace tools {
template <typename T> inline void push_stack(lua_State *L, T param);
template <typename T> inline T pop_stack(lua_State *L, int idx = -1);

// Calls a function that is on the top of the stack
template <typename T, typename... Types> inline T call_function(lua_State *L, Types... param);
} // namespace tools

// A wrapper class for luajit.
// It contains basic functions for interacting with lua scripts.
// The class is thread safe provided that you do not use __mutex functions.
// All functions without the __nmutex prefix block the RAII-style mutex.
// If an error occurs in any function, an exception is thrown, which at best should be handled!
// If a function (any) sees that lua_state is not defined, it will not do anything.
class lua {
  public:
    lua() = default;
    explicit lua(lua_State *_L) : L(_L) {
    }
    lua(std::string src) {
        create(src);
    }

    ~lua() {
        close();
    }

    lua(const lua &) = delete;
    lua &operator=(lua &&) = delete;

  public:
    // Used to call a function that does not take parameters, however you can also use it to indicate its return type
    // (i.e. it does not return anything)
    struct nil {
        // nothing
    };

    // special object - symbol, for accessing global lua symbols
    struct symbol {
        friend class lua;
        enum class type {
            variable = 0,
            function,
            nil
        };

        ~symbol() = default;

      private:
        symbol() = delete;
        symbol(symbol &&) = delete;
        symbol &operator=(symbol &&) = delete;

        explicit symbol(type _symbol_t, std::string _name_sym, lua *_L)
            : symbol_t(_symbol_t), name_sym(_name_sym), L(_L) {
        }

      public:
        // creates or modifies a variable with the specified value pointed to by the symbol object itself
        template <typename T> void operator=(T &&value) const {
            if (!L->is_created())
                throw std::runtime_error(LUA_SYMBOL_NE_LUA_STATE);
            else if (symbol_t == type::function)
                throw std::runtime_error(LUA_SYMBOL_WRONG_TYPE_VAR);

            lua_pop(L->L, 1);
            L->create_var(name_sym, value);
        }

        // registers a function in lua state
        template <typename T> void operator<<(T value) const {
            if (!L->is_created())
                throw std::runtime_error(LUA_SYMBOL_NE_LUA_STATE);
            else if (symbol_t != type::nil)
                throw std::runtime_error(LUA_SYMBOL_WRONG_TYPE_VAR);

            if constexpr (!std::is_same_v<T, cfunc>)
                throw std::runtime_error(LUA_SYMBOL_NREG_TYPE_CFUNC);

            lua_pop(L->L, 1);
            L->register_function(name_sym, value);
        }

        // calls a function with the specified parameters pointed to by the symbol object itself
        template <typename T, typename... Types> T operator()(Types... param) const {
            if (!L->is_created())
                throw std::runtime_error(LUA_SYMBOL_NE_LUA_STATE);
            else if (symbol_t != type::function)
                throw std::runtime_error(LUA_SYMBOL_WRONG_TYPE_FUNC);

            try {
                return L->call_symbol<T, Types...>(param...);
            }
            catch (std::runtime_error &what) {
                throw std::runtime_error(what.what() + (" " + name_sym));
            }
        }

        // returns the value of the variable pointed to by the symbol object
        template <typename T> T getval() const {
            if (!L->is_created())
                throw std::runtime_error(LUA_SYMBOL_NE_LUA_STATE);
            else if (symbol_t != type::variable)
                throw std::runtime_error(LUA_SYMBOL_WRONG_TYPE_VAR_VALUE);

            try {
                return L->get_valsymbol<T>();
            }
            catch (std::runtime_error &what) {
                throw std::runtime_error(what.what() + (" " + name_sym));
            }
        }

      private:
        std::string name_sym;
        type symbol_t;

        lua *L;
    };

    struct lcomp_anymap {
        bool operator()(const std::any &op1, const std::any &op2) const {
            if (op1.type() == typeid(std::string) || op1.type() == typeid(const char *)) {
                std::any tmp = op1;
                if (tmp.type() == typeid(const char *))
                    tmp = std::string(std::any_cast<const char *>(tmp));

                if (op2.type() == typeid(const char *))
                    return std::any_cast<std::string>(tmp) < std::string(std::any_cast<const char *>(op2));
                else
                    return std::any_cast<std::string>(tmp) < std::any_cast<std::string>(op2);
            }
            else if (op1.type() == typeid(integer) && op2.type() == typeid(integer))
                return std::any_cast<integer>(op1) < std::any_cast<integer>(op2);
            else if (op1.type() == typeid(number) && op2.type() == typeid(number))
                return std::any_cast<number>(op1) < std::any_cast<number>(op2);
            throw std::runtime_error(LUA_TABLE_KEYCMP + std::string(op1.type().name()));
        }
    };
    template <typename Key, typename Value> using keyValue = std::pair<Key, Value>;
    template <typename T> using array = std::vector<T>;
    template <typename Key, typename Value> using fastTable = std::vector<keyValue<Key, Value>>;
    template <typename Key, typename Value> using table = std::map<Key, Value, lcomp_anymap>;
    using cfunc = int (*)(lua_State *);

    template <typename> struct is_map : std::false_type {};
    template <typename K, typename V> struct is_map<table<K, V>> : std::true_type {};

    template <typename> struct is_vector : std::false_type {};
    template <typename U, typename A> struct is_vector<std::vector<U, A>> : std::true_type {};

    template <typename> struct is_pair : std::false_type {};
    template <typename K, typename V> struct is_pair<std::pair<K, V>> : std::true_type {};

    template <typename> struct is_vec_pairs : std::false_type {};
    template <typename A, typename K, typename V>
    struct is_vec_pairs<std::vector<std::pair<K, V>, A>> : std::true_type {};

    typedef ptrdiff_t integer;
    typedef double number;

    friend struct symbol;
    template <typename T> friend inline void tools::push_stack(lua_State *L, T param);
    template <typename T> friend inline T tools::pop_stack(lua_State *L, int idx);
    template <typename T, typename... Types> friend inline T tools::call_function(lua_State *L, Types... param);

  public:
    template <typename Key, typename Value> static std::map<Key, Value> to_map(table<Key, Value> _table) {
        std::map<Key, Value> _map;
        for (const auto &it : _table)
            _map[it.first] = it.second;
        return _map;
    }
    template <typename Key, typename Value> static table<Key, Value> to_table(std::map<Key, Value> _map) {
        table<Key, Value> _table;
        for (const auto &it : _map)
            _table[it.first] = it.second;
        return _table;
    }

  public:
    std::string get_stack__nmutex() {
        std::string str = "STACK(" + std::to_string(lua_gettop(L)) + "):\n";
        for (ptrdiff_t i = 0; i < lua_gettop(L); ++i) {
            if (i < lua_gettop(L) - 1)
                str += "| " + std::to_string(lua_gettop(L) - i) + " ";
            else
                str += "V " + std::to_string(lua_gettop(L) - i) + " ";

            if (lua_isboolean(L, -i) || lua_isnumber(L, -i))
                str += "[NUMBER] " + std::to_string(lua_tonumber(L, -i));
            else if (lua_isstring(L, -i))
                str += "[STRING] " + std::string(lua_tostring(L, -i));
            else if (lua_istable(L, -i))
                str += "[TABLE]";
            else if (lua_isfunction(L, -i))
                str += "[FUNCTION]";
            else if (lua_isnoneornil(L, -i))
                str += "[NIL]";
            else
                str += "???";

            if (i < lua_gettop(L) - 1)
                str += "\n";
        }

        return str;
    }

  public:
    symbol operator[](const std::string &&name_sym) {
        return get_symbol(std::move(name_sym));
    }

  public:
    // Initializes Lua state
    void create(std::string src) {
        if (L)
            lua_close(L);
        L = luaL_newstate();
        if (luaL_loadstring(L, src.c_str()) != LUA_OK || lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK)
            LUA_EXCEPTION()
    }

    // Resets Lua state
    void close() {
        if (L) {
            lua_close(L);
            L = NULL;
        }
    }

    // Includes lua libs
    void include_libs() {
        luaL_openlibs(L);
    }

    // Interprets (executes) the code that was provided when initializing the lua state
    void run_script() {
        if (!is_created())
            return;
        else if (lua_pcall(L, 0, 0, 0) != LUA_OK)
            LUA_EXCEPTION()
    }

    // Calls a function, provided that it exists and all parameters match.
    // The first parameter of the template is the return type of the function. The remaining parameters are the types of
    // function parameters accepted
    template <typename T, typename... Types> T call_function(std::string name_func, Types... param) {
        if (!is_created())
            return T{};
        lua_getglobal(L, name_func.c_str());
        try {
            return call_symbol<T, Types...>(param...);
        }
        catch (std::runtime_error &what) {
            throw std::runtime_error(what.what() + (" " + name_func));
        }
    }

    void register_function(std::string name_func, cfunc impl_cfunc) {
        lua_pushcfunction(L, impl_cfunc);
        lua_setglobal(L, name_func.c_str());
    }

    // execute lua code as a string
    void execute_str(std::string str_code) {
        if (!is_created())
            return;
        if (luaL_dostring(L, str_code.c_str()) != LUA_OK)
            LUA_EXCEPTION()
    }

    // gets the value of a variable of the given type
    // Possible types: string, integer, number, boolean and table
    template <typename T> T get_var(std::string name_var) {
        lua_getglobal(L, name_var.c_str());

        try {
            return get_valsymbol<T>();
        }
        catch (std::runtime_error &what) {
            throw std::runtime_error(what.what() + (" " + name_var));
        }
    }

    // creates a global variable with the provided name
    template <typename T> void create_var(std::string name_var, T &&value = {}) {
        try {
            push_stack_param(value);
        }
        catch (std::runtime_error &what) {
            throw std::runtime_error(what.what() + (" " + name_var));
        }
        lua_setglobal(L, name_var.c_str());
    }

    // Returns true if lua state is initialized
    inline bool is_created() {
        if (!L)
            return 0;
        return 1;
    }

    // Returns true if exists function(global) in lua state
    inline bool is_function(std::string name_func) {
        if (!is_created())
            return 0;
        else {
            lua_getglobal(L, name_func.c_str());

            if (!lua_isfunction(L, -1)) {
                lua_pop(L, 1);
                return 0;
            }
            lua_pop(L, 1);
        }
        return 1;
    }

    // Returns true if exists variable(global) in lua state
    inline bool is_var(std::string name_var) {
        if (!is_created())
            return 0;
        else {
            lua_getglobal(L, name_var.c_str());

            if (lua_isnoneornil(L, -1))
                return 0;
        }
        return 1;
    }

  private:
    template <typename Tuple, size_t... ind> void pre_init_stack(const Tuple &tp, std::index_sequence<ind...>) {
        ((push_stack_param<std::tuple_element_t<ind, Tuple>>(std::get<ind>(tp))), ...);
    }
    template <typename T> void push_stack_param(T param) {
        if constexpr (std::is_same_v<T, const char *>)
            lua_pushstring(L, param);
        else if constexpr (std::is_same_v<T, integer>)
            lua_pushinteger(L, param);
        else if constexpr (std::is_same_v<T, number>)
            lua_pushnumber(L, param);
        else if constexpr (std::is_same_v<T, std::string>)
            lua_pushstring(L, param.c_str());
        else if constexpr (is_map<T>::value) {
            lua_createtable(L, 0, param.size());
            for (const auto &[key, value] : param) {
                push_stack_param(key);
                push_stack_param(value);
                lua_settable(L, -3);
            }
        }
        else if constexpr (is_vector<T>::value) {
            lua_createtable(L, 0, param.size());
            for (size_t i = 0; i < param.size(); ++i) {
                push_stack_param(param[i]);
                lua_rawseti(L, -2, i + 1);
            }
        }
        else if constexpr (is_pair<T>::value) {
            lua_createtable(L, 0, 2);
            push_stack_param(param.first);
            push_stack_param(param.second);
            lua_settable(L, -3);
        }
        else if constexpr (std::is_same_v<T, std::any>) {
            if (param.type() == typeid(const char *))
                lua_pushstring(L, std::any_cast<const char *>(param));
            else if (param.type() == typeid(integer))
                lua_pushinteger(L, std::any_cast<integer>(param));
            else if (param.type() == typeid(number))
                lua_pushnumber(L, std::any_cast<number>(param));
            else if (param.type() == typeid(std::string))
                lua_pushstring(L, std::any_cast<std::string>(param).c_str());

            else if (param.type() == typeid(std::vector<integer>))
                push_stack_param(std::any_cast<std::vector<integer>>(param));
            else if (param.type() == typeid(std::vector<number>))
                push_stack_param(std::any_cast<std::vector<number>>(param));
            else if (param.type() == typeid(std::vector<const char *>))
                push_stack_param(std::any_cast<std::vector<const char *>>(param));
            else if (param.type() == typeid(std::vector<std::string>))
                push_stack_param(std::any_cast<std::vector<std::string>>(param));
            else if (param.type() == typeid(std::vector<std::any>))
                push_stack_param(std::any_cast<std::vector<std::any>>(param));

            else if (param.type() == typeid(table<std::string, std::string>))
                push_stack_param(std::any_cast<table<std::string, std::string>>(param));
            else if (param.type() == typeid(table<const char *, const char *>))
                push_stack_param(std::any_cast<table<const char *, const char *>>(param));
            else if (param.type() == typeid(table<std::string, integer>))
                push_stack_param(std::any_cast<table<std::string, integer>>(param));
            else if (param.type() == typeid(table<std::string, number>))
                push_stack_param(std::any_cast<table<std::string, integer>>(param));
            else if (param.type() == typeid(table<integer, std::any>))
                push_stack_param(std::any_cast<table<integer, std::any>>(param));
            else if (param.type() == typeid(table<number, std::any>))
                push_stack_param(std::any_cast<table<number, std::any>>(param));
            else if (param.type() == typeid(table<std::string, std::any>))
                push_stack_param(std::any_cast<table<std::string, std::any>>(param));
            else if (param.type() == typeid(table<const char *, std::any>))
                push_stack_param(std::any_cast<table<const char *, std::any>>(param));

            else if (param.type() == typeid(keyValue<std::string, std::string>))
                push_stack_param(std::any_cast<keyValue<std::string, std::string>>(param));
            else if (param.type() == typeid(keyValue<std::string, integer>))
                push_stack_param(std::any_cast<keyValue<std::string, integer>>(param));
            else if (param.type() == typeid(keyValue<integer, std::string>))
                push_stack_param(std::any_cast<keyValue<integer, std::string>>(param));
            else if (param.type() == typeid(keyValue<std::string, integer>))
                push_stack_param(std::any_cast<keyValue<std::string, integer>>(param));
            else if (param.type() == typeid(keyValue<integer, integer>))
                push_stack_param(std::any_cast<keyValue<integer, integer>>(param));
            else if (param.type() == typeid(keyValue<std::any, std::any>))
                push_stack_param(std::any_cast<keyValue<std::any, std::any>>(param));
            else
                throw std::runtime_error(LUA_PUSH_TYPE_UNK);
        }
        else if constexpr (std::is_same_v<T, nil>)
            lua_pushnil(L);
        else
            throw std::runtime_error(LUA_PUSH_TYPE_UNK);
    }
    template <typename T> T get_valsymbol(int idx = -1, int pop_last = 1) {
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
        else if constexpr (std::is_same_v<T, std::any>) {
            if (lua_isnumber(L, idx))
                value = lua_tonumber(L, idx);
            else if (lua_isstring(L, idx))
                value = std::string(lua_tostring(L, idx));
            else if (lua_isboolean(L, idx))
                value = lua_toboolean(L, idx);
            else if (lua_istable(L, idx)) {
                if (lua_isarray())
                    value = get_valsymbol<array<std::any>>(-1, 0);
                else if (lua_ispair())
                    value = get_valsymbol<keyValue<std::any, std::any>>(-1, 0);
                else
                    value = get_valsymbol<table<std::any, std::any>>(-1, 0);
            }
            else
                throw std::runtime_error(LUA_VARIABLE_UNK);
        }
        else if constexpr (is_vector<T>::value) {
            if (!lua_istable(L, idx))
                throw std::runtime_error(LUA_NFOUND_TABLE);

            T vec;
            if constexpr (is_pair<typename T::value_type>::value) {
                lua_pushnil(L);
                while (lua_next(L, -2)) {
                    vec.push_back(get_valsymbol<typename T::value_type>());
                    lua_pop(L, 1);
                }
            }
            else {
                size_t i = 1;
                while (1) {
                    lua_rawgeti(L, idx, i);
                    if (lua_isnil(L, -1)) {
                        lua_pop(L, 1);
                        break;
                    }
                    vec.push_back(get_valsymbol<typename T::value_type>());
                    ++i;
                }
            }
            value = vec;
        }
        else if constexpr (is_map<T>::value) {
            if (!lua_istable(L, idx))
                throw std::runtime_error(LUA_NFOUND_TABLE);

            T map;
            lua_pushnil(L);
            while (lua_next(L, -2)) {
                typename T::key_type key = get_valsymbol<typename T::key_type>(-2);
                map.emplace(key, get_valsymbol<typename T::mapped_type>());
            }
            value = map;
        }
        else if constexpr (is_pair<T>::value) {
            if (!lua_istable(L, idx))
                throw std::runtime_error(LUA_NFOUND_TABLE);

            lua_pushnil(L);

            if (lua_next(L, -2) == 0)
                return T();
            typename std::tuple_element<0, T>::type key = get_valsymbol<typename std::tuple_element<0, T>::type>(-2);
            value = std::make_pair<typename std::tuple_element<0, T>::type, typename std::tuple_element<1, T>::type>(
                std::move(key), get_valsymbol<typename std::tuple_element<1, T>::type>());
        }
        else
            throw std::runtime_error(LUA_VARIABLE_UNK);

        if (idx == -1 && pop_last || is_pair<T>::value)
            lua_pop(L, 1);
        if constexpr (std::is_same_v<T, std::any>)
            return value;
        return std::any_cast<T>(value);
    }

    // call symbol(function) in top of stack
    template <typename T, typename... Types> T call_symbol(Types... param) {
        std::tuple<Types...> params(param...);
        size_t count_params = std::tuple_size<decltype(params)>::value;

        pre_init_stack(params, std::index_sequence_for<Types...>{});

        if (!lua_isfunction(L, 1))
            throw std::runtime_error(LUA_FUNCTION_NFOUND);

        if constexpr (!std::is_same_v<T, void> && !std::is_same_v<T, nil>) {
            if (lua_pcall(L, count_params, 1, 0) != LUA_OK)
                LUA_EXCEPTION()
        }
        else if (lua_pcall(L, count_params, 0, 0) != LUA_OK)
            LUA_EXCEPTION()

        if constexpr (std::is_same_v<T, void> || std::is_same_v<T, nil>)
            return;
        else
            return get_valsymbol<T>();
    }

    // returns an object of class symbol, which is a temporary object (it points to a lua object)
    symbol get_symbol(const std::string &&name_sym) {
        lua_getglobal(L, name_sym.c_str());

        if (lua_isfunction(L, -1))
            return symbol(symbol::type::function, name_sym, this);
        else if (!lua_isnoneornil(L, -1))
            return symbol(symbol::type::variable, name_sym, this);
        return symbol(symbol::type::nil, name_sym, this);
    }

    // checks the last element (is it an array, that is, the index is the key)
    int lua_isarray() {
        lua_pushvalue(L, -1);
        lua_pushnil(L);
        while (lua_next(L, -2)) {
            if (lua_type(L, -2) != LUA_TNUMBER) {
                lua_pop(L, 3);
                return 0;
            }
            lua_pop(L, 1);
        }
        lua_pop(L, 1);

        return 1;
    }

    // checks that the last element is a table with one key-value pair
    int lua_ispair() {
        lua_pushvalue(L, -1);
        lua_pushnil(L);
        lua_next(L, -2);
        lua_pop(L, 1);

        if (!lua_next(L, -2)) {
            lua_pop(L, 1);
            return 1;
        }

        lua_pop(L, 3);

        return 0;
    }

  private:
    lua_State *L{NULL};
    std::mutex lmutex;
};

namespace tools {
template <typename T> inline void push_stack(lua_State *L, T param) {
    lua(L).push_stack_param(param);
}
template <typename T> inline T pop_stack(lua_State *L, int idx) {
    return lua(L).template get_valsymbol<T>(idx);
}

// Calls a function that is on the top of the stack
template <typename T, typename... Types> inline T call_function(lua_State *L, Types... param) {
    return lua(L).template call_symbol<T>(param...);
}
} // namespace tools

} // namespace bwlua

#endif