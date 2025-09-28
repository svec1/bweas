#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include <bwaliases.hpp>
#include <utility>

#include <bwstructs_context.hpp>
#include <lang/parser_utils.hpp>
#include <lang/tokens.hpp>

namespace bwlang {

namespace expression {

template <typename T> class base {
  public:
    base(parser_utils::context &_ctx) : ctx(_ctx) {
    }
    virtual ~base() = default;

    using value_type = T;

  public:
    virtual value_type get_value() = 0;
    virtual constexpr bool is_identifier() const {
        return false;
    }
    virtual constexpr bool is_keyword() const {
        return false;
    }
    virtual constexpr bool is_type() const {
        return false;
    }
    virtual constexpr bool is_access() const {
        return false;
    }

  public:
    parser_utils::context &ctx;
};

template <typename T> class constant : public base<T> {
  public:
    constant(parser_utils::context &_ctx, base<T>::value_type _val) : base<T>(_ctx), val(_val) {
    }
    virtual ~constant() = default;

  public:
    base<T>::value_type get_value() override {
        return val;
    }

  private:
    base<T>::value_type val;
};

template <typename T> class identifier : public constant<T> {
  public:
    identifier(parser_utils::context &_ctx, constant<T>::value_type _val) : constant<T>(_ctx, _val) {
    }
    ~identifier() override = default;

  public:
    constexpr bool is_identifier() const override {
        return true;
    }
};

template <typename T> class keyword : public constant<T> {
  public:
    keyword(parser_utils::context &_ctx, constant<T>::value_type _val, bool __is_type = false)
        : constant<T>(_ctx, _val), _is_type(__is_type) {
    }
    ~keyword() override = default;

  public:
    constexpr bool is_keyword() const override {
        return true;
    }
    constexpr bool is_type() const override {
        return _is_type;
    }

  private:
    bool _is_type;
};

template <typename T, typename A1, typename A2, typename Op,
          typename = std::enable_if_t<
              std::is_same_v<T, decltype(std::declval<Op>()(std::declval<parser_utils::context &>(),
                                                            std::declval<A1 &>(), std::declval<A2 &>()))>>>
class binary : public base<T> {
  public:
    binary(parser_utils::context &_ctx, std::unique_ptr<base<A1>> _lhs, std::unique_ptr<base<A2>> _rhs)
        : base<T>(_ctx), lhs(std::move(_lhs)), rhs(std::move(_rhs)) {
    }
    virtual ~binary() = default;

  public:
    base<T>::value_type get_value() override {
        return Op{}(this->ctx, lhs->get_value(), rhs->get_value());
    }

  public:
    const std::unique_ptr<base<A1>> &get_lhs() {
        return lhs;
    }
    const std::unique_ptr<base<A2>> &get_rhs() {
        return rhs;
    }

  private:
    std::unique_ptr<base<A1>> lhs;
    std::unique_ptr<base<A2>> rhs;
};

template <typename T, typename Op,
          typename = std::enable_if_t<std::is_same_v<
              decltype(std::declval<Op>()(std::declval<parser_utils::context &>(), std::declval<T &>())), T>>>
class unary : public base<T> {
  public:
    unary(parser_utils::context &_ctx, std::unique_ptr<base<T>> _rhs) : base<T>(_ctx), rhs(std::move(_rhs)) {
    }
    virtual ~unary() = default;

  public:
    base<T>::value_type get_value() override {
        return Op{}(this->ctx, rhs->get_value());
    }

  public:
    const std::unique_ptr<base<T>> &get_rhs() {
        return rhs;
    }

  private:
    std::unique_ptr<base<T>> rhs;
};

template <typename T, typename Construct, typename = std::void_t<decltype(T{Construct{}})>>
class pack : public base<T> {
  public:
    pack(parser_utils::context &_ctx, parser_utils::match_pack mpack) : base<T>(_ctx) {
        using namespace bweas;
        for (const auto &[name, value] : mpack) {
            parser_utils::value gen_value = std::visit([](auto &&val) -> parser_utils::value { return val; }, value);

            try {
                parser_utils::set_field(obj, name, gen_value);
            }
            catch (std::runtime_error &excp) {
                throw parser_utils::parser_error(excp.what() + string("\nMatch: \'") + name + "\'");
            }
        }
    }
    ~pack() override = default;

  public:
    base<T>::value_type get_value() override {
        return obj;
    }

