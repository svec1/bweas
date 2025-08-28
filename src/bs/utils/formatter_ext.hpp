#ifndef FORMATTER_EXT_HPP
#define FORMATTER_EXT_HPP

#include <bwaliases.hpp>

template <typename T> struct std::formatter<vec<T>> {
    constexpr auto parse(std::format_parse_context &ctx) {
        return ctx.begin();
    }

    auto format(const vec<T> &v, std::format_context &ctx) const {
        string out_str;

        for (const auto &it : v)
            out_str += std::format("{} ", it);

        return std::format_to(ctx.out(), "[{}]", out_str);
    }
};
template <typename... Types> struct std::formatter<std::variant<Types...>> {
    constexpr auto parse(std::format_parse_context &ctx) {
        return ctx.begin();
    }
    auto format(const std::variant<Types...> &v, std::format_context &ctx) const {
        return std::visit([&](auto &&value) ->
                          typename std::format_context::iterator { return std::format_to(ctx.out(), "{}", value); },
                          v);
    }
};
template <typename K, typename V> struct std::formatter<umap<K, V>> {
    constexpr auto parse(std::format_parse_context &ctx) {
        return ctx.begin();
    }

    auto format(const umap<K, V> &v, std::format_context &ctx) const {
        string out_str;

        for (const auto &[key, value] : v)
            out_str += std::format("({}: {}) ", key, value);

        return std::format_to(ctx.out(), "[{}]", out_str);
    }
};

#endif
