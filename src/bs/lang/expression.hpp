//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses />
// ------------------------------------------
//

#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include <bwaliases.hpp>
#include <utility>

#include <bwstructs_context.hpp>
#include <lang/parser_utils.hpp>
#include <lang/tokens.hpp>

/** \brief Defines the bweas scripting language. */
namespace bwlang {

/** \brief Possible expressions in bwlang are described. */
namespace expression {

/** \brief The base class of the expression. */
template <typename T> class base {
  public:
    base(parser_utils::context &_ctx, tokens::token _tk) : ctx(_ctx), tk(std::move(_tk)) {
    }
    virtual ~base() = default;

    using value_type = T;

  public:
    const tokens::token &get_token() const {
        return tk;
    }

    virtual value_type get_value() const = 0;

  public:
    virtual value_type &get_reference() {
        throw parser_utils::parser_error("Invalid reference definition.", tk);
    }
    virtual bool is_context_member() const {
        throw parser_utils::parser_error("Invalid check for existence in the context.", tk);
    }

  public:
    virtual constexpr bool is_identifier() const {
        return false;
    }
    virtual constexpr bool is_keyword() const {
        return false;
    }

  public:
    constexpr bool is_value() const {
        return !is_identifier() && !is_keyword() && !std::holds_alternative<std::monostate>(get_value());
    }

  protected:
    parser_utils::context &ctx;
    tokens::token tk;
};

/** \brief A class describing an expression that is a constant value. */
template <typename T> class constant : public base<T> {
  public:
    constant(parser_utils::context &_ctx, tokens::token _tk, base<T>::value_type _val)
        : base<T>(_ctx, _tk), val(std::move(_val)) {
    }
    virtual ~constant() = default;

  public:
    base<T>::value_type get_value() const override {
        return val;
    }

  private:
    const base<T>::value_type val;
};

/** \brief A class describing an expression that is an identifier. */
template <typename T> class identifier final : public constant<T> {
  public:
    identifier(parser_utils::context &_ctx, tokens::token _tk, string _val, bool _local = false)
        : constant<T>(_ctx, _tk, _val), local(_local) {
    }
    ~identifier() override = default;

  public:
    constant<T>::value_type &get_reference() override {
        auto name_variable = std::get<string>(this->get_value());

        if (!is_context_member())
            throw parser_utils::parser_error("Invalid access to a non-existent variable.", this->tk);

        return this->ctx.get(name_variable, local);
    }
    bool is_context_member() const override {
        return this->ctx.contains(std::get<string>(this->get_value()), local);
    }

  public:
    constexpr bool is_local() const {
        return local;
    }

  public:
    constexpr bool is_identifier() const override {
        return true;
    }

  private:
    bool local;
};

/** \brief A class describing an expression that is an keyword. */
template <typename T> class keyword final : public constant<T> {
  public:
    keyword(parser_utils::context &_ctx, tokens::token _tk, string _val, bool __is_type = false, bool _is_array = false)
        : constant<T>(_ctx, _tk, _val), _is_type(__is_type), _is_array(_is_array) {
    }
    ~keyword() override = default;

  public:
    constexpr bool is_keyword() const override {
        return true;
    }

  public:
    constexpr bool is_type() const {
        return _is_type;
    }
    constexpr bool is_array() const {
        return _is_type && _is_array;
    }

  private:
    bool _is_type, _is_array;
};

/** \brief A class describing binary expressions. */
template <typename T, typename A1, typename A2, typename Op,
          typename = std::enable_if_t<
              std::is_same_v<decltype(std::declval<Op &>()(std::declval<parser_utils::context &>(),
                                                           std::declval<const std::unique_ptr<base<A1>> &>(),
                                                           std::declval<const std::unique_ptr<base<A2>> &>())),
                             T>>>
class binary : public Op, public constant<T> {
  public:
    binary(parser_utils::context &_ctx, tokens::token _tk, std::unique_ptr<base<A1>> _lhs,
           std::unique_ptr<base<A2>> _rhs)
        : constant<T>(_ctx, _tk, this->operator()(_ctx, _lhs, _rhs)), lhs(std::move(_lhs)), rhs(std::move(_rhs)) {
    }
    virtual ~binary() = default;

  public:
    constexpr bool is_keyword() const override {
        return false;
    }

