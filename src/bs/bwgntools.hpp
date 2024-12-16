#ifndef BWGNTOOLS__H
#define BWGNTOOLS__H

#include "bw_defs.hpp"

#include <map>

namespace bweas {
namespace generator {

namespace tools {

// Generates a file name based on the pattern and the passed index of the given file
extern std::string get_name_output_file(std::string pattern_file, u32t index, std::string dir_work_endv);

// Creates an array of file names based on the mask
// passed to the function and an array of all files.
// ### The syntax is fully compliant with the glob() standard.
extern std::vector<std::string> file_slc_mask(std::string mask, const std::vector<std::string> &files);
} // namespace tools

} // namespace generator
} // namespace bweas

#endif