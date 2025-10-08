//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses />
// ------------------------------------------
//

#include <lang/parser.hpp>

using namespace bwlang;
using namespace expression::ext;

static parser_utils::context g_ctx_s;

parser::parser(string_v src) : lexer(src), g_ctx(&g_ctx_s) {
}

void parser::dump_global_context() {
    g_ctx_s.sc.clear();
}
void parser::parse() {
    try {
        parse_statements();
    }
    catch (parser_utils::parser_error &excp) {
        auto tk               = excp.get_token();
        string tk_number_line = std::to_string(tk.line_index);
        string tk_line        = get_string_line(tk);
        string tk_string      = get_string(tk);

        string output;
        if (pdiff it = tk_line.find(tk_string); tk_string.size() && it != tk_line.npos) {
            output += tk_number_line + ":    " + tk_line + "\n" + string(it + tk_number_line.size() + 5, ' ') +
                      string(tk_string.size(), '^');
        }
        else
            output += tk_number_line + ":    " + tk_line + "\n" + string(tk_number_line.size() + 5, ' ') +
                      string(tk_line.size(), '^');
        output = excp.what() + string("\n") + output;

        throw std::runtime_error(output);
    }
}
parser_utils::value parser::parse_statements(bool skip, bool is_branche, bool is_func) {
    parser_utils::value return_value;
    while (get_tokens().size()) {
        tokens::token next_token = peek();
        if (((tokens::is_keyword<tokens::_endif>(next_token) || tokens::is_keyword<tokens::_else>(next_token)) &&
             is_branche) ||
            (tokens::is_keyword<tokens::_endfunc>(next_token) && is_func))
            return return_value;

        std::visit(parser_utils::func_wrapper{[&](tokens::end_line) { consume(); },
                                              [&](tokens::keyword<> kw) {
                                                  consume();
                                                  if (tokens::is_keyword<tokens::_if>(kw))
                                                      return_value = parse_if_else_branche(skip, is_func);
                                                  else {
                                                      if (skip)
                                                          consume();
                                                      else if (tokens::is_keyword<tokens::_import>(kw))
                                                          parse_import();
                                                      else if (tokens::is_keyword<tokens::_func>(kw))
                                                          parse_function();
                                                      else if (tokens::is_keyword<tokens::_return>(kw) && is_func)
                                                          return_value = parse_expression()->get_value();
                                                      else
                                                          throw parser_utils::parser_error("Undefined keyword.",
                                                                                           next_token);
                                                  }
                                              },
                                              [&](auto &&) {
                                                  if (skip)
                                                      consume();
                                                  else
                                                      parse_expression();
                                              }},
                   next_token.value);

        if (!std::holds_alternative<std::monostate>(return_value))
            break;
    }

    skip_token_end_line();
    return return_value;
}

