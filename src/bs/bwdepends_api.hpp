//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWDEPENDS_FILES_HPP
#define BWDEPENDS_FILES_HPP

#include <bwaliases.hpp>

namespace bweas {
class depends_files;
class depends_integral_cxx;
class depends_lua;
} // namespace bweas

// File dependency system interface, defines specific functions
class bweas::depends_files {
  public:
    depends_files()          = default;
    virtual ~depends_files() = default;

  public:
    // file, dependencies
    using depends_file = pair<string, uset<string>>;
    using depends_map  = umap<typename depends_file::first_type, typename depends_file::second_type>;

    // Dependency graphs of source files are built
    depends_map &build_graphs_depends_files(const vec<string> &src_files);
    depends_map &build_graphs_depends_file_v(const string &name_file);

    // depends_file - list of file dependencies(name_file)
    depends_map &build_graph_depends_file_string(string name_file, string depends_file);

  public:
    depends_map &get_graphs_depends_files();
    // Returns all dependent files of the transmitted file as a string
    string get_string_depends_file(string name_file);

    void set_include_paths(const vec<string> _include_paths);

  public:
    static inline depends_files *create_depends_integral_cxx();
    static inline depends_files *create_depends_lua(const string_v src_lua);

  protected:
    virtual uset<string> build_graph_depends_file(string_v name_file) = 0;

  protected:
    vec<string> include_paths;

  private:
    depends_map mdepends;
};

// Implementing an internal file dependency system
class bweas::depends_integral_cxx : private bweas::depends_files {
    friend depends_files *depends_files::create_depends_integral_cxx();

  private:
    depends_integral_cxx() = default;

  public:
    ~depends_integral_cxx() = default;

  private:
    uset<string> build_graph_depends_file(string_v name_file) override final;
};

class bweas::depends_lua : private bweas::depends_files {
    friend depends_files *depends_files::create_depends_lua(const string_v src_lua);

  private:
    depends_lua(const string_v src_lua);

  public:
    ~depends_lua() = default;

  private:
    uset<string> build_graph_depends_file(string_v name_file) override final;
};

bweas::depends_files *bweas::depends_files::create_depends_integral_cxx() {
    return dynamic_cast<depends_files *>(new depends_integral_cxx());
}
bweas::depends_files *bweas::depends_files::create_depends_lua(const string_v src_lua) {
    return dynamic_cast<depends_files *>(new depends_lua(src_lua));
}

#endif