  public:
    const std::unique_ptr<base<A1>> lhs;
    const std::unique_ptr<base<A2>> rhs;
};

/** \brief A class describing unary expressions. */
template <typename T, typename Op,
          typename = std::enable_if_t<
              std::is_same_v<decltype(std::declval<Op &>()(std::declval<parser_utils::context &>(),
                                                           std::declval<const std::unique_ptr<base<T>> &>())),
                             T>>>
class unary : protected Op, public constant<T> {
  public:
    unary(parser_utils::context &_ctx, tokens::token _tk, std::unique_ptr<base<T>> _rhs)
        : constant<T>(_ctx, _tk, this->operator()(_ctx, _rhs)), rhs(std::move(_rhs)) {
    }
    virtual ~unary() = default;

  public:
    constexpr bool is_keyword() const override {
        return false;
    }

  public:
    const std::unique_ptr<base<T>> rhs;
};

/** \brief A class describing an expression that constructs an object Construct. */
template <typename T, typename Construct, typename = std::void_t<decltype(T{Construct{}})>>
class pack final : public constant<T> {
  public:
    static Construct build(parser_utils::context &ctx, const parser_utils::match_pack &mpack) {
        using namespace bweas;

        Construct obj;

        for (const auto &[name, value] : mpack) {
            parser_utils::value gen_value = std::visit([](auto &&val) -> parser_utils::value { return val; }, value);

            try {
                parser_utils::set_field(obj, name, gen_value);
            }
            catch (parser_utils::parser_error &excp) {
                throw parser_utils::parser_error(excp.what(), {tokens::identifier{name}});
            }
        }
        return obj;
    }

