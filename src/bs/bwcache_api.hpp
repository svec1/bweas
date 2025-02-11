//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWCACHE__H
#define BWCACHE__H

#include "bw_defs.hpp"
#include "bwluatools.hpp"

#define NAME_FUNCTION_GENERATE_CACHE_LUA "create_cache"
#define DEFINITION_FUNCTION_GENERATE_CACHE_LUA                                                                         \
    std::string, bwluatools::ref<bwluatools::array<bwluatools::table<std::string, std::any>>>,                         \
        bwluatools::ref<bwluatools::array<bwluatools::table<std::string, std::any>>>,                                  \
        bwluatools::ref<bwluatools::array<bwluatools::table<std::string, std::any>>>,                                  \
        bwluatools::ref<bwluatools::array<bwluatools::key_value<std::string, std::string>>>

#define NAME_FUNCTION_GET_DATA_CACHE_LUA "get_cache_data"
#define DEFINITION_FUNCTION_GET_DATA_CACHE_LUA void, bwluatools::nil

#define NAME_VARIABLE_TARGETS_F_EXTERN_LUA "targets"
#define NAME_VARIABLE_TEMPLATES_F_EXTERN_LUA "templates"
#define NAME_VARIABLE_CCOMPONENTS_F_EXTERN_LUA "call_components"
#define NAME_VARIABLE_GEARGS_F_EXTERN_LUA "global_external_args"

namespace bweas {

namespace cache_api {

// Abstract class that bases API for creating cache generators
class base_bwcache {
  public:
    // A structure that is intermediate for transferring data to be hashed (and back).
    struct cache_data {
        cache_data() = default;

      public:
        std::vector<var::struct_sb::target_out> *targets_o_p{NULL};

        std::vector<var::struct_sb::target_out> targets_o;
        std::vector<var::struct_sb::template_command> templates;
        std::vector<var::struct_sb::call_component> call_components;
        std::vector<std::pair<std::string, std::string>> global_external_args;
    };

    base_bwcache() = default;

  public:
    // A function that must be defined in a child class, and return a cache of data
    virtual std::string create_cache() = 0;

    // A function that must be defined in a child class and return cache data
    virtual const cache_data &get_cache_data(std::string cache_str) = 0;

    virtual void delete_cache() = 0;

    // A number of factory functions to create all possible child classes that implement the bwcache cache generator
    // based on the bwcache API
  public:
    static inline base_bwcache *create_fast_bwcache();
    static inline base_bwcache *create_json_bwcache();
    static inline base_bwcache *create_lua_bwcache(std::string src_lua);

  public:
    cache_data _cache_data;
};

// A basic cache generator that is fast but also creates a hard-to-read cache for humans to use
class fast_bwcache final : private base_bwcache {
  public:
    fast_bwcache();

  public:
    std::string create_cache() override final;
    const cache_data &get_cache_data(std::string cache_str) override final;

    void delete_cache() override final;

  private:
    static inline bool init_glob_chfast{0};
};

// The second basic cache generator, which in turn has a human readable form,
// but is also slow compared to fast_bwcache
class json_bwcache final : private base_bwcache {
  public:
    json_bwcache();

  public:
    std::string create_cache() override final;
    const cache_data &get_cache_data(std::string cache_str) override final;

    void delete_cache() override final;

  private:
    static inline bool init_glob_chjson{0};
};

// A class providing an API for creating cache generators in lua, based on the bwcache API
class lua_bwcache final : private base_bwcache {
  public:
    lua_bwcache() = delete;
    lua_bwcache(std::string);

  public:
    std::string create_cache() override final;
    const cache_data &get_cache_data(std::string cache_str) override final;

    void delete_cache() override final;

  private:
    static inline bool init_glob_chlua{0};

    bwlua::lua lua;
};

base_bwcache *base_bwcache::create_fast_bwcache() {
    return (base_bwcache *)new fast_bwcache;
}
base_bwcache *base_bwcache::create_json_bwcache() {
    return (base_bwcache *)new json_bwcache;
}
base_bwcache *base_bwcache::create_lua_bwcache(std::string src_lua) {
    return (base_bwcache *)new lua_bwcache(src_lua);
}

} // namespace cache_api

} // namespace bweas

#endif
