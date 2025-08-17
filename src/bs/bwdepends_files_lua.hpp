#ifndef BWDEPENDS_FILES_HPP
#define BWDEPENDS_FILES_HPP

#include <bwdepends_files.hpp>

namespace bweas {
class depends_lua;
}

class depends_lua : private depends_files {
  public:
    depends_lua(const string_v language, const string_v work_directory) : depends_files(language, work_directory) {
    }
    ~depends_lua() = default;

  private:
    uset<string> build_graph_depends_file(string_v name_file) override final;
};

#endif