  public:
    pack(parser_utils::context &_ctx, tokens::token _tk, parser_utils::match_pack mpack)
        : constant<T>(_ctx, _tk, build(_ctx, mpack)) {
    }
    ~pack() override = default;
};

/** \brief A class describing the expression of a function call. */
template <typename T> class call final : public constant<T> {
  public:
    static T call_function(parser_utils::context &ctx, std::unique_ptr<identifier<T>> &&id,
                           vec<parser_utils::value> &&args) {
        parser_utils::context c_ctx;
        c_ctx.g_ctx      = &ctx;
        const auto &func = std::get<parser_utils::func>(id->get_reference());
        if (func.declared) {
            if (func.name_args.size() != args.size())
                throw parser_utils::parser_error("The number of values passed does not match the number "
                                                 "accepted by the function.",
                                                 id->get_token());

            for (pdiff i = 0; i < func.name_args.size(); ++i)
                c_ctx.sc[func.name_args[i]] = args[i];
        }
        else
            for (pdiff i = 0; i < args.size(); ++i)
                c_ctx.sc[std::to_string(i)] = args[i];
        try {
            return func.ref(std::get<string>(id->get_value()), c_ctx);
        }
        catch (parser_utils::parser_error &excp) {
            throw parser_utils::parser_error(excp.what(), std::holds_alternative<std::monostate>(excp.get_token().value)
                                                              ? id->get_token()
                                                              : excp.get_token());
        }
    }

  public:
    call(parser_utils::context &_ctx, tokens::token _tk, std::unique_ptr<identifier<T>> id,
         vec<parser_utils::value> &&args)
        : constant<T>(_ctx, _tk, call_function(_ctx, std::move(id), std::move(args))) {
    }
    ~call() override = default;
};

/** \brief Describes an extension that satisfies the bwlang syntax. */
namespace ext {

using base       = base<parser_utils::value>;
using constant   = constant<parser_utils::value>;
using keyword    = keyword<parser_utils::value>;
using identifier = identifier<parser_utils::value>;
using call       = call<parser_utils::value>;

template <typename Construct> using pack = pack<parser_utils::value, Construct>;
template <typename Op> using binary      = binary<parser_utils::value, parser_utils::value, parser_utils::value, Op>;
template <typename Op> using unary       = unary<parser_utils::value, Op>;

/** \brief Describes the rules for checking an expression for any semantic affiliation. */
namespace convention {

using check_expression = std::function<bool(const std::unique_ptr<base> &)>;

static bool expect_identifier(const std::unique_ptr<base> &expr) {
    if (!expr->is_identifier())
        throw parser_utils::parser_error("An ID is expected.", expr->get_token());
    return true;
}
static bool expect_n_existing_variable(const std::unique_ptr<base> &expr) {
    expect_identifier(expr);
    if (expr->is_context_member())
        throw parser_utils::parser_error("A non-existing variable was expected.", expr->get_token());
    return true;
}
static bool expect_existing_variable(const std::unique_ptr<base> &expr) {
    expect_identifier(expr);
    if (!expr->is_context_member())
        throw parser_utils::parser_error("A existing variable was expected.", expr->get_token());
    return true;
}
template <typename T = std::monostate>
static std::integral_constant<
    bool, std::is_same_v<decltype(parser_utils::value{std::declval<T &>()}), parser_utils::value>>::value_type
expect_value(const std::unique_ptr<base> &expr) {
    if (!expr->is_value())
        throw parser_utils::parser_error("Expected value.", expr->get_token());

    if (!std::is_same_v<T, std::monostate> && !std::holds_alternative<T>(expr->get_value()))
        throw parser_utils::parser_error("Expected " + parser_utils::get_type_name<T>() + " type.", expr->get_token());

    return true;
}
static bool expect_type(const std::unique_ptr<base> &expr) {
    if (!expr->is_keyword() || !dynamic_cast<keyword *>(expr.get())->is_type())
        throw parser_utils::parser_error("Expected keyword of type.", expr->get_token());
    return true;
}
} // namespace convention

/** \brief Binary bwlang expressions. */
namespace binary_operation {

struct init_variable {
    template <bool is_vector> static void init(parser_utils::scope &sc, string_v type, const string &name_var) {
        using namespace bweas;

        if (type == tokens::number_t::s_value)
            sc[name_var] = typename std::conditional<is_vector, vec<pdiff>, pdiff>::type{};
        else if (type == tokens::string_t::s_value)
            sc[name_var] = typename std::conditional<is_vector, vec<string>, string>::type{};
        else if (type == tokens::cc_t::s_value)
            sc[name_var] = typename std::conditional<is_vector, vec<sc::call_component>, sc::call_component>::type{};
        else if (type == tokens::ctemplate_t::s_value)
            sc[name_var] =
                typename std::conditional<is_vector, vec<sc::template_command>, sc::template_command>::type{};
        else if (type == tokens::language_t::s_value)
            sc[name_var] = typename std::conditional<is_vector, vec<sc::language>, sc::language>::type{};
        else if (type == tokens::profile_t::s_value)
            sc[name_var] = typename std::conditional<is_vector, vec<sc::profile>, sc::profile>::type{};
        else if (type == tokens::target_t::s_value)
            sc[name_var] = typename std::conditional<is_vector, vec<sc::target>, sc::target>::type{};
        else
            throw parser_utils::parser_error("Unknown type.");
    }
    parser_utils::value operator()(parser_utils::context &ctx, const std::unique_ptr<base> &lhs,
                                   const std::unique_ptr<base> &rhs) {
        using namespace bweas;

        string name_var = std::get<string>(lhs->get_value());
        try {
            if (!dynamic_cast<keyword *>(rhs.get())->is_array())
                init<false>(ctx.sc, std::get<string>(rhs->get_value()), name_var);
            else
                init<true>(ctx.sc, std::get<string>(rhs->get_value()), name_var);
        }
        catch (parser_utils::parser_error &excp) {
            throw parser_utils::parser_error(excp.what(), rhs->get_token());
        }
        return name_var;
    }
    parser_utils::value &get_reference(parser_utils::context &ctx, const std::unique_ptr<base> &lhs) {
        return ctx.sc[std::get<string>(lhs->get_value())];
    }
};
struct access_by_id {
    static bool expect(const std::unique_ptr<base> &expr) {
        return convention::expect_n_existing_variable(expr);
    }

  public:
    parser_utils::value operator()(parser_utils::context &ctx, const std::unique_ptr<base> &lhs,
                                   const std::unique_ptr<base> &rhs) {
        try {
            value =
                std::visit([&](auto &&val) { return parser_utils::get_field(val, std::get<string>(rhs->get_value())); },
                           lhs->get_reference());
        }
        catch (parser_utils::parser_error &excp) {
            throw parser_utils::parser_error(excp.what(), rhs->get_token());
        }
        return value;
    }
    parser_utils::value &get_reference() {
        return value;
    }
    void update(parser_utils::context &ctx, const std::unique_ptr<base> &lhs, const std::unique_ptr<base> &rhs) {
        try {
            std::visit([&](auto &&var) { parser_utils::set_field(var, std::get<string>(rhs->get_value()), value); },
                       lhs->get_reference());
        }
        catch (parser_utils::parser_error &excp) {
            throw parser_utils::parser_error(excp.what(), rhs->get_token());
        }
    }

