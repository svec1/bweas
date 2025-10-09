//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWCACHE_API_HPP
#define BWCACHE_API_HPP

#include <bw_defs.hpp>

/* \brief Bweas cache api namespace. */
namespace bweas::cache_api {

class interface_cache;
class base_cache;

class fast_cache;
class json_cache;
class lua_cache;

} // namespace bweas::cache_api

/** \brief
 * Bweas cache api namespace.
 */
class bweas::cache_api::interface_cache {
  protected:
    virtual ~interface_cache() = default;

  public:
    /** \brief A function that must be defined in a child class, and return a cache of data
     *  \return string A cache data.
     */
    virtual string create_cache() const = 0;

    /** \brief A function that should be defined in the child class and extract the path to the configuration file from
     * the cache file.
     *
     * \param A cache data.
     * \return string Path to bweasconf.txt.
     */
    virtual string get_path_config(const string &cache_str) const = 0;

    /** \brief A function that must be defined in a child class and return cache data.
     * \param cache_str A cache data.
     */
    virtual void extract_cache_data(const string &cache_str) const = 0;
};
/// \brief Abstract class that bases API for creating cache generators.
class bweas::cache_api::base_cache : public bweas::cache_api::interface_cache {
  public:
    base_cache() = default;

    virtual ~base_cache() = default;

    /** \brief A number of factory functions to create all possible child classes that implement the bwcache cache
     * generator based on the bwcache API.
     */
  public:
    static inline base_cache *create_fast_cache();
    static inline base_cache *create_json_cache();
    static inline base_cache *create_lua_cache(string_v src_lua);

  public:
    /** Cache initialization function. */
    void init(context *const __context) {
        if (!__context)
            bweas::logger{"BWCACHE"} << (bweas::log_message(bweas::log_type::fatal)
                                         << "Bweas the context is not defined");
        _context = __context;
    }

  protected:
    context *_context;
};

/** \brief A basic cache generator that is fast but also creates a hard-to-read cache for humans to use. */
class bweas::cache_api::fast_cache final : private bweas::cache_api::base_cache {
    friend base_cache *base_cache::create_fast_cache();

  private:
    fast_cache() = default;

  public:
    ~fast_cache() = default;

  public:
    string create_cache() const override;
    string get_path_config(const string &cache_str) const override;
    void extract_cache_data(const string &cache_str) const override;
};

/// \brief A basic cache generator, which in turn has a human readable form, but is also slow compared to fast_bwcache.
class bweas::cache_api::json_cache final : private bweas::cache_api::base_cache {
    friend base_cache *base_cache::create_json_cache();

  private:
    json_cache() = default;

  public:
    ~json_cache() = default;

  public:
    string create_cache() const override;
    string get_path_config(const string &cache_str) const override;
    void extract_cache_data(const string &cache_str) const override;
};

/** \brief A class providing an api for creating cache generators in lua, based on the bweas cache api. */
class bweas::cache_api::lua_cache final : private bweas::cache_api::base_cache {
    friend base_cache *base_cache::create_lua_cache(string_v src_lua);

  private:
    lua_cache(string_v);

  public:
    ~lua_cache() = default;

  public:
    string create_cache() const override;
    string get_path_config(const string &cache_str) const override;
    void extract_cache_data(const string &cache_str) const override;
};

bweas::cache_api::base_cache *bweas::cache_api::base_cache::create_fast_cache() {
    return dynamic_cast<base_cache *>(new fast_cache);
}
bweas::cache_api::base_cache *bweas::cache_api::base_cache::create_json_cache() {
    return dynamic_cast<base_cache *>(new json_cache);
}
bweas::cache_api::base_cache *bweas::cache_api::base_cache::create_lua_cache(string_v src_lua) {
    return dynamic_cast<base_cache *>(new lua_cache(src_lua));
}

#endif
