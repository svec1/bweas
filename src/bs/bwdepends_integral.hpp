//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWDEPENDS_INTEGRAL__H
#define BWDEPENDS_INTEGRAL__H

#include <bwdepends_files.hpp>

namespace bweas {
class depends_integral;
}

// Implementing an internal file dependency system
class bweas::depends_integral : private bweas::depends_files {
  public:
    depends_integral(const string_v language, const string_v work_directory) : depends_files(language, work_directory) {
    }
    ~depends_integral() = default;

  private:
    uset<string> build_graph_depends_file(string_v name_file) override final;

  private:
    static uset<string> build_graph_depends_file_c_cpp(string_v name_file, const vec<string> &include_paths);
};

#endif
