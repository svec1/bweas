//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses />
// ------------------------------------------
//

#ifndef PARSER_UTILS_HPP
#define PARSER_UTILS_HPP

#include <functional>

#include <bwstructs_context.hpp>
#include <lang/tokens.hpp>

namespace bwlang {
namespace parser_utils {
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
template <typename T, typename Op> struct binary_operator : binary_operation_one_type<T> {
    static constexpr binary_operation_one_type<T>::result_type operator()(T v1, T v2) {
        return Op{}(v1, v2);
    }
};
} // namespace basic_operation

template <typename... Types> struct func_wrapper : Types... {
    using Types::operator()...;
};
template <typename... Types> func_wrapper(Types...) -> func_wrapper<Types...>;

template <typename> struct is_vector : std::false_type {};
template <typename U, typename A> struct is_vector<std::vector<U, A>> : std::true_type {};

class parser_error : public std::exception {
  public:
    parser_error(string _what_str, tokens::token _tk = {}) noexcept : what_str(_what_str), tk(_tk) {
    }

    ~parser_error() override = default;

  public:
    const char *what() const noexcept override {
        return what_str.c_str();
    }

    tokens::token get_token() {
        return tk;
    }

  private:
    string what_str;
    tokens::token tk;
};
struct context;
struct func;

using value      = std::variant<std::monostate, pdiff, string, bweas::sc::call_component, bweas::sc::template_command,
                                bweas::sc::language, bweas::sc::profile, bweas::sc::target, vec<pdiff>, vec<string>,
                                vec<bweas::sc::call_component>, vec<bweas::sc::template_command>, vec<bweas::sc::language>,
                                vec<bweas::sc::profile>, vec<bweas::sc::target>, func>;
using match_pack = vec<pair<string, value>>;
using scope      = umap<string, value>;
struct func {
    using func_t = std::function<value(string, context &)>;

    func() = default;
    func(func_t _ref, vec<string> _name_args, bool _declared) : ref(_ref), name_args(_name_args), declared(_declared) {
    }

  public:
    func_t ref;
    vec<string> name_args;
    bool declared = false;
};
struct context {
    value &get(string_v name, bool is_local) {
        if (sc.contains(name.data()))
            return sc.at(name.data());
        else if (g_ctx && !is_local) {
            context *t_ctx = g_ctx;
            while (t_ctx && !t_ctx->sc.contains(name.data()))
                t_ctx = t_ctx->g_ctx;

            if (t_ctx)
                return t_ctx->sc.at(name.data());
        }
        throw std::runtime_error("Unknown variable \'" + string(name) + "\'.");
    }

  public:
    bool contains(string_v name, bool is_local) {
        if (g_ctx && !is_local) {
            context *t_ctx = g_ctx;
            while (t_ctx && !t_ctx->sc.contains(name.data()))
                t_ctx = t_ctx->g_ctx;

            if (t_ctx)
                return t_ctx->sc.contains(name.data());
        }
        return sc.contains(name.data());
    }

  public:
    context *g_ctx = nullptr;

    scope sc;
};

template <typename T> static constexpr string get_type_name() {
    if constexpr (is_vector<T>::value)
        return "[" + get_type_name<typename T::value_type>() + "]";

    if constexpr (std::is_same_v<T, pdiff>)
        return "number";
    else if constexpr (std::is_same_v<T, string>)
        return "string";
    else if constexpr (std::is_same_v<T, vec<pdiff>>)
        return "number";
    else if constexpr (std::is_same_v<T, vec<string>>)
        return "string";
    else if constexpr (std::is_same_v<T, bweas::sc::call_component>)
        return "call_component";
    else if constexpr (std::is_same_v<T, bweas::sc::template_command>)
        return "template_command";
    else if constexpr (std::is_same_v<T, bweas::sc::language>)
        return "language";
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
            throw parser_utils::parser_error("A non-existent field.");
    }
    else if constexpr (std::is_same_v<T, sc::template_command>) {
        if (name == "name")
            return obj.name;
        else
            throw parser_utils::parser_error("A non-existent field.");
    }
    else if constexpr (std::is_same_v<T, sc::language>) {
        if (name == "name")
            return obj.name;
        else if (name == "search_regex")
            return obj.dfinder_data.search_regex;
        else if (name == "char_global_search")
            return obj.dfinder_data.char_global_search;
        else
            throw parser_utils::parser_error("A non-existent field.");
    }
    else if constexpr (std::is_same_v<T, sc::profile>) {
        return std::visit([](auto &&val) -> value { return val; }, obj.get_fields()[name]);
    }
    else if constexpr (std::is_same_v<T, sc::target>) {
        if (name == "name")
            return obj.name;
        else if (name == "extension")
            return obj.ext;
        else if (name == "templates")
            return obj.templates;
        else if (name == "dependencies")
            return obj.dependencies;
        else
            return std::visit([](auto &&val) -> value { return val; }, obj.ext.get_fields()[name]);
    }
    else
        throw parser_utils::parser_error("The " + get_type_name<T>() + " type is not a structure.");
}

