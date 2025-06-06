//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWDEPENDS_GENERATOR_HPP
#define BWDEPENDS_GENERATOR_HPP

#include "bwgenerator_api.hpp"

namespace bweas {

// Implementing a file dependency system based on the bweas generators API
class bwdepends_generator : public bweas::bwdepends_files {
  public:
    bwdepends_generator(std::shared_ptr<generator_api::base_generator> _generator, const string_v language,
                        const string_v work_directory)
        : generator(_generator), bweas::bwdepends_files(language, work_directory) {
    }
    ~bwdepends_generator() = default;

  public:
    uset<string> build_graph_depends_file(string_v name_file) override final;

  private:
    std::shared_ptr<generator_api::base_generator> generator;
};

uset<string> bweas::bwdepends_generator::build_graph_depends_file(string_v name_file) {
    return generator->build_graph_depends_file(language, name_file, work_directory, include_paths);
}
} // namespace bweas
#endif