  private:
    parser_utils::value value;
};
struct access_by_index {
    static bool expect(const std::unique_ptr<base> &expr) {
        return convention::expect_value<pdiff>(expr);
    }

  public:
    parser_utils::value operator()(parser_utils::context &ctx, const std::unique_ptr<base> &lhs,
                                   const std::unique_ptr<base> &rhs) {
        value = std::visit(
            [&](auto &&val) -> parser_utils::value {
                size_t index = std::get<pdiff>(rhs->get_value());

                if constexpr (!parser_utils::is_vector<std::decay_t<decltype(val)>>::value)
                    throw parser_utils::parser_error("Expected array.", lhs->get_token());
                else if (index > val.size())
                    throw parser_utils::parser_error("The index is larger than the size of the array.",
                                                     rhs->get_token());
                else
                    return val[index];
            },
            lhs->get_reference());

        return value;
    } // namespace binary_operation
    parser_utils::value &get_reference() {
        return value;
    }
    void update(parser_utils::context &ctx, const std::unique_ptr<base> &lhs, const std::unique_ptr<base> &rhs) {
        std::visit(
            [&](auto &&val) {
                using T = std::decay_t<decltype(val)>;

                if constexpr (!parser_utils::is_vector<T>::value)
                    throw parser_utils::parser_error("Expected array.", lhs->get_token());
                else {
                    if (!std::holds_alternative<typename T::value_type>(value))
                        throw parser_utils::parser_error(
                            "Expected " + parser_utils::get_type_name<typename T::value_type>() + " type.",
                            lhs->get_token());
                    val[std::get<pdiff>(rhs->get_value())] = std::get<typename T::value_type>(value);
                }
            },
            lhs->get_reference());
    }

  private:
    parser_utils::value value;
}; // namespace ext
struct assign_variable {
    parser_utils::value operator()(parser_utils::context &ctx, const std::unique_ptr<base> &lhs,
                                   const std::unique_ptr<base> &rhs) {
        auto rhs_value = rhs->get_value();

        std::visit(
            [&](auto &&val) {
                using T = std::decay_t<decltype(val)>;
                if (!std::holds_alternative<T>(rhs_value))
                    throw parser_utils::parser_error("Expected " + parser_utils::get_type_name<T>() + " type.",
                                                     lhs->get_token());
                val = std::get<T>(rhs_value);
            },
            lhs->get_reference());

        return rhs_value;
    }
};

template <typename Op,
          typename = std::void_t<decltype(std::declval<Op>()(std::declval<typename Op::arguments_type &>(),
                                                             std::declval<typename Op::arguments_type &>())),
                                 decltype(parser_utils::value{typename Op::result_type{}})>>
struct basic_binary_operation {
    parser_utils::value operator()(parser_utils::context &ctx, const std::unique_ptr<base> &lhs,
                                   const std::unique_ptr<base> &rhs) {
        using T = typename Op::arguments_type;

        return Op{}(std::get<T>(lhs->get_value()), std::get<T>(rhs->get_value()));
    }
};

/** \brief A wrapper over a simple binary expression.
 * \detail Adds the ability to check the semantic affiliation of lhs and rhs.
 */
template <typename BinaryOperation> class binary_ext : public binary<BinaryOperation> {
  public:
    binary_ext(parser_utils::context &_ctx, tokens::token _tk, std::unique_ptr<base> _lhs, std::unique_ptr<base> _rhs,
               convention::check_expression lhs_check, convention::check_expression rhs_check)
        : binary<BinaryOperation>(_ctx, _tk, std::move(lhs_check(_lhs) ? _lhs : _lhs),
                                  std::move(rhs_check(_rhs) ? _rhs : _rhs)) {
    }
    virtual ~binary_ext() = default;
};
/** \brief A binary expression that accepts two values of the same type.
 * \detail It is used for mathematical and logical operations.
 * Expects lhs and rhs values corresponding to type c Op::arguments_type.
 */
template <typename Op> class basic final : public binary_ext<basic_binary_operation<Op>> {
  public:
    basic(parser_utils::context &_ctx, tokens::token _tk, std::unique_ptr<base> _lhs, std::unique_ptr<base> _rhs)
        : binary_ext<basic_binary_operation<Op>>(_ctx, _tk, std::move(_lhs), std::move(_rhs),
                                                 convention::expect_value<typename Op::arguments_type>,
                                                 convention::expect_value<typename Op::arguments_type>) {
    }
    ~basic() override = default;
};

/** \brief An expression that initializes a variable in the context.
 * \detail Expects that lhs is not an existing identifier in the context, and rhs is a keyword of the type.
 */
class init final : public binary_ext<init_variable> {
  public:
    init(parser_utils::context &_ctx, tokens::token _tk, std::unique_ptr<base> _lhs, std::unique_ptr<base> _rhs)
        : binary_ext<init_variable>(_ctx, _tk, std::move(_lhs), std::move(_rhs), convention::expect_n_existing_variable,
                                    convention::expect_type) {
    }
    ~init() override = default;

