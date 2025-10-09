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

/** \brief File Dependency Finder. */
class bweas::dependency_finder final {
  public:
    dependency_finder() = default;
    dependency_finder(string_v _search_regex, char _char_global_finder)
        : search_regex(_search_regex), char_global_finder(_char_global_finder) {
    }

  public:
    using depends_file   = pair<string, uset<string>>;
    using dependency_map = umap<typename depends_file::first_type, typename depends_file::second_type>;

  public:
    /** \brief Builds dependency graphs for each source file.
     *  \param [in] _include_paths An array of paths where dependent files can be located.
     *  \param [in] src_files An array of source files for which the dependency graph depends_file will be built.
     *  \return dependency_map A set of dependency graphs.
     */
    dependency_map &build_graphs(const vec<string> &_include_paths, const vec<string> &src_files);

    /** \brief Builds dependency graphs for source file. An analog of the build_graphs function.
     *  \param [in] _include_paths An array of paths where dependent files can be located.
     *  \param [in] name_file The name of the file for which the dependency graph will be built.
     *  \return dependency_map A set of dependency graphs.
     */
    dependency_map &build_graph(const vec<string> &_include_paths, const string &name_file);

    /** \brief Builds dependency graphs for a specific source file from a serialized graph.
     *  \param [in] name_file The name of the source file.
     *  \param [in] depends_file A string of the serialized graph.
     *  \return dependency_map A set of dependency graphs.
     */
    dependency_map &build_graph_depends_file_string(string name_file, string depends_file);

  public:
    /** \return dependency_map A set of dependency graphs. */
    dependency_map &get_graphs_depends_files();
    /** \brief Returns all dependent files of the transmitted file as a string.
     * \param [in] name_file The name of the file that has the dependency graph.
     * \return string The serialized dependency graph of the specified file.
     */
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
