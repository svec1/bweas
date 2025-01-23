#ifndef BWGNTOOLS__H
#define BWGNTOOLS__H

#include "bw_defs.hpp"

#include <map>

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