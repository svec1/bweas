#ifndef PARSER_UTILS_HPP
#define PARSER_UTILS_HPP

#include <bwstructs_context.hpp>
#include <lang/tokens.hpp>

namespace bwlang {

namespace parser_utils {

template <typename... Types> struct func_wrapper : Types... {
    using Types::operator()...;
};

template <typename... Types> func_wrapper(Types...) -> func_wrapper<Types...>;

class parser_error : public std::exception {
  public:
    parser_error(string _what_str, tokens::token _tk = {}) noexcept : what_str(_what_str), tk(_tk) {
    }

    ~parser_error() override = default;

  public:
    const char *what() const noexcept override {
        return what_str.c_str();
    }

    string get_token() {
        return tokens::get_string(tk);
    }

  private:
    string what_str;
    tokens::token tk;
};

namespace basic_operation {

template <typename TArgs, typename R> struct binary_operation {
    using arguments_type = TArgs;
    using result_type    = R;
};

template <typename TArgs> struct binary_operation_one_type : binary_operation<TArgs, TArgs> {};

template <typename T> struct equals : binary_operation<T, pdiff> {
    static constexpr binary_operation<T, pdiff>::result_type operator()(T v1, T v2) {
        return v1 == v2;
    }
};
template <typename T> struct plus : binary_operation_one_type<T> {
    static constexpr binary_operation_one_type<T>::result_type operator()(T v1, T v2) {
        return v1 + v2;
    }
};
template <typename T> struct minus : binary_operation_one_type<T> {
    static constexpr binary_operation_one_type<T>::result_type operator()(T v1, T v2) {
        return v1 - v2;
    }
};
template <typename T> struct multiplies : binary_operation_one_type<T> {
    static constexpr binary_operation_one_type<T>::result_type operator()(T v1, T v2) {
        return v1 * v2;
    }
};
template <typename T> struct divides : binary_operation_one_type<T> {
    static constexpr binary_operation_one_type<T>::result_type operator()(T v1, T v2) {
        return v1 / v2;
    }
};
} // namespace basic_operation

using access = std::pair<string, string>;
using match_pack =
    umap<string, std::variant<std::monostate, pdiff, string, vec<pdiff>, vec<string>, bweas::sc::profile>>;
using value = std::variant<std::monostate, pdiff, string, bweas::sc::call_component, bweas::sc::template_command,
                           bweas::sc::profile, bweas::sc::target, vec<pdiff>, vec<string>, vec<bweas::sc::profile>,
                           vec<bweas::sc::target>, access>;
using scope = umap<string, value>;
struct func {
    using func_t = std::function<value(string, scope &)>;

    func() = default;
    func(func_t _ref, vec<string> _name_args, bool _declared) : ref(_ref), name_args(_name_args), declared(_declared) {
    }

  public:
    func_t ref;
    vec<string> name_args;
    bool declared = false;
};
struct context {
    context(scope &_sc) : sc(&_sc) {
    }

  public:
    scope *sc;
    umap<string, func> funcs;
};

template <typename T> static constexpr string get_type_name() {
    if constexpr (std::is_same_v<T, pdiff>)
        return "number";
    else if constexpr (std::is_same_v<T, string>)
        return "string";
    else if constexpr (std::is_same_v<T, vec<pdiff>>)
        return "[number]";
    else if constexpr (std::is_same_v<T, vec<string>>)
        return "[string]";
    else if constexpr (std::is_same_v<T, bweas::sc::call_component>)
        return "call_component";
    else if constexpr (std::is_same_v<T, bweas::sc::template_command>)
        return "template_command";
    else if constexpr (std::is_same_v<T, bweas::sc::profile>)
        return "profile";
    else if constexpr (std::is_same_v<T, bweas::sc::target>)
        return "target";
    else
        return "undefined";
}

template <typename T> value get_field(T &obj, string name) {
    using namespace bweas;

    if constexpr (std::is_same_v<T, sc::call_component>) {
        if (name == "name")
            return obj.name;
        else if (name == "program")
            return obj.name_program;
        else if (name == "pattern_files")
            return obj.pattern_ret_files;
        else
            throw std::runtime_error("A non-existent field.");
    }
    else if constexpr (std::is_same_v<T, sc::template_command>) {
        if (name == "name")
            return obj.name;
        else
            throw std::runtime_error("A non-existent field.");
    }
    else if constexpr (std::is_same_v<T, sc::target>) {
        if (name == "name")
            return obj.name;
        else if (name == "type")
            return (pdiff)obj.type;
        else if (name == "templates")
            return obj.templates;
        else if (name == "dependencies")
            return obj.dependencies;
        else
            return std::visit([](auto &&val) -> value { return val; }, obj.ext.get_fields()[name]);
    }
    else
        throw std::runtime_error("Unexpected type.");
}

template <typename T> void set_field(T &obj, string name, value val) {
    using namespace bweas;

    if constexpr (std::is_same_v<T, sc::call_component>) {
        if (!std::holds_alternative<string>(val))
            throw std::runtime_error("Undefined type for match.");
        if (name == "name")
            obj.name = std::get<string>(val);
        else if (name == "program")
            obj.name_program = std::get<string>(val);
        else if (name == "pattern_files")
            obj.pattern_ret_files = std::get<string>(val);
        else
            throw std::runtime_error("A non-existent field.");
    }
    else if constexpr (std::is_same_v<T, sc::template_command>) {
        if (!std::holds_alternative<string>(val))
            throw std::runtime_error("Undefined type for match.");
        if (name == "name")
            obj.name = std::get<string>(val);
        else if (name == "str")
            obj = sc::template_command::create_template_command(obj.name, std::get<string>(val));
        else
            throw std::runtime_error("A non-existent field.");
    }
    else if constexpr (std::is_same_v<T, sc::target>) {
        if (name == "name") {
            if (!std::holds_alternative<string>(val))
                throw std::runtime_error("Undefined type for match.");
            obj.name = std::get<string>(val);
        }
        else if (name == "profile") {
            if (!std::holds_alternative<sc::profile>(val))
                throw std::runtime_error("Undefined type for match.");
            obj.ext.merge(std::get<sc::profile>(val));
        }
        else if (name == "type") {
            if (!std::holds_alternative<pdiff>(val))
                throw std::runtime_error("Undefined type for match.");
            obj.type = (sc::target::e_type)std::get<pdiff>(val);
        }
        else if (name == "cfg") {
            if (!std::holds_alternative<pdiff>(val))
                throw std::runtime_error("Undefined type for match.");
            obj.ext.set_fields(std::get<pdiff>(val));
        }
        else if (name == "templates") {
            if (!std::holds_alternative<vec<string>>(val))
                throw std::runtime_error("Undefined type for match.");
            obj.templates = std::get<vec<string>>(val);
        }
        else if (name == "dependencies") {
            if (!std::holds_alternative<vec<string>>(val))
                throw std::runtime_error("Undefined type for match.");
            obj.dependencies = std::get<vec<string>>(val);
        }
        else {
            if (std::holds_alternative<string>(val))
                obj.ext.get_fields()[name] = std::get<string>(val);
            else if (std::holds_alternative<vec<string>>(val))
                obj.ext.get_fields()[name] = std::get<vec<string>>(val);
            else
                throw std::runtime_error("A non-existent field \'" + name + "\'.");
        }
    }
    else
        throw std::runtime_error("Unexpected type.");
}

} // namespace parser_utils
} // namespace bwlang

#endif