template <typename T> void set_field(T &obj, string name, value val) {
    using namespace bweas;

    if constexpr (std::is_same_v<T, sc::call_component>) {
        if (!std::holds_alternative<string>(val))
            throw parser_utils::parser_error("Expected " + get_type_name<string>() + " type.");
        if (name == "name")
            obj.name = std::get<string>(val);
        else if (name == "program")
            obj.name_program = std::get<string>(val);
        else if (name == "pattern_files")
            obj.pattern_ret_files = std::get<string>(val);
        else
            throw parser_utils::parser_error("A non-existent field.");
    }
    else if constexpr (std::is_same_v<T, sc::template_command>) {
        if (!std::holds_alternative<string>(val))
            throw parser_utils::parser_error("Expected " + get_type_name<string>() + " type.");
        if (name == "name")
            obj.name = std::get<string>(val);
        else if (name == "str")
            obj = sc::template_command::create_template_command(obj.name, std::get<string>(val));
        else
            throw parser_utils::parser_error("A non-existent field.");
    }
    else if constexpr (std::is_same_v<T, sc::language>) {
        if (!std::holds_alternative<string>(val))
            throw parser_utils::parser_error("Expected " + get_type_name<string>() + " type.");

        if (name == "name")
            obj.name = std::get<string>(val);
        else if (name == "search_regex")
            obj.dfinder_data.search_regex = std::get<string>(val);
        else if (name == "char_global_search")
            obj.dfinder_data.char_global_search = std::get<string>(val)[0];
        else
            throw parser_utils::parser_error("A non-existent field.");
    }
    else if constexpr (std::is_same_v<T, sc::profile>) {
        if (name == "derive") {
            if (!std::holds_alternative<sc::profile>(val))
                throw parser_utils::parser_error("Expected " + get_type_name<sc::profile>() + " type.");
            obj.merge(std::get<sc::profile>(val));
        }
        else if (name == "lang") {
            if (!std::holds_alternative<sc::language>(val))
                throw parser_utils::parser_error("Expected " + get_type_name<sc::language>() + " type.");

            obj.lang = std::get<sc::language>(val);
        }
        else if (name == "cfg") {
            if (!std::holds_alternative<pdiff>(val))
                throw parser_utils::parser_error("Expected " + get_type_name<pdiff>() + " type.");
            obj.set_fields(std::get<pdiff>(val));
        }
        else {
            bool exists = obj.get_fields().contains(name);
            std::visit(
                [&](auto &&var) {
                    using T2 = std::decay_t<decltype(var)>;

                    if (exists && !std::holds_alternative<T2>(val))
                        throw parser_utils::parser_error("Expected " + get_type_name<T2>() + " type.");
                    else if (!exists)
                        obj.get_fields()[name] = std::visit(
                            func_wrapper{[](pdiff &val) -> sc::profile::fields::mapped_type { return val; },
                                         [](string &val) -> sc::profile::fields::mapped_type { return val; },
                                         [](vec<string> &val) -> sc::profile::fields::mapped_type { return val; },
                                         [](auto &&val) -> sc::profile::fields::mapped_type {
                                             throw parser_utils::parser_error(
                                                 "Unexpected type \'" + get_type_name<std::decay_t<decltype(val)>>() +
                                                 "\'");
                                         }},
                            val);
                    else
                        var = std::get<T2>(val);
                },
                obj.get_fields()[name]);
        }
    }
    else if constexpr (std::is_same_v<T, sc::target>) {
        if (name == "name") {
            if (!std::holds_alternative<string>(val))
                throw parser_utils::parser_error("Expected " + get_type_name<string>() + " type.");
            obj.name = std::get<string>(val);
        }
        else if (name == "extension") {
            if (!std::holds_alternative<sc::profile>(val))
                throw parser_utils::parser_error("Expected " + get_type_name<sc::profile>() + " type.");
            obj.ext = std::get<sc::profile>(val);
        }
        else if (name == "cfg") {
            if (!std::holds_alternative<pdiff>(val))
                throw parser_utils::parser_error("Expected " + get_type_name<pdiff>() + " type.");
            obj.ext.set_fields(std::get<pdiff>(val));
        }
        else if (name == "templates") {
            if (!std::holds_alternative<vec<string>>(val))
                throw parser_utils::parser_error("Expected " + get_type_name<vec<string>>() + " type.");
            obj.templates = std::get<vec<string>>(val);
        }
        else if (name == "dependencies") {
            if (!std::holds_alternative<vec<string>>(val))
                throw parser_utils::parser_error("Expected " + get_type_name<vec<string>>() + " type.");
            obj.dependencies = std::get<vec<string>>(val);
        }
        else {
            bool exists = obj.ext.get_fields().contains(name);
            std::visit(
                [&](auto &&var) {
                    using T2 = std::decay_t<decltype(var)>;
                    if (exists && !std::holds_alternative<T2>(val))
                        throw parser_utils::parser_error("Expected " + get_type_name<T2>() + " type.");
                    else if (!exists)
                        obj.ext.get_fields()[name] = std::visit(
                            func_wrapper{[](pdiff &val) -> sc::profile::fields::mapped_type { return val; },
                                         [](string &val) -> sc::profile::fields::mapped_type { return val; },
                                         [](vec<string> &val) -> sc::profile::fields::mapped_type { return val; },
                                         [](auto &&val) -> sc::profile::fields::mapped_type {
                                             throw parser_utils::parser_error(
                                                 "Unexpected type \'" + get_type_name<std::decay_t<decltype(val)>>() +
                                                 "\'");
                                         }},
                            val);
                    else
                        var = std::get<T2>(val);
                },
                obj.ext.get_fields()[name]);
        }
    }
    else
        throw parser_utils::parser_error("Unexpected " + get_type_name<T>() + " type.");
}

} // namespace parser_utils
} // namespace bwlang

#endif
