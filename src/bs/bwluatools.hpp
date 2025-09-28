//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWLUATOOLS_HPP
#define BWLUATOOLS_HPP

#include <bwdependency_finder.hpp>

#include <bwstructs_context.hpp>
#include <utils/lua_wrapper.hpp>

namespace bweas {
class lua_tools;
}

// A class providing tools for simplified interaction between bweas and lua structures
class bweas::lua_tools {
  public:
    lua_tools() = delete;

  public:
    template <typename T> using ref = utils::lua::ref<T>;

    template <typename T> using array                 = utils::lua::array<T>;
    template <typename K, typename V> using key_value = utils::lua::key_value<K, V>;
    template <typename K, typename V> using table     = utils::lua::table<K, V>;

    using integer = utils::lua::integer;
    using number  = utils::lua::number;

    using nil = utils::lua::nil;

    // Ready-made alias parameters for standard bweas structures.
  public:
    using param_targets     = lua_tools::array<lua_tools::table<string_v, any>>;
    using param_templates   = lua_tools::array<lua_tools::table<string_v, any>>;
    using param_ccomponents = lua_tools::array<lua_tools::table<string_v, string>>;

    // Functions for converting standard bweas structures into appropriate containers for lua
  public:
    static table<string, any> conv_to_table(const sc::profile &ext);
    static array<any> conv_to_table(const vec<sc::template_command::arg> &args);
    static array<any> conv_to_table(const sc::template_command::return_value &returnable);
    static table<string_v, any> conv_to_table(const sc::template_command &tmp_c);
    static table<string_v, string> conv_to_table(const sc::call_component &ccmp);
    static table<string_v, any> conv_to_table(const sc::target &trg_o);
    static table<string, array<string>> conv_to_table(const bweas::dependency_finder::dependency_map &dfiles);

    // Functions for converting containers for lua to the corresponding bweas structures
  public:
    static sc::profile conv_to_extension(table<string, any> ext);
    static vec<sc::template_command::arg> conv_to_args(array<array<any>> args);
    static sc::template_command::return_value conv_to_return_value(lua_tools::array<any> returnable);
    static sc::template_command conv_to_template(table<string, any> &tcmd);
    static sc::call_component conv_to_call_components(table<string, any> &ccmp);
    static sc::target conv_to_target(table<string, any> &trg_o_t);
};
#endif
