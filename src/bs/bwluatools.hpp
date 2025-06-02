//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWLUATOOLS_H
#define BWLUATOOLS_H

#include <bwdepends_files.hpp>
#include <bwgntools.hpp>
#include <lang/scope.hpp>
#include <lang/static_struct.hpp>

#include <tools/bwfile.hpp>
#include <tools/bwlua.hpp>

namespace bweas {
class bwluatools;
} // namespace bweas

// A class providing tools for simplified interaction between bweas and lua structures
class bweas::bwluatools {
  public:
    bwluatools() = delete;

  public:
    template <typename T> using ref = bwlua::lua::ref<T>;

    template <typename T> using array                 = bwlua::lua::array<T>;
    template <typename K, typename V> using key_value = bwlua::lua::key_value<K, V>;
    template <typename K, typename V> using table     = bwlua::lua::table<K, V>;

    using integer = bwlua::lua::integer;
    using number  = bwlua::lua::number;

    using nil = bwlua::lua::nil;

    // Functions for converting standard bweas structures into appropriate containers for lua
  public:
    static table<std::string, std::any> conv_to_table(const var::struct_sb::project &prj);

    static bwluatools::array<std::any> conv_to_table(const std::vector<var::struct_sb::template_command::arg> &args);
    static table<std::string, std::any> conv_to_table(const var::struct_sb::template_command &tmp_c);

    static table<std::string, std::string> conv_to_table(const var::struct_sb::call_component &ccmp);

    static table<std::string, std::any> conv_to_table(const var::struct_sb::target &trg_o);
    static table<std::string, std::any> conv_to_table(const var::struct_sb::target_out &trg_o);

    static table<std::string, array<std::string>> conv_to_table(const bweas::bwdepends_files::depends_map &dfiles);

    // Functions for converting containers for lua to the corresponding bweas structures
  public:
    static var::struct_sb::project conv_to_project(table<std::string, std::any> prj_t);

    static std::vector<var::struct_sb::template_command::arg> conv_to_args(array<array<std::any>> args);
    static var::struct_sb::template_command conv_to_template(table<std::string, std::any> &tcmd);

    static var::struct_sb::call_component conv_to_call_components(table<std::string, std::any> &ccmp);

    static var::struct_sb::target_out conv_to_target(table<std::string, std::any> &trg_o_t);

    // Auxiliary functions for lua scripts for interacting with bweas (wrappers over bweas functions)
  public:
    static int get_var(lua_State *L);
    static int set_var(lua_State *L);

    static int get_name_output_file_lua(lua_State *L);
    static int file_slc_mask_lua(lua_State *L);
};
#endif
