//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWGNTOOLS__H
#define BWGNTOOLS__H

#include <map>

#include <bw_defs.hpp>

namespace bweas {
namespace generator {

namespace tools {

// Generates a file name based on the pattern and the passed index of the given file
extern std::string get_name_output_file(std::string pattern_file, u32t index, std::string dir_work_endv);

// Parses all the basic arguments for further template command generation
// Types of arguments that will be parsed:
//  1. extglobal
//  2. trgfield(except T_PROJECT_SRC_FILES)
extern void parse_basic_args(const var::struct_sb::target_out &target, bwqueue_templates &target_queue_templates,
                             const bwargs &global_extern_args);

} // namespace tools

} // namespace generator
} // namespace bweas

#endif
