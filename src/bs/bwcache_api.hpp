#ifndef BWCACHE__H
#define BWCACHE__H

#include "bw_defs.hpp"
#include "tools/bwlua.hpp"

#define NAME_FUNCTION_GENCACHE "create_cache"
#define DEFINITION_FUNCTION_GENCACHE                                                                                   \
<std::string, bwlua::lua::array<bwlua::lua::table<std::string, std::any>>, bwlua::lua::array<bwlua::lua::table<std::string, std::any>>, bwlua::lua::array<bwlua::lua::table<std::string, std::any>>, bwlua::lua::array<bwlua::lua::keyValue<std::string, std::string>>>

#define NAME_FUNCTION_GETDATA_CACHE "get_cache_data"
#define DEFINITION_FUNCTION_GETDATA_CACHE <void, bwlua::lua::nil>

#define NAME_VARIABLE_TARGETS_F_EXTERN "targets"
#define NAME_VARIABLE_TEMPLATES_F_EXTERN "templates"
#define NAME_VARIABLE_CCOMPONENTS_F_EXTERN "call_components"
#define NAME_VARIABLE_GEARGS_F_EXTERN "global_external_args"

namespace bweas {

namespace cache_api {

class base_bwcache {
  public:
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
    virtual std::string create_cache() = 0;
    virtual const cache_data &get_cache_data(std::string cache_str) = 0;

    virtual void delete_cache() = 0;

  public:
    static inline base_bwcache *create_fast_bwcache();
    static inline base_bwcache *create_json_bwcache();
    static inline base_bwcache *create_lua_bwcache(std::string src_lua);

  public:
    cache_data _cache_data;
};

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