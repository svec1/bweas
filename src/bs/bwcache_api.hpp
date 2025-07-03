//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWCACHE_API_HPP
#define BWCACHE_API_HPP

#include <bw_defs.hpp>
#include <bwluatools.hpp>

namespace bweas {

namespace cache_api {
class base_cache;

class fast_cache;
class json_cache;
class lua_cache;
} // namespace cache_api

} // namespace bweas

// Abstract class that bases API for creating cache generators
class bweas::cache_api::base_cache {
  public:
    base_cache(bw_context *const _context) : context(_context) {};

  public:
    // A function that must be defined in a child class, and return a cache of data
    virtual string create_cache() = 0;

    // A function that should be defined in the child class and extract the path to the configuration file from the
    // cache file
    virtual string get_path_config(const string &cache_str) = 0;

    // A function that must be defined in a child class and return cache data
    virtual void extract_cache_data(const string &cache_str) = 0;

    // A number of factory functions to create all possible child classes that implement the bwcache cache generator
    // based on the bwcache API
  public:
    static inline base_cache *create_fast_cache(bw_context *const _context);
    static inline base_cache *create_json_cache(bw_context *const _context);
    static inline base_cache *create_lua_cache(bw_context *const _context, string_v src_lua);

  public:
    virtual ~base_cache() = default;

  public:
    bw_context *const context;
};

// A basic cache generator that is fast but also creates a hard-to-read cache for humans to use
class bweas::cache_api::fast_cache final : private bweas::cache_api::base_cache {
    friend base_cache *base_cache::create_fast_cache(bw_context *const);

  private:
    fast_cache(bw_context *const);

  public:
    ~fast_cache() = default;

  public:
    string create_cache() override;
    string get_path_config(const string &cache_str) override;
    void extract_cache_data(const string &cache_str) override;
};

// The second basic cache generator, which in turn has a human readable form,
// but is also slow compared to fast_bwcache
class bweas::cache_api::json_cache final : private bweas::cache_api::base_cache {
    friend base_cache *base_cache::create_json_cache(bw_context *const);

  private:
    json_cache(bw_context *const);

  public:
    ~json_cache() = default;

  public:
    string create_cache() override;
    string get_path_config(const string &cache_str) override;
    void extract_cache_data(const string &cache_str) override;
};

// A class providing an API for creating cache generators in lua, based on the bwcache API
class bweas::cache_api::lua_cache final : private bweas::cache_api::base_cache {
    friend base_cache *base_cache::create_lua_cache(bw_context *const, string_v src_lua);

  private:
    lua_cache(bw_context *const, string_v);

  public:
    ~lua_cache() = default;

  public:
    string create_cache() override;
    string get_path_config(const string &cache_str) override;
    void extract_cache_data(const string &cache_str) override;

  private:
    bwlua::lua lua;
};

bweas::cache_api::base_cache *bweas::cache_api::base_cache::create_fast_cache(bw_context *const _context) {
    return dynamic_cast<base_cache *>(new fast_cache(_context));
}
bweas::cache_api::base_cache *bweas::cache_api::base_cache::create_json_cache(bw_context *const _context) {
    return dynamic_cast<base_cache *>(new json_cache(_context));
}
bweas::cache_api::base_cache *bweas::cache_api::base_cache::create_lua_cache(bw_context *const _context,
                                                                             string_v src_lua) {
    return dynamic_cast<base_cache *>(new lua_cache(_context, src_lua));
}

#endif
