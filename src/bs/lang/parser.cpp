#include <lang/parser.hpp>

using namespace bwlang;
using namespace expression::ext;

static parser_utils::context g_ctx_s;

parser::parser(string_v src) : lexer(src), g_ctx(&g_ctx_s) {
}

void parser::dump_global_context() {
    g_ctx_s.sc.clear();
    g_ctx_s.funcs.clear();
}
void parser::parse() {
    try {
        parse_statements();
    }
    catch (parser_utils::parser_error &excp) {
        string number_last_line = std::to_string(get_number_last_line());
        string last_line        = get_string_last_line();
        string tk               = excp.get_token();

        string output = number_last_line + ":\t" + last_line + "\n";
        if (pdiff it = last_line.find(tk); tk.size() && it != last_line.npos) {
            output += string(it + number_last_line.size() + 7, ' ') + "\033[91m" + string(tk.size(), '^') + "\033[0m";
        }
        else {
            pdiff output_prev_size = output.size();
            output = std::to_string(get_number_last_line() - 1) + ":\t" + get_string_previous_line() + "\n" + output;
            output += string(number_last_line.size() + 1, ' ') + "\t\033[91m" +
                      string(output_prev_size - number_last_line.size() - 3, '^') + "\033[0m";
        }
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
                                                          throw parser_utils::parser_error("Undefined keyword.", kw);
                                                  }
                                              },
                                              [&](auto &&) {
                                                  if (skip)
                                                      consume();
                                                  else
                                                      parse_expression()->get_value();
                                              }},
                   next_token);

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

    if (!convention::is_value(expr) || !std::holds_alternative<pdiff>(value))
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
    string name_module = std::get<tokens::literal_string>(expect_token<tokens::literal_string>()).value;
    auto it            = std::find_if(modules.begin(), modules.end(),
                                      [&](const bweas::module_manager::_module &md) { return md.name == name_module; });
    if (it == modules.end())
        throw parser_utils::parser_error("Unknown module.", tokens::literal_string{name_module});

    g_ctx->sc.merge(it->ctx.sc);
    g_ctx->funcs.merge(it->ctx.funcs);
}
void parser::parse_function() {
    static umap<string, vec<tokens::token>> decl_funcs;

    vec<string> args;
    string name_func = expect_identifier().value;
    expect_token<tokens::open_round_bracket>();

    tokens::token current_token = peek();
    while (!std::holds_alternative<tokens::close_round_bracket>(current_token)) {
        args.push_back(expect_identifier().value);

        current_token = expect_tokens<tokens::comma, tokens::close_round_bracket>();
    }

    std::function<void()> copy_all_token = [&]() {
        while (!tokens::is_keyword<tokens::_endfunc>(current_token)) {
            if (current_token = consume_if(); std::holds_alternative<std::monostate>(current_token))
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

    g_ctx->funcs[name_func] = parser_utils::func{
        [&](string name_func, parser_utils::context &c_ctx) -> parser_utils::value {
            get_tokens().insert(get_tokens().begin(), decl_funcs.at(name_func).begin(), decl_funcs.at(name_func).end());

            g_ctx                            = &c_ctx;
            parser_utils::value return_value = parse_statements(false, false, true);
            g_ctx                            = &g_ctx_s;
            if (!std::holds_alternative<std::monostate>(return_value))
                while (!std::holds_alternative<std::monostate>(peek()) && !tokens::is_keyword<tokens::_endfunc>(peek()))
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
            [&](tokens::literal_number lnum) -> std::unique_ptr<base> {
                return std::make_unique<constant>(*g_ctx, lnum.value);
            },
            [&](tokens::literal_string lstr) -> std::unique_ptr<base> {
                return std::make_unique<constant>(*g_ctx, lstr.value);
            },
            [&](tokens::keyword<> kw) -> std::unique_ptr<base> {
                if (tokens::is_type(kw.value)) {
                    if (std::holds_alternative<tokens::open_square_bracket>(peek())) {
                        consume();
                        expect_token<tokens::close_square_bracket>();

                        kw.value = tokens::make_array_type(kw.value);
                    }
                    return std::make_unique<keyword>(*g_ctx, kw.value, true);
                }
                else if (tokens::is_keyword<tokens::_not>(kw)) {
                    return std::make_unique<unary_operation::negative>(*g_ctx, parse_expression(3));
                }

                return std::make_unique<keyword>(*g_ctx, kw.value);
            },
            [&](tokens::identifier id) -> std::unique_ptr<base> {
                if (std::holds_alternative<tokens::open_round_bracket>(peek())) {
                    current_token = consume();

                    vec<parser_utils::value> args;

                    while (!std::holds_alternative<tokens::close_round_bracket>(current_token)) {
                        args.push_back(parse_expression()->get_value());

                        current_token = expect_tokens<tokens::comma, tokens::close_round_bracket>();
                    }

                    if (!g_ctx->is_function(id.value))
                        throw parser_utils::parser_error("Calling a non-existent function.", id);

                    return std::make_unique<call>(*g_ctx, id.value, std::move(args));
                }
                return std::make_unique<identifier>(*g_ctx, id.value);
            },
            [&](tokens::open_init_bracket) -> std::unique_ptr<base> {
                parser_utils::match_pack args;

                while (!std::holds_alternative<tokens::close_init_bracket>(current_token)) {
                    string match_name = expect_identifier().value;
                    expect_token<tokens::equal>();

                    args.emplace(std::visit(
                        [&](auto &&val) -> parser_utils::match_pack::value_type {
                            using T = std::decay_t<decltype(val)>;
                            if constexpr (std::is_constructible_v<parser_utils::match_pack::mapped_type, T>)
                                return parser_utils::match_pack::value_type{match_name, val};
                            throw parser_utils::parser_error("Unexpected match type.", peek());
                        },
                        parse_expression()->get_value()));

                    current_token = expect_tokens<tokens::comma, tokens::close_init_bracket>();
                }
                if (lhs_id.empty())
                    throw parser_utils::parser_error("It is impossible to determine the type of the "
                                                                                           "pack.",
                                                                        peek());
                if (!args.contains("name"))
                    args["name"] = lhs_id;

                const auto &lhs_value = g_ctx->sc.at(lhs_id);
                if (std::holds_alternative<bweas::sc::language>(lhs_value))
                    return std::make_unique<pack<bweas::sc::language>>(*g_ctx, std::move(args));
                else if (std::holds_alternative<bweas::sc::profile>(lhs_value))
                    return std::make_unique<pack<bweas::sc::profile>>(*g_ctx, std::move(args));
                else if (std::holds_alternative<bweas::sc::call_component>(lhs_value))
                    return std::make_unique<pack<bweas::sc::call_component>>(*g_ctx, std::move(args));
                else if (std::holds_alternative<bweas::sc::template_command>(lhs_value))
                    return std::make_unique<pack<bweas::sc::template_command>>(*g_ctx, std::move(args));
                else if (std::holds_alternative<bweas::sc::target>(lhs_value))
                    return std::make_unique<pack<bweas::sc::target>>(*g_ctx, std::move(args));
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
                parser_utils::value type_value;
                while (!std::holds_alternative<tokens::close_square_bracket>(current_token)) {
                    auto el = parse_expression();
                    if (!convention::is_value(el))
                        throw parser_utils::parser_error("Expected constant value.", peek());

                    auto value = el->get_value();
                    std::visit(
                        parser_utils::func_wrapper{
                            [&](std::monostate) {
                                type_value = value;
                                if (std::holds_alternative<pdiff>(value))
                                    arr = vec<pdiff>{std::get<pdiff>(value)};
                                else if (std::holds_alternative<string>(value))
                                    arr = vec<string>{std::get<string>(value)};
                                else if (std::holds_alternative<bweas::sc::target>(value))
                                    arr = vec<bweas::sc::target>{std::get<bweas::sc::target>(value)};
                                else
                                    throw parser_utils::parser_error("Unexpected type expression.", peek());
                            },
                            [&](pdiff) {
                                if (!std::holds_alternative<pdiff>(value))
                                    throw parser_utils::parser_error(
                                        "Expected \'" + parser_utils::get_type_name<pdiff>() + "\' type.", peek());
                                std::get<vec<pdiff>>(arr).push_back(std::get<pdiff>(value));
                            },
                            [&](string) {
                                if (!std::holds_alternative<string>(value))
                                    throw parser_utils::parser_error(
                                        "Expected \'" + parser_utils::get_type_name<string>() + "\' type.", peek());
                                std::get<vec<string>>(arr).push_back(std::get<string>(value));
                            },
                            [&](bweas::sc::target) {
                                if (!std::holds_alternative<bweas::sc::target>(value))
                                    throw parser_utils::parser_error(
                                        "Expected \'" + parser_utils::get_type_name<bweas::sc::target>() + "\' type.",
                                        peek());
                                std::get<vec<bweas::sc::target>>(arr).push_back(std::get<bweas::sc::target>(value));
                            },
                            [&](auto &&) { throw parser_utils::parser_error("Unexpected type.", peek()); }},
                        type_value);

                    current_token = expect_tokens<tokens::comma, tokens::close_square_bracket>();
                }

                return std::make_unique<constant>(*g_ctx, arr);
            },
            [&](auto &&tk) -> std::unique_ptr<base> { throw parser_utils::parser_error("Unexpected token.", tk); }},
        current_token);

    auto get_if_variable = [&](auto &&expr) {
        if (convention::is_identifier(expr)) {
            string name_var = std::get<string>(expr->get_value());
            if (convention::is_variable(expr))
                expr = std::move(std::unique_ptr<base>(new constant(*g_ctx, g_ctx->get_variable(name_var))));
            else
                throw parser_utils::parser_error("A non-existent variable.", tokens::identifier(name_var));
        }
        else if (convention::is_accessing_variable(expr)) {
            auto access_info = std::get<parser_utils::access>(expr->get_value());

            expr = std::move(std::unique_ptr<base>(new constant(*g_ctx, std::visit(
                                                                            [&](auto &&obj) -> parser_utils::value {
                                                                                return parser_utils::get_field(
                                                                                    obj, access_info.second);
                                                                            },
                                                                            g_ctx->get_variable(access_info.first)))));
        }
    };
    auto check_sameless_expr = [&](auto &&type, const std::unique_ptr<base> &lhs, const std::unique_ptr<base> &rhs) {
        using T = std::decay_t<decltype(type)>;
        if (!std::holds_alternative<T>(lhs->get_value()))
            return false;
        if (!std::holds_alternative<T>(rhs->get_value()))
            throw parser_utils::parser_error("Expected \'" + parser_utils::get_type_name<T>() + "\' type.", peek());

        return true;
    };

    while (get_tokens().size()) {
        current_token = peek();

        if (std::holds_alternative<tokens::end_line>(current_token) ||
            std::holds_alternative<tokens::close_init_bracket>(current_token) ||
            std::holds_alternative<tokens::close_round_bracket>(current_token) ||
            std::holds_alternative<tokens::close_square_bracket>(current_token) ||
            std::holds_alternative<tokens::comma>(current_token))
            break;

        pdiff rbinding_power =
            std::visit(parser_utils::func_wrapper{
                           [&](tokens::keyword<> kw) -> pdiff {
                               if (tokens::is_keyword<tokens::_is>(kw))
                                   return 0;
                               throw parser_utils::parser_error("Unexpected operator-keyword.", peek());
                           },
                           [&](tokens::equal) -> pdiff { return 0; }, [&](tokens::plus) -> pdiff { return 1; },
                           [&](tokens::minus) -> pdiff { return 1; }, [&](tokens::multiply) -> pdiff { return 2; },
                           [&](tokens::devide) -> pdiff { return 2; }, [&](tokens::init_type) -> pdiff { return 3; },
                           [&](tokens::dot) -> pdiff { return 4; },
                           [](auto &&tk) -> pdiff { throw parser_utils::parser_error("Unexpected operator.", tk); }},
                       current_token);

        if (lbinding_power > rbinding_power)
            break;

        {
            current_token = consume();
            auto rhs      = parse_expression(rbinding_power);
            lhs           = std::visit(
                parser_utils::func_wrapper{
                    [&](tokens::keyword<> kw) -> std::unique_ptr<base> {
                        get_if_variable(lhs);

                        if (check_sameless_expr(pdiff{}, lhs, rhs)) {
                            return std::make_unique<
                                          binary_operation::basic<parser_utils::basic_operation::equals<pdiff>>>(
                                *g_ctx, std::move(lhs), std::move(rhs));
                        }
                        else if (check_sameless_expr(string{}, lhs, rhs)) {
                            return std::make_unique<
                                          binary_operation::basic<parser_utils::basic_operation::equals<string>>>(
                                *g_ctx, std::move(lhs), std::move(rhs));
                        }
                        else
                            throw parser_utils::parser_error(
                                "Invalid type for the operation.",
                                tokens::token{tokens::keyword<>{tokens::string_matching::_is}});
                    },
                    [&](tokens::dot) -> std::unique_ptr<base> {
                        return std::make_unique<binary_operation::access>(*g_ctx, std::move(lhs), std::move(rhs));
                    },
                    [&](tokens::init_type) -> std::unique_ptr<base> {
                        lhs_id.clear();
                        std::unique_ptr<binary_operation::init> new_lhs =
                            std::make_unique<binary_operation::init>(*g_ctx, std::move(lhs), std::move(rhs));
                        new_lhs->get_value();
                        lhs_id = std::get<string>(new_lhs->get_lhs()->get_value());

                        return std::move(new_lhs);
                    },
                    [&](tokens::equal) -> std::unique_ptr<base> {
                        std::unique_ptr<binary_operation::assign> new_lhs =
                            std::make_unique<binary_operation::assign>(*g_ctx, std::move(lhs), std::move(rhs));
                        lhs_id.clear();
                        return std::move(new_lhs);
                    },
                    [&](tokens::plus) -> std::unique_ptr<base> {
                        get_if_variable(lhs);

                        if (check_sameless_expr(pdiff{}, lhs, rhs)) {
                            return std::make_unique<
                                          binary_operation::basic<parser_utils::basic_operation::plus<pdiff>>>(
                                *g_ctx, std::move(lhs), std::move(rhs));
                        }
                        else if (check_sameless_expr(string{}, lhs, rhs)) {
                            return std::make_unique<
                                          binary_operation::basic<parser_utils::basic_operation::plus<string>>>(
                                *g_ctx, std::move(lhs), std::move(rhs));
                        }
                        else
                            throw parser_utils::parser_error("Invalid type for the operation.", tokens::plus{});
                    },
                    [&](tokens::minus) -> std::unique_ptr<base> {
                        get_if_variable(lhs);
                        if (check_sameless_expr(pdiff{}, lhs, rhs))
                            return std::make_unique<
                                          binary_operation::basic<parser_utils::basic_operation::minus<pdiff>>>(
                                *g_ctx, std::move(lhs), std::move(rhs));
                        else
                            throw parser_utils::parser_error("Invalid type for the operation", tokens::minus{});
                    },
                    [&](tokens::multiply) -> std::unique_ptr<base> {
                        get_if_variable(lhs);
                        if (check_sameless_expr(pdiff{}, lhs, rhs))
                            return std::make_unique<
                                          binary_operation::basic<parser_utils::basic_operation::multiplies<pdiff>>>(
                                *g_ctx, std::move(lhs), std::move(rhs));
                        else
                            throw parser_utils::parser_error("Invalid type for the operation", tokens::multiply{});
                    },
                    [&](tokens::devide) -> std::unique_ptr<base> {
                        get_if_variable(lhs);
                        if (check_sameless_expr(pdiff{}, lhs, rhs))
                            return std::make_unique<
                                          binary_operation::basic<parser_utils::basic_operation::divides<pdiff>>>(
                                *g_ctx, std::move(lhs), std::move(rhs));
                        else
                            throw parser_utils::parser_error("Invalid type for the operation", tokens::devide{});
                    },
                    [](auto &&tk) -> std::unique_ptr<base> {
                        throw parser_utils::parser_error("Unexpected operator.", tk);
                    }},
                current_token);
        }
    }

    if (lbinding_power != 4)
        get_if_variable(lhs);
    return lhs;
}
