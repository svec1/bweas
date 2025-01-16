#ifndef BWGNTOOLS__H
#define BWGNTOOLS__H

#include "bw_defs.hpp"

#include <map>

namespace bweas {
namespace generator {

namespace tools {

// Generates a file name based on the pattern and the passed index of the given file
extern std::string get_name_output_file(std::string pattern_file, u32t index, std::string dir_work_endv);

} // namespace tools

} // namespace generator
} // namespace bweas

#endif