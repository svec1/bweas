//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWDEPENDS_FILES__H
#define BWDEPENDS_FILES__H

#include <string>
#include <string_view>

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace bweas {

// File dependency system interface, defines specific functions
class bwdepends_files {
  public:
    bwdepends_files(const std::string_view _language, const std::string_view _work_directory)
        : language(_language), work_directory(_work_directory) {
    }
    ~bwdepends_files() = default;

  public:
    // file, dependencies
    using depends_file = std::pair<std::string, std::unordered_set<std::string>>;
    using depends_map = std::unordered_map<typename depends_file::first_type, typename depends_file::second_type>;

    // Dependency graphs of source files are built
    depends_map &build_graphs_depends_files(const std::vector<std::string> &src_files,
                                            const std::vector<std::string> &include_paths);
    // depends_file - list of file dependencies(name_file)
    depends_map &build_graph_depends_file_string(std::string name_file, std::string depends_file);

    depends_map &get_graphs_depends_files();

  public:
    // Returns all dependent files of the transmitted file as a string
    std::string get_string_depends_file(std::string name_file);

  protected:
    virtual std::unordered_set<std::string> build_graph_depends_file(std::string_view name_file,
                                                                     const std::vector<std::string> &include_paths) = 0;

  protected:
    std::string language;
    std::string work_directory;

  private:
    depends_map mdepends;
};
} // namespace bweas
#endif
