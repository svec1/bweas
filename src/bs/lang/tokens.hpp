//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses />
// ------------------------------------------
//

#ifndef TOKENS_HPP
#define TOKENS_HPP

#include <bwaliases.hpp>

namespace bwlang {

namespace tokens {
namespace string_matching {

inline constexpr char _empty[] = "";

inline constexpr char _is[]         = "is";
inline constexpr char _not[]        = "not";
inline constexpr char _import[]     = "import";
inline constexpr char _func[]       = "func";
inline constexpr char _endfunc[]    = "endfunc";
inline constexpr char _return[]     = "return";
inline constexpr char _if[]         = "if";
inline constexpr char _else[]       = "else";
inline constexpr char _endif[]      = "endif";
inline constexpr char number_t[]    = "number";
inline constexpr char string_t[]    = "string";
inline constexpr char cc_t[]        = "cc";
inline constexpr char ctemplate_t[] = "ctemplate";
inline constexpr char language_t[]  = "language";
inline constexpr char profile_t[]   = "profile";
inline constexpr char target_t[]    = "target";

static constexpr array<string_v, 16> keywords = {_is,         _not,       _import,   _func,    _endfunc, _if,
                                                 _return,     _else,      _endif,    number_t, string_t, cc_t,
                                                 ctemplate_t, language_t, profile_t, target_t};

static constexpr array<string_v, 7> types{number_t, string_t, cc_t, ctemplate_t, language_t, profile_t, target_t};

} // namespace string_matching

struct number_value {
    number_value(pdiff _value) : value(_value) {
    }

    pdiff value;
};
struct string_value {
    string_value(string_v _value) : value(_value) {
    }

    string value;
};

template <const char *Value = string_matching::_empty> struct keyword : public string_value {
    keyword(string_v value = string_matching::_empty) : string_value(value.empty() ? Value : value) {
    }

    bool operator==(const keyword &kw) {
        return this->value == kw->value;
    }

    static constexpr const char *s_value = Value;
};
struct identifier : public string_value {
    identifier(string_v value) : string_value(value) {
    }
};
struct literal_number : public number_value {
    literal_number() : number_value(0) {
    }

    literal_number(pdiff value) : number_value(value) {
    }
};
struct literal_string : public string_value {
    literal_string() : string_value("") {
    }
    literal_string(string_v value) : string_value(value) {
    }
};

struct end_line {};
struct open_init_bracket {};   // {
struct open_round_bracket {};  // (
struct open_square_bracket {}; // [
struct close_init_bracket {};
struct close_round_bracket {};
struct close_square_bracket {};
struct comma {};     // ,
struct dot {};       // .
struct init_type {}; // :
struct equal {};     // =
struct plus {};      // +
struct minus {};     // -
struct multiply {};  // *
struct divide {};    // /
struct less {};      // <
struct more {};      // >

struct _is : public keyword<string_matching::_is> {};
struct _not : public keyword<string_matching::_not> {};
struct _import : public keyword<string_matching::_import> {};
struct _func : public keyword<string_matching::_func> {};
struct _endfunc : public keyword<string_matching::_endfunc> {};
struct _return : public keyword<string_matching::_return> {};
struct _if : public keyword<string_matching::_if> {};
struct _else : public keyword<string_matching::_else> {};
struct _endif : public keyword<string_matching::_endif> {};
struct number_t : public keyword<string_matching::number_t> {};
struct string_t : public keyword<string_matching::string_t> {};
struct cc_t : public keyword<string_matching::cc_t> {};
struct ctemplate_t : public keyword<string_matching::ctemplate_t> {};
struct language_t : public keyword<string_matching::language_t> {};
struct profile_t : public keyword<string_matching::profile_t> {};
struct target_t : public keyword<string_matching::target_t> {};

using token_value =
    std::variant<std::monostate, keyword<>, identifier, literal_string, literal_number, end_line, open_init_bracket,
                 open_round_bracket, open_square_bracket, close_init_bracket, close_round_bracket, close_square_bracket,
                 comma, dot, init_type, equal, plus, minus, multiply, divide, less, more>;

struct token {
    token() = default;
    token(token_value _value, size_t _line_index) : value(_value), line_index(_line_index) {
    }
    token(token_value &&_value) : value(_value) {
    }

  public:
    template <typename T> T &get() {
        return std::get<T>(value);
    }
    template <typename T> const T &get() const {
        return std::get<T>(value);
    }
    template <typename T> bool is() const {
        return std::holds_alternative<T>(value);
    }

  public:
    token_value value;
    size_t line_index;
};

template <typename Kw> static constexpr bool is_keyword(const token &tk) {
    return tk.is<keyword<>>() && tk.get<keyword<>>().value == Kw::s_value;
}
template <typename Kw> static constexpr bool is_keyword(const token_value &tk) {
    return std::holds_alternative<keyword<>>(tk) && std::get<keyword<>>(tk).value == Kw::s_value;
}

static constexpr bool is_type(string kw) {
    return std::find_if(string_matching::types.begin(), string_matching::types.end(),
                        [&](string_v type) { return kw == type; }) != string_matching::types.end();
}

static inline constexpr string get_string(token tk) {
    return std::visit(
        [](auto &token) -> string {
            using Type = typename std::decay_t<decltype(token)>;
            if constexpr (std::is_same_v<Type, keyword<>> || std::is_same_v<Type, identifier>)
                return token.value;
            else if constexpr (std::is_same_v<Type, literal_number>)
                return std::to_string(token.value);
            else if constexpr (std::is_same_v<Type, literal_string>)
                return "\'" + token.value + "\'";
            else if constexpr (std::is_same_v<Type, end_line>)
                return "\n";
            else if constexpr (std::is_same_v<Type, open_init_bracket>)
                return "{";
            else if constexpr (std::is_same_v<Type, open_round_bracket>)
                return "(";
            else if constexpr (std::is_same_v<Type, open_square_bracket>)
                return "[";
            else if constexpr (std::is_same_v<Type, close_init_bracket>)
                return "}";
            else if constexpr (std::is_same_v<Type, close_round_bracket>)
                return ")";
            else if constexpr (std::is_same_v<Type, close_square_bracket>)
                return "]";
            else if constexpr (std::is_same_v<Type, comma>)
                return ",";
            else if constexpr (std::is_same_v<Type, dot>)
                return ".";
            else if constexpr (std::is_same_v<Type, init_type>)
                return ":";
            else if constexpr (std::is_same_v<Type, equal>)
                return "=";
            else if constexpr (std::is_same_v<Type, plus>)
                return "+";
            else if constexpr (std::is_same_v<Type, minus>)
                return "-";
            else if constexpr (std::is_same_v<Type, multiply>)
                return "*";
            else if constexpr (std::is_same_v<Type, divide>)
                return "/";
            else if constexpr (std::is_same_v<Type, less>)
                return "<";
            else if constexpr (std::is_same_v<Type, more>)
                return ">";
            else
                return "";
        },
        tk.value);
}
} // namespace tokens
} // namespace bwlang
#endif
