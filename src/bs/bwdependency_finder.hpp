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
class dependency_finder;
} // namespace bweas

// File dependency system interface, defines specific functions
class bweas::dependency_finder {
  public:
    dependency_finder() = default;
    dependency_finder(string_v _search_regex, char _char_global_finder)
        : search_regex(_search_regex), char_global_finder(_char_global_finder) {
    }

  public:
    // file, dependencies
    using depends_file   = pair<string, uset<string>>;
    using dependency_map = umap<typename depends_file::first_type, typename depends_file::second_type>;

    // Dependency graphs of source files are built
    dependency_map &build_graphs_depends_files(const vec<string> &_include_paths, const vec<string> &src_files);
    dependency_map &build_graphs_depends_file_v(const vec<string> &_include_paths, const string &name_file);

    // depends_file - list of file dependencies(name_file)
    dependency_map &build_graph_depends_file_string(string name_file, string depends_file);

  public:
    dependency_map &get_graphs_depends_files();
    // Returns all dependent files of the transmitted file as a string
    string get_string_depends_file(string name_file);

  private:
    void build_graph_depends_file(string_v name_file, uset<string> &graph_depends_file);

  private:
    dependency_map mdepends;

    vec<string> include_paths;
    string search_regex;
    char char_global_finder;
};

#endif
