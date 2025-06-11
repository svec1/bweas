//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWDEPENDS_FILES_HPP
#define BWDEPENDS_FILES_HPP

#include <bw_defs.hpp>

namespace bweas {
class depends_files;
} // namespace bweas

// File dependency system interface, defines specific functions
class bweas::depends_files {
  public:
    depends_files(const string_v _language, const string_v _work_directory)
        : language(_language), work_directory(_work_directory) {
    }
    ~depends_files() = default;

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

  protected:
    virtual uset<string> build_graph_depends_file(string_v name_file) = 0;

  protected:
    string language;
    string work_directory;

    vec<string> include_paths;

  private:
    depends_map mdepends;
};

#endif