  public:
    parser_utils::value &get_reference() override {
        return init_variable::get_reference(this->ctx, this->lhs);
    }
    bool is_context_member() const override {
        return true;
    }

  public:
    constexpr bool is_identifier() const override {
        return true;
    }
};
/** \brief An expression that accesses the internal data of a variable, if it has one.
 * \detail Expects lhs to be an existing identifier in the context, and rhs to be a non-existing identifier.
 */
template <typename AccessBy> class access final : public binary_ext<AccessBy> {
  public:
    access(parser_utils::context &_ctx, tokens::token _tk, std::unique_ptr<base> _lhs, std::unique_ptr<base> _rhs)
        : binary_ext<AccessBy>(_ctx, _tk, std::move(_lhs), std::move(_rhs), convention::expect_existing_variable,
                               AccessBy::expect) {};
    ~access() override {
        this->update(this->ctx, this->lhs, this->rhs);
    }

  public:
    parser_utils::value &get_reference() override {
        return AccessBy::get_reference();
    }
    bool is_context_member() const override {
        return true;
    }

  public:
    constexpr bool is_identifier() const override {
        return true;
    }
};
/** \brief An expression that assigns a variable.
 * \detail Expects lhs to be an existing identifier in the context, and rhs to be a value.
 */
class assign final : public binary_ext<assign_variable> {
  public:
    assign(parser_utils::context &_ctx, tokens::token _tk, std::unique_ptr<base> _lhs, std::unique_ptr<base> _rhs)
        : binary_ext<assign_variable>(_ctx, _tk, std::move(_lhs), std::move(_rhs), convention::expect_existing_variable,
                                      convention::expect_value<>) {};
    ~assign() override = default;
};
} // namespace binary_operation

/** \brief Unary bwlang expressions. */
namespace unary_operation {
struct negative_unary_operation {
    parser_utils::value operator()(parser_utils::context &ctx, const std::unique_ptr<base> &rhs) const {
        return -std::get<pdiff>(rhs->get_value());
    }
};
struct not_unary_operation {
    parser_utils::value operator()(parser_utils::context &ctx, const std::unique_ptr<base> &rhs) const {
        return !std::get<pdiff>(rhs->get_value());
    }
};

/** \brief A wrapper over a simple unary expression.
 * \detail Adds the ability to check the semantic affiliation of lhs and rhs.
 */
template <typename UnaryOperation> class unary_ext : public unary<UnaryOperation> {

  public:
    unary_ext(parser_utils::context &_ctx, tokens::token _tk, std::unique_ptr<base> _rhs,
              convention::check_expression rhs_check)
        : unary<UnaryOperation>(_ctx, _tk, std::move(rhs_check(_rhs) ? _rhs : _rhs)) {
    }
    virtual ~unary_ext() = default;
};

/** \brief An expression that changes the sign of an integer.
 * \detail Expects the rhs to be a value(num).
 */
class negative final : public unary_ext<negative_unary_operation> {
  public:
    negative(parser_utils::context &_ctx, tokens::token _tk, std::unique_ptr<base> rhs)
        : unary_ext<negative_unary_operation>(_ctx, _tk, std::move(rhs), convention::expect_value<pdiff>) {
    }
    ~negative() override = default;
};
/** \brief An expression that performs the logical negation of an integer.
 * \detail Expects the rhs to be a value(num).
 */
class logical_not final : public unary_ext<not_unary_operation> {
  public:
    logical_not(parser_utils::context &_ctx, tokens::token _tk, std::unique_ptr<base> rhs)
        : unary_ext<not_unary_operation>(_ctx, _tk, std::move(rhs), convention::expect_value<pdiff>) {
    }
    ~logical_not() override = default;
};
} // namespace unary_operation
} // namespace ext
} // namespace expression
} // namespace bwlang
#endif
