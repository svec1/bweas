//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWGNTOOLS__H
#define BWGNTOOLS__H

#include <map>
#include <unordered_set>

#include <bw_defs.hpp>

namespace bweas {

class generator_tools {
  public:
    generator_tools() = delete;

  public:
    // Generates a file name based on the pattern and the passed index of the given file.
    static std::string get_name_output_file(std::string pattern_file, u32t index, std::string dir_work_endv);

    // Determines, based on the time of creation or modification of the file and its dependencies, whether it should be
    // passed to the generator.
    static bool should_uses_src_file(std::string_view src_file, std::string_view output_file,
                                     const std::unordered_set<std::string> &dfiles);

    // Parses all the basic arguments for further template command generation.
    // Types of arguments that will be parsed:
    //  1. extglobal
    //  2. trgfield(except T_PROJECT_SRC_FILES)
    static void parse_basic_args(const var::struct_sb::target_out &target, bwqueue_templates &target_queue_templates,
                                 const bwargs &global_extern_args);
};

} // namespace bweas

#endif
