//
// BWEAS is distributed under the gnu general public license 2.0 (gpl-2.0).
// you can view the license text at the link:
//     <https://www.gnu.org/licenses>
// ------------------------------------------
//

#ifndef BWGNTOOLS_HPP
#define BWGNTOOLS_HPP

#include <bw_defs.hpp>
#include <bwgenerator_api.hpp>

namespace bweas {
class generator_tools;
}
class bweas::generator_tools {
  public:
    generator_tools() = delete;

  public:
    // Generates a file name based on the pattern and the passed index of the given file.
    static string get_name_output_file(string pattern_file, size_t index, string dir_work_endv);

    // Determines, based on the time of creation or modification of the file and its dependencies, whether it should be
    // passed to the generator.
    static bool should_uses_src_file(string_v src_file, string_v output_file, const uset<string> &dfiles);

    // Parses all the basic arguments for further template command generation.
    // Types of arguments that will be parsed:
    //  1. extglobal
    //  2. trgfield(except T_PROJECT_SRC_FILES)
    static void parse_basic_args(const sc::target_out &target, vec<sc::template_command> &target_queue_templates,
                                 const vec<pair<string, string>> &global_extern_args);

  public:
    static string build_string_command(const generator_api::command &cmd);
};

#endif
