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

// Abstract class that bases API for creating cache generators
class base_bwcache {
  public:
    base_bwcache(bw_context *const _context) : context(_context) {};

  public:
    // A function that must be defined in a child class, and return a cache of data
    virtual string create_cache() = 0;

    // A function that must be defined in a child class and return cache data
    virtual void extract_cache_data(string &&cache_str) = 0;

    virtual void delete_cache() = 0;

    // A number of factory functions to create all possible child classes that implement the bwcache cache generator
    // based on the bwcache API
  public:
    static inline base_bwcache *create_fast_bwcache(bw_context *const _context);
    static inline base_bwcache *create_json_bwcache(bw_context *const _context);
    static inline base_bwcache *create_lua_bwcache(bw_context *const _context, string_v src_lua);

  public:
    bw_context *const context;
};

// A basic cache generator that is fast but also creates a hard-to-read cache for humans to use
class fast_bwcache final : private base_bwcache {
  public:
    fast_bwcache(bw_context *const);

  public:
    string create_cache() override final;
    void extract_cache_data(string &&cache_str) override final;

    void delete_cache() override final;
};

// The second basic cache generator, which in turn has a human readable form,
// but is also slow compared to fast_bwcache
class json_bwcache final : private base_bwcache {
  public:
    json_bwcache(bw_context *const);

  public:
    string create_cache() override final;
    void extract_cache_data(string &&cache_str) override final;

    void delete_cache() override final;
};

// A class providing an API for creating cache generators in lua, based on the bwcache API
class lua_bwcache final : private base_bwcache {
  public:
    lua_bwcache(bw_context *const, string_v);

  public:
    string create_cache() override final;
    void extract_cache_data(string &&cache_str) override final;

    void delete_cache() override final;

  private:
    bwlua::lua lua;
};

base_bwcache *base_bwcache::create_fast_bwcache(bw_context *const _context) {
    return (base_bwcache *)new fast_bwcache(_context);
}
base_bwcache *base_bwcache::create_json_bwcache(bw_context *const _context) {
    return (base_bwcache *)new json_bwcache(_context);
}
base_bwcache *base_bwcache::create_lua_bwcache(bw_context *const _context, string_v src_lua) {
    return (base_bwcache *)new lua_bwcache(_context, src_lua);
}

} // namespace cache_api

} // namespace bweas

#endif