  private:
    Construct obj;
};

template <typename T> class call : public base<T> {
  public:
    call(parser_utils::context &_ctx, string name, vec<parser_utils::value> &&args) : base<T>(_ctx) {
        parser_utils::context c_ctx;
        c_ctx.g_ctx      = &this->ctx;
        const auto &func = c_ctx.get_function(name);
        if (func.declared) {
            if (func.name_args.size() != args.size())
                throw parser_utils::parser_error("The number of values passed does not match the number "
                                                 "accepted by the function.");

            for (pdiff i = 0; i < func.name_args.size(); ++i)
                c_ctx.sc[func.name_args[i]] = args[i];
        }
        else
            for (pdiff i = 0; i < args.size(); ++i)
                c_ctx.sc[std::to_string(i)] = args[i];
        try {
            return_value = func.ref(name, c_ctx);
        }
        catch (std::exception &excp) {
            throw parser_utils::parser_error(excp.what());
        }
    }
    ~call() override = default;

  public:
    base<T>::value_type get_value() override {
        return return_value;
    }

  private:
    T return_value;
};

namespace ext {

using base       = base<parser_utils::value>;
using constant   = constant<parser_utils::value>;
using keyword    = keyword<parser_utils::value>;
using identifier = identifier<parser_utils::value>;
using call       = call<parser_utils::value>;

template <typename Construct> using pack = pack<parser_utils::value, Construct>;
template <typename Op> using binary      = binary<parser_utils::value, parser_utils::value, parser_utils::value, Op>;
template <typename Op> using unary       = unary<parser_utils::value, Op>;

namespace convention {

static bool is_identifier(const std::unique_ptr<base> &expr) {
    return expr->is_identifier() && std::holds_alternative<string>(expr->get_value());
}
static bool is_keyword(const std::unique_ptr<base> &expr) {
    return expr->is_keyword() && std::holds_alternative<string>(expr->get_value());
}
static bool is_variable(const std::unique_ptr<base> &expr) {
    return is_identifier(expr) && expr->ctx.is_variable(std::get<string>(expr->get_value()));
}
static bool is_accessing_variable(const std::unique_ptr<base> &expr) {
    return expr->is_access() && expr->ctx.is_variable(std::get<parser_utils::access>(expr->get_value()).first);
}
static bool is_value(const std::unique_ptr<base> &expr) {
    return !expr->is_identifier() && !expr->is_keyword() && !std::holds_alternative<std::monostate>(expr->get_value());
}
} // namespace convention

namespace binary_operation {

struct init_binary_operation {
    static parser_utils::value operator()(parser_utils::context &ctx, const parser_utils::value &lhs,
                                          const parser_utils::value &rhs) {
        using namespace bweas;

        parser_utils::scope &sc = ctx.sc;

        string name_var    = std::get<string>(lhs);
        string kw_var_type = std::get<string>(rhs);

        if (kw_var_type == tokens::number_t::s_value)
            sc[name_var] = pdiff{};
        else if (kw_var_type == tokens::string_t::s_value)
            sc[name_var] = string{};
        else if (kw_var_type == tokens::cc_t::s_value)
            sc[name_var] = sc::call_component{};
        else if (kw_var_type == tokens::ctemplate_t::s_value)
            sc[name_var] = sc::template_command{};
        else if (kw_var_type == tokens::language_t::s_value)
            sc[name_var] = sc::language{};
        else if (kw_var_type == tokens::profile_t::s_value)
            sc[name_var] = sc::profile{};
        else if (kw_var_type == tokens::target_t::s_value)
            sc[name_var] = sc::target{};
        else if (kw_var_type == tokens::string_matching::anumber_t)
            sc[name_var] = vec<pdiff>{};
        else if (kw_var_type == tokens::string_matching::astring_t)
            sc[name_var] = vec<string>{};
        else if (kw_var_type == tokens::string_matching::atarget_t)
            sc[name_var] = vec<sc::target>{};
        else
            throw parser_utils::parser_error("Unknown type \'" + kw_var_type + "\'");
        return name_var;
    }
};
struct access_binary_operation {
    static parser_utils::value operator()(parser_utils::context &ctx, const parser_utils::value &lhs,
                                          const parser_utils::value &rhs) {
        return parser_utils::access{std::get<string>(lhs), std::get<string>(rhs)};
    }
};
struct assign_binary_operation {
    static parser_utils::value operator()(parser_utils::context &ctx, const parser_utils::value &lhs,
                                          const parser_utils::value &rhs) {
        using namespace bweas;

        if (std::holds_alternative<parser_utils::access>(lhs)) {
            const auto &acc = std::get<parser_utils::access>(lhs);
            std::visit([&](auto &&val) { parser_utils::set_field(val, acc.second, rhs); }, ctx.get_variable(acc.first));
            return rhs;
        }

        string name = std::get<string>(lhs);

        std::visit(
            [&](auto &&val) {
                using T = std::decay_t<decltype(val)>;
                if (!std::holds_alternative<T>(rhs))
                    throw parser_utils::parser_error("Expected \'" + parser_utils::get_type_name<T>() + "\' type.");
                val = std::get<T>(rhs);
            },
            ctx.get_variable(name));

        return rhs;
    }
};

template <typename Op,
          typename = std::void_t<decltype(std::declval<Op>()(std::declval<typename Op::arguments_type &>(),
                                                             std::declval<typename Op::arguments_type &>())),
                                 decltype(parser_utils::value{typename Op::result_type{}})>>
struct basic_binary_operation {
    static parser_utils::value operator()(parser_utils::context &ctx, const parser_utils::value &lhs,
                                          const parser_utils::value &rhs) {
        using T = typename Op::arguments_type;

        if (!std::holds_alternative<T>(lhs))
            throw parser_utils::parser_error("Lhs is expected with type \'" + parser_utils::get_type_name<T>() + "\'");
        if (!std::holds_alternative<T>(rhs))
            throw parser_utils::parser_error("Rhs is expected with type \'" + parser_utils::get_type_name<T>() + "\'");
        return Op{}(std::get<T>(lhs), std::get<T>(rhs));
    }
};

template <typename Op> class basic : public binary<basic_binary_operation<Op>> {
  public:
    basic(parser_utils::context &_ctx, std::unique_ptr<base> _lhs, std::unique_ptr<base> _rhs)
        : binary<basic_binary_operation<Op>>(_ctx, std::move(_lhs), std::move(_rhs)) {
        const auto &lhs = this->get_lhs();
        const auto &rhs = this->get_rhs();

        if (!convention::is_value(lhs) || !convention::is_value(rhs))
            throw parser_utils::parser_error("Lhs and rhs are expected values.");
    };
    ~basic() override = default;
};

class init : public binary<init_binary_operation> {
  public:
    init(parser_utils::context &_ctx, std::unique_ptr<base> _lhs, std::unique_ptr<base> _rhs)
        : ext::binary<init_binary_operation>(_ctx, std::move(_lhs), std::move(_rhs)) {
        const auto &lhs = get_lhs();
        const auto &rhs = get_rhs();

        if (!convention::is_identifier(lhs) || convention::is_variable(lhs))
            throw parser_utils::parser_error("Expected non-existent variable identifier.", tokens::init_type{});
        if (!convention::is_keyword(rhs) || !rhs->is_type())
            throw parser_utils::parser_error("The type for initializing the variable was expected.",
                                             tokens::init_type{});
    };
    ~init() override = default;

