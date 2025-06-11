//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWGENERATOR_INTEGRAL_HPP
#define BWGENERATOR_INTEGRAL_HPP

#include <map>

#include "bw_defs.hpp"
#include "bwgenerator_api.hpp"

namespace bweas {
class integral_generator;
}

class bweas::integral_generator {
  public:
    integral_generator() = delete;

  public:
    static void get_input_files(bweas::generator_api::data_transfer &data_t);
    // First and basic template-based command generator
    static generator_api::commands generate(bweas::generator_api::data_transfer &data_t);
};

#endif
