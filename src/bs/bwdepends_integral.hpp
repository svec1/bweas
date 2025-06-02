//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWDEPENDS_INTEGRAL__H
#define BWDEPENDS_INTEGRAL__H

#include "bwdepends_files.hpp"
#include "bwgenerator_integral.hpp"

namespace bweas {

// Implementing an internal file dependency system
class bwdepends_integral : public bwdepends_files {
  public:
    bwdepends_integral(const std::string_view language, const std::string_view work_directory)
        : bwdepends_files(language, work_directory) {
    }
    ~bwdepends_integral() = default;

  public:
    std::unordered_set<std::string> build_graph_depends_file(std::string_view name_file) override final;

  private:
    static std::unordered_set<std::string> build_graph_depends_file_c_cpp(
        std::string_view name_file, const std::vector<std::string> &include_paths);
};
} // namespace bweas

#endif