parser_utils::value parser::parse_if_else_branche(bool in_skip_branche, bool is_func) {
    expect_token<tokens::open_round_bracket>();

    auto expr  = parse_expression();
    auto value = expr->get_value();

    if (!std::holds_alternative<pdiff>(value))
        throw parser_utils::parser_error("Expected number expression.", peek());

    expect_token<tokens::close_round_bracket>();

    bool skip_branche                = in_skip_branche || !(bool)std::get<pdiff>(value);
    parser_utils::value return_value = parse_statements(skip_branche, true, is_func);

    if (tokens::is_keyword<tokens::_else>(peek())) {
        consume();
        parser_utils::value return_value_else = parse_statements(!skip_branche, true, is_func);
        if (!std::holds_alternative<std::monostate>(return_value_else))
            return_value = return_value_else;
    }

    expect_keyword<tokens::_endif>();

    return return_value;
}
void parser::parse_import() {
    tokens::token current_token = expect_token<tokens::literal_string>();
    auto it                     = std::find_if(modules.begin(), modules.end(),
                                               [name_module = current_token.get<tokens::literal_string>().value](
                               const bweas::module_manager::_module &md) { return md.name == name_module; });
    if (it == modules.end())
        throw parser_utils::parser_error("Unknown module.", current_token);

    g_ctx->sc.merge(it->ctx.sc);
}
void parser::parse_function() {
    static umap<string, vec<tokens::token>> decl_funcs;

    vec<string> args;
    string name_func = expect_identifier().value;
    expect_token<tokens::open_round_bracket>();

    tokens::token current_token = peek();
    while (!current_token.is<tokens::close_round_bracket>()) {
        args.push_back(expect_identifier().value);

        current_token = expect_tokens<tokens::comma, tokens::close_round_bracket>();
    }

    std::function<void()> copy_all_token = [&]() {
        while (!tokens::is_keyword<tokens::_endfunc>(current_token)) {
            if (current_token = consume_if(); current_token.is<std::monostate>())
                expect_keyword<tokens::_endfunc>();

            if (tokens::is_keyword<tokens::_func>(current_token)) {
                decl_funcs[name_func].push_back(current_token);
                copy_all_token();
                current_token = consume_if();
            }

            decl_funcs[name_func].push_back(current_token);
        }
    };

    copy_all_token();

    g_ctx->sc[name_func] = parser_utils::func{
        [&](string name_func, parser_utils::context &c_ctx) -> parser_utils::value {
            get_tokens().insert(get_tokens().begin(), decl_funcs.at(name_func).begin(), decl_funcs.at(name_func).end());

            g_ctx                            = &c_ctx;
            parser_utils::value return_value = parse_statements(false, false, true);
            g_ctx                            = &g_ctx_s;
            if (!std::holds_alternative<std::monostate>(return_value))
                while (!peek().is<std::monostate>() && !tokens::is_keyword<tokens::_endfunc>(peek()))
                    consume();

            if (!tokens::is_keyword<tokens::_endfunc>(consume_if()))
                throw parser_utils::parser_error("Expected \'endfunc\' keyword.", peek());

            return return_value;
        },
        args, true};
}

