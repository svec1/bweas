#include <lang/lexer.hpp>

#include <cctype>

using namespace bwlang;

bool isnumber(string_v value) {
    for (auto ch : value)
        if (!std::isdigit(ch))
            return 0;
    return 1;
}

bool is_separator(char ch) {
    switch (ch) {
    case '\n':
    case '{':
    case '(':
    case '[':
    case '}':
    case ')':
    case ']':
    case ',':
    case '.':
    case ':':
    case '=':
    case '+':
    case '-':
    case '*':
    case '/':
    case '<':
    case '>':
        return 1;
    default:
        return 0;
    }
}
bool is_keyword(string_v value) {
    using namespace tokens::string_matching;
    for (const auto &kw : keywords)
        if (value == kw)
            return 1;
    return 0;
}

lexer::lexer(string_v src) {
    string tk_value;

    bool open_quote = 0;

    for (pdiff i = 0; i < src.size(); ++i) {
        if (!open_quote &&
            (is_separator(src[i]) || (!tk_value.empty() && is_separator(tk_value[0])) || std::isspace(src[i]))) {
            if (!tk_value.empty()) {
                tokens.push_back(get_token(tk_value));
                tk_value.clear();
            }

            if (std::isspace(src[i]) && src[i] != '\n')
                continue;
        }
        if (src[i] == '\'') {
            if ((open_quote = !open_quote) == 0) {
                tokens.push_back(get_token(tk_value, 1));
                tk_value.clear();
            }
            continue;
        }
        tk_value += src[i];
    }

    if (!tk_value.empty())
        tokens.push_back(get_token(tk_value));
}

tokens::token lexer::peek() {
    return tokens.size() ? tokens[0] : tokens::token{};
}

tokens::token lexer::consume() {
    if (!tokens.size())
        throw std::runtime_error("An invalid attempt to get the current token");

    tokens::token tmp = tokens[0];
    tokens.erase(tokens.begin());

    if (std::holds_alternative<tokens::end_line>(tmp)) {
        ++number_last_line;
        previous_line = string_last_line;
        string_last_line.clear();
    }
    else
        string_last_line += get_string(tmp);

    return tmp;
}
tokens::token lexer::consume_if() {
    return tokens.size() ? consume() : tokens::token{};
}

vec<tokens::token> &lexer::get_tokens() {
    return tokens;
}

tokens::token lexer::get_token(string_v value, bool in_quote) {
    if (in_quote)
        return tokens::literal_string(value);
    else if (std::isalpha(value[0])) {
        if (is_keyword(value))
            return tokens::keyword(value);
        return tokens::identifier(value);
    }
    else if (isnumber(value))
        return tokens::literal_number(std::stoi(value.data()));

    switch (value[0]) {
    case '\n':
        return tokens::end_line{};
    case '{':
        return tokens::open_init_bracket{};
    case '(':
        return tokens::open_round_bracket{};
    case '[':
        return tokens::open_square_bracket{};
    case '}':
        return tokens::close_init_bracket{};
    case ')':
        return tokens::close_round_bracket{};
    case ']':
        return tokens::close_square_bracket{};
    case ',':
        return tokens::comma{};
    case '.':
        return tokens::dot{};
    case ':':
        return tokens::init_type{};
    case '+':
        return tokens::plus{};
    case '-':
        return tokens::minus{};
    case '*':
        return tokens::multiply{};
    case '/':
        return tokens::devide{};
    case '<':
        return tokens::less{};
    case '>':
        return tokens::more{};
    case '=':
        return tokens::equal{};
    default:
        throw std::runtime_error("Invalid syntax: unknown character \'" + string(value) + "\'");
    }
}