    constexpr bool is_identifier() const override {
        return true;
    }
};
class access : public binary<access_binary_operation> {
  public:
    access(parser_utils::context &_ctx, std::unique_ptr<base> _lhs, std::unique_ptr<base> _rhs)
        : ext::binary<access_binary_operation>(_ctx, std::move(_lhs), std::move(_rhs)) {
        const auto &lhs = get_lhs();
        const auto &rhs = get_rhs();

        if (!convention::is_variable(lhs))
            throw parser_utils::parser_error("An existing variable is expected", tokens::dot{});
        if (!convention::is_identifier(rhs))
            throw parser_utils::parser_error("The ID of the structure field is expected.", tokens::dot{});
    };
    ~access() override = default;

    constexpr bool is_identifier() const override {
        return true;
    }
    constexpr bool is_access() const override {
        return true;
    }
};
class assign : public binary<assign_binary_operation> {
  public:
    assign(parser_utils::context &_ctx, std::unique_ptr<base> _lhs, std::unique_ptr<base> _rhs)
        : ext::binary<assign_binary_operation>(_ctx, std::move(_lhs), std::move(_rhs)) {
        const auto &lhs = get_lhs();
        const auto &rhs = get_rhs();

        if (!convention::is_variable(lhs) && !convention::is_accessing_variable(lhs))
            throw parser_utils::parser_error("A reference to the variable is expected.", tokens::equal{});
        if (!convention::is_value(rhs))
            throw parser_utils::parser_error("Expected expression with value.", tokens::equal{});
    };
    ~assign() override = default;
};
} // namespace binary_operation

namespace unary_operation {
struct negative_unary_operation {
    parser_utils::value operator()(parser_utils::context &ctx, const parser_utils::value &rhs) {
        if (!std::holds_alternative<pdiff>(rhs))
            throw parser_utils::parser_error("Expected number type",
                                             tokens::token{tokens::keyword<>{tokens::string_matching::_not}});

        return !std::get<pdiff>(rhs);
    }
};

class negative : public unary<negative_unary_operation> {
  public:
    negative(parser_utils::context &_ctx, std::unique_ptr<base> _rhs)
        : ext::unary<negative_unary_operation>(_ctx, std::move(_rhs)) {
        const auto &rhs = get_rhs();

        if (!convention::is_value(rhs))
            throw parser_utils::parser_error("A reference to the variable is expected.",
                                             tokens::token{tokens::keyword<>{tokens::string_matching::_not}});
        if (!convention::is_value(rhs))
            throw parser_utils::parser_error("Expected expression with value.",
                                             tokens::token{tokens::keyword<>{tokens::string_matching::_not}});
    };
    ~negative() override = default;
};
} // namespace unary_operation
} // namespace ext
} // namespace expression
} // namespace bwlang
#endif