std::unique_ptr<base> parser::parse_expression(pdiff lbinding_power) {
    static string lhs_id;

    tokens::token current_token = consume();
    auto lhs                    = std::visit(
        parser_utils::func_wrapper{
            [&](tokens::minus) -> std::unique_ptr<base> {
                return std::make_unique<unary_operation::negative>(*g_ctx, current_token, parse_expression(3));
            },
            [&](tokens::literal_number lnum) -> std::unique_ptr<base> {
                return std::make_unique<constant>(*g_ctx, current_token, lnum.value);
            },
            [&](tokens::literal_string lstr) -> std::unique_ptr<base> {
                return std::make_unique<constant>(*g_ctx, current_token, lstr.value);
            },
            [&](tokens::keyword<> kw) -> std::unique_ptr<base> {
                if (tokens::is_type(kw.value)) {
                    if (peek().is<tokens::open_square_bracket>()) {
                        consume();
                        expect_token<tokens::close_square_bracket>();

                        return std::make_unique<keyword>(*g_ctx, current_token, kw.value, true, true);
                    }
                    return std::make_unique<keyword>(*g_ctx, current_token, kw.value, true);
                }
                else if (tokens::is_keyword<tokens::_not>(kw)) {
                    return std::make_unique<unary_operation::logical_not>(*g_ctx, current_token, parse_expression(3));
                }

                return std::make_unique<keyword>(*g_ctx, current_token, kw.value);
            },
            [&](tokens::identifier id) -> std::unique_ptr<base> {
                std::unique_ptr<identifier> expr_id;
                if (peek().is<tokens::multiply>()) {
                    expr_id = std::make_unique<identifier>(*g_ctx, current_token, id.value, true);
                    consume();
                }
                else
                    expr_id = std::make_unique<identifier>(*g_ctx, current_token, id.value, false);

                if (peek().is<tokens::open_round_bracket>()) {
                    current_token = consume();

                    vec<parser_utils::value> args;

                    while (!current_token.is<tokens::close_round_bracket>()) {
                        args.push_back(parse_expression()->get_value());

                        current_token = expect_tokens<tokens::comma, tokens::close_round_bracket>();
                    }

                    if (!g_ctx->contains(id.value, false) &&
                        !std::holds_alternative<parser_utils::func>(g_ctx->get(id.value, false)))
                        throw parser_utils::parser_error("Calling a non-existent function.", current_token);

                    return std::make_unique<call>(*g_ctx, current_token, std::move(expr_id), std::move(args));
                }
                return std::move(expr_id);
            },
            [&](tokens::open_init_bracket) -> std::unique_ptr<base> {
                parser_utils::match_pack args;

                while (!current_token.is<tokens::close_init_bracket>()) {
                    string match_name = expect_identifier().value;
                    expect_token<tokens::equal>();
                    args.emplace_back(
                        match_name,
                        std::visit(
                            [&](auto &&val) -> parser_utils::match_pack::value_type::second_type {
                                using T = std::decay_t<decltype(val)>;
                                if constexpr (std::is_constructible_v<parser_utils::match_pack::value_type::second_type,
                                                                                         T>)
                                    return parser_utils::match_pack::value_type::second_type{val};
                                throw parser_utils::parser_error("Unexpected match type.", peek());
                            },
                            parse_expression()->get_value()));

                    current_token = expect_tokens<tokens::comma, tokens::close_init_bracket>();
                }
                if (lhs_id.empty())
                    throw parser_utils::parser_error("It is impossible to determine the type of the "
                                                                                           "pack.",
                                                                        peek());
                if (std::find_if(args.begin(), args.end(), [&](const parser_utils::match_pack::value_type &mpack) {
                        return mpack.first == "name";
                    }) == args.end())
                    args.emplace_back(parser_utils::match_pack::value_type{"name", lhs_id});

                const auto &lhs_value = g_ctx->sc.at(lhs_id);
                if (std::holds_alternative<bweas::sc::language>(lhs_value))
                    return std::make_unique<pack<bweas::sc::language>>(*g_ctx, current_token, std::move(args));
                else if (std::holds_alternative<bweas::sc::profile>(lhs_value))
                    return std::make_unique<pack<bweas::sc::profile>>(*g_ctx, current_token, std::move(args));
                else if (std::holds_alternative<bweas::sc::call_component>(lhs_value))
                    return std::make_unique<pack<bweas::sc::call_component>>(*g_ctx, current_token, std::move(args));
                else if (std::holds_alternative<bweas::sc::template_command>(lhs_value))
                    return std::make_unique<pack<bweas::sc::template_command>>(*g_ctx, current_token, std::move(args));
                else if (std::holds_alternative<bweas::sc::target>(lhs_value))
                    return std::make_unique<pack<bweas::sc::target>>(*g_ctx, current_token, std::move(args));
                else
                    throw parser_utils::parser_error("It is not possible to define a pack for the "
                                                                                           "specified "
                                                                                           "type.",
                                                                        peek());
            },
            [&](tokens::open_round_bracket) -> std::unique_ptr<base> {
                auto new_lhs = parse_expression();
                expect_token<tokens::close_round_bracket>();
                return new_lhs;
            },
            [&](tokens::open_square_bracket) -> std::unique_ptr<base> {
                parser_utils::value arr;
                while (!current_token.is<tokens::close_square_bracket>()) {
                    auto el = parse_expression();
                    convention::expect_value(el);

                    auto value = el->get_value();
                    std::visit(
                        [&](auto &&val) {
                            using T = std::decay_t<decltype(val)>;

                            if constexpr (!parser_utils::is_vector<T>::value)
                                std::visit(
                                    [&](auto &&val2) {
                                        using T2 = std::decay_t<decltype(val2)>;
                                        if constexpr (std::is_same_v<T2, std::monostate> ||
                                                      std::is_same_v<T2, parser_utils::func> ||
                                                      parser_utils::is_vector<T2>::value)
                                            throw parser_utils::parser_error(
                                                "Unexpected " + parser_utils::get_type_name<T2>() + " type.",
                                                el->get_token());
                                        else
                                            arr = vec<T2>{val2};
                                    },
                                    value);
                            else {
                                if (!std::holds_alternative<typename T::value_type>(value))
                                    throw parser_utils::parser_error(
                                        "Expected \'" + parser_utils::get_type_name<T>() + "\' type.", el->get_token());

                                val.push_back(std::get<typename T::value_type>(value));
                            }
                        },
                        arr);

                    current_token = expect_tokens<tokens::comma, tokens::close_square_bracket>();
                }

                return std::make_unique<constant>(*g_ctx, current_token, arr);
            },
            [&](auto &&tk) -> std::unique_ptr<base> {
                throw parser_utils::parser_error("Unexpected token.", current_token);
            }},
        current_token.value);

    auto get_if_variable = [&](auto &&expr) {
        if (!expr->is_identifier())
            return;
        expr = std::move(std::unique_ptr<base>(new constant(*g_ctx, current_token, expr->get_reference())));
    };
    auto check_sameless_expr = [&](auto &&type, const std::unique_ptr<base> &lhs, const std::unique_ptr<base> &rhs) {
        using T = std::decay_t<decltype(type)>;
        if (!std::holds_alternative<T>(lhs->get_value()))
            return false;
        if (!std::holds_alternative<T>(rhs->get_value()))
            throw parser_utils::parser_error("Expected \'" + parser_utils::get_type_name<T>() + "\' type.",
                                             current_token);

        return true;
    };

    while (get_tokens().size()) {
        current_token = peek();

        if (current_token.is<tokens::end_line>() || current_token.is<tokens::close_init_bracket>() ||
            current_token.is<tokens::close_round_bracket>() || current_token.is<tokens::close_square_bracket>() ||
            current_token.is<tokens::comma>())
            break;

        pdiff rbinding_power = std::visit(
            parser_utils::func_wrapper{
                [&](tokens::keyword<> kw) -> pdiff {
                    if (tokens::is_keyword<tokens::_is>(kw))
                        return 0;
                    throw parser_utils::parser_error("Unexpected operator-keyword.", current_token);
                },
                [&](tokens::equal) -> pdiff { return 0; }, [&](tokens::plus) -> pdiff { return 1; },
                [&](tokens::minus) -> pdiff { return 1; }, [&](tokens::less) -> pdiff { return 1; },
                [&](tokens::more) -> pdiff { return 1; }, [&](tokens::multiply) -> pdiff { return 2; },
                [&](tokens::divide) -> pdiff { return 2; }, [&](tokens::init_type) -> pdiff { return 3; },
                [&](tokens::dot) -> pdiff { return lbinding_power < 4 ? 4 : lbinding_power - 1; },
                [&](tokens::open_square_bracket) -> pdiff { return 0; },
                [&](auto &&tk) -> pdiff { throw parser_utils::parser_error("Unexpected operator.", current_token); }},
            current_token.value);

        if (lbinding_power > rbinding_power)
            break;

        {
            consume();
            auto rhs = parse_expression(rbinding_power);
            lhs      = std::visit(
                parser_utils::func_wrapper{
                    [&](tokens::keyword<> kw) -> std::unique_ptr<base> {
                        get_if_variable(lhs);

                        if (tokens::is_keyword<tokens::_is>(kw)) {
                            if (check_sameless_expr(pdiff{}, lhs, rhs)) {
                                return std::make_unique<
                                         binary_operation::basic<parser_utils::basic_operation::equals<pdiff>>>(
                                    *g_ctx, current_token, std::move(lhs), std::move(rhs));
                            }
                            else if (check_sameless_expr(string{}, lhs, rhs)) {
                                return std::make_unique<
                                         binary_operation::basic<parser_utils::basic_operation::equals<string>>>(
                                    *g_ctx, current_token, std::move(lhs), std::move(rhs));
                            }
                            else
                                throw parser_utils::parser_error("Invalid type for the operation.", current_token);
                        }
                        return {};
                    },
                    [&](tokens::dot) -> std::unique_ptr<base> {
                        return std::make_unique<binary_operation::access<binary_operation::access_by_id>>(
                            *g_ctx, current_token, std::move(lhs), std::move(rhs));
                    },
                    [&](tokens::open_square_bracket) -> std::unique_ptr<base> {
                        if (!peek().is<tokens::close_square_bracket>())
                            throw parser_utils::parser_error("Expected ']' token.", current_token);
                        consume();

                        return std::make_unique<binary_operation::access<binary_operation::access_by_index>>(
                            *g_ctx, current_token, std::move(lhs), std::move(rhs));
                    },
                    [&](tokens::init_type) -> std::unique_ptr<base> {
                        lhs_id.clear();
                        std::unique_ptr<binary_operation::init> new_lhs = std::make_unique<binary_operation::init>(
                            *g_ctx, current_token, std::move(lhs), std::move(rhs));
                        lhs_id = std::get<string>(new_lhs->lhs->get_value());

                        return std::move(new_lhs);
                    },
                    [&](tokens::equal) -> std::unique_ptr<base> {
                        std::unique_ptr<binary_operation::assign> new_lhs = std::make_unique<binary_operation::assign>(
                            *g_ctx, current_token, std::move(lhs), std::move(rhs));
                        lhs_id.clear();
                        return std::move(new_lhs);
                    },
                    [&](tokens::plus) -> std::unique_ptr<base> {
                        get_if_variable(lhs);

                        if (check_sameless_expr(pdiff{}, lhs, rhs)) {
                            return std::make_unique<binary_operation::basic<
                                     parser_utils::basic_operation::binary_operator<pdiff, std::plus<>>>>(
                                *g_ctx, current_token, std::move(lhs), std::move(rhs));
                        }
                        else if (check_sameless_expr(string{}, lhs, rhs)) {
                            return std::make_unique<binary_operation::basic<
                                     parser_utils::basic_operation::binary_operator<string, std::plus<>>>>(
                                *g_ctx, current_token, std::move(lhs), std::move(rhs));
                        }
                        else
                            throw parser_utils::parser_error("Invalid type for the operation.", current_token);
                    },
                    [&](tokens::minus) -> std::unique_ptr<base> {
                        get_if_variable(lhs);
                        if (check_sameless_expr(pdiff{}, lhs, rhs))
                            return std::make_unique<binary_operation::basic<
                                     parser_utils::basic_operation::binary_operator<pdiff, std::minus<>>>>(
                                *g_ctx, current_token, std::move(lhs), std::move(rhs));
                        else
                            throw parser_utils::parser_error("Invalid type for the operation.", current_token);
                    },
                    [&](tokens::multiply) -> std::unique_ptr<base> {
                        get_if_variable(lhs);
                        if (check_sameless_expr(pdiff{}, lhs, rhs))
                            return std::make_unique<binary_operation::basic<
                                     parser_utils::basic_operation::binary_operator<pdiff, std::multiplies<>>>>(
                                *g_ctx, current_token, std::move(lhs), std::move(rhs));
                        else
                            throw parser_utils::parser_error("Invalid type for the operation", current_token);
                    },
                    [&](tokens::divide) -> std::unique_ptr<base> {
                        get_if_variable(lhs);
                        if (check_sameless_expr(pdiff{}, lhs, rhs))
                            return std::make_unique<binary_operation::basic<
                                     parser_utils::basic_operation::binary_operator<pdiff, std::divides<>>>>(
                                *g_ctx, current_token, std::move(lhs), std::move(rhs));
                        else
                            throw parser_utils::parser_error("Invalid type for the operation", current_token);
                    },
                    [&](tokens::less) -> std::unique_ptr<base> {
                        get_if_variable(lhs);
                        if (check_sameless_expr(pdiff{}, lhs, rhs))
                            return std::make_unique<binary_operation::basic<
                                     parser_utils::basic_operation::binary_operator<pdiff, std::less<>>>>(
                                *g_ctx, current_token, std::move(lhs), std::move(rhs));
                        else
                            throw parser_utils::parser_error("Invalid type for the operation", current_token);
                    },
                    [&](tokens::more) -> std::unique_ptr<base> {
                        get_if_variable(lhs);
                        if (check_sameless_expr(pdiff{}, lhs, rhs))
                            return std::make_unique<binary_operation::basic<
                                     parser_utils::basic_operation::binary_operator<pdiff, std::greater<>>>>(
                                *g_ctx, current_token, std::move(lhs), std::move(rhs));
                        else
                            throw parser_utils::parser_error("Invalid type for the operation", current_token);
                    },
                    [&](auto &&tk) -> std::unique_ptr<base> {
                        throw parser_utils::parser_error("Unexpected operator.", current_token);
                    }},
                current_token.value);
        }
    }

    if (lbinding_power != 4)
        get_if_variable(lhs);
    return lhs;
}
