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
    bwdepends_integral(const string_v language, const string_v work_directory)
        : bwdepends_files(language, work_directory) {
    }
    ~bwdepends_integral() = default;

  public:
    uset<string> build_graph_depends_file(string_v name_file) override final;

  private:
    static uset<string> build_graph_depends_file_c_cpp(string_v name_file, const vec<string> &include_paths);
};
} // namespace bweas

#endif
