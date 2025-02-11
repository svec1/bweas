//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWGENERATOR_INTEGRAL__H
#define BWGENERATOR_INTEGRAL__H

#include <map>

#include "bw_defs.hpp"
#include "bwgenerator_api.hpp"

namespace bweas {
namespace generator {

extern std::unordered_set<std::string> bwbuild_graph_depends_file(std::string_view, std::string_view, std::string_view,
                                                                  std::vector<std::string>);

extern void bwget_input_files(bweas::generator_api::data_transfer &);
// First and basic template-based command generator
extern bweas::generator_api::gen_command bwgenerator(bweas::generator_api::data_transfer &);
} // namespace generator
} // namespace bweas

#endif
